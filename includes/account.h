#ifndef ACCOUNT_H
#define ACCOUNT_H


/*-----account object to be used when dealing with linked list of all accounts----------*/
typedef struct acc_node {
    char* name;			/*The name of the account*/
    char* password;		/*The password for the given account*/
    char* username;		/*The username used for the account*/
    struct acc_node* next;	/*Pointer to next account in linked list*/
    struct acc_node* prev; 	/*Pointer to previous account in linked list*/
}acc_node, *Acc_node;

/*-------accounts structure-------*/
typedef struct account_list {
    struct acc_node* head;
    struct acc_node* tail;
}account_list, *Account_list;

/*------Account object to be used as a stand alone object to represent a single account------*/
typedef struct account {
    char* name;			/*The name of the account*/
    char* password;;		/*The password for the given account*/
    char* username;;		/*The username used for the account*/
}account, *Account;

Account_list createAccList();
int insert_acc(Account_list lst, char* name, char* pass, char* uname);
int delete_acc(Account_list lst, char* name);
int search_acc(Account_list lst, char* name);
int change_passwd(Account_list lst, char* acc_name, char* new_pass);
int change_acc_name(Account_list lst, char* old_name, char* new_name);
void destroyAccList(Account_list lst);
int change_user_name(Account_list lst, char* acc_name, char* new_name);
#endif
