// SPDX-License-Identifier: MIT
#pragma once
#include <stdbool.h>

enum kvds_snap_direction {
  KVDS_SNAP_LOWER,
  KVDS_SNAP_HIGHER,
  KVDS_SNAP_CLOSEST_LOW,
};

typedef void kvds_db;
typedef void kvds_cursor;

struct kvds_database_algo {
  kvds_db *(*create_db)(); // NOTE: may need options
  void (*destroy_db)(kvds_db *db, void (*free_data)(char *data)); // Ownership: may assume all cursors are freed ---- but what about char *data-s?

  kvds_cursor *(*create_cursor)(kvds_db *db, long long key); // Ownership: cursor borrows DB
  void (*move_cursor)(kvds_db *db, kvds_cursor *cursor, long long key);
  void (*destroy_cursor)(kvds_db *db, kvds_cursor *cursor);

  long long (*key)(kvds_db *db, kvds_cursor *cursor);
  bool (*exists)(kvds_db *db, kvds_cursor *cursor);
  void (*snap)(kvds_db *db, kvds_cursor *cursor, enum kvds_snap_direction dir);

  char *(*write)(kvds_db *db, kvds_cursor *cursor, char *data); // Ownership: data owned to the db, returned value owned by caller
  char *(*read)(kvds_db *db, kvds_cursor *cursor); // Ownership: returned value borrowed by caller
  char *(*remove)(kvds_db *db, kvds_cursor *cursor); // Ownership: returned value owned by caller
};
