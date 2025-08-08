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




#define SERVER_CONFIG_FILE  "/home/pius/Dev/Projects/Cred_Manager/config/server.conf"

#define DB_CONFIG_FILE      "/home/pius/Dev/Projects/Cred_Manager/config/db.conf"



DB_INFO* get_dbinfo();
void free_dbinfo(DB_INFO* db_info);
void get_results();
#endif
