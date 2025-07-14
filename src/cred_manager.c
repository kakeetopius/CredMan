#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "../includes/account.h"
#include "../includes/error_messages.h"
#include "../includes/main.h"
#include "../includes/secure.h"

/*------Function pointer to use for dispatch table----*/
typedef int (*handlers)(char **, int);

/*----account list object-----*/
Account_list a_lst = NULL;

int main(int argc, char *argv[]) {

    char pass[30] = {0};
    int status = get_password(pass, sizeof(pass));
    if (status != 0)
        return -1;

    status = handle_input(argc, argv, pass);
    if (status == -1) {
        return -1;
    }

    return 0;
}

void write_to_file(char *pass) {
    /*Opening credential file*/
    FILE *cred_file = fopen(CREDS_FILE, "wb");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return;
    }

    /*Getting key*/
    int key = encrypt_key(pass);

    char buff[100];
    Acc_node n = NULL;
    for (n = a_lst->head; n != NULL; n = n->next) {
        snprintf(buff, 99, "%s:%s:%s", n->name, n->password, n->username);
        encrypt_line(key, buff);
        fprintf(cred_file, "%s", buff);
    }

    fclose(cred_file);
}

int initialize_accounts(char *pass) {
    /*Opening credential file*/
    FILE *cred_file = fopen(CREDS_FILE, "rb");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return -1;
    }

    /*Getting key*/
    int key = encrypt_key(pass);
    char *name;
    char *passwd;
    char *uname;

    /*--Account list---*/
    a_lst = createAccList();
    if (a_lst == NULL) {
        printf("Error initializing accounts\n");
        return -1;
    }

    char buff[100];

    while (fgets(buff, 100, cred_file) != NULL) {
        decrypt_line(key, buff);

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

/*Function to get master password from user which is also essential for
 * encryption*/
int get_password(char *buff, int buff_len) {

    /*---TODO: Get password from hash environment variable--*/

#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    // Get current console mode
    GetConsoleMode(hStdin, &mode);

    // Disable echo input
    DWORD newMode = mode & ~ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, newMode);
#else
    struct termios oldt, newt;

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        perror("tcgetattr");
    }

    // Make a copy and modify it: turn off echo
    newt = oldt;
    newt.c_lflag &= ~ECHO;

    // Apply the new settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        perror("tcsetattr");
    }
#endif

    int tries = 3;
    do {
        printf("%s: \n", tries == 3 ? "Enter Master Password "
                                    : "Wrong Password Try Again ");
        fgets(buff, buff_len, stdin);
#ifdef _WIN32
        buff[strcspn(buff, "\r\n")] = '\0'; // Remove \r or \n
#else
        buff[strcspn(buff, "\n")] = '\0'; // Strip newline
#endif
        tries--;
        if (strcmp(buff, "Kapila.707403") == 0) {
            break;
        }
    } while (tries > 0);

#ifdef _WIN32
    // Restore original console mode
    SetConsoleMode(hStdin, mode);
#else
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    if (tries == 0) {
        printf("Too many tries");
        return -1;
    }
    return 0;
}

/*--- Dispatch function to handle adding new accounts-----*/
int add_acc(char **argv, int argc) {
    if (argc == 3) {
        if (strcmp(argv[2], "help")) {
            printf("%s", ADD_MESSAGE);
            return 0;
        } else {
            printf("%s", ADD_MESSAGE);
            return -1;
        }
    }

    if (argc != 4) {
        printf("%s", ADD_MESSAGE);
        return -1;
    }

    char *account = argv[2];

    if (search_acc(a_lst, account) == 0) {
        printf("Password For %s already exists\n", account);
        return -1;
    }

    char user_name[50];
    char pass[50];
    printf("Enter the User Name: ");
    scanf("%49s", user_name);

    if (strcmp("no-auto", argv[3]) == 0) {
        printf("Enter Password: ");
        scanf("%49s", pass);
    } else if (strcmp("auto", argv[3]) == 0) {
        get_pass_string(pass, 16);
    } else {
        printf("Unknown option: %s\n", argv[3]);
    }

    printf("New Password for %s: %s\n", account, pass);
    if (insert_acc(a_lst, account, pass, user_name) == 0) {
        printf("Account added successfully\n");
    } else {
        printf("Error inserting account\n");
    }
    return 0;
}

/*--- Dispatch function to handle changing account details-----*/
int change_details(char **argv, int argc) {
    if (argc == 3) {
        if (strcmp(argv[2], "help")) {
            printf("%s", CHANGE_MESSAGE);
            return 0;
        } else {
            printf("Unknown option: %s", argv[2]);
            return -1;
        }
    }

    if (argc != 4) {
        printf("%s", CHANGE_MESSAGE);
        return -1;
    }

    char *account = argv[2];
    char *option = argv[3];
    char value[50];

    if (search_acc(a_lst, account) != 0) {
        printf("Account doesn't exist\n");
        return -1;
    }

    if (strcmp(option, "user") == 0) {
        printf("Enter New User Name: ");
        scanf("%49s", value);
        change_user_name(a_lst, account, value);
    }
    if (strcmp(option, "name") == 0) {
        printf("Enter New Account Name: ");
        scanf("%49s", value);
        change_acc_name(a_lst, account, value);
    }
    if (strcmp(option, "pass") == 0) {
        if (argc != 5) {
            printf("%s", CHANGE_MESSAGE);
            return -1;
        }
        char pass[50];
        if (strcmp(argv[4], "auto") == 0) {
            get_pass_string(pass, 16);
        } else if (strcmp(argv[4], "no-auto") == 0) {
            printf("Enter New Password: ");
            scanf("%49s", value);
        }
        change_passwd(a_lst, account, pass);
    }

    return 0;
}

/*--- Dispatch function to handle deleting of accounts-----*/
int delete_account(char **argv, int argc) {
    if (argc != 3) {
        printf("%s", DELETE_MESSAGE);
        return -1;
    }

    if (strcmp("help", argv[2]) == 0) {
        printf("%s", DELETE_MESSAGE);
        return -1;
    }

    char *account = argv[2];
    if (search_acc(a_lst, account) != 0) {
        printf("Account %s doesn't exist\n", account);
        return -1;
    }

    if (delete_acc(a_lst, account) == 0)
        printf("Account Deleted\n");
    else
        printf("Not Deleted\n");

    return 0;
}

/*--- Dispatch function to handle searching of account details-----*/
int get_details(char **argv, int argc) {
    if (argc != 3) {
        printf("%s", SEARCH_MESSAGE);
        return -1;
    }

    if (strcmp("help", argv[2]) == 0) {
        printf("%s", SEARCH_MESSAGE);
        return -1;
    }

    char *account = argv[2];

    Acc_node n = NULL;

    for (n = a_lst->head; n != NULL; n = n->next) {
        if (strcmp(n->name, account) == 0) {
            printf("Account:   %s\n", account);
            printf("User Name: %s\n", n->username);
            printf("Password:  %s\n", n->password);
            return 0;
        }
    }
    printf("Account %s doesn't exist\n", account);
    return -1;
}

/*--- Dispatch function to list the details of all accounts-----*/
int list_accounts(char** argv, int argc) {
    Acc_node n = NULL;

    for (n = a_lst->head; n != NULL; n = n->next) {
        printf("Acc:        %s\n", n->name);
        printf("Username:   %s\n", n->username);
        printf("Pass:       %s\n", n->password);
        printf("\n");
    }
    return 0;
}

int handle_input(int argc, char *argv[], char *pass) {
    if (argc < 2) {
        printf("%s", GENERAL_MESSAGE);
        return -1;
    } else if (strcmp(argv[1], "help") == 0) {
        printf("%s", GENERAL_MESSAGE);
        return 0;
    }

    int status = initialize_accounts(pass);

    if (status != 0)
        return -1;

    write_to_file(pass);
    destroyAccList(a_lst);
    return 0;
}

void get_pass_string(char *buff, int buff_size) {
    memset(buff, 0, buff_size);
    srand(time(NULL));
    int num;
    int alpha = 0;
    int numeric = 0;
    int s_char = 0;

    for (int i = 0; i < buff_size; i++) {
        num = rand() % 88 + 35; // random number between 35 and 122
        if ((num >= 37 && num <= 47) || (num >= 58 && num <= 63) ||
            (num >= 91 && num <= 96)) {
            i--;
            continue;
        }

        if (isalpha(num))
            alpha++;
        else if (isdigit(num))
            numeric++;

        if (alpha > 8) {
            num = rand() % 10 + 48; // choose a number
            numeric++;
            if (numeric > 5) {
                s_char = rand() % 3;
                switch (s_char) {
                case 0:
                    num = 35;
                    break; // #
                case 1:
                    num = 36;
                    break; //$
                case 2:
                    num = 64;
                    break; //@
                }
            }
        }

        buff[i] = num;
    }
    buff[buff_size - 1] = '\0';
}
