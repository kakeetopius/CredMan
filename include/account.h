#ifndef ACCOUNT_H
#define ACCOUNT_H

/*-----account object to be used when dealing with linked list of all accounts----------*/
typedef struct acc_node {
    char *name;		   /*The name of the account*/
    char *password;	   /*The password for the given account*/
    char *username;	   /*The username used for the account*/
    struct acc_node *next; /*Pointer to next account in linked list*/
    struct acc_node *prev; /*Pointer to previous account in linked list*/
} acc_node, *Acc_node;

/*-------accounts structure-------*/
typedef struct account_list {
    struct acc_node *head;
    struct acc_node *tail;
} account_list, *Account_list;

/*------Account object to be used as a stand alone object to represent a single account that is not part of a linked list------*/
/*----- This object can be typecast to struct acc_node incase required----*/
typedef struct account {
    char *name;	    /*The name of the account*/
    char *password; /*The password for the given account*/
    char *username; /*The username used for the account*/
} account, *Account;


/*
 * createAccList() initialises a a linked list of type struct account_list and sets all fields to their zero values.
 * On success it returns a pointer to struct account_list that must be cleared after use with a call to destroyAccList()
 * On failure it returns NULL
 */
Account_list createAccList();

/*
 * insert_acc() is used to create and insert an acc_node into an Account_list.
 * 1. lst: A pointer to an initialised struct account_list.
 * 2. name: The name of account to be added.
 * 3. pass: The password of account to be added.
 * 4. uname: The username associated with the account.
 *
 * Returns SUCCESS_OP on success and GENERAL_FAILURE on failure.
 */
int insert_acc(Account_list lst, char *name, char *pass, char *uname);

/*
 * delete_acc() is used to search and delete a node from an Account_list.
 * 1. lst: A pointer to an initialised struct account_list
 * 2. name: The name of the account to remove
 * 
 * Returns SUCCESS_OP on success and GENERAL_FAILURE on failure..
 */
int delete_acc(Account_list lst, char *name);

/*
 * search_acc() is used to search for a node in an Account_list.
 * 1. lst: A pointer to an initialised struct account_list
 * 2. name: The name of the account to search for. 
 * 
 * Returns SUCCESS_OP if the account is found and GENERAL_FAILURE otherwise.
 */
int search_acc(Account_list lst, char *name);

/*
 * change_passwd() is used to search for a node in an Account_list and change its password field..
 * 1. lst: A pointer to an initialised struct account_list
 * 2. acc_name: The name of the account to change password for.
 * 3  new_pass: The new password
 * 
 * Returns SUCCESS_OP on success and GENERAL_FAILURE otherwise.
 *
 */
int change_passwd(Account_list lst, char *acc_name, char *new_pass);

/*
 * change_acc_name() is used to search for a node in an Account_list and change its name field..
 * 1. lst: A pointer to an initialised struct account_list
 * 2. old_name: The name of the account to change password for.
 * 3  new_name: The new name for the account
 * 
 * Returns SUCCESS_OP on success and GENERAL_FAILURE otherwise.
 *
 */
int change_acc_name(Account_list lst, char *old_name, char *new_name);

/*
 * change_user_name() is used to search for a node in an Account_list and change its uname field..
 * 1. lst: A pointer to an initialised struct account_list
 * 2. acc_name: The name of the account to change password for.
 * 3  new_name: The new name for the account
 * 
 * Returns SUCCESS_OP on success and GENERAL_FAILURE otherwise.
 *
 */
int change_user_name(Account_list lst, char *acc_name, char *new_name);

/*
* destroyAccList() is used to free all memory used by a linked list of type Account_list including all
* string allocations for the fields. 
* It should be called once the list is no longer needed and no operations should be done on the list subsequently
* 1. lst: The list that was previously initialised by createAccList()
*/
void destroyAccList(Account_list lst);
#endif
