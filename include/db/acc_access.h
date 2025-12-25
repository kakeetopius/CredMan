#ifndef ACC_ACCESS_H
#define ACC_ACCESS_H

#include "db/general.h"
#include "objects/acc_obj.h"

enum account_db_fields {
    DB_ACC_NAME,
    DB_USER_NAME,
    DB_ACC_PASSWORD,
};

int get_all_db_accounts(sqlite3 *db, struct account_list *acc_list);

int get_db_account_by_name(sqlite3 *db, char *acc_name, struct account *acc);

int update_acc_db_field(sqlite3 *db, enum account_db_fields toUpdate, char *acc_name, char *new_field_value);

int delete_account_from_db(sqlite3 *db, char *acc_name);

int add_account_to_db(sqlite3 *db, Account acc);

int check_account_exists(sqlite3 *db, char *acc_name);
#endif
