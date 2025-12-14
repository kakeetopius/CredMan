/*
* This source file contains all the implementations of handling 
* Account objects which are objects that hold the details about
* the different accounts during the running of the program
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "account.h"
#include "util.h"

Account_list createAccList() {
    Account_list lst = (Account_list)malloc(sizeof(struct account_list));
    if (lst == NULL) {
	printf("Error allocating memory\n");
	return NULL;
    }
    lst->head = NULL;
    lst->tail = NULL;
    return lst;
}

int insert_acc(Account_list lst, char *name, char *pass, char *uname) {
    if (lst == NULL) {
	printf("Uninitialized account list\n");
	return GENERAL_ERROR;
    }

    Acc_node n = (Acc_node)malloc(sizeof(struct acc_node));
    if (n == NULL) {
	printf("Error allocating memory\n");
	return GENERAL_ERROR;
    }

    if (!name)
	printf("Account name is NULL\n");
    if (!pass)
	printf("Password is NULL\n");
    if (!uname)
	printf("Username is Null\n");

    if (!name || !pass || !uname) {
	return GENERAL_ERROR;
    }
    n->name = strdup(name);
    n->password = strdup(pass);
    n->username = strdup(uname);
    n->prev = NULL;
    n->next = NULL;

    if (lst->head == NULL) {
	lst->head = n;
	lst->tail = n;
    } else {
	lst->tail->next = n;
	n->prev = lst->tail;
	lst->tail = n;
    }

    return SUCCESS_OP;
}

int delete_acc(Account_list lst, char *name) {
    if (lst == NULL || lst->head == NULL) {
	return GENERAL_ERROR;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
	if (strcmp(iter->name, name) == 0) {
	    if (iter == lst->head) {
		lst->head = iter->next;
		iter->next->prev = NULL;
	    } else if (iter == lst->tail) {
		lst->tail = iter->prev;
		iter->prev->next = NULL;
	    } else {
		iter->prev->next = iter->next;
		iter->next->prev = iter->prev;
	    }
	    free(iter->name);
	    free(iter->password);
	    free(iter->username);
	    free(iter);
	    return SUCCESS_OP;
	}
    }

    return GENERAL_ERROR;
}

int search_acc(Account_list lst, char *name) {
    if (lst == NULL) {
	printf("Uninitialized account list\n");
	return GENERAL_ERROR;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
	if (strcmp(name, iter->name) == 0) {
	    return SUCCESS_OP;
	}
    }

    return GENERAL_ERROR;
}

int change_passwd(Account_list lst, char *acc_name, char *new_pass) {
    if (lst == NULL) {
	printf("Uninitialized account list\n");
	return GENERAL_ERROR;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
	if (strcmp(acc_name, iter->name) == 0) {
	    free(iter->password);
	    iter->password = strdup(new_pass);
	    return SUCCESS_OP;
	}
    }

    return GENERAL_ERROR;
}

int change_acc_name(Account_list lst, char *old_name, char *new_name) {
    if (lst == NULL) {
	printf("Uninitialized account list\n");
	return GENERAL_ERROR;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
	if (strcmp(old_name, iter->name) == 0) {
	    free(iter->name);
	    iter->name = strdup(new_name);
	    return SUCCESS_OP;
	}
    }

    return GENERAL_ERROR;
}

int change_user_name(Account_list lst, char *acc_name, char *new_name) {
    if (lst == NULL) {
	printf("Unintialized account list\n");
	return GENERAL_ERROR;
    }

    Acc_node iter = NULL;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
	if (strcmp(acc_name, iter->name) == 0) {
	    free(iter->username);
	    iter->username = strdup(new_name);
	    return SUCCESS_OP;
	}
    }

    return GENERAL_ERROR;
}
void destroyAccList(Account_list lst) {
    if (lst == NULL) {
	return;
    } else if (lst->head == NULL) {
	free(lst);
	return;
    }

    Acc_node n = lst->head;
    Acc_node tmp = NULL;

    while (n != NULL) {
	free(n->name);
	free(n->password);
	free(n->username);
	tmp = n->next;
	free(n);
	n = tmp;
    }

    free(lst);
}
