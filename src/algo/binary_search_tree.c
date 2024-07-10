#include "../registry.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#define BST_SCRAPEGOAT_FACTOR 10 / 16

typedef struct bst_db {
  struct bst_node *top;
} bst_db;

typedef struct bst_node {
  long long key;
  char *data;
  struct bst_node *left;
  struct bst_node *right;
  
  struct bst_node *parent;
  int size;
} bst_node;

typedef struct bst_cursor {
  long long key;
  struct bst_node *best;
  // Node under which the key would be if it were to exist in the tree
  // Guarantees: if key < best->key, then for each P of best->parent...->parent,
  //                p->key > best->key || p->key < key; and vice versa for flipped >/<
} bst_cursor;

static inline int bst_get_size(bst_node *node) {
  return node == NULL ? 0 : node->size;
}

static inline bool bst_is_left(bst_node *node) {
  return node->parent && node->parent->left == node;
}

static void _bst_print_tree(bst_node *node, int depth) {
  if (node == NULL) {
    fprintf(stderr, "%*c<>\n", depth * 2, ' ');
    return;
  }
  if (depth > 5) {
    fprintf(stderr, "%*c...\n", depth * 2, ' ');
    return;
  }
  fprintf(stderr, "%*c Node: %lld, size: %d\n", depth * 2, bst_is_left(node) ? '-' : '+', node->key, node->size);
  
  int calc_size = 1;
  
  if (node->left != NULL) {
    _bst_print_tree(node->left, depth + 1);
  }
  if (node->right != NULL) {
    _bst_print_tree(node->right, depth + 1);
  }
}
#ifndef NDEBUG
typedef struct bst_invariants {
  long long range_min;
  long long range_max;
} bst_invariants;
static bst_invariants _bst_assert_invariants(bst_node *node, int depth) {
  //fprintf(stderr, "%*c Node: %lld, size: %d\n", depth * 2, bst_is_left(node) ? '-' : '+', node->key, node->size);
  
  bst_invariants inv;
  int left_size = 0;
  int right_size = 0;
  
  if (node->left == NULL) {
    inv.range_min = node->key; 
  } else {
    assert(node->left->parent == node);
    bst_invariants inv_left = _bst_assert_invariants(node->left, depth + 1);
    inv.range_min = inv_left.range_min;
    assert(inv_left.range_max < node->key);
    left_size = node->left->size;
  }
  if (node->right == NULL) {
    inv.range_max = node->key; 
  } else {
    assert(node->right->parent == node);
    bst_invariants inv_right = _bst_assert_invariants(node->right, depth + 1);
    inv.range_max = inv_right.range_max;
    assert(node->key < inv_right.range_min);
    right_size = node->right->size;
  }
  
  assert(node->size == left_size + right_size + 1);
  assert(left_size <= node->size * BST_SCRAPEGOAT_FACTOR);
  assert(right_size <= node->size * BST_SCRAPEGOAT_FACTOR);
  
  return inv;
}
static void bst_assert_invariants(bst_db *db) {
  _bst_assert_invariants(db->top, 0);
  assert(db->top->parent == NULL);
}
#else
static void bst_assert_invariants(bst_db *db) {
  // pass
}
#endif

static kvds_db *bst_create_db() {
  bst_db *db = malloc(sizeof(bst_db));
  db->top = NULL;
  return db;
}

static void bst_node_destroy(bst_node *node, void (*free_data)(char *data)) {
  free_data(node->data);
  if (node->left) bst_node_destroy(node->left, free_data);
  if (node->right) bst_node_destroy(node->right, free_data);
  free(node);
}

static void bst_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  bst_db *db = _db;
  if (db->top) bst_node_destroy(db->top, free_data);
  free(db);
}


static bst_node *bst_node_locate(bst_db *db, long long key) {
  bst_node *best = db->top;
  
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
static bst_node *bst_node_navigate_left(bst_node *node) {
  if (node->left) { // descend left if we can
    bst_node *result = node->left;
    while(result->right != NULL) result = result->right;
    return result;
  } else {
    while(node->parent != NULL) {
      if (node->parent->right == node) { // We were right of that parent, meaning it's left of us
        return node->parent;
      }
      node = node->parent;
    }
    return NULL;
  }
}
static bst_node *bst_node_navigate_right(bst_node *node) {
  if (node->right) { // descend right if we can
    bst_node *result = node->right;
    while(result->left != NULL) result = result->left;
    return result;
  } else {
    while(node->parent != NULL) {
      if (node->parent->left == node) { // We were left of that parent, meaning it's right of us
        return node->parent;
      }
      node = node->parent;
    }
    return NULL;
  }
}

static kvds_cursor *bst_create_cursor(kvds_db *_db, long long key) {
  bst_db *db = _db;
  bst_cursor *cursor = malloc(sizeof(bst_cursor));
  
  cursor->key = key;
  cursor->best = bst_node_locate(db, key);
  
  return cursor;
}

static void bst_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  cursor->key = key;
  cursor->best = bst_node_locate(db, key);
}

static void bst_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  free(cursor);
}


static long long bst_key(kvds_db *_db, kvds_cursor *_cursor) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  return cursor->key;
}

static bool bst_exists(kvds_db *_db, kvds_cursor *_cursor) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  return cursor->best != NULL && cursor->best->key == cursor->key;
}

static void bst_node_detach(bst_db *db, bst_node *node, bool update_size) {
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
    for(bst_node *old_parent = node->parent; old_parent != NULL; old_parent = old_parent->parent) {
      old_parent->size -= node->size;
    }
  }
  node->parent = NULL;
}
static void bst_node_attach(bst_db *db, bst_node *node, bst_node *parent, bool on_left, bool update_size) {
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
    for(bst_node *new_parent = node->parent; new_parent != NULL; new_parent = new_parent->parent) {
      assert(new_parent != node);
      new_parent->size += node->size;
    }
  }
}
static bst_node *bst_node_rotate(bst_db *db, bst_node *node) {
  assert(node->parent != NULL);
  bst_node *parent = node->parent;
  bool is_left = parent->left == node;
  bst_node *parent_other = is_left ? parent->right : parent->left;
  bst_node *middle = is_left ? node->right : node->left;
  bst_node *parent_old_loc = parent->parent;
  bool parent_old_is_left = bst_is_left(parent);
  
  bst_node_detach(db, parent, true);
  bst_node_detach(db, node, true);
  if (middle != NULL) bst_node_detach(db, middle, true);
  
  bst_node_attach(db, parent, node, !is_left, true);
  bst_node_attach(db, node, parent_old_loc, parent_old_is_left, true);
  if (middle != NULL) bst_node_attach(db, middle, parent, is_left, true);
  return node;
}

static void _bst_node_recreate_collect(bst_node *node, bst_node ***nodes_i_p) {
  if (node != NULL) {
    bst_node *left_to_process = node->left;
    bst_node *right_to_process = node->right;
    _bst_node_recreate_collect(left_to_process, nodes_i_p);
    // Process nodes in order:
    
    **nodes_i_p = node;
    (*nodes_i_p)++;
    
    _bst_node_recreate_collect(right_to_process, nodes_i_p);
  }
}
static bst_node *_bst_node_recreate_reparent(bst_node **nodes, int count, bst_node *parent) {
  if (count == 0) {
    return NULL;
  }
  bst_node *median = nodes[count / 2];
  
  median->left = _bst_node_recreate_reparent(nodes, count / 2, median);
  median->right = _bst_node_recreate_reparent(&nodes[count / 2] + 1, (count - 1) / 2, median);
  median->parent = parent;
  median->size = 1 + bst_get_size(median->left) + bst_get_size(median->right);
  
  return median;
}

static void bst_node_recreate(bst_db *db, bst_node *old_root, int size) {
  bst_node *old_parent = old_root->parent;
  bool old_parent_loc = bst_is_left(old_root);
  
  bst_node_detach(db, old_root, false);
  
  bst_node **nodes = malloc(size * sizeof(bst_node *));
  
  bst_node **nodes_i = nodes;
  _bst_node_recreate_collect(old_root, &nodes_i);
  assert(&nodes[size] == nodes_i);
  
  bst_node *new_root = _bst_node_recreate_reparent(nodes, size, NULL); // old_root
  
  free(nodes);
  
  bst_node_attach(db, new_root, old_parent, old_parent_loc, false);
}

static void bst_node_rebalance_from(bst_db *db, bst_node *node) {
  // Using the general algorithm for a Scrapegoat tree via https://en.wikipedia.org/wiki/Scapegoat_tree
  // After plenty of sweat and tears trying to come up with something more efficient on my own
  bst_node *to_recreate = NULL;
  for(; node != NULL; node = node->parent) {
    int left_size = bst_get_size(node->left);
    int right_size = bst_get_size(node->right);
    int node_size = bst_get_size(node);
    
    if (left_size > node_size * BST_SCRAPEGOAT_FACTOR || right_size > node_size * BST_SCRAPEGOAT_FACTOR) {
      to_recreate = node;
    }
  }
  if (to_recreate != NULL) {
    bst_node_recreate(db, to_recreate, bst_get_size(to_recreate));
  }
}

static char *bst_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  if (cursor->best != NULL && cursor->best->key == cursor->key) { // Special case: already exists
    char *old_data = cursor->best->data;
    cursor->best->data = data;
    return old_data;
  }
  
  bst_node *new_node = malloc(sizeof(bst_node));
  
  new_node->data = data;
  new_node->key = cursor->key;
  new_node->left = NULL;
  new_node->right = NULL;
  new_node->parent = NULL;
  new_node->size = 1;
  
  bst_node_attach(db, new_node, cursor->best, (cursor->best && new_node->key < cursor->best->key), true);
  bst_node_rebalance_from(db, new_node);

  cursor->best = new_node;
  
  bst_assert_invariants(db);
  return NULL;
}

static char *bst_read(kvds_db *_db, kvds_cursor *_cursor) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  if (cursor->best != NULL && cursor->best->key == cursor->key) { // Special case: already exists
    return cursor->best->data;
  } else { 
    return NULL;
  }
}

static char *bst_remove(kvds_db *_db, kvds_cursor *_cursor) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  if (cursor->best == NULL || cursor->best->key != cursor->key) {
    return NULL;
  } else {
    char *data = cursor->best->data;
    bst_node *node = cursor->best;
    bst_node *swap_node = NULL;
    if (node->left == NULL && node->right == NULL) {
      // YAY!
    } else if (bst_get_size(node->right) > bst_get_size(node->left)) { // Swap with a node from the side that's heavier
      assert(node->right != NULL);
      swap_node = node->right;
      while (swap_node->left != NULL) swap_node = swap_node->left;
      if (swap_node->right != NULL) {
        bst_node_rotate(db, swap_node->right);
      }
    } else {
      assert(node->left != NULL);
      swap_node = node->left;
      while (swap_node->right != NULL) swap_node = swap_node->right;
      if (swap_node->left != NULL) {
        bst_node_rotate(db, swap_node->left);
      }
    }
    bst_node *old_parent = node->parent;
    bool old_was_left = bst_is_left(node);
    
    bst_node_detach(db, node, true);
    
    if (swap_node != NULL) {
      bst_node *node_left = node->left;
      bst_node *node_right = node->right;
      if (node_left != NULL) bst_node_detach(db, node_left, true);
      if (node_right != NULL) bst_node_detach(db, node_right, true);
    
      bst_node *rebalance_from = swap_node;
      if (node_left != swap_node && node_right != swap_node) {
        rebalance_from = swap_node->parent;
        bst_node_detach(db, swap_node, true);
      }
      
      if (node_left != swap_node && node_left != NULL) bst_node_attach(db, node_left, swap_node, true, true);
      if (node_right != swap_node && node_right != NULL) bst_node_attach(db, node_right, swap_node, false, true);
      bst_node_attach(db, swap_node, old_parent, old_was_left, true);
      
      bst_node_rebalance_from(db, rebalance_from);
    } else {
      bst_node_rebalance_from(db, old_parent);
    }
    
    cursor->best = bst_node_locate(db, cursor->key);
    
    return data;
  }
}

static void bst_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  bst_db *db = _db;
  bst_cursor *cursor = _cursor;
  
  if (cursor->best == NULL) {
    return; // Nothing in the database, nothing to find
  }
  
  switch (dir) {
    case KVDS_SNAP_CLOSEST_LOW: {
      if (cursor->best->key == cursor->key) {
        // Already at closest
      } else if (cursor->key < cursor->best->key) {
        bst_node *alternative = bst_node_navigate_left(cursor->best);
        if (alternative != NULL && alternative->key - cursor->key < cursor->key - cursor->best->key) {
          cursor->best = alternative;
        }
      } else {
        bst_node *alternative = bst_node_navigate_right(cursor->best);
        if (alternative != NULL && cursor->key - alternative->key < cursor->best->key - cursor->key) {
          cursor->best = alternative;
        }
      }
      cursor->key = cursor->best->key;
    } break;
    case KVDS_SNAP_HIGHER: {
      if (cursor->best->key <= cursor->key) {
        bst_node *alternative = bst_node_navigate_right(cursor->best);
        if (alternative != NULL) {
          cursor->best = alternative;
        }
      }
      cursor->key = cursor->best->key;
    } break;
    case KVDS_SNAP_LOWER: {
      if (cursor->key <= cursor->best->key) {
        bst_node *alternative = bst_node_navigate_left(cursor->best);
        if (alternative != NULL) {
          cursor->best = alternative;
        }
      }
      cursor->key = cursor->best->key;
    } break;
  }
}

REGISTER("binary_search_tree", "bst") {
  .create_db = bst_create_db,
  .destroy_db = bst_destroy_db,
  .create_cursor = bst_create_cursor,
  .move_cursor = bst_move_cursor,
  .destroy_cursor = bst_destroy_cursor,
  
  .key = bst_key,
  .exists = bst_exists,
  .snap = bst_snap,
  
  .write = bst_write,
  .read = bst_read,
  .remove = bst_remove,
};

