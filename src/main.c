#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "commands.h"
#include "interface.h"
#include "registry.h"

void print_usage(char** argv) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s [algorithm]\n\n", argv[0]);
  fprintf(stderr, "Available algorithms:");
  struct kvds_registry_entry *last_entry = NULL;
  for (struct kvds_registry_entry *entry = kvds_get_algos_list(); entry != NULL; entry = entry->next) {
    if (last_entry != NULL && entry->algo == last_entry->algo) { // List multiple name of an algorithm on the same line
      fprintf(stderr, ", %s", entry->name);
    } else {
      if (last_entry != NULL) fprintf(stderr, " - %s", last_entry->description);
      fprintf(stderr, "\n  %s", entry->name);
    }
    last_entry = entry;
  }
    if (last_entry != NULL) fprintf(stderr, " - %s", last_entry->description);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
#ifndef NDEBUG
  char* algo_name = "default";
#else
  char* algo_name = "scapegoat";
#endif
  if (argc == 2) {
    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_usage(argv);
      return 0;
    }
    
    algo_name = argv[1];
  }
  if (argc > 2) {
    fprintf(stderr, "Error: Too many arguments.\n");
    print_usage(argv);
    return 2;
  }
  
  struct kvds_database_algo *algo = kvds_get_algo(algo_name);
  
  if (algo == NULL) {
    fprintf(stderr, "Error: No such algorithm: %s\n", algo_name);
    print_usage(argv);
    return 2;
  }
  
  bool interactive = isatty(fileno(stdin));
  
  kvds_db *db = algo->create_db();
  
  if (db == NULL) {
      fprintf(stderr, "Error: Failed to create database");
  }
  
  if (interactive) {
    fprintf(stderr, "Created a database with algorithm: %s\n", algo_name);
    fprintf(stderr, "Use \"help\" for a list of commands.\n");
  }
  
  struct kvds_command_state *state = kvds_create_command_state(algo, db);
  
  int exit_code = 0;
  
  char line[1024];
  while(true) {
    if (interactive) {
      fflush(stdout);
      fprintf(stderr, "> ");
    }
    
    if (fgets(line, sizeof line / sizeof line[0], stdin)) {
      int err = kvds_execute_command(state, line, stdout);
      if (err != KVDS_OK) {
        fprintf(stderr, "Error: %s\n", kvds_describe_error(err));
        if (err == KVDS_QUIT) {
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
