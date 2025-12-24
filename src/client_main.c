/*
 *client_main.c is the entry point for the application.
 *It contains all the logic necessary to deal with
 *user input and launching the necessary handler depending
 *on what is required by user (adding, changing, searching
 *deleting and listing of account credentials).
 */

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <time.h>
#endif

#include "argparser.h"
#include "client_main.h"
#include "util.h"

int main(int argc, char *argv[]) {
    struct Command *cmd = NULL;

    int status = parse_args(argc, argv, &cmd);
    if (status == USER_REQUESTED_HELP) {
	return 0;
    } else if (!cmd) {
	return GENERAL_ERROR;
    } else if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }

    sqlite3 *db_con = open_db_con();
    if (!db_con) {
	free_arguments(cmd);
	return GENERAL_ERROR;
    }

    status = cmd->Run(cmd->arguments, db_con);

    free_arguments(cmd);
    sqlite3_close(db_con);

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
