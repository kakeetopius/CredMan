#ifndef COMMANDS_H
#define COMMANDS_H

#include "db_access.h"

typedef int (*Runner)(void *, sqlite3*);
typedef int (*ArgParser)(int, char **, void *); // argv, argc, struct to fill with options

struct Command {
    char *name;
    void *arguments;
    ArgParser argparser;
    Runner Run;
};

int runAdd(void *arguments, sqlite3* db);
int runChange(void *arguments, sqlite3* db);
int runGet(void *arguments, sqlite3* db);
int runDelete(void *arguments, sqlite3* db);
int runList(void *arguments, sqlite3* db);
#endif
