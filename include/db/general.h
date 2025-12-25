#ifndef GENERAL_H
#define GENERAL_H

#include <sqlcipher/sqlite3.h>

sqlite3 *open_db_con();

int create_new_database();

int change_db_master_password(sqlite3 *db);

int decrypt_db(sqlite3 *db);
#endif
