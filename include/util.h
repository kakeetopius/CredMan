#ifndef UTIL_H
#define UTIL_H

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

/*Included here to provide definition of Account_list for the get_creds_from_batch_file() function*/
#include "account.h"

/*---GENERIC ERROR CODES----*/
#define SUCCESS_OP 0
#define USER_REQUESTED_HELP 1
#define GENERAL_ERROR -1
#define LINE_EMPTY 21

/*---------CREDEENTIAL BUFFER SIZE DEFINITIONS---------*/
#define PASSWORD_LENGTH 16 
#define CRED_BUFF_LEN 128
#define BATCH_FILE_LINE_LEN ((CRED_BUFF_LEN) * 3 + 3)

/*
 * get_user_input() is used to safely get input from the user.
 * Parameters:
 * 1. buff: A buffer in which the user input will be returned.
 * 2. buff_len: The length of the buffer in which the input is to be returned and it also dictates the maximum size to get from user.
 * 3. prompt: A string that will be shown to user with a colon added at the end when asking for input.
 * 4. confirm: A number to indicate if the user should be prompted once again and confirm if the inputs match.
 *    1 indicates that confirmation is required
 *    any other value will indicate confirmation is not required.
 * 5. secret: A number used to indicate whether 'echo' property should be removed from the terminal while the user types which is useful for passwords.
 *    1 indicates that echo property should be removed.
 *    any other value indicates that the terminal should be left as it is.
 *
 * Returns SUCCESS_OP on success and GENERAL_ERROR on failure.
 */
int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret);

/*
 * print_help() is used as a generic function to print help message for any subcommand.
 *
 * If subcommand exists and corresponding message exists it prints the message and returns SUCCESS_OP
 * Else it returns GENERAL_ERROR and prints the GENERAL_MESSAGE.
 */
int print_help(char* subcommand);

/*
 *get_creds_from_batch_file() is used to read lines from a credential batch file and add them to an Account_list.
 1. a_lst: A pointer to an already initialsed Account_list object.
 2. batch_file_name: The name on the file system of the batch file.

 Returns SUCCESS_OP on success and GENERAL_ERROR on failure.
 */
int get_creds_from_batch_file(Account_list a_lst, char *batch_file_name);

/*
 * split_batch_line() is an internal helper function used to split lines read from batch file with a comma as the delimiter
 * and subsequently initialise the given buffers with the strings split.
 *
 * 1. batch_line: A single line string read from the batch line containing commas separating the fields.
 * 2. acc_name : A buffer of at least size CRED_BUFF_LEN where the first string from split results will be stored.
 * 3. user_name : A buffer of at least size CRED_BUFF_LEN where the second string from split results will be stored.
 * 4. password : A buffer of at least size CRED_BUFF_LEN where the last string from split results will be stored.
 * 5. line_no: The current line number in the batch file useful for reporting errors.
*  
*  Return Values:
*  SUCCESS_OP: If the line was successfully split and buffers successfully initialsed
*  LINE_EMPTY: If the line only containes a newline character.
*  GENERAL_ERROR: If the a_lst is Null or the line contains very long fields that are above CRED_BUFF_LEN for each field.
*/

int split_batch_line(char* batch_line, char* acc_name, char* user_name, char* password, int line_no);

/*
 * set_secure_input() is an internal helper function used by the get_user_input() function to remove echo from the terminal
 * device.
 */
void set_secure_input(struct termios *oldt);

/*
 *remove_secure_input() is an internal helper used by the get_user_input() function to restore the default terminal settings if they 
 * were modified before.
*/
void remove_secure_input(struct termios *oldt);

/*
 * flush_stdin() is an internal helper function used by the get_user_input() function to make sure the input buffer from stdin is
 * clean incase fgets() reads some user input but does not complete due to the buffer where the input is being put becoming full
 * in order to prevent subsquent calls to fgets from behaving unexpectedly.
 */
void flush_stdin();

void print_result(char* fieldname, char* value);
#endif
