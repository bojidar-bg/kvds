#ifndef NDEBUG
#include "../registry.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct inv_db {
  int algos_count;
  struct kvds_database_algo **algos;
  kvds_db **databases;
} inv_db;

typedef struct inv_cursor {
  kvds_cursor **cursors;
} inv_cursor;

static kvds_db *inv_create_db();

static int inv_list_algos(struct kvds_database_algo **result) {
  int algos_count = 0;
  struct kvds_database_algo *last_algo = NULL;
  for (struct kvds_registry_entry *entry = kvds_get_algos_list(); entry; entry = entry->next) {
    if (entry->algo == last_algo) { // Skip multiple names of the same algo
      continue;
    }
    last_algo = entry->algo;
    if (entry->algo->create_db == inv_create_db) { // Don't call ourselves recursively
      continue;
    }
    algos_count++;
    if (result != NULL) {
      *result = entry->algo;
      result++;
    }
  }
  return algos_count;
}

static kvds_db *inv_create_db() {
  inv_db *db = malloc(sizeof(inv_db));

  db->algos_count = inv_list_algos(NULL);
  db->algos = calloc(db->algos_count, sizeof(struct kvds_database_algo *));
  inv_list_algos(db->algos);

  db->databases = calloc(db->algos_count, sizeof(kvds_db *));

  for (int i = 0; i < db->algos_count; i++) {
    db->databases[i] = db->algos[i]->create_db();
  }

  return db;
}

static void inv_dummy_free(char *data) {
  // pass
}

static void inv_destroy_db(kvds_db *_db, void (*free_data)(char *data)) {
  inv_db *db = _db;

  for (int i = 0; i < db->algos_count; i++) {
    if (i == db->algos_count - 1) {
      db->algos[i]->destroy_db(db->databases[i], free_data); // Only free the last db's data, to avoid a double-free
    } else {
      db->algos[i]->destroy_db(db->databases[i], inv_dummy_free);
    }
  }

  free(db->algos);
  free(db->databases);
  free(db);
}

static kvds_cursor *inv_create_cursor(kvds_db *_db, long long key) {
  inv_db *db = _db;
  inv_cursor *cursor = malloc(sizeof(inv_cursor));

  cursor->cursors = calloc(db->algos_count, sizeof(kvds_cursor *));

  for (int i = 0; i < db->algos_count; i++) {
    cursor->cursors[i] = db->algos[i]->create_cursor(db->databases[i], key);
  }

  return cursor;
}

static void inv_move_cursor(kvds_db *_db, kvds_cursor *_cursor, long long key) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  for (int i = 0; i < db->algos_count; i++) {
    if (db->algos[i]->move_cursor != NULL) {
      db->algos[i]->move_cursor(db->databases[i], cursor->cursors[i], key);
    } else {
      db->algos[i]->destroy_cursor(db->databases[i], cursor->cursors[i]);
      cursor->cursors[i] = db->algos[i]->create_cursor(db->databases[i], key);
    }
  }
}

static void inv_destroy_cursor(kvds_db *_db, kvds_cursor *_cursor) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  for (int i = 0; i < db->algos_count; i++) {
    db->algos[i]->destroy_cursor(db->databases[i], cursor->cursors[i]);
  }

  free(cursor->cursors);
  free(cursor);
}

// #define CONCAT_(a,b) a##b
// #define CONCAT(a,b) CONCAT_(a,b)

#define _INV_ASSERT(db, i, result, result_i, expression) \
  int i = 0; \
  typeof(expression) result; \
  for (; i < db->algos_count; i++) { \
    typeof(expression) result_i = expression; \
    if (i == 0) { \
      result = result_i; \
    } else { \
      assert(result == result_i); \
    } \
  }

#define INV_ASSERT(db, i, result, expression) \
  _INV_ASSERT(db, i, result, CONCAT(result_i, __LINE__), expression)

#define INV_ASSERT_RETURN(db, i, expression) \
  INV_ASSERT(db, i, CONCAT(result, __LINE__), expression) \
  return CONCAT(result, __LINE__)

static long long inv_key(kvds_db *_db, kvds_cursor *_cursor) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  INV_ASSERT_RETURN(db, i, (db->algos[i]->key(db->databases[i], cursor->cursors[i])));
}

static bool inv_exists(kvds_db *_db, kvds_cursor *_cursor) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  INV_ASSERT_RETURN(db, i, (db->algos[i]->exists(db->databases[i], cursor->cursors[i])));
}

static void inv_snap(kvds_db *_db, kvds_cursor *_cursor, enum kvds_snap_direction dir) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  // Here, we would like to snap and immediatelly check the keys
  INV_ASSERT(db, i, _key, (db->algos[i]->snap(db->databases[i], cursor->cursors[i], dir), db->algos[i]->key(db->databases[i], cursor->cursors[i])));
}

static char *inv_write(kvds_db *_db, kvds_cursor *_cursor, char *data) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  INV_ASSERT_RETURN(db, i, (db->algos[i]->write(db->databases[i], cursor->cursors[i], data)));
}

static char *inv_read(kvds_db *_db, kvds_cursor *_cursor) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  INV_ASSERT_RETURN(db, i, (db->algos[i]->read(db->databases[i], cursor->cursors[i])));
}

static char *inv_remove(kvds_db *_db, kvds_cursor *_cursor) {
  inv_db *db = _db;
  inv_cursor *cursor = _cursor;

  INV_ASSERT_RETURN(db, i, (db->algos[i]->remove(db->databases[i], cursor->cursors[i])));
}

#undef INV_ASSERT_RETURN
#undef INV_ASSERT
#undef _INV_ASSERT

REGISTER("default", "inv", "Run all algorithms and compare the results") = {
  .create_db = inv_create_db,
  .destroy_db = inv_destroy_db,
  .create_cursor = inv_create_cursor,
  .move_cursor = inv_move_cursor,
  .destroy_cursor = inv_destroy_cursor,

  .key = inv_key,
  .exists = inv_exists,
  .snap = inv_snap,

  .write = inv_write,
  .read = inv_read,
  .remove = inv_remove,
};

#endif
