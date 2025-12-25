#ifndef RUNNERS_H
#define RUNNERS_H

#include <sqlcipher/sqlite3.h>

typedef int (*Runner)(void *, sqlite3 *);

int runAdd(void *arguments, sqlite3 *db);
int runChange(void *arguments, sqlite3 *db);
int runGet(void *arguments, sqlite3 *db);
int runDelete(void *arguments, sqlite3 *db);
int runList(void *arguments, sqlite3 *db);

#endif
