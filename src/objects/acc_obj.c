#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "objects/acc_obj.h"
#include "util/errors.h"

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
