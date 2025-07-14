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
#include "../includes/main.h"
#include "../includes/secure.h"
#include "../includes/error_messages.h"

/*------Function pointer to use for dispatch table----*/
typedef int (*handlers)(char**, int);

/*----account list object-----*/
Account_list a_lst = NULL;

int main(int argc, char *argv[]) {
    char pass[30] = {0};
    int status = get_password(pass, sizeof(pass));
    if (status != 0)
        return -1;

    char input_buff[100];
    memset(input_buff, 0, 100);
    int flags = handle_input(argc, argv, input_buff, 99);

    status = initialize_accounts(pass);

    if (status != 0)
        return -1;

    if (flags == -1) {
        return -1;
    } else if (flags & n_flag) {
        create_pass(input_buff, NULL);
    } else if (flags & a_flag) {
        add_pass_from_user(input_buff);
    } else if (flags & l_flag) {
        list_accounts();
    } else if (flags & s_flag) {
        get_pass(input_buff);
    } else if (flags & c_flag) {
        char option[10];
        printf("Are you sure you want to change password for %s: ", input_buff);
        scanf("%s", option);
        if (strcmp(option, "yes") == 0)
            change_pass(input_buff, NULL);
        else
            printf("Not changed\n");
    } else if (flags & o_flag) {
        char option[10];
        printf("Are you sure you want to change password for %s: ", input_buff);
        scanf("%s", option);
        if (strcmp(option, "yes") == 0)
            change_pass_from_user(input_buff);
        else
            printf("Not changed\n");
    } else if (flags & d_flag) {
        char option[10] = {0};
        printf("Are you sure you want to delete this account: ");
        scanf("%s", option);
        if (strcmp(option, "yes") == 0) {
            delete_account(input_buff);
        } else {
            printf("Not Deleted\n");
        }
    } else if (flags & u_flag) {
        change_uname(input_buff);
    }

    if ((flags & l_flag) || (flags & s_flag)) {
        destroyAccList(a_lst);
        return 0;
    }

    write_to_file(pass);
    destroyAccList(a_lst);
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
    FILE *cred_file = fopen(CREDS_FILE, "r");
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

int get_password(char *buff, int buff_len) {

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

void change_pass_from_user(char *pass_for) {
    char pass[50];
    printf("Enter the password for %s: ", pass_for);
    scanf("%49s", pass);
    change_pass(pass_for, pass);
}

void add_pass_from_user(char *pass_for) {
    char pass[50];
    printf("Enter the password for %s: ", pass_for);
    scanf("%49s", pass);
    create_pass(pass_for, pass);
}

void change_uname(char *account) {
    if (search_acc(a_lst, account) != 0) {
        printf("Account doesn't exist\n");
        return;
    }

    char uname[50];
    printf("Enter the new username: ");
    scanf("%49s", uname);
    change_user_name(a_lst, account, uname);
}

void change_pass(char *pass_for, char *password) {
    if (search_acc(a_lst, pass_for) != 0) {
        printf("Account doesn't exist\n");
        return;
    }

    if (password == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        printf("New Password: %s\n", passwd);

        if (change_passwd(a_lst, pass_for, passwd) == 0)
            printf("Password Changed successfully\n");
        else
            printf("Error changing Password\n");

    } else {
        if (change_passwd(a_lst, pass_for, password) == 0)
            printf("Error changing password\n");
        else
            printf("Error changing Password\n");
        printf("New Password: %s\n", password);
    }
}

void delete_account(char *account) {
    if (search_acc(a_lst, account) != 0) {
        printf("Account doesn't exist\n");
        return;
    }

    if (delete_acc(a_lst, account) == 0)
        printf("Account Deleted\n");
    else
        printf("Not Deleted\n");
}

void create_pass(char *pass_for, char *pass) {

    if (search_acc(a_lst, pass_for) == 0) {
        printf("Password For %s already exists\n", pass_for);
        return;
    }

    char uname[50] = {0};
    printf("Enter username for account: ");
    scanf("%49s", uname);

    if (pass == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        printf("New Password for %s: %s\n", pass_for, passwd);

        if (insert_acc(a_lst, pass_for, passwd, uname) == 0) {
            printf("Account added successfully\n");
        } else {
            printf("Error inserting account\n");
        }

    } else {
        printf("New Password for %s: %s\n", pass_for, pass);
        if (insert_acc(a_lst, pass_for, pass, uname) == 0) {
            printf("Account added successfully\n");
        } else {
            printf("Error inserting account\n");
        }
    }
}

void get_pass(char *pass_for) {
    Acc_node n = NULL;

    for (n = a_lst->head; n != NULL; n = n->next) {
        if (strcmp(n->name, pass_for) == 0) {
            printf("Account:   %s\n", pass_for);
            printf("User Name: %s\n", n->username);
            printf("Password:  %s\n", n->password);
            return;
        }
    }
    printf("Account doesn't exist\n");
}

void list_accounts() {
    Acc_node n = NULL;

    for (n = a_lst->head; n != NULL; n = n->next) {
        printf("Acc:        %s\n", n->name);
        printf("Username:   %s\n", n->username);
        printf("Pass:       %s\n", n->password);
        printf("\n");
    }
}

int handle_input(int argc, char *argv[], char *input_buff, int buff_size) {
    if (strcmp(argv[1], "-h") == 0) {

	printf("%s", GENERAL_MESSAGE);
    }
    return -1;
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
