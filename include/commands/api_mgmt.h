#ifndef API_MGMT_H
#define API_MGMT_H

#include "db/general.h"
#include "util/argparser.h"

int add_apikey(struct AddArgs *args, sqlite3 *db);

int change_apikey_details(struct ChangeArgs *args, sqlite3 *db);

int delete_apikey(struct DeleteArgs *args, sqlite3 *db);

int get_apikey_details(struct GetArgs *args, sqlite3 *db);

int list_api_keys(struct ListArgs *args, sqlite3 *db);
#endif
