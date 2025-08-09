#ifndef DATABASE_H
#define DATABASE_H

/*-----------DATABASE DETAILS STRUCT------------*/
typedef struct db_info {
    char* host;
    char* user;
    char* passwd;
    char* dbname;
    int port;
} DB_INFO;



#include "../includes/db_structures.h"

#define SERVER_CONFIG_FILE  "/home/pius/Dev/Projects/Cred_Manager/config/server.conf"

#define DB_CONFIG_FILE      "/home/pius/Dev/Projects/Cred_Manager/config/db.conf"


/*-------------------------OPTIONS FOR THE query_database FUNCTION-----------------*/
typedef enum {
    QUERY_ONLY,
    QUERY_AND_GET_RESULTS
}QUERY_OPTIONS;



DB_INFO* get_dbinfo();
int query_database(char* query, DB_BIND_SET* bind_set, int num_of_binds, DB_RESULT_SET* results, QUERY_OPTIONS options);
void free_dbinfo(DB_INFO* db_info);
#endif
