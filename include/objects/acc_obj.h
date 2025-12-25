#ifndef ACC_OBJ_H
#define ACC_OBJ_H

/*-----account object to be used when dealing with linked list of all accounts----------*/
typedef struct acc_node {
    char *name;		   /*The name of the account*/
    char *password;	   /*The password for the given account*/
    char *username;	   /*The username used for the account*/
    struct acc_node *next; /*Pointer to next account in linked list*/
    struct acc_node *prev; /*Pointer to previous account in linked list*/
} acc_node, *Acc_node;

typedef struct account_list {
    struct acc_node *head;
    struct acc_node *tail;
} account_list, *Account_list;

typedef struct account {
    char *name;	    /*The name of the account*/
    char *password; /*The password for the given account*/
    char *username; /*The username used for the account*/
} account, *Account;

Account_list createAccList();

int insert_acc_node(Account_list lst, char *name, char *pass, char *uname);

void destroyAccList(Account_list lst);
#endif
