#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparser.h"
#include "client_main.h"
#include "commands.h"
#include "util.h"

/*--- Dispatch function to handle adding new accounts-----*/
int add_acc(struct AddArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }
    int status;
    char *account_name = args->secretName;

    if (check_account_exists(db, account_name) == DB_ROW_EXISTS) {
	printf("Credentials For %s already exists\n", account_name);
	return GENERAL_ERROR;
    }

    char user_name[CRED_BUFF_LEN];
    char pass[CRED_BUFF_LEN];

    if (args->flags & ADD_FLAG_NOAUTO) {
	status = get_user_input(pass, CRED_BUFF_LEN, "Enter Password", 1, 1);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else {
	status = get_pass_string(pass, PASSWORD_LENGTH);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	pass[PASSWORD_LENGTH] = '\0';
    }

    status = get_user_input(user_name, CRED_BUFF_LEN, "Enter User Name", 0, 0);
    if (status != SUCCESS_OP)
	return GENERAL_ERROR;

    struct account acc;
    acc.name = account_name;
    acc.username = user_name;
    acc.password = pass;

    status = add_account_to_db(db, &acc);
    if (status == SUCCESS_OP) {
	printf("Account %s added successfully with password: %s\n", acc.name, acc.password);
    } else {
	printf("Could not add Account\n");
    }
    return status;
}

int add_apikey(struct AddArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }
    int status;
    char *api_name = args->secretName;

    if (check_apikey_exists(db, api_name) == DB_ROW_EXISTS) {
	printf("Api Key %s already exists\n", api_name);
	return GENERAL_ERROR;
    }

    char user_name[CRED_BUFF_LEN];
    char service[CRED_BUFF_LEN];
    char api_key[CRED_BUFF_LEN];

    status = get_user_input(user_name, CRED_BUFF_LEN, "Enter the user name for the API Key", 0, 0);
    if (status != SUCCESS_OP)
	return GENERAL_ERROR;
    status = get_user_input(service, CRED_BUFF_LEN, "Enter the service the API Key is for", 0, 0);
    if (status != SUCCESS_OP)
	return GENERAL_ERROR;
    status = get_user_input(api_key, CRED_BUFF_LEN, "Enter the API Key", 0, 0);
    if (status != SUCCESS_OP)
	return GENERAL_ERROR;

    struct api_key key;
    key.name = api_name;
    key.username = user_name;
    key.service = service;
    key.key = api_key;

    status = add_apikey_to_db(db, &key);
    if (status == SUCCESS_OP) {
	printf("API KEY added successfully.\n");
    } else {
	printf("Could not add API Key.\n");
    }
    return status;
}

int add_acc_via_batch(char *batch_file_name, sqlite3 *db) {
    if (!batch_file_name) {
	return GENERAL_ERROR;
    }

    Account_list a_list = createAccList();
    int status;
    int any_erros = 0;

    status = get_creds_from_batch_file(a_list, batch_file_name);
    if (status != SUCCESS_OP) {
	destroyAccList(a_list);
	return status;
    }

    for (Acc_node n = a_list->head; n != NULL; n = n->next) {
	if (check_account_exists(db, n->name) == DB_ROW_EXISTS) {
	    printf("Credentials For %s already exists\n", n->name);
	    any_erros = 1;
	    continue;
	}

	status = add_account_to_db(db, (Account)n);
	if (status != SUCCESS_OP) {
	    any_erros = 1;
	    continue;
	}
    }

    if (any_erros == 0) {
	printf("Succes..Use cman ls to confirm\n");
    } else if (any_erros == 1) {
	printf("Got one or more errors from batch file. Use cman ls to see which were added successfully\n");
    }
    destroyAccList(a_list);
    return SUCCESS_OP;
}

/*--- Dispatch function to handle changing account details-----*/
int change_details(struct ChangeArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    int status;
    if (args->flags & CHANGE_FLAG_MASTER) {
	status = change_db_master_password(db);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	else {
	    printf("Master Password Changed Successfully\n");
	    return SUCCESS_OP;
	}
    }

    char *account = args->secretName;
    char new_value[CRED_BUFF_LEN];

    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account %s doesn't exist\n", account);
	return GENERAL_ERROR;
    }

    if (args->flags & CHANGE_FLAG_FIELD_USERNAME) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New User Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_acc_db_field(db, DB_USER_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (args->flags & CHANGE_FLAG_FIELD_ACCNAME) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New Account Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_acc_db_field(db, DB_ACC_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (args->flags & CHANGE_FLAG_FIELD_PASS) {
	char pass[CRED_BUFF_LEN];
	char choice[5];
	char confirmation[128];
	snprintf(confirmation, sizeof(confirmation), "Are you sure you want to change the password for account %s (yes/no)?", account);

	status = get_user_input(choice, 5, confirmation, 0, 0);
	if (status != SUCCESS_OP) {
	    return GENERAL_ERROR;
	}
	if (strcmp(choice, "yes") != 0) {
	    printf("Password not changed\n");
	    return GENERAL_ERROR;
	}
	if (args->flags & CHANGE_FLAG_NOAUTO) {
	    status = get_user_input(pass, CRED_BUFF_LEN, "Enter New Password", 1, 1);
	    if (status != SUCCESS_OP)
		return GENERAL_ERROR;
	} else {
	    status = get_pass_string(pass, PASSWORD_LENGTH);
	    if (status != SUCCESS_OP)
		return GENERAL_ERROR;
	    pass[PASSWORD_LENGTH] = '\0';
	}

	status = update_acc_db_field(db, DB_ACC_PASSWORD, account, pass);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	printf("New Password: %s\n", pass);
    } else {
	printf("No field to change specified. Use cman change -h for more information\n");
	return GENERAL_ERROR;
    }

    printf("Changed Successfully\n");
    return SUCCESS_OP;
}

int change_apikey_details(struct ChangeArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    int status;

    char *api_name = args->secretName;
    char new_value[CRED_BUFF_LEN];

    if (check_apikey_exists(db, api_name) == DB_ROW_NX) {
	printf("API Key %s doesn't exist\n", api_name);
	return GENERAL_ERROR;
    }

    if (args->flags & CHANGE_FLAG_FIELD_USERNAME) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New User Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_api_db_field(db, DB_API_USERNAME, api_name, new_value);
    } else if (args->flags & CHANGE_FLAG_FIELD_APIKEY) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New API Key", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_api_db_field(db, DB_API_KEY, api_name, new_value);
    } else if (args->flags & CHANGE_FLAG_FIELD_APISERVICE) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New API Service", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_api_db_field(db, DB_API_SERVICE, api_name, new_value);
    } else if (args->flags & CHANGE_FLAG_FIELD_APINAME) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New API Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_api_db_field(db, DB_API_NAME, api_name, new_value);
    } else {
	printf("No valid field to change specified. Use cman change -h for more information\n");
	return GENERAL_ERROR;
    }

    if (status != SUCCESS_OP)
	return GENERAL_ERROR;
    printf("Changed Successfully\n");
    return SUCCESS_OP;
}

int delete_account(struct DeleteArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    char *account = args->secretName;

    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account %s doesn't exist\n", account);
	return GENERAL_ERROR;
    }

    int status;
    char choice[5];
    char confirmation[64];
    sprintf(confirmation, "Are you sure you want to delete account %s (yes/no)?", account);

    status = get_user_input(choice, 5, confirmation, 0, 0);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }
    if (strcmp(choice, "yes") != 0) {
	printf("Account not deleted\n");
	return GENERAL_ERROR;
    }
    status = delete_account_from_db(db, account);
    if (status == SUCCESS_OP) {
	printf("Account Deleted\n");
	return SUCCESS_OP;
    } else {
	printf("Error Deleting Account\n");
	return GENERAL_ERROR;
    }
}

int delete_apikey(struct DeleteArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    char *api_name = args->secretName;

    if (check_apikey_exists(db, api_name) == DB_ROW_NX) {
	printf("API Key %s does not exist\n", api_name);
	return GENERAL_ERROR;
    }

    int status;
    char choice[5];
    char confirmation[64];
    sprintf(confirmation, "Are you sure you want to delete API Key %s (yes/no)?", api_name);

    status = get_user_input(choice, 5, confirmation, 0, 0);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }
    if (strcmp(choice, "yes") != 0) {
	printf("Account not deleted\n");
	return GENERAL_ERROR;
    }
    status = delete_apikey_from_db(db, api_name);
    if (status == SUCCESS_OP) {
	printf("API Key Deleted\n");
	return SUCCESS_OP;
    } else {
	printf("Error Deleting API Key\n");
	return GENERAL_ERROR;
    }
}

int get_details(struct GetArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    char *account = args->secretName;
    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account Doesn't Exist\n");
	return GENERAL_ERROR;
    }

    struct account acc;
    memset(&acc, '\0', sizeof(struct account));

    int status = get_account_by_name(db, account, &acc);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }
    if (args->flags & GET_FLAG_FIELD_USERNAME)
	print_result("User Name:       ", acc.username);
    else if (args->flags & GET_FLAG_FIELD_PASS)
	print_result("Password:        ", acc.password);
    else {
	print_result("Account:         ", acc.username);
	print_result("User Name:       ", acc.username);
	print_result("Password:        ", acc.password);
    }

    free(acc.username);
    free(acc.name);
    free(acc.password);
    return SUCCESS_OP;
}

int get_apikey_details(struct GetArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    char *api_name = args->secretName;
    if (check_apikey_exists(db, api_name) == DB_ROW_NX) {
	printf("API Key %s Doesn't Exist\n", api_name);
	return GENERAL_ERROR;
    }

    struct api_key key;
    int status = get_apikey_by_name(db, api_name, &key);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }
    if (args->flags & GET_FLAG_FIELD_USERNAME)
	print_result("User Name:       ", key.username);
    else if (args->flags & GET_FLAG_FIELD_APISERVICE)
	print_result("Service:         ", key.service);
    else if (args->flags & GET_FLAG_FIELD_APIKEY)
	print_result("Key:             ", key.key);
    else {
	print_result("User Name:       ", key.username);
	print_result("Service:         ", key.service);
	print_result("Key:             ", key.key);
    }

    free(key.username);
    free(key.name);
    free(key.key);
    free(key.service);
    return SUCCESS_OP;
}

int list_accounts(struct ListArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    Account_list a_lst = createAccList();

    int status = get_all_credentials(db, a_lst);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }

    for (Acc_node n = a_lst->head; n != NULL; n = n->next) {
	printf("Acc:        %s\n", n->name);
	printf("Username:   %s\n", n->username);
	printf("Pass:       %s\n", n->password);
	printf("\n");
    }
    destroyAccList(a_lst);
    return SUCCESS_OP;
}

int list_api_keys(struct ListArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    Api_list api_lst = createApiList();

    int status = get_all_apikeys(db, api_lst);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }

    for (Api_node n = api_lst->head; n != NULL; n = n->next) {
	printf("API Name:        %s\n", n->name);
	printf("Username:   	 %s\n", n->username);
	printf("Service:         %s\n", n->service);
	printf("Key:             %s\n", n->key);
	printf("\n");
    }
    destroyApiList(api_lst);
    return SUCCESS_OP;
}

int runAdd(void *arguments, sqlite3 *db) {
    struct AddArgs *args = (struct AddArgs *)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return add_apikey(args, db);
    }
    if (args->flags & ADD_FLAG_BATCHFILE) {
	return add_acc_via_batch(args->secretName, db);
    }

    return add_acc(args, db);
}

int runChange(void *arguments, sqlite3 *db) {
    struct ChangeArgs *args = (struct ChangeArgs*)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return change_apikey_details(args, db);
    }
    return change_details(args, db);
}

int runGet(void *arguments, sqlite3 *db) {
    struct GetArgs *args = (struct GetArgs*)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return get_apikey_details(args, db);
    }
    return get_details(args, db);
}

int runList(void *arguments, sqlite3 *db) {
    struct ListArgs *args = (struct ListArgs*)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return list_api_keys(args, db);
    }
    return list_accounts(args, db);
}

int runDelete(void *arguments, sqlite3 *db) {
    struct DeleteArgs *args = (struct DeleteArgs*)arguments;
    if (args->flags & ADD_FLAG_TYPE_APIKEY) {
	return delete_apikey(args, db);
    }
    return delete_account(args, db);
}
