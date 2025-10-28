#ifndef DB_ACCESS_H
#define DB_ACCESS_H

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <sqlcipher/sqlite3.h>

#include "../includes/account.h"

#define DB_FILE "util/creds.db"
#define DB_BACKUP "util/creds_backup.db"



int get_password(char *buff, int buff_len, const char *prompt, int confirm);
sqlite3* open_db_con();
int create_new_database();
void flush_stdin();
void remove_secure_input(struct termios *oldt);
void set_secure_input(struct termios *oldt);
int get_all_credentials(sqlite3 *db, struct account_list *acc_list);
int get_account_by_name(sqlite3* db, char* acc_name, struct account *acc);
int update_db_field(sqlite3 *db, enum db_fields toUpdate, char *acc_name, char *new_field_value);
int delete_account_from_db(sqlite3* db, char* acc_name);
#endif
