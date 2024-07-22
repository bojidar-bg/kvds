#include "registry.h"
#include "stdlib.h"
#include "string.h"

static struct kvds_registry_entry *registry_entries;

void kvds_register_algo_entry(struct kvds_registry_entry *entry) {
  entry->next = registry_entries;
  registry_entries = entry;
}

struct kvds_database_algo *kvds_get_algo(const char *name) {
  for (struct kvds_registry_entry *entry = registry_entries; entry; entry = entry->next) {
    if (strcmp(name, entry->name) == 0) {
      return entry->algo;
    }
  }
  return NULL;
}

struct kvds_registry_entry *kvds_get_algos_list() {
  return registry_entries;
}
