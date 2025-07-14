#ifndef ACCOUNT_H
#define ACCOUNT_H

/*-----account object----------*/
typedef struct acc_node {
    char* name;
    char* password;
    char* username;
    struct acc_node* next;
    struct acc_node* prev;
}acc_node, *Acc_node;

/*-------accounts structure-------*/
typedef struct account_list {
    struct acc_node* head;
    struct acc_node* tail;
}account_list, *Account_list;

Account_list createAccList();
int insert_acc(Account_list lst, char* name, char* pass, char* uname);
int delete_acc(Account_list lst, char* name);
int search_acc(Account_list lst, char* name);
int change_passwd(Account_list lst, char* acc_name, char* new_pass);
int change_acc_name(Account_list lst, char* old_name, char* new_name);
void destroyAccList(Account_list lst);
int change_user_name(Account_list lst, char* acc_name, char* new_name);
#endif
