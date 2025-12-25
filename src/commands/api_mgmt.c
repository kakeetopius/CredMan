#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db/api_access.h"
#include "util/argparser.h"
#include "util/errors.h"
#include "util/input.h"
#include "util/output.h"
#include "util/passwd_gen.h"

int add_apikey(struct AddArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }
    int status;
    char *api_name = args->secretName;
    if (strlen(api_name) < 1) {
	printf("Api name can't be empty\n");
	return GENERAL_ERROR;
    }

    if (check_apikey_exists(db, api_name) == DB_ROW_EXISTS) {
	printf("Api Key %s already exists\n", api_name);
	return GENERAL_ERROR;
    }

    char user_name[CRED_BUFF_LEN];
    char service[CRED_BUFF_LEN];
    char api_key[CRED_BUFF_LEN];

    status = get_user_input(user_name, CRED_BUFF_LEN, "Enter the user name associated with the API Key", 0, 0);
    if (status != SUCCESS_OP)
	return GENERAL_ERROR;
    status = get_user_input(service, CRED_BUFF_LEN, "Enter the service the API Key is for or a brief description", 0, 0);
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
    int status = get_db_apikey_by_name(db, api_name, &key);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }
    if (args->flags & GET_FLAG_FIELD_USERNAME)
	print_result("User Name:       ", key.username);
    else if (args->flags & GET_FLAG_FIELD_APISERVICE)
	print_result("Service:         ", key.service);
    else if (args->flags & GET_FLAG_FIELD_APIKEY)
	print_result("Key:             ", key.key);
    else if (args->flags == 0) {
	print_result("Service:         ", key.service);
	print_result("User Name:       ", key.username);
	print_result("Key:             ", key.key);
    } else {
	printf("Unknown field. Use cman get -h for more information.\n");
    }

    free(key.username);
    free(key.name);
    free(key.key);
    free(key.service);
    return SUCCESS_OP;
}

int list_api_keys(struct ListArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    Api_list api_lst = createApiList();

    int status = get_all_db_apikeys(db, api_lst);
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
