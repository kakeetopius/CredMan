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
	status = update_db_field(db, DB_USER_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (args->flags & CHANGE_FLAG_FIELD_ACCNAME) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New Account Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_db_field(db, DB_ACC_NAME, account, new_value);
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

	status = update_db_field(db, DB_ACC_PASSWORD, account, pass);
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

/*--- Dispatch function to handle deleting of accounts-----*/
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

/*--- Dispatch function to handle searching of account details-----*/
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
    print_result("Account Name:    ", acc.name);
    print_result("User Name:       ", acc.username);
    print_result("Password:        ", acc.password);

    free(acc.username);
    free(acc.name);
    free(acc.password);
    return SUCCESS_OP;
}

/*--- Dispatch function to list the details of all accounts-----*/
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
int runAdd(void *arguments, sqlite3 *db) {
    struct AddArgs *args = (struct AddArgs *)arguments;
    if (args->flags & ADD_FLAG_BATCHFILE) {
	return add_acc_via_batch(args->secretName, db);
    }

    return add_acc(args, db);
}

int runChange(void *arguments, sqlite3 *db) {
    return change_details((struct ChangeArgs *)arguments, db);
}

int runGet(void *arguments, sqlite3 *db) {
    return get_details((struct GetArgs *)arguments, db);
}

int runList(void *arguments, sqlite3 *db) {
    return list_accounts((struct ListArgs *)arguments, db);
}

int runDelete(void *arguments, sqlite3 *db) {
    return delete_account((struct DeleteArgs *)arguments, db);
}
