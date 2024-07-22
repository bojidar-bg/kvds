// SPDX-License-Identifier: MIT
#include "../registry.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SCG_SCAPEGOAT_FACTOR
#define SCG_SCAPEGOAT_FACTOR 10 / 16
#endif

typedef struct scg_db {
  struct scg_node *top;
} scg_db;

typedef struct scg_node {
  long long key;
  char *data;
  struct scg_node *left;
  struct scg_node *right;

  struct scg_node *parent;
  int size;
} scg_node;

typedef struct scg_cursor {
  long long key;
  struct scg_node *best;
  // Node under which the key would be if it were to exist in the tree
  // Guarantees: if key < best->key, then for each P of best->parent...->parent,
  //                p->key > best->key || p->key < key; and vice versa for flipped >/<
} scg_cursor;

static inline int scg_get_size(scg_node *node) {
  return node == NULL ? 0 : node->size;
}

static inline bool scg_is_left(scg_node *node) {
  return node->parent && node->parent->left == node;
}

static void _scg_print_tree(scg_node *node, int depth) {
  if (node == NULL) {
    fprintf(stderr, "%*c<>\n", depth * 2, ' ');
    return;
  }
  if (depth > 5) {
    fprintf(stderr, "%*c...\n", depth * 2, ' ');
    return;
  }
  fprintf(stderr, "%*c Node: %lld, size: %d\n", depth * 2, scg_is_left(node) ? '-' : '+', node->key, node->size);

  int calc_size = 1;

  if (node->left != NULL) {
    _scg_print_tree(node->left, depth + 1);
  }
  if (node->right != NULL) {
    _scg_print_tree(node->right, depth + 1);
  }
}
#ifndef NDEBUG
typedef struct scg_invariants {
  long long range_min;
  long long range_max;
} scg_invariants;
static scg_invariants _scg_assert_invariants(scg_node *node, int depth) {
  // fprintf(stderr, "%*c Node: %lld, size: %d\n", depth * 2, scg_is_left(node) ? '-' : '+', node->key, node->size);

  scg_invariants inv;
  int left_size = 0;
  int right_size = 0;

  if (node->left == NULL) {
    inv.range_min = node->key;
  } else {
    assert(node->left->parent == node);
    scg_invariants inv_left = _scg_assert_invariants(node->left, depth + 1);
    inv.range_min = inv_left.range_min;
    assert(inv_left.range_max < node->key);
    left_size = node->left->size;
  }
  if (node->right == NULL) {
    inv.range_max = node->key;
  } else {
    assert(node->right->parent == node);
    scg_invariants inv_right = _scg_assert_invariants(node->right, depth + 1);
    inv.range_max = inv_right.range_max;
    assert(node->key < inv_right.range_min);
    right_size = node->right->size;
  }

  assert(node->size == left_size + right_size + 1);
  assert(left_size <= node->size * SCG_SCAPEGOAT_FACTOR);
  assert(right_size <= node->size * SCG_SCAPEGOAT_FACTOR);

  return inv;
}
static void scg_assert_invariants(scg_db *db) {
  _scg_assert_invariants(db->top, 0);
  assert(db->top->parent == NULL);
}
#else
static void scg_assert_invariants(scg_db *db) {
  // pass
}
#endif

static kvds_db *scg_create_db() {
  scg_db *db = malloc(sizeof(scg_db));
  db->top = NULL;
  return db;
}

static void scg_node_destroy(scg_node *node, void (*free_data)(char *data)) {
  free_data(node->data);
  if (node->left) scg_node_destroy(node->left, free_data);
  if (node->right) scg_node_destroy(node->right, free_data);
  free(node);
}

static void scg_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  scg_db *db = _db;
  if (db->top) scg_node_destroy(db->top, free_data);
  free(db);
}

static scg_node *scg_node_locate(scg_db *db, long long key) {
  scg_node *best = db->top;

  while (best != NULL && best->key != key) {
    if (key < best->key) {
      if (best->left == NULL) break;
      best = best->left;
    } else { // key < best->key
      if (best->right == NULL) break;
      best = best->right;
    }
  }

  return best;
}
static scg_node *scg_node_navigate_left(scg_node *node) {
  if (node->left) { // descend left if we can
    scg_node *result = node->left;
    while (result->right != NULL) result = result->right;
    return result;
  } else {
    while (node->parent != NULL) {
      if (node->parent->right == node) { // We were right of that parent, meaning it's left of us
        return node->parent;
      }
      node = node->parent;
    }
    return NULL;
  }
}
static scg_node *scg_node_navigate_right(scg_node *node) {
  if (node->right) { // descend right if we can
    scg_node *result = node->right;
    while (result->left != NULL) result = result->left;
    return result;
  } else {
    while (node->parent != NULL) {
      if (node->parent->left == node) { // We were left of that parent, meaning it's right of us
        return node->parent;
      }
      node = node->parent;
    }
    return NULL;
  }
}

static kvds_cursor *scg_create_cursor(kvds_db *_db, long long key) {
  scg_db *db = _db;
  scg_cursor *cursor = malloc(sizeof(scg_cursor));

  cursor->key = key;
  cursor->best = scg_node_locate(db, key);

  return cursor;
}

static void scg_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  cursor->key = key;
  cursor->best = scg_node_locate(db, key);
}

static void scg_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  free(cursor);
}

static long long scg_key(kvds_db *_db, kvds_cursor *_cursor) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  return cursor->key;
}

static bool scg_exists(kvds_db *_db, kvds_cursor *_cursor) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  return cursor->best != NULL && cursor->best->key == cursor->key;
}

static void scg_node_detach(scg_db *db, scg_node *node, bool update_size) {
  if (node->parent == NULL) {
    assert(db->top == node);
    db->top = NULL;
  } else if (node->parent->left == node) {
    node->parent->left = NULL;
  } else if (node->parent->right == node) {
    node->parent->right = NULL;
  } else {
    assert(false);
  }
  if (update_size) {
    for (scg_node *old_parent = node->parent; old_parent != NULL; old_parent = old_parent->parent) {
      old_parent->size -= node->size;
    }
  }
  node->parent = NULL;
}
static void scg_node_attach(scg_db *db, scg_node *node, scg_node *parent, bool on_left, bool update_size) {
  assert(node->parent == NULL);
  node->parent = parent;

  if (node->parent == NULL) {
    assert(db != NULL && db->top == NULL);
    db->top = node;
  } else if (on_left) {
    assert(parent->left == NULL);
    node->parent->left = node;
  } else {
    assert(parent->right == NULL);
    node->parent->right = node;
  }
  if (update_size) {
    for (scg_node *new_parent = node->parent; new_parent != NULL; new_parent = new_parent->parent) {
      assert(new_parent != node);
      new_parent->size += node->size;
    }
  }
}
static scg_node *scg_node_rotate(scg_db *db, scg_node *node) {
  assert(node->parent != NULL);
  scg_node *parent = node->parent;
  bool is_left = parent->left == node;
  scg_node *parent_other = is_left ? parent->right : parent->left;
  scg_node *middle = is_left ? node->right : node->left;
  scg_node *parent_old_loc = parent->parent;
  bool parent_old_is_left = scg_is_left(parent);

  scg_node_detach(db, parent, true);
  scg_node_detach(db, node, true);
  if (middle != NULL) scg_node_detach(db, middle, true);

  scg_node_attach(db, parent, node, !is_left, true);
  scg_node_attach(db, node, parent_old_loc, parent_old_is_left, true);
  if (middle != NULL) scg_node_attach(db, middle, parent, is_left, true);
  return node;
}

static void _scg_node_recreate_collect(scg_node *node, scg_node ***nodes_i_p) {
  if (node != NULL) {
    scg_node *left_to_process = node->left;
    scg_node *right_to_process = node->right;
    _scg_node_recreate_collect(left_to_process, nodes_i_p);
    // Process nodes in order:

    **nodes_i_p = node;
    (*nodes_i_p)++;

    _scg_node_recreate_collect(right_to_process, nodes_i_p);
  }
}
static scg_node *_scg_node_recreate_reparent(scg_node **nodes, int count, scg_node *parent) {
  if (count == 0) {
    return NULL;
  }
  scg_node *median = nodes[count / 2];

  median->left = _scg_node_recreate_reparent(nodes, count / 2, median);
  median->right = _scg_node_recreate_reparent(&nodes[count / 2] + 1, (count - 1) / 2, median);
  median->parent = parent;
  median->size = 1 + scg_get_size(median->left) + scg_get_size(median->right);

  return median;
}

static void scg_node_recreate(scg_db *db, scg_node *old_root, int size) {
  scg_node *old_parent = old_root->parent;
  bool old_parent_loc = scg_is_left(old_root);

  scg_node_detach(db, old_root, false);

  scg_node **nodes = malloc(size * sizeof(scg_node *));

  scg_node **nodes_i = nodes;
  _scg_node_recreate_collect(old_root, &nodes_i);
  assert(&nodes[size] == nodes_i);

  scg_node *new_root = _scg_node_recreate_reparent(nodes, size, NULL); // old_root

  free(nodes);

  scg_node_attach(db, new_root, old_parent, old_parent_loc, false);
}

static void scg_node_rebalance_from(scg_db *db, scg_node *node) {
  // Using the general algorithm for a Scrapegoat tree via https://en.wikipedia.org/wiki/Scapegoat_tree
  // After plenty of sweat and tears trying to come up with something more efficient on my own
  scg_node *to_recreate = NULL;
  for (; node != NULL; node = node->parent) {
    int left_size = scg_get_size(node->left);
    int right_size = scg_get_size(node->right);
    int node_size = scg_get_size(node);

    if (left_size > node_size * SCG_SCAPEGOAT_FACTOR || right_size > node_size * SCG_SCAPEGOAT_FACTOR) {
      to_recreate = node;
    }
  }
  if (to_recreate != NULL) {
    scg_node_recreate(db, to_recreate, scg_get_size(to_recreate));
  }
}

static char *scg_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  if (cursor->best != NULL && cursor->best->key == cursor->key) { // Special case: already exists
    char *old_data = cursor->best->data;
    cursor->best->data = data;
    return old_data;
  }

  scg_node *new_node = malloc(sizeof(scg_node));

  new_node->data = data;
  new_node->key = cursor->key;
  new_node->left = NULL;
  new_node->right = NULL;
  new_node->parent = NULL;
  new_node->size = 1;

  scg_node_attach(db, new_node, cursor->best, (cursor->best && new_node->key < cursor->best->key), true);
  scg_node_rebalance_from(db, new_node);

  cursor->best = new_node;

  scg_assert_invariants(db);
  return NULL;
}

static char *scg_read(kvds_db *_db, kvds_cursor *_cursor) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  if (cursor->best != NULL && cursor->best->key == cursor->key) { // The node exists
    return cursor->best->data;
  } else {
    return NULL;
  }
}

static char *scg_remove(kvds_db *_db, kvds_cursor *_cursor) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  if (cursor->best == NULL || cursor->best->key != cursor->key) {
    return NULL;
  } else {
    char *data = cursor->best->data;
    scg_node *node = cursor->best;
    scg_node *swap_node = NULL;
    if (node->left == NULL && node->right == NULL) {
      // YAY!
    } else if (scg_get_size(node->right) > scg_get_size(node->left)) { // Swap with a node from the side that's heavier
      assert(node->right != NULL);
      swap_node = node->right;
      while (swap_node->left != NULL) swap_node = swap_node->left;
      if (swap_node->right != NULL) {
        scg_node_rotate(db, swap_node->right);
      }
    } else {
      assert(node->left != NULL);
      swap_node = node->left;
      while (swap_node->right != NULL) swap_node = swap_node->right;
      if (swap_node->left != NULL) {
        scg_node_rotate(db, swap_node->left);
      }
    }
    scg_node *old_parent = node->parent;
    bool old_was_left = scg_is_left(node);

    scg_node_detach(db, node, true);

    if (swap_node != NULL) {
      scg_node *node_left = node->left;
      scg_node *node_right = node->right;
      if (node_left != NULL) scg_node_detach(db, node_left, true);
      if (node_right != NULL) scg_node_detach(db, node_right, true);

      scg_node *rebalance_from = swap_node;
      if (node_left != swap_node && node_right != swap_node) {
        rebalance_from = swap_node->parent;
        scg_node_detach(db, swap_node, true);
      }

      if (node_left != swap_node && node_left != NULL) scg_node_attach(db, node_left, swap_node, true, true);
      if (node_right != swap_node && node_right != NULL) scg_node_attach(db, node_right, swap_node, false, true);
      scg_node_attach(db, swap_node, old_parent, old_was_left, true);

      scg_node_rebalance_from(db, rebalance_from);
    } else {
      scg_node_rebalance_from(db, old_parent);
    }

    cursor->best = scg_node_locate(db, cursor->key);

    return data;
  }
}

static void scg_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  scg_db *db = _db;
  scg_cursor *cursor = _cursor;

  if (cursor->best == NULL) {
    return; // Nothing in the database, nothing to find
  }

  switch (dir) {
  case KVDS_SNAP_HIGHER: {
    if (cursor->best->key <= cursor->key) {
      scg_node *alternative = scg_node_navigate_right(cursor->best);
      if (alternative != NULL) {
        cursor->best = alternative;
      }
    }
    cursor->key = cursor->best->key;
  } break;
  case KVDS_SNAP_LOWER: {
    if (cursor->key <= cursor->best->key) {
      scg_node *alternative = scg_node_navigate_left(cursor->best);
      if (alternative != NULL) {
        cursor->best = alternative;
      }
    }
    cursor->key = cursor->best->key;
  } break;
  case KVDS_SNAP_CLOSEST_LOW: {
    if (cursor->best->key == cursor->key) {
      // Already at closest
    } else {
      scg_node *left;
      scg_node *right;
      if (cursor->key < cursor->best->key) {
        left = scg_node_navigate_left(cursor->best);
        right = cursor->best;
      } else {
        left = cursor->best;
        right = scg_node_navigate_right(cursor->best);
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
  }
}

REGISTER("scapegoat", "scg", "Store entries in a scapegoat-balanced binary search tree.") = {
  .create_db = scg_create_db,
  .destroy_db = scg_destroy_db,
  .create_cursor = scg_create_cursor,
  .move_cursor = scg_move_cursor,
  .destroy_cursor = scg_destroy_cursor,

  .key = scg_key,
  .exists = scg_exists,
  .snap = scg_snap,

  .write = scg_write,
  .read = scg_read,
  .remove = scg_remove,
};
