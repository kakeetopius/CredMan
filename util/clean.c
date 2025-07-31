#include <stdio.h>
#include <string.h>
#include "account.h"
#include "main.h"
#include <stdlib.h>
void clean_line(char *str, int len);

Account_list a_lst = NULL;

int main(void) {
    initialize_accounts(NULL);
    write_to_file(NULL);
}

void write_to_file(char *pass) {
    /*Opening credential file*/
    FILE *cred_file = fopen("dout.txt", "wb");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return;
    }
    
    int cred_size = sizeof(struct credential);
    struct credential* cred = (struct credential*)malloc(cred_size);

    Acc_node n = NULL;
    for (n = a_lst->head; n != NULL; n = n->next) {
	memset(cred, '\0', cred_size);	
	strncpy(cred->acc_name, n->name, 63);
	strncpy(cred->user_name, n->username, 63);
	strncpy(cred->pass, n->password, 63);
	fwrite(cred, cred_size, 1, cred_file);
    }
    
    free(cred);
    fclose(cred_file);
}

int initialize_accounts(char *pass) {
    /*Opening credential file*/
    FILE *cred_file = fopen("creds.txt", "r");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return -1;
    }
    
    char *name;
    char *passwd;
    char *uname;

    /*--Account list---*/
    a_lst = createAccList();
    if (a_lst == NULL) {
        printf("Error initializing accounts\n");
        return -1;
    }

    struct credential* cred;
    int cred_size = sizeof(struct credential);
    char buff[100];

    while (fgets(buff, 99, cred_file)) {
        char *saveptr;
        name = strtok_r(buff, ":", &saveptr);

        if (name == NULL) {
            printf("Error parsing credential file(1)\n");
            return -1;
        }
        passwd = strtok_r(NULL, ":", &saveptr);
        if (passwd == NULL) {
            printf("Error parsing credential file(2)\n");
            return -1;
        }

        uname = strtok_r(NULL, "\n", &saveptr);
        if (uname == NULL) {
            printf("Error parsing credential file(3)\n");
            return -1;
        }

        insert_acc(a_lst, name, passwd, uname);
        memset(buff, 0, 100);
    }

    fclose(cred_file);
    return 0;
}
