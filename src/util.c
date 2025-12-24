/*
 * This source file contains some helper functions
 * used throughout other source files within the project
 * for example proper handling of user input printing
 * relevant error messages, modifying terminal settings
 * to enable entering credentials with 'echo' off etc
 */

#include "util.h"
#include "client_main.h"
#include "error_messages.h"

#include <stdio.h>
#include <string.h>

int print_help(char *subcommand) {
    if (strcmp(subcommand, "add") == 0) {
	printf("%s", ADD_MESSAGE);
    } else if (strcmp(subcommand, "change") == 0) {
	printf("%s", CHANGE_MESSAGE);
    } else if (strcmp(subcommand, "delete") == 0) {
	printf("%s", DELETE_MESSAGE);
    } else if (strcmp(subcommand, "get") == 0) {
	printf("%s", GET_MESSAGE);
    } else if (strcmp(subcommand, "ls") == 0) {
	printf("%s", LS_MESSAGE);
    } else {
	printf("Unknown subcommand: %s\n", subcommand);
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }

    return 0;
}

void print_result(char *fieldname, char *value) {
    if (!fieldname || !value) {
	return;
    }
    printf("%s %s\n", isatty(STDIN_FILENO) ? fieldname : "", value);
}

int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret) {
    struct termios oldt; // to store old terminal settings
    // Remove echo when typing input.
    if (secret == 1)
	set_secure_input(&oldt);

    char temp_buff[64];
    if (isatty(STDIN_FILENO)) {
	printf("%s: ", prompt);
    }
    fgets(temp_buff, sizeof(temp_buff), stdin);
    printf("\n");
    int temp_buff_strlen;

    // if some text remained in stdin
    if (strchr(temp_buff, '\n') == NULL) {
	printf("Limit input to %d characters\n", buff_len);
	flush_stdin();
	if (secret == 1)
	    remove_secure_input(&oldt);
	return GENERAL_ERROR;
    } else {
	/*removing new line character*/
	int new_line_pos = strcspn(temp_buff, "\n");
	temp_buff[new_line_pos] = '\0';
	temp_buff_strlen = strlen(temp_buff);

	if (temp_buff_strlen == 0) {
	    printf("Input can't be Empty\n");
	    if (secret == 1)
		remove_secure_input(&oldt);
	    return GENERAL_ERROR;
	}

	if (confirm == 1) {
	    printf("Enter again to confirm: ");
	    char temp_buff2[64];
	    fgets(temp_buff2, sizeof(temp_buff2), stdin);
	    printf("\n");
	    // if input is too long the second time
	    if (strchr(temp_buff2, '\n') == NULL) {
		flush_stdin();
		printf("Inputs don't match\n");
		if (secret == 1)
		    remove_secure_input(&oldt);
		return GENERAL_ERROR;
	    } else {
		temp_buff2[strcspn(temp_buff2, "\n")] = '\0';
		if (strcmp(temp_buff, temp_buff2) != 0) {
		    printf("Inputs don't match\n");
		    if (secret == 1)
			remove_secure_input(&oldt);
		    return GENERAL_ERROR;
		}
	    }
	}
    }

    if (secret == 1)
	remove_secure_input(&oldt);

    if (buff_len < temp_buff_strlen) {
	printf("Limit input to %d characters\n", buff_len);
	return GENERAL_ERROR;
    }
    snprintf(buff, buff_len, "%s", temp_buff);

    return SUCCESS_OP;
}

void set_secure_input(struct termios *oldt) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    // Get current console mode
    GetConsoleMode(hStdin, &mode);

    // Disable echo input
    DWORD newMode = mode & ~ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, newMode);
#else
    if (!isatty(STDIN_FILENO)) {
	return;
    }

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, oldt) != 0) {
	perror("tcgetattr");
    }

    struct termios newt;
    // Make a copy and modify it: turn off echo
    newt = *oldt;
    newt.c_lflag &= ~ECHO;

    // Apply the new settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
	perror("tcsetattr");
    }
#endif
}

void remove_secure_input(struct termios *oldt) {
#ifdef _WIN32
    // Restore original console mode
    SetConsoleMode(hStdin, mode);
#else
    if (!isatty(STDIN_FILENO)) {
	return;
    }
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, oldt);
#endif
}

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
	;
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
	status = insert_acc(a_lst, acc_name, password, user_name);
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
	status = get_pass_string(password, CRED_BUFF_LEN);
	if (status != SUCCESS_OP) {
	    return GENERAL_ERROR;
	}
	password[PASSWORD_LENGTH] = '\0';
    }
    user_name[user_name_len] = '\0';
    acc_name[acc_name_len] = '\0';

    return SUCCESS_OP;
}
