#include "commands/acc_mgmt.h"
#include "commands/api_mgmt.h"

int runAdd(void *arguments, sqlite3 *db) {
    struct AddArgs *args = (struct AddArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return add_apikey(args, db);
    }
    if (args->flags & ADD_FLAG_BATCHFILE) {
	return add_acc_via_batch(args->secretName, db);
    }

    return add_account(args, db);
}

int runChange(void *arguments, sqlite3 *db) {
    struct ChangeArgs *args = (struct ChangeArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return change_apikey_details(args, db);
    }
    return change_account_details(args, db);
}

int runGet(void *arguments, sqlite3 *db) {
    struct GetArgs *args = (struct GetArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return get_apikey_details(args, db);
    }
    return get_account_details(args, db);
}

int runList(void *arguments, sqlite3 *db) {
    struct ListArgs *args = (struct ListArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return list_api_keys(args, db);
    }
    return list_accounts(args, db);
}

int runDelete(void *arguments, sqlite3 *db) {
    struct DeleteArgs *args = (struct DeleteArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return delete_apikey(args, db);
    }
    return delete_account(args, db);
}
