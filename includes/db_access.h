#ifndef DB_ACCESS_H
#define DB_ACCESS_H

#include <sqlcipher/sqlite3.h>

#include "../includes/account.h"

#ifndef _WIN32
#define DB_FILE "/home/pius/.creds.db"
#define DB_BACKUP "/home/pius/.creds_backup.db"
#endif

#define SUCCESS_OP 0
#define GENERAL_ERROR 10
#define NON_AFFECTED_ERROR 11
#define SQLITE_RELATED_ERROR 12
#define DB_ROW_EXISTS 13
#define DB_ROW_NX 14

sqlite3* open_db_con();
int create_new_database();
int get_all_credentials(sqlite3 *db, struct account_list *acc_list);
int get_account_by_name(sqlite3* db, char* acc_name, struct account *acc);
int update_db_field(sqlite3 *db, enum db_fields toUpdate, char *acc_name, char *new_field_value);
int delete_account_from_db(sqlite3* db, char* acc_name);
int add_account_to_db(sqlite3 *db, Account acc);
int decrypt_db(sqlite3* db);
int change_db_master_password(sqlite3 *db);
int check_account_exists(sqlite3* db, char* acc_name);

#endif
