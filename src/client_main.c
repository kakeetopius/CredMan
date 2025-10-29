#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../includes/account.h"
#include "../includes/client_main.h"
#include "../includes/error_messages.h"
#include "../includes/util.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
	printf("%s", GENERAL_MESSAGE);
	return -1;
    } else if (strcmp(argv[1], "help") == 0) {
	printf("%s", GENERAL_MESSAGE);
	return 0;
    } else if (argc == 3 && (strcmp(argv[2], "help") == 0)) {
	return print_help(argv[1]);
    }

    sqlite3 *db_con = open_db_con();
    if (!db_con) {
	return -1;
    }

    int status=  handle_input(argc, argv, db_con);

    sqlite3_close(db_con);

    return status;
}

/*--- Dispatch function to handle adding new accounts-----*/
int add_acc(char **argv, int argc, sqlite3 *db) {
    if (argc != 4) {
	printf("%s", ADD_MESSAGE);
	return -1;
    }

    int status;
    char *account_name = argv[2];

    if (check_account_exists(db, account_name) == DB_ROW_EXISTS) {
	printf("Credentials For %s already exists\n", account_name);
	return -1;
    }

    char user_name[50];
    char pass[50];

    if (strcmp("no-auto", argv[3]) == 0) {
	status = get_user_input(pass, 50, "Enter Password", 1, 1);
	if (status != SUCCESS_OP)
	    return -1;
    } else if (strcmp("auto", argv[3]) == 0) {
	get_pass_string(pass, 16);
    } else {
	printf("Unknown option: %s\n", argv[3]);
    }

    status = get_user_input(user_name, 50, "Enter User Name", 0, 0);
    if (status != SUCCESS_OP)
	return -1;

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

/*--- Dispatch function to handle changing account details-----*/
int change_details(char **argv, int argc, sqlite3 *db) {
    if (argc < 3) {
	printf("%s", CHANGE_MESSAGE);
	return -1;
    }

    int status;
    if (argc < 4) {
	if ((strcmp(argv[2], "master") == 0)) {
	    status = change_db_master_password(db);
	    if (status != SUCCESS_OP)
		return -1;
	    else  {
		printf("Master Password Changed Successfully\n");
		return SUCCESS_OP;
	    }
	} else {
	    printf("%s", CHANGE_MESSAGE);
	    return -1;
	}
    }

    char *account = argv[2];
    char *option = argv[3];
    char new_value[50];

    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account %s doesn't exist\n", account);
	return -1;
    }

    if (strcmp(option, "user") == 0) {
	status = get_user_input(new_value, 50, "Enter New User Name", 0, 0);
	if (status != SUCCESS_OP)
	    return -1;
	status = update_db_field(db, DB_USER_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return -1;
    } else if (strcmp(option, "name") == 0) {
	status = get_user_input(new_value, 50, "Enter New Account Name", 0, 0);
	if (status != SUCCESS_OP)
	    return -1;
	status = update_db_field(db, DB_ACC_NAME, account, new_value);
	if (status != SUCCESS_OP)
	    return -1;
    } else if (strcmp(option, "pass") == 0) {
	if (argc != 5) {
	    printf("%s", CHANGE_MESSAGE);
	    return -1;
	}
	char pass[50];
	if (strcmp(argv[4], "auto") == 0) {
	    get_pass_string(pass, 16);
	} else if (strcmp(argv[4], "no-auto") == 0) {
	    status = get_user_input(pass, 50, "Enter New Password", 1, 1);
	    if (status != SUCCESS_OP)
		return -1;
	} else {
	    printf("Unknown option: %s\n", argv[4]);
	    return -1;
	}
	status = update_db_field(db, DB_ACC_PASSWORD, account, pass);
	if (status != SUCCESS_OP)
	    return -1;
    } else {
	printf("Unknown option: %s\n", option);
	printf("%s", CHANGE_MESSAGE);
	return -1;
    }

    if (status == SUCCESS_OP)
	printf("Changed Successfully\n");
    return SUCCESS_OP;
}

/*--- Dispatch function to handle deleting of accounts-----*/
int delete_account(char **argv, int argc, sqlite3 *db) {
    if (argc != 3) {
	printf("%s", DELETE_MESSAGE);
	return -1;
    }

    char *account = argv[2];
    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account %s doesn't exist\n", account);
	return -1;
    }

    int status = delete_account_from_db(db, account);
    if (status == SUCCESS_OP) {
	printf("Account Deleted\n");
	return SUCCESS_OP;
    } else {
	printf("Error Deleting Account\n");
	return -1;
    }
}

/*--- Dispatch function to handle searching of account details-----*/
int get_details(char **argv, int argc, sqlite3 *db) {
    if (argc != 3) {
	printf("%s", SEARCH_MESSAGE);
	return -1;
    }

    char *account = argv[2];
    if (check_account_exists(db, account) == DB_ROW_NX) {
	printf("Account Doesn't Exist\n");
	return -1;
    }

    struct account acc;
    memset(&acc, '\0', sizeof(struct account));

    int status = get_account_by_name(db, account, &acc);
    if (status != SUCCESS_OP) {
	return -1;
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
	return -1;
    }

    Account_list a_lst = createAccList();

    int status = get_all_credentials(db, a_lst);
    if (status != SUCCESS_OP) {
	return -1;
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
	return -1;
    }

    int status = 0;

    char *command = argv[1];

    /*------------Dispatch table for subcommands------------*/
    struct sub_command dispatch[] = {
	{"ls", list_accounts},
	{"add", add_acc},
	{"search", get_details},
	{"change", change_details},
	{"delete", delete_account},
	{NULL, NULL}};

    int dispatch_size = sizeof(dispatch) / sizeof(dispatch[0]);

    for (int i = 0; i < dispatch_size; i++) {
	if (dispatch[i].name == NULL) { /*If the end of the dispatch array is reached and no matches*/
	    printf("Unknown Command: %s\n", command);
	    printf("%s", GENERAL_MESSAGE);
	    return -1;
	} else if (strcmp(command, dispatch[i].name) == 0) {
	    status = dispatch[i].command_handler(argv, argc, db);
	    break;
	}
    }

    return status;
}

void get_pass_string(char *buff, int buff_size) {
    memset(buff, 0, buff_size);
    srand(time(NULL));
    int num;
    int alpha = 0;
    int numeric = 0;
    int s_char = 0;

    for (int i = 0; i < buff_size; i++) {
	num = rand() % 88 + 35; // random number between 35 and 122
	if ((num >= 37 && num <= 47) || (num >= 58 && num <= 63) ||
	    (num >= 91 && num <= 96)) {
	    i--;
	    continue;
	}

	if (isalpha(num))
	    alpha++;
	else if (isdigit(num))
	    numeric++;

	if (alpha > 8) {
	    num = rand() % 10 + 48; // choose a number
	    numeric++;
	    if (numeric > 5) {
		s_char = rand() % 3;
		switch (s_char) {
		case 0:
		    num = 35;
		    break; // #
		case 1:
		    num = 36;
		    break; //$
		case 2:
		    num = 64;
		    break; //@
		}
	    }
	}

	buff[i] = num;
    }
    buff[buff_size - 1] = '\0';
}
