#ifndef NDEBUG
#include "../registry.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct invar_db {
  int algos_count;
  struct kvds_database_algo **algos;
  kvds_db **databases;
} invar_db;

typedef struct invar_cursor {
  kvds_cursor **cursors;
} invar_cursor;

static kvds_db *invar_create_db();

static int invar_list_algos(struct kvds_database_algo **result) {
  int algos_count = 0;
  struct kvds_database_algo *last_algo = NULL;
  for (struct kvds_registry_entry *entry = kvds_get_algos_list(); entry; entry = entry->next) {
    if (entry->algo == last_algo) { // Skip multiple names of the same algo
      continue;
    }
    last_algo = entry->algo;
    if (entry->algo->create_db == invar_create_db) { // Don't call ourselves recursively
      continue;
    }
    algos_count ++;
    if (result != NULL) {
      *result = entry->algo;
      result ++;
    }
  }
  return algos_count;
}
  
static kvds_db *invar_create_db() {
  invar_db *db = malloc(sizeof(invar_db));
  
  db->algos_count = invar_list_algos(NULL);
  db->algos = calloc(db->algos_count, sizeof(struct kvds_database_algo *));
  invar_list_algos(db->algos);
  
  db->databases = calloc(db->algos_count, sizeof(kvds_db *));
  
  for (int i = 0; i < db->algos_count; i ++) {
    db->databases[i] = db->algos[i]->create_db();
  }
  
  return db;
}

static void invar_dummy_free(char *data) {
  // pass
}

static void invar_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  invar_db *db = _db;
  
  for (int i = 0; i < db->algos_count; i ++) {
    if (i == db->algos_count - 1) {
      db->algos[i]->destroy_db(db->databases[i], free_data); // Only free the last db's data, to avoid a double-free
    } else {
      db->algos[i]->destroy_db(db->databases[i], invar_dummy_free);
    }
  }
  
  free(db->algos);
  free(db->databases);
  free(db);
}

static kvds_cursor *invar_create_cursor(kvds_db *_db, long long key) {
  invar_db *db = _db;
  invar_cursor *cursor = malloc(sizeof(invar_cursor));
  
  cursor->cursors = calloc(db->algos_count, sizeof(kvds_cursor *));
  
  for (int i = 0; i < db->algos_count; i ++) {
    cursor->cursors[i] = db->algos[i]->create_cursor(db->databases[i], key); 
  }
  
  return cursor;
}

static void invar_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  for (int i = 0; i < db->algos_count; i ++) {
    if (db->algos[i]->move_cursor != NULL) {
      db->algos[i]->move_cursor(db->databases[i], cursor->cursors[i], key);
    } else {
      db->algos[i]->destroy_cursor(db->databases[i], cursor->cursors[i]);
      cursor->cursors[i] = db->algos[i]->create_cursor(db->databases[i], key);
    }
  }
}

static void invar_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  for (int i = 0; i < db->algos_count; i ++) {
    db->algos[i]->destroy_cursor(db->databases[i], cursor->cursors[i]);
  }
  
  free(cursor->cursors);
  free(cursor);
}

// #define CONCAT_(a,b) a##b
// #define CONCAT(a,b) CONCAT_(a,b)

#define _INVAR_ASSERT(db, i, result, result_i, expression) \
    int i = 0; \
    typeof(expression) result; \
    for (; i < db->algos_count; i ++) { \
      typeof(expression) result_i = expression; \
      if (i == 0) { \
        result = result_i; \
      } else { \
        assert(result == result_i); \
      } \
    }

#define INVAR_ASSERT(db, i, result, expression) \
    _INVAR_ASSERT(db, i, result, CONCAT(result_i, __LINE__), expression)
    
#define INVAR_ASSERT_RETURN(db, i, expression) \
    INVAR_ASSERT(db, i, CONCAT(result, __LINE__), expression) \
    return CONCAT(result, __LINE__)

static long long invar_key(kvds_db *_db, kvds_cursor *_cursor) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  INVAR_ASSERT_RETURN(db, i, (db->algos[i]->key(db->databases[i], cursor->cursors[i])));
}

static bool invar_exists(kvds_db *_db, kvds_cursor *_cursor) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  INVAR_ASSERT_RETURN(db, i, (db->algos[i]->exists(db->databases[i], cursor->cursors[i])));
}

static void invar_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  // Here, we would like to snap and immediatelly check the keys
  INVAR_ASSERT(db, i, _key, (
    db->algos[i]->snap(db->databases[i], cursor->cursors[i], dir),
    db->algos[i]->key(db->databases[i], cursor->cursors[i])
  ));
}

static char *invar_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  INVAR_ASSERT_RETURN(db, i, (db->algos[i]->write(db->databases[i], cursor->cursors[i], data)));
}

static char *invar_read(kvds_db *_db, kvds_cursor *_cursor) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  INVAR_ASSERT_RETURN(db, i, (db->algos[i]->read(db->databases[i], cursor->cursors[i])));
}

static char *invar_remove(kvds_db *_db, kvds_cursor *_cursor) {
  invar_db *db = _db;
  invar_cursor *cursor = _cursor;
  
  INVAR_ASSERT_RETURN(db, i, (db->algos[i]->remove(db->databases[i], cursor->cursors[i])));
}

#undef INVAR_ASSERT_RETURN
#undef INVAR_ASSERT
#undef _INVAR_ASSERT


REGISTER("compare", "inv") {
  .create_db = invar_create_db,
  .destroy_db = invar_destroy_db,
  .create_cursor = invar_create_cursor,
  .move_cursor = invar_move_cursor,
  .destroy_cursor = invar_destroy_cursor,
  
  .key = invar_key,
  .exists = invar_exists,
  .snap = invar_snap,
  
  .write = invar_write,
  .read = invar_read,
  .remove = invar_remove,
};


#endif
