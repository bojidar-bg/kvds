#include "../registry.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct dlist_db {
  struct dlist_node *head; // lowest
  struct dlist_node *tail; // highest
  // int size;
} dlist_db;

typedef struct dlist_node {
  long long key;
  char *data;
  
  struct dlist_node *prev; // lower
  struct dlist_node *next; // higher
} dlist_node;

typedef struct dlist_cursor {
  long long key;
  struct dlist_node *best; // Either exact key or either node that would be next to the key
} dlist_cursor;

#ifndef NDEBUG
static void dlist_assert_invariants(dlist_db *db) {
  dlist_node *prev_node = NULL;
  dlist_node *node = db->head;
  while (node != NULL) {
    assert(node->prev == prev_node);
    if (prev_node != NULL) assert(node->key > prev_node->key);
    prev_node = node;
    node = node->next;
  }
  assert(db->tail == prev_node);
  if (prev_node != NULL) assert(prev_node->next == NULL);
}
#else
static void dlist_assert_invariants(dlist_db *db) {
  // pass
}
#endif

static kvds_db *dlist_create_db() {
  dlist_db *db = malloc(sizeof(dlist_db));
  db->head = NULL;
  db->tail = NULL;
  
  dlist_assert_invariants(db);
  
  return db;
}

static void dlist_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  dlist_db *db = _db;
  dlist_node *node = db->head;
  while (node != NULL) {
    dlist_node *next = node->next;
    free_data(node->data);
    free(node);
    node = next;
  }
  free(db);
}


static dlist_node *dlist_node_locate(dlist_db* db, dlist_node *node, long long key) {
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

static kvds_cursor *dlist_create_cursor(kvds_db *_db, long long key) {
  dlist_db *db = _db;
  dlist_cursor *cursor = malloc(sizeof(dlist_cursor));
  
  cursor->key = key;
  cursor->best = dlist_node_locate(db, NULL, key);
  
  return cursor;
}

static void dlist_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  cursor->key = key;
  cursor->best = dlist_node_locate(db, cursor->best, key); // Assume that we are moving to a close-by node
}

static void dlist_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  free(cursor);
}


static long long dlist_key(kvds_db *_db, kvds_cursor *_cursor) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  return cursor->key;
}

static bool dlist_exists(kvds_db *_db, kvds_cursor *_cursor) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  return cursor->best != NULL && cursor->best->key == cursor->key;
}

static char *dlist_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  if (cursor->best != NULL && cursor->best->key == cursor->key) { // Special case: already exists
    char *old_data = cursor->best->data;
    cursor->best->data = data;
    return old_data;
  }
  
  dlist_node *new_node = malloc(sizeof(dlist_node));
  
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
  
  dlist_assert_invariants(db);
  return NULL;
}

static char *dlist_read(kvds_db *_db, kvds_cursor *_cursor) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  if (cursor->best != NULL && cursor->best->key == cursor->key) { // The node exists
    return cursor->best->data;
  } else { 
    return NULL;
  }
}

static char *dlist_remove(kvds_db *_db, kvds_cursor *_cursor) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  if (cursor->best == NULL || cursor->best->key != cursor->key) {
    return NULL;
  }
  
  char *data = cursor->best->data;
  
  dlist_node *old_node = cursor->best;
  
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
  
  dlist_assert_invariants(db);
  
  return data;
}

static void dlist_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  dlist_db *db = _db;
  dlist_cursor *cursor = _cursor;
  
  if (cursor->best == NULL) {
    return; // Nothing in the database, nothing to find
  }
  
  switch (dir) {
    case KVDS_SNAP_CLOSEST_LOW: {
      if (cursor->best->key == cursor->key) {
        // Already at closest
      } else {
        dlist_node *left;
        dlist_node *right;
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

REGISTER("doubly_linked_list", "dlist") {
  .create_db = dlist_create_db,
  .destroy_db = dlist_destroy_db,
  .create_cursor = dlist_create_cursor,
  .move_cursor = dlist_move_cursor,
  .destroy_cursor = dlist_destroy_cursor,
  
  .key = dlist_key,
  .exists = dlist_exists,
  .snap = dlist_snap,
  
  .write = dlist_write,
  .read = dlist_read,
  .remove = dlist_remove,
};

