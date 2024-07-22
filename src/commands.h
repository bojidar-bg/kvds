#pragma once
#include "interface.h"
#include <stdio.h>

typedef int kvds_error;
static const kvds_error KVDS_QUIT = -1;
static const kvds_error KVDS_OK = 0;

char* kvds_describe_error(kvds_error error);

struct kvds_command_state* kvds_create_command_state(struct kvds_database_algo *algo, void *db);
void kvds_destroy_command_state(struct kvds_command_state *state);
kvds_error kvds_execute_command(struct kvds_command_state *state, char *command, FILE *output);
