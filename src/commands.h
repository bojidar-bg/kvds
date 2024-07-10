#pragma once
#include "interface.h"
#include <stdio.h>

struct kvds_command_state* kvds_create_command_state(struct kvds_database_algo *algo, void *db);
void kvds_destroy_command_state(struct kvds_command_state *state);
int kvds_execute_command(struct kvds_command_state *state, char *command, FILE *output);

char* kvds_get_error(int error);
