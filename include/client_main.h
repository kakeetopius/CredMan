#ifndef MAIN_H
#define MAIN_H

/* This is included here because of the sqlite3 interface required by all function prototypes. */
#include "db_access.h"


/*------Function pointer to use for dispatch table----*/
/*
 * Paramaters for the handlers.
 * 1. An array of arguments passed via command line i.e argv
 * 2. The number of arguments passed via command line ie argc
 * 3. The database handle that will be used when invoking function calls to the db_acess module.
 */
typedef int (*Handler)(char **, int, sqlite3 *);

/*-------------Sub Command Structure--------------------*/
struct sub_command {
    const char *name;		// The name of the sub command to be carried out.
    Handler command_handler;	// The function to carry out the subcommand.
};

/*-----------------------------------Function declarations------------------------------------------------------------*/


/*--The following functions are of type Handler and are used to carry out the different functions for the sub commands*/

/*
* get_pass_string is used to generate a password string whose size is defined by the macro PASSWORD_LENGTH
* on linux/unix systems it will use /dev/urandom for the randomness.
* Paramaters:
* 1. buff: A buffer in which the randomly generated password string will be placed. Note that the string won't be NULL terminated.
* 2. buff_size: The size of the buffer in which the password will be placed. This size should be at least PASSWORD_LENGTH
*/
int get_pass_string(char *buff, int buff_size);


/*------------------------RETURN VALUES----------------------------------*/
/*All functions return SUCCESS_OP on succes and GENERAL_ERROR otherwise.*/

#endif
