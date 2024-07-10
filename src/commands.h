#include "interface.h"
#pragma once

struct kvds_command_state* kvds_create_command_state(struct kvds_database_algo *algo, void *db);
void kvds_destroy_command_state(struct kvds_command_state *state);
int kvds_execute_command(struct kvds_command_state *state, char *command, char **output, int output_n);

char* kvds_get_error(int error);
