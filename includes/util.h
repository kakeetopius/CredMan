#ifndef UTIL_H
#define UTIL_H

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "../includes/account.h"

#define PASSWORD_LENGTH 16 
#define CRED_BUFF_LEN 42
#define BATCH_FILE_LINE_LEN ((CRED_BUFF_LEN) * 3 + 3)

#define LINE_EMPTY 21

void flush_stdin();
void remove_secure_input(struct termios *oldt);
void set_secure_input(struct termios *oldt);
int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret);
int print_help(char* subcommand);
int get_creds_from_batch_file(Account_list a_lst, char *batch_file_name);
int split_batch_line(char* batch_line, char* acc_name, char* user_name, char* password, int line_no);

#endif
