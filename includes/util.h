#ifndef UTIL_H
#define UTIL_H

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#define PASSWORD_LENGTH 16 
#define CRED_BUFF_LEN 32 


void flush_stdin();
void remove_secure_input(struct termios *oldt);
void set_secure_input(struct termios *oldt);
int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret);
int print_help(char* subcommand);

#endif
