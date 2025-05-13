#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
    #include <unistd.h>
    #include <termios.h>
#else 
    #include <windows.h>
#endif

#include "../includes/secure.h"
#include "../includes/main.h"
#include "../includes/account.h"

/*----account list object-----*/
Account_list a_lst = NULL;

int main(int argc, char* argv[]) {
    char pass[30] = {0};
    int status;

    status = get_password(pass, sizeof(pass));
    if (status != 0) return -1;

    char input_buff[100];
    memset(input_buff, 0, 100);
    int flags = handle_input(argc, argv, input_buff, 99);

    status = initialize_accounts(pass);
    
    if (status != 0) return -1;

    if(flags == -1) {
        return -1;
    }
    else if(flags & n_flag) {
        create_pass(input_buff, NULL);
    }
    else if (flags & a_flag) {
        add_pass_from_user(input_buff);
    }
    else if(flags & l_flag) {
        list_accounts();
    }
    else if(flags & s_flag) {
        get_pass(input_buff);
    }
    else if (flags & c_flag) {
        char option[10];
        printf("Are you sure you want to change password for %s: ", input_buff);
        scanf("%s", option);
        if(strcmp(option, "yes") == 0)
            change_pass(input_buff, NULL);
        else 
            printf("Not changed\n");
    }
    else if (flags & o_flag) {
        char option[10];
        printf("Are you sure you want to change password for %s: ", input_buff);
        scanf("%s", option);
        if(strcmp(option, "yes") == 0)
            change_pass_from_user(input_buff);
        else 
            printf("Not changed\n");
    }
    else if(flags & d_flag) {
        char option[10] = {0};
        printf("Are you sure you want to delete this account: ");
        scanf("%s", option);
        if(strcmp(option, "yes") == 0) {
            delete_account(input_buff);
        }
        else{
            printf("Not Deleted\n");
        }
    }

    if ((flags&l_flag) || (flags&s_flag)) {
        destroyAccList(a_lst);
        return 0;
    }

    write_to_file(pass);
    destroyAccList(a_lst);
    return 0;
}

void write_to_file(char* pass) {
     /*Opening credential file*/
    FILE* cred_file = fopen(CREDS_FILE, "w");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return;
    }

    /*Getting key*/
    int key = encrypt_key(pass);

    char buff[100];
    Acc_node n = NULL;
    for(n = a_lst->head; n != NULL; n = n->next) {
        snprintf(buff, 99, "%s:%s\n", n->name, n->password);
        encrypt_line(key, buff);
        fprintf(cred_file, "%s", buff);
    }
    
    fclose(cred_file);
}   

int initialize_accounts(char* pass) {
    /*Opening credential file*/
    FILE* cred_file = fopen(CREDS_FILE, "r");
    if (cred_file == NULL) {
        perror("Error opening credential file ");
        return -1;
    }
    
    /*Getting key*/
    int key = encrypt_key(pass);
    char* name;
    char* passwd;
    
    /*--Account list---*/
    a_lst = createAccList();
    int index;
    char buff[100];
    while(fgets(buff, 99, cred_file) != NULL) {
        index = strcspn(buff, "\n");
        buff[index] = '\0';
        decrypt_line(key, buff);
        name = strtok(buff, ":");
        if (name == NULL) {
            printf("Error parsing credential file(1)\n");
            return -1;
        }
        else {
            passwd = strtok(NULL, ":");
            if(passwd == NULL) {
                printf("Error parsing credential file(2)\n");
                return -1;
            }
        }
        
        insert_acc(a_lst, name, passwd);
    } 
    
    fclose(cred_file);
    return 0;
}

int get_password(char* buff, int buff_len) {

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
        printf("%s: \n", tries == 3 ? "Enter Master Password " : "Wrong Password Try Again ");
        fgets(buff, buff_len, stdin);
        tries--;
        if(strcmp(buff, "Kapila.707403\n") == 0) {
            break;
        }
    }
    while(tries > 0);

    #ifdef _WIN32
        // Restore original console mode
        SetConsoleMode(hStdin, mode);
    #else
         // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
    if(tries == 0) {
        printf("Too many tries");
        return -1;
    }
    return 0;
}

void change_pass_from_user(char* pass_for) {
    char pass[50];
    printf("Enter the password for %s: ", pass_for);
    scanf("%49s", pass);
    change_pass(pass_for, pass);
}

void add_pass_from_user(char* pass_for) {
    char pass[50];
    printf("Enter the password for %s: ", pass_for);
    scanf("%49s", pass);
    create_pass(pass_for, pass);
}

void change_pass(char* pass_for, char* password) {
    if(search_acc(a_lst, pass_for) != 0) {
        printf("Account doesn't exist\n");
        return;
    }
    
    if(password == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        printf("New Password: %s\n", passwd);

        if(change_passwd(a_lst, pass_for, passwd) == 0) printf("Password Changed successfully\n");
        else printf("Error changing Password\n");
        
    }
    else {
        if(change_passwd(a_lst, pass_for, password) == 0) printf("Error changing password\n");
        else printf("Error changing Password\n");
        printf("New Password: %s\n", password);
    }
}

void delete_account(char* account) {
    if(search_acc(a_lst, account) != 0) {
        printf("Account doesn't exist\n");
        return;
    }

    if(delete_acc(a_lst, account) == 0 )
        printf("Account Deleted\n");
    else 
        printf("Not Deleted\n");
    
}

void create_pass(char* pass_for, char* pass) {

    if(search_acc(a_lst, pass_for) == 0) {
        printf("Password For %s already exists\n", pass_for);
        return;
    }

    if(pass == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        printf("New Password for %s: %s\n", pass_for, passwd);
        
        if(insert_acc(a_lst, pass_for, passwd) == 0){
            printf("Account added successfully\n");
        } 
        else {
            printf("Error inserting account\n");
        } 
        
    }
    else {
        printf("New Password for %s: %s\n", pass_for, pass);
         
        if(insert_acc(a_lst, pass_for, pass) == 0){
            printf("Account added successfully\n");
        } 
        else {
            printf("Error inserting account\n");
        } 
    }
    
}

void get_pass(char* pass_for) {
    Acc_node n = NULL;

    for(n = a_lst->head; n != NULL; n = n->next) {
        if(strcmp(n->name, pass_for) == 0) {
            printf("Password for %s: %s\n", pass_for, n->password);
            return;
        }
    }
    printf("Account doesn't exist\n");
}

void list_accounts() {
    Acc_node n = NULL;

    for(n = a_lst->head; n != NULL; n = n->next) {
        printf("Acc: %s\n", n->name);
        printf("Pass: %s\n", n->password);
        printf("\n");
    }
    
}

int handle_input(int argc, char* argv[], char* input_buff, int buff_size) {
    
    char* help_message = "Usage: ./cman -n <account> to generate new password for given account\n"
                         "OR:    ./cman -a <account> to add a password for a given account\n"
                         "OR:    ./cman -s <account> to search and get password for given account\n"
                         "OR:    ./cman -c <account> to change password for given account. Password generated from program\n"
                         "OR:    ./cman -o <account> to change password for given account. Password given by user.\n"
                         "OR:    ./cman -l to list all accounts and passwords\n"
                         "OR:    ./cman -d <account> to delete account given\n"
                         "OR:    ./cman -i to enter interactive mode\n"
                         "OR:    ./cman -h to display this message";

                            
    if (argc < 2 || argc > 3) {
        printf("%s", help_message);
        return -1;
    }

    int flags = 0;

    for (int i = 0; i < argc; i++) {
        char* p = strchr(argv[i], '-');
        if (p != NULL) {
            switch(*(p+1)){
                case 'n': flags = flags | n_flag; break;
                case 's': flags = flags | s_flag; break;
                case 'c': flags = flags | c_flag; break;
                case 'l': flags = flags | l_flag; break;
                case 'd': flags = flags | d_flag; break;
                case 'i': flags = flags | i_flag; break;
                case 'a': flags = flags | a_flag; break;
                case 'o': flags = flags | o_flag; break;
                case 'h': printf("%s", help_message); return -1;
                default: printf("Unknown option: %c\n%s", *(p+1), help_message); return -1;
            }
        }
    }

    int input_flags[] = {n_flag, s_flag, c_flag, l_flag, d_flag, i_flag, a_flag, o_flag};
    int num_flags = sizeof(input_flags) / sizeof(input_flags[0]);
    int correct_inp = 0;

    for(int i = 0; i < num_flags; i++) {
        if(flags == input_flags[i]) {
            correct_inp = 1;
            break;
        }
    }

    if (correct_inp == 0) {
        printf("%s", help_message);
        return -1;
    }
    else if(flags & l_flag || flags & i_flag) {
        return flags;
    }
    else{
        if (argc != 3) {
            printf("%s", help_message);
            return -1;
        }
        int input_len = strlen(argv[2]);

        if (input_len > buff_size) {
            printf("The input string is too long\n");
            return -1;
        }

        snprintf(input_buff, buff_size, "%s", argv[2]); 
    }

    return flags;
}

void get_pass_string(char* buff, int buff_size) {
    memset(buff, 0, buff_size);
    srand(time(NULL));
    int num;
    int alpha = 0;
    int numeric = 0;
    int s_char = 0;

    for(int i = 0; i < buff_size; i++) {
        num = rand() % 88 + 35; // random number between 35 and 122
        if((num>=37&&num<=47) || (num>=58&&num<=63) ||(num>=91&&num<=96)) {
            i--;
            continue;
        }       

        if(isalpha(num)) alpha++;
        else if(isdigit(num)) numeric++;

        if(alpha > 8) {
            num = rand() % 10 + 48; //choose a number
            numeric++;
            if(numeric > 5) {
                s_char = rand() % 3;
                switch(s_char) {
                    case 0: num = 35; break; //#
                    case 1: num = 36; break; //$
                    case 2: num = 64; break;//@
                }
            }
        }

        buff[i] = num;
    }
    
    buff[buff_size-1] = '\0';
}