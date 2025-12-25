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

Api_list createApiList() {
    Api_list lst = (Api_list)malloc(sizeof(struct api_list));
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

int insert_apinode(Api_list lst, char *name, char *uname, char *service, char *key) {
    if (lst == NULL) {
	printf("Uninitialized account list\n");
	return GENERAL_ERROR;
    }

    Api_node n = (Api_node)malloc(sizeof(struct api_node));
    if (n == NULL) {
	printf("Error allocating memory\n");
	return GENERAL_ERROR;
    }

    if (!name || !service || !key || !uname) {
	return GENERAL_ERROR;
    }
    n->name = strdup(name);
    n->service = strdup(service);
    n->username = strdup(uname);
    n->key = strdup(key);
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

void destroyApiList(Api_list lst) {
    if (lst == NULL) {
	return;
    } else if (lst->head == NULL) {
	free(lst);
	return;
    }

    Api_node n = lst->head;
    Api_node tmp = NULL;

    while (n != NULL) {
	free(n->name);
	free(n->username);
	free(n->service);
	free(n->key);
	tmp = n->next;
	free(n);
	n = tmp;
    }

    free(lst);
}

