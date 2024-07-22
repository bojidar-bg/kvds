// SPDX-License-Identifier: MIT
#include "../registry.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct lst_db {
  struct lst_node *head; // lowest
  struct lst_node *tail; // highest
  // int size;
} lst_db;

typedef struct lst_node {
  long long key;
  char *data;

  struct lst_node *prev; // lower
  struct lst_node *next; // higher
} lst_node;

typedef struct lst_cursor {
  long long key;
  struct lst_node *best; // Either exact key or either node that would be next to the key
} lst_cursor;

#ifndef NDEBUG
static void lst_assert_invariants(lst_db *db) {
  lst_node *prev_node = NULL;
  lst_node *node = db->head;
  while (node != NULL) {
    assert(node->prev == prev_node);
    if (prev_node != NULL) {
      assert(node->key > prev_node->key);
    }
    prev_node = node;
    node = node->next;
  }
  assert(db->tail == prev_node);
  if (prev_node != NULL) {
    assert(prev_node->next == NULL);
  }
}
#else
static void lst_assert_invariants(lst_db *db) {
  // pass
}
#endif

static kvds_db *lst_create_db() {
  lst_db *db = malloc(sizeof(lst_db));
  db->head = NULL;
  db->tail = NULL;

  lst_assert_invariants(db);

  return db;
}

static void lst_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  lst_db *db = _db;
  lst_node *node = db->head;
  while (node != NULL) {
    lst_node *next = node->next;
    free_data(node->data);
    free(node);
    node = next;
  }
  free(db);
}

static lst_node *lst_node_locate(lst_db *db, lst_node *node, long long key) {
  if (node == NULL) {
    if (db->head == NULL) {
      return NULL; // Empty list; not much we can do
    }
    // We start from the "closest" end of the list, hoping that the keys are uniformly distributed
    // Since this is a linked list, we can't do much better than hope anyway.
    node = (db->tail->key - key < key - db->head->key) ? db->tail : db->head;
  }
  if (node->key > key) { // We need to follow the prev pointer
    do {
      node = node->prev;
    } while (node != NULL && node->key > key);
    return node != NULL ? node : db->head;
  } else if (node->key < key) { // We need to follow the next pointer
    do {
      node = node->next;
    } while (node != NULL && node->key < key);
    return node != NULL ? node : db->tail;
  } else {
    return node;
  }
}

static kvds_cursor *lst_create_cursor(kvds_db *_db, long long key) {
  lst_db *db = _db;
  lst_cursor *cursor = malloc(sizeof(lst_cursor));

  cursor->key = key;
  cursor->best = lst_node_locate(db, NULL, key);

  return cursor;
}

static void lst_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  cursor->key = key;
  cursor->best = lst_node_locate(db, cursor->best, key); // Assume that we are moving to a close-by node
}

static void lst_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  free(cursor);
}

static long long lst_key(kvds_db *_db, kvds_cursor *_cursor) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  return cursor->key;
}

static bool lst_exists(kvds_db *_db, kvds_cursor *_cursor) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  return cursor->best != NULL && cursor->best->key == cursor->key;
}

static char *lst_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  if (cursor->best != NULL && cursor->best->key == cursor->key) { // Special case: already exists
    char *old_data = cursor->best->data;
    cursor->best->data = data;
    return old_data;
  }

  lst_node *new_node = malloc(sizeof(lst_node));

  new_node->data = data;
  new_node->key = cursor->key;
  new_node->prev = NULL;
  new_node->next = NULL;

  if (cursor->best == NULL) { // First node in the list
    db->head = new_node;
    db->tail = new_node;
  } else { // Insert ourselves on the correct side of the cursor

    if (cursor->best->key < cursor->key) {
      new_node->prev = cursor->best;
      new_node->next = cursor->best->next;
    } else {
      new_node->prev = cursor->best->prev;
      new_node->next = cursor->best;
    }

    if (new_node->next != NULL) {
      new_node->next->prev = new_node;
    } else {
      db->tail = new_node;
    }

    if (new_node->prev != NULL) {
      new_node->prev->next = new_node;
    } else {
      db->head = new_node;
    }
  }

  cursor->best = new_node;

  lst_assert_invariants(db);
  return NULL;
}

static char *lst_read(kvds_db *_db, kvds_cursor *_cursor) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  if (cursor->best != NULL && cursor->best->key == cursor->key) { // The node exists
    return cursor->best->data;
  } else {
    return NULL;
  }
}

static char *lst_remove(kvds_db *_db, kvds_cursor *_cursor) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  if (cursor->best == NULL || cursor->best->key != cursor->key) {
    return NULL;
  }

  char *data = cursor->best->data;

  lst_node *old_node = cursor->best;

  if (old_node->next != NULL) {
    old_node->next->prev = old_node->prev;
  } else {
    db->tail = old_node->prev;
  }

  if (old_node->prev != NULL) {
    old_node->prev->next = old_node->next;
  } else {
    db->head = old_node->next;
  }

  cursor->best = old_node->next != NULL ? old_node->next : old_node->prev; // Either one is fine, just pick the non-NULL one

  free(old_node);

  lst_assert_invariants(db);

  return data;
}

static void lst_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  lst_db *db = _db;
  lst_cursor *cursor = _cursor;

  if (cursor->best == NULL) {
    return; // Nothing in the database, nothing to find
  }

  switch (dir) {
  case KVDS_SNAP_CLOSEST_LOW: {
    if (cursor->best->key == cursor->key) {
      // Already at closest
    } else {
      lst_node *left;
      lst_node *right;
      if (cursor->key < cursor->best->key) {
        left = cursor->best->prev;
        right = cursor->best;
      } else {
        left = cursor->best;
        right = cursor->best->next;
      }
      if (left != NULL && right != NULL) { // Not past the edge
        if (cursor->key - left->key <= right->key - cursor->key) {
          cursor->best = left;
        } else {
          cursor->best = right;
        }
      } else {
        // cursor->best already contains closest
      }
    }
    cursor->key = cursor->best->key;
  } break;
  case KVDS_SNAP_HIGHER: {
    if (cursor->key >= cursor->best->key) {
      if (cursor->best->next != NULL) {
        cursor->best = cursor->best->next;
      }
    }
    cursor->key = cursor->best->key;
  } break;
  case KVDS_SNAP_LOWER: {
    if (cursor->key <= cursor->best->key) {
      if (cursor->best->prev != NULL) {
        cursor->best = cursor->best->prev;
      }
    }
    cursor->key = cursor->best->key;
  } break;
  }
}

REGISTER("linkedlist", "lst", "Store entries in a sorted doubly-linked list") = {
  .create_db = lst_create_db,
  .destroy_db = lst_destroy_db,
  .create_cursor = lst_create_cursor,
  .move_cursor = lst_move_cursor,
  .destroy_cursor = lst_destroy_cursor,

  .key = lst_key,
  .exists = lst_exists,
  .snap = lst_snap,

  .write = lst_write,
  .read = lst_read,
  .remove = lst_remove,
};
