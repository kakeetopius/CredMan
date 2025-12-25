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

typedef struct api_node {
    char *name;
    char *service;
    char *username;
    char *key;
    struct api_node *next;
    struct api_node *prev;
} api_node, *Api_node;

/*-------accounts structure-------*/
typedef struct account_list {
    struct acc_node *head;
    struct acc_node *tail;
} account_list, *Account_list;

typedef struct api_list {
    struct api_node *head;
    struct api_node *tail;
} api_list, *Api_list;

/*------Account object to be used as a stand alone object to represent a single account that is not part of a linked list------*/
/*----- This object can be typecast to struct acc_node incase required----*/
typedef struct account {
    char *name;	    /*The name of the account*/
    char *password; /*The password for the given account*/
    char *username; /*The username used for the account*/
} account, *Account;

typedef struct api_key {
    char *name;
    char *service;
    char *username;
    char *key;
} api_key, *Api_Key;
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
 * destroyAccList() is used to free all memory used by a linked list of type Account_list including all
 * string allocations for the fields.
 * It should be called once the list is no longer needed and no operations should be done on the list subsequently
 * 1. lst: The list that was previously initialised by createAccList()
 */
void destroyAccList(Account_list lst);


void destroyApiList(Api_list lst);
int insert_apinode(Api_list lst, char *name, char *uname, char *service, char *key);
Api_list createApiList();
#endif
