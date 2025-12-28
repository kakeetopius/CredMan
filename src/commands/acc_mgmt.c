#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db/acc_access.h"
#include "util/argparser.h"
#include "util/errors.h"
#include "util/input.h"
#include "util/output.h"
#include "util/passwd_gen.h"

int get_creds_from_batch_file(Account_list a_lst, char *batch_file_name);
int split_batch_line(char *batch_line, char *acc_name, char *user_name, char *password, int line_no);

int add_account(struct AddArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }
    int status;
    char *account_name = args->secretName;
    if (strlen(account_name) < 1) {
	printf("Account name can't be empty\n");
	return GENERAL_ERROR;
    }

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
	status = gen_pass_string(pass, PASSWORD_LENGTH);
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

int change_account_details(struct ChangeArgs *args, sqlite3 *db) {
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
	    status = gen_pass_string(pass, PASSWORD_LENGTH);
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

int get_account_details(struct GetArgs *args, sqlite3 *db) {
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

    int status = get_db_account_by_name(db, account, &acc);
    if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }

    int get_flags = args->flags & (GET_FLAG_FIELD_USERNAME | GET_FLAG_FIELD_PASS | GET_FLAG_FIELD_ACCNAME);

    if (get_flags & GET_FLAG_FIELD_USERNAME)
	print_result("User Name:       ", acc.username);
    else if (get_flags & GET_FLAG_FIELD_PASS)
	print_result("Password:        ", acc.password);
    else if (get_flags & GET_FLAG_FIELD_ACCNAME)
	print_result("Acc Name:         ", acc.name);
    else {
	print_result("Account:         ", acc.name);
	print_result("User Name:       ", acc.username);
	print_result("Password:        ", acc.password);
    }

    free(acc.username);
    free(acc.name);
    free(acc.password);
    return SUCCESS_OP;
}

int list_accounts(struct ListArgs *args, sqlite3 *db) {
    if (!args || !db) {
	return GENERAL_ERROR;
    }

    Account_list a_lst = createAccList();

    int status = get_all_db_accounts(db, a_lst);
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

int get_creds_from_batch_file(Account_list a_lst, char *batch_file_name) {
    FILE *batch_file = fopen(batch_file_name, "r");
    if (!batch_file) {
	perror("Error opening batch file");
	return GENERAL_ERROR;
    }
    if (!a_lst) {
	printf("Account_list object is NULL\n");
	return GENERAL_ERROR;
    }

    char batch_file_line[BATCH_FILE_LINE_LEN];
    char acc_name[CRED_BUFF_LEN];
    char user_name[CRED_BUFF_LEN];
    char password[CRED_BUFF_LEN];

    int status;
    int line_no = 1;
    while (fgets(batch_file_line, BATCH_FILE_LINE_LEN, batch_file) != NULL) {
	if (feof(batch_file))
	    break;
	else if (ferror(batch_file)) {
	    perror("Error reading batch file");
	    fclose(batch_file);
	    return GENERAL_ERROR;
	}
	status = split_batch_line(batch_file_line, acc_name, user_name, password, line_no);
	if (status == GENERAL_ERROR) {
	    fclose(batch_file);
	    return GENERAL_ERROR;
	} else if (status == LINE_EMPTY) {
	    line_no++;
	    continue;
	}
	status = insert_acc_node(a_lst, acc_name, password, user_name);
	if (status != SUCCESS_OP) {
	    fclose(batch_file);
	    return GENERAL_ERROR;
	}
	line_no++;
    }
    fclose(batch_file);
    return SUCCESS_OP;
}

int split_batch_line(char *batch_line, char *acc_name, char *user_name, char *password, int line_no) {
    if (!batch_line || !acc_name || !user_name || !password) {
	return GENERAL_ERROR;
    }

    if (batch_line[0] == '\n') {
	printf("Line %d is empty. Skipping it.\n", line_no);
	return LINE_EMPTY;
    }

    int first_comma_index = 0;
    int second_comma_index = 0;
    int batch_line_strlen = strlen(batch_line);

    for (int i = 0; i < batch_line_strlen; i++) {
	if (batch_line[i] == ',') {
	    first_comma_index = i;
	    break;
	}
    }

    for (int i = first_comma_index + 1; i < batch_line_strlen; i++) {
	if (batch_line[i] == ',') {
	    second_comma_index = i;
	    break;
	}
    }
    if (first_comma_index == 0 || second_comma_index == 0) {
	printf("Parse error on line number: %d\n", line_no);
	return GENERAL_ERROR;
    }

    /*
     * Acc_name from index 0 to first_comma_index - 1
     * user_name from first_comma_index + 1 to second_comma_index -1
     * password from second_comma_index + 1 to batch_line_strlen - 1
     */

    int acc_name_len = first_comma_index;
    int user_name_len = second_comma_index - first_comma_index - 1;
    int password_len = batch_line_strlen - second_comma_index - 1;

    if (acc_name_len >= CRED_BUFF_LEN) {
	printf("Account name on line %d is too long. Limit it to %d characters.\n", line_no, CRED_BUFF_LEN);
	return GENERAL_ERROR;
    } else if (user_name_len >= CRED_BUFF_LEN) {
	printf("User name on line %d is too long. Limit it to %d characters\n", line_no, CRED_BUFF_LEN);
	return GENERAL_ERROR;
    } else if (password_len >= CRED_BUFF_LEN) {
	printf("Password on line %d is too long. Limit it to %d characters.\n", line_no, CRED_BUFF_LEN);
	return GENERAL_ERROR;
    }

    for (int i = 0; i < first_comma_index; i++) {
	acc_name[i] = batch_line[i];
    }
    for (int i = 0, j = first_comma_index + 1; j < second_comma_index; j++, i++) {
	user_name[i] = batch_line[j];
    }
    for (int i = 0, j = second_comma_index + 1; j < batch_line_strlen; i++, j++) {
	password[i] = batch_line[j];
    }

    /* Clearing any newline characters for the last field(password) and properly null terminating*/
    if (strchr(password, '\n') != NULL) {
	password[strcspn(password, "\n")] = '\0';
    } else {
	password[password_len] = '\0';
    }

    /*If password needs to be automatically generated*/
    if (strcmp(password, "?") == 0) {
	int status;
	status = gen_pass_string(password, CRED_BUFF_LEN);
	if (status != SUCCESS_OP) {
	    return GENERAL_ERROR;
	}
	password[PASSWORD_LENGTH] = '\0';
    }
    user_name[user_name_len] = '\0';
    acc_name[acc_name_len] = '\0';

    return SUCCESS_OP;
}
