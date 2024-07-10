#include "commands.h"
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvds_command_state {
  struct kvds_database_algo *algo;
  kvds_db *db;
  
  kvds_cursor *cursor;
} kvds_command_state;

typedef struct kvds_execute_result kvds_execute_result;

struct kvds_command_state* kvds_create_command_state(struct kvds_database_algo *algo, void *db) {
  kvds_command_state *state = malloc(sizeof(kvds_command_state));
  *state = (kvds_command_state) {
    .algo = algo,
    .db = db,
    .cursor = algo->create_cursor(db, 0),
  };
  return state;
}

void kvds_destroy_command_state(struct kvds_command_state *state) {
  state->algo->destroy_cursor(state->db, state->cursor);
  free(state);
}

char* kvds_get_error(int error) {
  if (error == 1) {
    return "Invalid command";
  }
  if (error == -1) {
    return "Quit";
  }
  return "Unknown Error";
}

int kvds_execute_command(struct kvds_command_state *state, char *command, char **output, int output_n) {
  char command_name[10];
  
  unsigned long command_len = 0;
  while (command[command_len] != '\0' && command[command_len] != ' ' && command[command_len] != '\n') {
    command_name[command_len] = command[command_len];
    
    command_len ++;
    if (command_len >= sizeof command_name / sizeof command_name[0]) {
      return 1;
    }
  }
  command_name[command_len] = '\0';
  char* args = command + command_len + (command[command_len] == '\0' ? 0 : 1);
  
  if (strcmp(command_name, "select") == 0 || strcmp(command_name, "s") == 0) {
    char* end;
    long long key = strtoll(args, &end, 10);
    args = end;
    
    state->algo->move_cursor(state->db, state->cursor, key);
    return 0;
  }
  if (strcmp(command_name, "key") == 0 || strcmp(command_name, "k") == 0) {
    long long key = state->algo->key(state->db, state->cursor);
    
    snprintf(*output, output_n, "%lld\n", key);
    return 0;
  }
  if (strcmp(command_name, "exists") == 0 || strcmp(command_name, "e") == 0) {
    bool exists = state->algo->exists(state->db, state->cursor);
    
    if (exists) {
      snprintf(*output, output_n, "yes\n");
    } else {
      snprintf(*output, output_n, "no\n");
    }
    return 0;
  }
  if (strcmp(command_name, "read") == 0 || strcmp(command_name, "r") == 0) {
    char *stored = state->algo->read(state->db, state->cursor);
    if (stored == NULL) {
      snprintf(*output, output_n, "(nil)\n");
    } else {
      *output = stored; // no copy \o/
    }
    return 0;
  }
  if (strcmp(command_name, "write") == 0 || strcmp(command_name, "w") == 0) {
    unsigned long args_len = strlen(args);
    char* copy = malloc(args_len + 1);
    strncpy(copy, args, args_len + 1);
    char *old_stored = state->algo->write(state->db, state->cursor, copy);
    free(old_stored);
    //snprintf(*output, output_n, "Stored %lu bytes\n", args_len);
    return 0;
  }
  if (strcmp(command_name, "delete") == 0 || strcmp(command_name, "d") == 0) {
    char *old_stored = state->algo->remove(state->db, state->cursor);
    free(old_stored);
    return 0;
  }
  if (strcmp(command_name, "prev") == 0 || strcmp(command_name, "p") == 0 || strcmp(command_name, "<") == 0) {
    state->algo->snap(state->db, state->cursor, KVDS_SNAP_LOWER);
    return 0;
  }
  if (strcmp(command_name, "next") == 0 || strcmp(command_name, "n") == 0 || strcmp(command_name, ">") == 0) {
    state->algo->snap(state->db, state->cursor, KVDS_SNAP_HIGHER);
    return 0;
  }
  if (strcmp(command_name, "closest") == 0 || strcmp(command_name, "c") == 0 || strcmp(command_name, "=") == 0) {
    state->algo->snap(state->db, state->cursor, KVDS_SNAP_CLOSEST_LOW);
    return 0;
  }
  if (strcmp(command_name, "quit") == 0 || strcmp(command_name, "q") == 0) {
    return -1;
  }
  if (strcmp(command_name, "qtl") == 0) {
    for (int i = 0; i < 10; i ++) {
      fprintf(stderr, "> s %d\n> w\n", i);
      state->algo->destroy_cursor(state->db, state->cursor);
      state->cursor = state->algo->create_cursor(state->db, i);
      char *old_stored = state->algo->write(state->db, state->cursor, NULL);
      free(old_stored);
    }
    return 0;
  }
  
  return 1;
}
