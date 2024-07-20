#include "registry.h"
#include "stdlib.h"
#include "string.h"

static struct kvds_registry_entry *registry_entries;

void kvds_register_algo_entry(struct kvds_registry_entry *entry) {
  entry->next = registry_entries;
  registry_entries = entry;
}

struct kvds_database_algo *kvds_get_algo(char *name) {
  for (struct kvds_registry_entry *entry = registry_entries; entry; entry = entry->next) {
    if(strcmp(name, entry->name) == 0) {
      return entry->algo;
    }
  }
  return NULL;
}

int kvds_list_algos(const char **result, int result_n) {
  int i = 0;
  for (struct kvds_registry_entry *entry = registry_entries; entry; entry = entry->next) {
    if (i < result_n) {
      result[i++] = entry->name;
    }
  }
  if (i < result_n) {
    result[i++] = 0;
  }
  return i;
}
