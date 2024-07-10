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
  if (error == 2) {
    return "Unimplemented command";
  }
  if (error == -1) {
    return "Quit";
  }
  return "Unknown Error";
}

int kvds_execute_command(struct kvds_command_state *state, char *command, FILE *output) {
  while (command[0] != '\0') {
    
    unsigned long command_len = 0;
    while (command[0] != '\0' && (command[0] == ' ' || command[0] == '\n')) {
      command ++;
    }
    if (command[0] == '\0') {
      break;
    }
    
    while (command[command_len] != '\0' && command[command_len] != ' ' && command[command_len] != '\n') {
      command_len ++;
    }
    char* args = &command[command_len];
    while (args[0] != '\0' && (args[0] == ' ' || args[0] == '\n')) {
      args ++;
    }
  
#define ISCMD(cmd) (command_len == strlen(cmd) && strncmp(command, cmd, command_len) == 0)
    
    if (ISCMD("select") || ISCMD("s")) {
      char* end;
      long long key = strtoll(args, &end, 10);
      args = end;
      if (!state->algo->move_cursor) {
        state->algo->destroy_cursor(state->db, state->cursor);
        state->algo->create_cursor(state->db, key);
      } else {
        state->algo->move_cursor(state->db, state->cursor, key);
      }
    } else if (ISCMD("key") || ISCMD("k")) {
      if (!state->algo->key) {
        return 2; // Unimplemented command.
      }
      long long key = state->algo->key(state->db, state->cursor);
      
      fprintf(output, "%lld\n", key);
    } else if (ISCMD("exists") || ISCMD("e")) {
      if (!state->algo->exists) {
        return 2; // Unimplemented command.
      }
      bool exists = state->algo->exists(state->db, state->cursor);
      
      if (exists) {
        fprintf(output, "yes\n");
      } else {
        fprintf(output, "no\n");
      }
    } else if (ISCMD("read") || ISCMD("r")) {
      if (!state->algo->read) {
        return 2; // Unimplemented command.
      }
      char *stored = state->algo->read(state->db, state->cursor);
      if (stored == NULL) {
        printf("(nil)\n");
      } else {
        fprintf(output, "%s", stored);
      }
    } else if (ISCMD("write") || ISCMD("w")) {
      if (!state->algo->write) {
        return 2; // Unimplemented command.
      }
      unsigned long args_len = strlen(args);
      
      char* copy = malloc(args_len + 1);
      strncpy(copy, args, args_len + 1);
      
      args = &args[args_len];
      
      char *old_stored = state->algo->write(state->db, state->cursor, copy);
      free(old_stored);
      //fprintf(output, "Stored %lu bytes\n", args_len);
    } else if (ISCMD("delete") || ISCMD("d")) {
      if (!state->algo->remove) {
        return 2; // Unimplemented command.
      }
      char *old_stored = state->algo->remove(state->db, state->cursor);
      free(old_stored);
    } else if (ISCMD("prev") || ISCMD("p") || ISCMD("<")) {
      if (!state->algo->snap) {
        return 2; // Unimplemented command.
      }
      state->algo->snap(state->db, state->cursor, KVDS_SNAP_LOWER);
    } else if (ISCMD("next") || ISCMD("n") || ISCMD(">")) {
      if (!state->algo->snap) {
        return 2; // Unimplemented command.
      }
      state->algo->snap(state->db, state->cursor, KVDS_SNAP_HIGHER);
    } else if (ISCMD("closest") || ISCMD("c") || ISCMD("=")) {
      if (!state->algo->snap) {
        return 2; // Unimplemented command.
      }
      state->algo->snap(state->db, state->cursor, KVDS_SNAP_CLOSEST_LOW);
    } else if (ISCMD("quit") || ISCMD("q")) {
      return -1; // Quit.
    } else { 
      return 1; // Invalid command.
    }
    
    command = args;
  }
  return 0;
#undef ISCMD
}
