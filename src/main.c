#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "commands.h"
#include "interface.h"
#include "registry.h"

int main(int argc, char **argv) {
  char* algo_name = "binary_search_tree";
  if (argc > 1) {
    algo_name = argv[argc - 1]; // TODO: Flag parsing
  }
  
  struct kvds_database_algo *algo = kvds_get_algo(algo_name);
  
  if (algo == NULL) {
      fprintf(stderr, "Error: No such algorithm: %s", algo_name);
  }
  
  kvds_db *db = algo->create_db();
  
  if (db == NULL) {
      fprintf(stderr, "Error: Failed to create database");
  }
  
  struct kvds_command_state *state = kvds_create_command_state(algo, db);
  
  bool interactive = isatty(fileno(stdin));
  
  int exit_code = 0;
  
  char line[1000];
  char output_buf[1000];
  while(true) {
    if (interactive) {
      fflush(stdout);
      fprintf(stderr, "> ");
    }
    
    if (fgets(line, sizeof line / sizeof line[0], stdin)) {
      int err = kvds_execute_command(state, line, stdout);
      if (err) {
        fprintf(stderr, "Error: %s\n", kvds_get_error(err));
        if (err == -1) {
          break;
        }
        exit_code = 1;
      } else {
        exit_code = 0;
      }
    }
    if (ferror(stdin)) {
      fprintf(stderr, "Read error: %d", ferror(stdin));
      exit_code = 2;
      break;
    }
    if (feof(stdin)) {
      break;
    }
  }
  
  kvds_destroy_command_state(state);
  algo->destroy_db(db, (void*)&free);
  
  
  
  return exit_code;
}
