#include "commands.h"
#include "util.h"
#include <stdio.h>

int runAdd(void *arguments, sqlite3 *db) {

    printf("Running add..\n");
    return SUCCESS_OP;
}

int runChange(void *argument, sqlite3 *dbs) {

    printf("Running change..\n");
    return SUCCESS_OP;
}

int runGet(void *argument, sqlite3 *dbs) {
    printf("Running get...\n");
    return SUCCESS_OP;
}

int runList(void *argument, sqlite3 *dbs) {
    printf("Running ls.....\n");
    return SUCCESS_OP;
}

int runDelete(void *argument, sqlite3 *dbs) {
    printf("running delete....\n");
    return SUCCESS_OP;
}
