#ifndef MAIN_H
#define MAIN_H

#include "../includes/db_access.h"
#include <stdio.h>

/*------Function pointer to use for dispatch table----*/
typedef int (*Handler)(char **, int, sqlite3 *);

/*-------------Sub Command Structure--------------------*/
struct sub_command {
    const char *name;
    Handler command_handler;
};

/*----------------Credential Structure to be stored----------*/
struct credential {
    char acc_name[64];
    char user_name[64];
    char pass[64];
};

/*----------------Function declarations----------*/
int handle_input(int argc, char *argv[], sqlite3 *db);
int get_pass_string(char *buff, int buff_size);
int change_details(char **argv, int argc, sqlite3 *db);
int delete_account(char **argv, int argc, sqlite3 *db);
int get_details(char **argv, int argc, sqlite3 *db);
int list_accounts(char **argv, int argc, sqlite3 *db);
int add_acc(char **argv, int argc, sqlite3 *db);
int add_acc_via_batch(sqlite3 *db, char *batch_file_name);
#endif
