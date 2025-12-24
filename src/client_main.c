/*
*client_main.c is the entry point for the application.
*It contains all the logic necessary to deal with 
*user input and launching the necessary handler depending
*on what is required by user (adding, changing, searching
*deleting and listing of account credentials).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <time.h>
#endif

#include "account.h"
#include "client_main.h"
#include "error_messages.h"
#include "util.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    } else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0) {
	printf("%s", GENERAL_MESSAGE);
	return 0;
    } else if (argc == 3 && (strcmp(argv[2], "help") == 0)) {
	return print_help(argv[1]);
    }

    sqlite3 *db_con = open_db_con();
    if (!db_con) {
	return GENERAL_ERROR;
    }

    int status = handle_input(argc, argv, db_con);

    sqlite3_close(db_con);

    return status;
}

/*--- Dispatch function to handle adding new accounts-----*/
int add_acc(char **argv, int argc, sqlite3 *db) {
    if (argc != 4) {
	printf("%s", ADD_MESSAGE);
	return GENERAL_ERROR;
    }

    int status;
    if (strcmp(argv[2], "--batch") == 0) {
	status = add_acc_via_batch(db, argv[3]);
	return status;
    }
    char *account_name = argv[2];

    if (check_account_exists(db, account_name) == DB_ROW_EXISTS) {
	printf("Credentials For %s already exists\n", account_name);
	return GENERAL_ERROR;
    }

    char user_name[CRED_BUFF_LEN];
    char pass[CRED_BUFF_LEN];

    if (strcmp("no-auto", argv[3]) == 0) {
	status = get_user_input(pass, CRED_BUFF_LEN, "Enter Password", 1, 1);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (strcmp("auto", argv[3]) == 0) {
	status = get_pass_string(pass, PASSWORD_LENGTH);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	pass[PASSWORD_LENGTH] = '\0';
    } else {
	printf("Unknown option: %s\n", argv[3]);
	return GENERAL_ERROR;
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

int add_acc_via_batch(sqlite3 *db, char *batch_file_name) {
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
int change_details(char **argv, int argc, sqlite3 *db) {
    if (argc < 3) {
	printf("%s", CHANGE_MESSAGE);
	return GENERAL_ERROR;
    }

    int status;
    if (argc < 4) {
	if ((strcmp(argv[2], "master") == 0)) {
	    status = change_db_master_password(db);
	    if (status != SUCCESS_OP)
		return GENERAL_ERROR;
	    else {
		printf("Master Password Changed Successfully\n");
		return SUCCESS_OP;
	    }
	} else {
	    printf("%s", CHANGE_MESSAGE);
	    return GENERAL_ERROR;
	}
    }

    char *account = argv[2];
    char *option = argv[3];
    char new_value[CRED_BUFF_LEN];

    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account %s doesn't exist\n", account);
	return GENERAL_ERROR;
    }

    if (strcmp(option, "user") == 0) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New User Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_db_field(db, DB_USER_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (strcmp(option, "name") == 0) {
	status = get_user_input(new_value, CRED_BUFF_LEN, "Enter New Account Name", 0, 0);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	status = update_db_field(db, DB_ACC_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
    } else if (strcmp(option, "pass") == 0) {
	if (argc != 5) {
	    printf("%s", CHANGE_MESSAGE);
	    return GENERAL_ERROR;
	}
	char pass[CRED_BUFF_LEN];
	if (strcmp(argv[4], "auto") == 0) {
	    status = get_pass_string(pass, PASSWORD_LENGTH);
	    if (status != SUCCESS_OP)
		return GENERAL_ERROR;
	    pass[PASSWORD_LENGTH] = '\0';
	} else if (strcmp(argv[4], "no-auto") == 0) {
	    status = get_user_input(pass, CRED_BUFF_LEN, "Enter New Password", 1, 1);
	    if (status != SUCCESS_OP)
		return GENERAL_ERROR;
	} else {
	    printf("Unknown option: %s\n", argv[4]);
	    return GENERAL_ERROR;
	}
	status = update_db_field(db, DB_ACC_PASSWORD, account, pass);
	if (status != SUCCESS_OP)
	    return GENERAL_ERROR;
	printf("New Password: %s\n", pass);
    } else {
	printf("Unknown option: %s\n", option);
	printf("%s", CHANGE_MESSAGE);
	return GENERAL_ERROR;
    }

    if (status == SUCCESS_OP)
	printf("Changed Successfully\n");
    return SUCCESS_OP;
}

/*--- Dispatch function to handle deleting of accounts-----*/
int delete_account(char **argv, int argc, sqlite3 *db) {
    if (argc != 3) {
	printf("%s", DELETE_MESSAGE);
	return GENERAL_ERROR;
    }

    char *account = argv[2];
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
int get_details(char **argv, int argc, sqlite3 *db) {
    if (argc != 3) {
	printf("%s", GET_MESSAGE);
	return GENERAL_ERROR;
    }

    char *account = argv[2];
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
    printf("\n");
    printf("Account Name: %s\n", acc.name);
    printf("User Name: %s\n", acc.username);
    printf("Password: %s\n", acc.password);

    free(acc.username);
    free(acc.name);
    free(acc.password);
    return SUCCESS_OP;
}

/*--- Dispatch function to list the details of all accounts-----*/
int list_accounts(char **argv, int argc, sqlite3 *db) {
    if (argc != 2) {
	printf("%s", LS_MESSAGE);
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

int handle_input(int argc, char *argv[], sqlite3 *db) {
    if (argc < 2 || argc > 5) {
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }

    int status = 0;

    char *command = argv[1];

    /*------------Dispatch table for subcommands------------*/
    struct sub_command dispatch[] = {
	{"ls", list_accounts},
	{"add", add_acc},
	{"get", get_details},
	{"change", change_details},
	{"delete", delete_account},
	{NULL, NULL}};

    int dispatch_size = sizeof(dispatch) / sizeof(dispatch[0]);

    for (int i = 0; i < dispatch_size; i++) {
	if (dispatch[i].name == NULL) { /*If the end of the dispatch array is reached and no matches*/
	    printf("Unknown Command: %s\n", command);
	    printf("%s", GENERAL_MESSAGE);
	    return GENERAL_ERROR;
	} else if (strcmp(command, dispatch[i].name) == 0) {
	    status = dispatch[i].command_handler(argv, argc, db);
	    break;
	}
    }

    return status;
}

int get_pass_string(char *pass_buff, int buff_size) {
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		     "abcdefghijklmnopqrstuvwxyz"
		     "1234567890"
		     "!@#$%&^*()";
    int charset_size = sizeof(charset) - 1; // minus to not consider null terminator.

    if (!pass_buff) {
	printf("Buff is NULL\n");
	return GENERAL_ERROR;
    }

    memset(pass_buff, 0, buff_size);

#ifdef _WIN32
    srand(time(NULL));
    int random_bytes[PASSWORD_LENGTH];
    srand(time(NULL));
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	random_bytes[i] = rand() % charset_size;
    }

    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	pass_buff[i] = charset[random_bytes[i]];
    }
    return SUCCESS_OP;
#else
    FILE *urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
	perror("Error opening /dev/urandom");
	return GENERAL_ERROR;
    }

    unsigned char random[PASSWORD_LENGTH];
    int read_numbers = fread(random, sizeof(char), PASSWORD_LENGTH, urandom);

    if (read_numbers < PASSWORD_LENGTH) {
	printf("Error reading from /dev/urandom\n");
	fclose(urandom);
	return GENERAL_ERROR;
    }

    int pass_index;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	pass_index = random[i] % charset_size;
	pass_buff[i] = charset[pass_index];
    }

    fclose(urandom);
    return SUCCESS_OP;
#endif
}
