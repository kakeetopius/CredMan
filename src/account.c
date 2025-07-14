#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../includes/account.h"


Account_list createAccList() {
    Account_list lst = (Account_list) malloc(sizeof(struct account_list));
    if (lst == NULL) {
        printf("Error allocating memory\n");
        return NULL;
    }
    lst->head = NULL;
    lst->tail = NULL;
    return lst;
}

int insert_acc(Account_list lst, char* name, char* pass, char* uname) {
    if (lst == NULL) {
        printf("Uninitialized account list\n");
        return -1;
    }

    Acc_node n = (Acc_node) malloc(sizeof(struct acc_node));
    if (n == NULL) {
        printf("Error allocating memory\n");
        return -1;
    }

    n->name = strdup(name);
    n->password = strdup(pass);
    n->username = strdup(uname);
    n->prev = NULL;
    n->next = NULL;

    if (lst->head == NULL) {
        lst->head = n;
        lst->tail = n;
    }
    else {
       lst->tail->next = n;
       n->prev = lst->tail;
       lst->tail = n;
    }
    
    return 0;
}

int delete_acc(Account_list lst, char* name) {
    if(lst == NULL || lst->head == NULL) {
        return -1;
    } 

    Acc_node iter = NULL;

    for(iter = lst->head; iter != NULL; iter = iter->next) {
        if(strcmp(iter->name, name) == 0) {
            if(iter == lst->head) {
                lst->head = iter->next;
                iter->next->prev = NULL;
            }
            else if (iter == lst->tail) {
                lst->tail = iter->prev;
                iter->prev->next = NULL;
            }
            else {
                iter->prev->next = iter->next;
                iter->next->prev = iter->prev;
            }
            free(iter->name);
            free(iter->password);
            free(iter->username);
            free(iter);
            return 0;
        }
    }

    return -1;
}

int search_acc(Account_list lst, char* name) {
    if (lst == NULL) {
        printf("Uninitialized account list\n");
        return -1;
    }

    Acc_node iter = NULL;
    
    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if(strcmp(name, iter->name) == 0) {
            return 0;
        }
    }

    return -1;
}

int change_passwd(Account_list lst, char* acc_name, char* new_pass) {
    if (lst == NULL) {
        printf("Uninitialized account list\n");
        return -1;
    }

    Acc_node iter = NULL;
    
    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if(strcmp(acc_name, iter->name) == 0) {
            free(iter->password);
            iter->password = strdup(new_pass);
            return 0;
        }
    }

    return -1;
}

int change_acc_name(Account_list lst, char* old_name, char* new_name) {
    if (lst == NULL) {
        printf("Uninitialized account list\n");
        return -1;
    }

    Acc_node iter = NULL;
    
    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if(strcmp(old_name, iter->name) == 0) {
            free(iter->name);
            iter->name = strdup(new_name);
            return 0;
        }
    }

    return -1;
}

int change_user_name(Account_list lst, char* acc_name, char* new_name) {
    if (lst == NULL) {
        printf("Unintialized account list\n");
        return -1;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if (strcmp(acc_name, iter->name) == 0) {
            free(iter->username);
            iter->username = strdup(new_name);
            return 0;
        }
    }

    return -1;
}
void destroyAccList(Account_list lst) {
    if (lst == NULL) {
        return;
    }
    else if (lst->head == NULL) {
        free(lst);
        return;
    }

    Acc_node n = lst->head; 
    Acc_node tmp = NULL; 

    while(n != NULL) {
        free(n->name);
        free(n->password);
        free(n->username); 
        tmp = n->next;
        free(n);
        n = tmp;
    }

    free(lst);
}
