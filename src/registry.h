#pragma once
#include "interface.h"

#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)

#define REGISTER(name, shortname) \
  static struct kvds_database_algo CONCAT(_register_alg, __LINE__); \
  static struct kvds_registry_entry CONCAT(_register_alg_l, __LINE__); \
  static struct kvds_registry_entry CONCAT(_register_alg_s, __LINE__); \
  __attribute__((constructor)) \
  static void CONCAT(_register, __LINE__)() { \
    kvds_register_algo_entry(&CONCAT(_register_alg_l, __LINE__)); \
    kvds_register_algo_entry(&CONCAT(_register_alg_s, __LINE__)); \
  } \
  static struct kvds_registry_entry CONCAT(_register_alg_l, __LINE__) = {name, &CONCAT(_register_alg, __LINE__)}; \
  static struct kvds_registry_entry CONCAT(_register_alg_s, __LINE__) = {shortname, &CONCAT(_register_alg, __LINE__)}; \
  static struct kvds_database_algo CONCAT(_register_alg, __LINE__) =

struct kvds_registry_entry {
  const char *name;
  struct kvds_database_algo *algo;
  struct kvds_registry_entry *next;
};

void kvds_register_algo_entry(struct kvds_registry_entry *entry);
struct kvds_database_algo* kvds_get_algo(char *name);
