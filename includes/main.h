#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

#ifdef _WIN32
    #define CREDS_FILE "C:\\Documents\\creds.txt"
    #define BACKUP "C:\\Log\\wcred.txxt"
#else
    #define CREDS_FILE "util/creds.txt"
    // #define CREDS_FILE "/media/pius/Windows/Documents/creds.txt"
    #define BACKUP "/home/kapila/.creds.txt"
#endif

/*------Function pointer to use for dispatch table----*/
typedef int (*Handler)(char **, int);

/*-------------Sub Command Structure--------------------*/
struct sub_command {
    const char* name;
    Handler command_handler; 
};


/*----------------Credential Structure to be stored----------*/
struct credential {
    char acc_name[32];
    char user_name[32];
    char pass[32];
};

/*----------------Function declarations----------*/
int handle_input(int argc, char* argv[], char* pass);
void get_pass_string(char* buff, int buff_size);
int get_password(char* buff, int buff_len);
int initialize_accounts(char* pass);
void write_to_file(char* pass);
int change_details(char **argv, int argc); 
int delete_account(char** argv, int argc);
int get_details(char** argv, int argc);
int list_accounts(char** argv, int argc);
int add_acc(char **argv, int argc);
#endif
