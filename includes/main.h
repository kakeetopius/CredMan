#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

#ifdef _WIN32
    #define CREDS_FILE "C:\\Documents\\creds.txt"
    #define BACKUP "C:\\Log\\wcred.txxt"
#else
    #define CREDS_FILE "/home/kapila/Documents/Win_Docs/creds.txt"
    #define BACKUP "/home/kapila/.creds.txt"
#endif


/*----------input flags----------*/
#define n_flag 0x01  //00000001
#define s_flag 0x02  //00000010
#define c_flag 0x04  //00000100
#define l_flag 0x08  //00001000
#define d_flag 0x10  //00010000
#define i_flag 0x20  //00100000
#define a_flag 0x40  //01000000
#define o_flag 0x80  //10000000
#define u_flag 0x100 //100000000

/*----------------Function declarations----------*/
int handle_input(int argc, char* argv[], char* input_buff, int buff_size);
void create_pass(char* pass_for, char* pass);
void list_accounts();
int account_exists(char* account);
void get_pass(char* passwd_for);
void change_pass(char* pass_for, char* pass);
void get_pass_string(char* buff, int buff_size);
void delete_account(char* account);
void change_pass_from_user(char* pass_for);
void add_pass_from_user(char* pass_for);
int get_password(char* buff, int buff_len);
int initialize_accounts(char* pass);
void write_to_file(char* pass);
void change_uname(char* account);

#endif
