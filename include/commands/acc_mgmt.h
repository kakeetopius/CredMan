#ifndef ACC_MGMT_H
#define ACC_MGMT_H

#include "db/general.h"
#include "util/argparser.h"

int add_account(struct AddArgs *args, sqlite3 *db);

int add_acc_via_batch(char *batch_file_name, sqlite3 *db);

int change_account_details(struct ChangeArgs *args, sqlite3 *db);

int delete_account(struct DeleteArgs *args, sqlite3 *db);

int get_account_details(struct GetArgs *args, sqlite3 *db);

int list_accounts(struct ListArgs *args, sqlite3 *db);
#endif
