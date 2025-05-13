#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
    #include <termios.h>
#endif

#include "../includes/secure.h"


int handle_input(int argc, char* argv[], char* input_buff, int buff_size);
void create_pass(char* pass_for, char* pass);
void list_accounts();
int account_exists(char* account);
void get_pass(char* passwd_for);
void change_pass(char* pass_for, char* pass);
void get_pass_string(char* buff, int buff_size);
void delete_account(char* account);
void change_pass_from_user(char* pass_for);
void add_pass_from_user(char* pass_for);


#ifdef _WIN32
    #define CREDS_FILE "C:\\Documents\\creds.txt"
#else
   // #define CREDS_FILE "/home/kapila/Windows_Files/Documents/creds.txt"
   #define CREDS_FILE "../creds.txt"
#endif

/*----------input flags----------*/
#define n_flag 0x01  //00000001
#define s_flag 0x02  //00000010
#define c_flag 0x04  //00000100
#define l_flag 0x08  //00001000
#define d_flag 0x10  //00010000
#define i_flag 0x20  //00100000
#define a_flag 0x40  //01000000
#define o_flag 0x80  //10000000


/*The File Pointer for the credit file*/
FILE* creds_file = NULL;

int main(int argc, char* argv[]) {
    /*#ifndef _WIN32
    //Removing echo on terminal
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag = ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    #endif

    char pass[20];
    
    char* env = getenv("CMP");
    if(env == NULL) {
        printf("Cannot get hash\n");
        return -1;
    }
    int tries = 3;
    char* hash = NULL;
    do {
        printf("%s: \n", tries == 3 ? "Enter Master Password" : "Wrong Password Try Again");
        scanf("%19s", pass);
        tries--;
        if(strcmp(env, hash) == 0) {
            break;
        }
    }
    while(tries > 0);

    if(tries == 0) {
        printf("Too many tries\n");
        #ifndef _WIN32 
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 
        #endif
        return -1;
    }
    
    #ifndef _WIN32
    //Restoring echo on terminal
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
    */

    /*Opening credential file*/
    creds_file = fopen(CREDS_FILE, "rw");
    if (creds_file == NULL) {
        perror("Error opening credential file ");
        return -1;
    }
    
    char input_buff[100];
    memset(input_buff, 0, 100);
    int flags = handle_input(argc, argv, input_buff, 99);
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
    if(account_exists(pass_for) == 0) {
        printf("Account doesn't exist\n");
        return;
    }

    FILE* temp;
    FILE* file = fopen(CREDS_FILE, "r");
    
    temp = fopen(TEMPFILE, "a+");

    if(file == NULL || temp == NULL) {
        perror("Error opening file: ");
        return;
    }

    char buff[100];
    char* acc = NULL;
    char* pass = NULL;

    while(fgets(buff, 100, file) != NULL) {
        acc = strtok(buff, ":");
        pass = strtok(NULL, "\n");

        if(strcmp(acc, pass_for) == 0) {
            continue;
        }
        fprintf(temp, "%s:%s\n", acc, pass);
    }

   
    if(password == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        fprintf(temp, "%s:%s\n", pass_for, passwd);
        printf("New Password: %s\n", passwd);
    }
    else {
        fprintf(temp, "%s:%s\n", pass_for, password);
        printf("New Password: %s\n", password);
    }
    fclose(temp);
    fclose(file);

    if(remove(CREDS_FILE) != 0) {
        perror("Error changing password(1) ");
        return;
    }
    if(rename(TEMPFILE, CREDS_FILE) != 0) {
        perror("Error changing password(2) ");
        return;
    }
}

void delete_account(char* account) {
    if(account_exists(account) == 0) {
        printf("Account doesn't exist\n");
        return;
    }

    FILE* file = fopen(CREDS_FILE, "r");
    FILE* temp =  fopen(TEMPFILE, "a+");


    if(file == NULL || temp == NULL) {
        perror("Error opening file: ");
        return;
    }

    char buff[100];
    char* acc = NULL;
    char* pass = NULL;

    while(fgets(buff, 100, file) != NULL) {
        acc = strtok(buff, ":");
        pass = strtok(NULL, "\n");
        if(strcmp(acc, account) == 0) {
            continue;
        }
        fprintf(temp, "%s:%s\n", acc, pass);
    }

    fclose(temp);
    fclose(file);

    if(remove(CREDS_FILE) != 0) {
        perror("Error changing password(1): ");
        return;
    }
    if(rename(TEMPFILE, CREDS_FILE) != 0) {
        perror("Error changing password(2): ");
        return;
    }
    
    printf("Account Deleted\n");
}

void create_pass(char* pass_for, char* pass) {
    FILE* file = NULL;
    file = fopen(CREDS_FILE, "a+");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    if(account_exists(pass_for)) {
        printf("Password For %s already exists\n", pass_for);
        return;
    }

    if(pass == NULL) {
        char passwd[16];
        get_pass_string(passwd, 16);
        printf("New Password for %s: %s\n", pass_for, passwd);
        fprintf(file, "%s:%s\n", pass_for, passwd);
    }
    else {
        printf("New Password for %s: %s\n", pass_for, pass);
        fprintf(file, "%s:%s\n", pass_for, pass);
    }
   
    fclose(file);
}

void get_pass(char* pass_for) {
    char buff[100];
    char* acc = NULL;
    char* pass = NULL;

    FILE* file = fopen(CREDS_FILE, "r");
    if (file == NULL) {
        perror("Error opening credential file: ");
        return;
    }
    
    while(fgets(buff, 100, file) != NULL) {
        acc = strtok(buff, ":");
        pass = strtok(NULL, "\n");
    
        if(strcmp(pass_for, acc) == 0) {
            printf("Password For %s: %s", pass_for, pass);
            return;
        }
    }
    printf("Account doesn't exist\n");
}

int account_exists(char* account) {
    FILE* file = fopen(CREDS_FILE, "r");
    if (file == NULL) {
        perror("Error opening credential file: ");
        return -1;
    }

    char buff[200];
    char* acc = NULL;

    while(fgets(buff, 199, file) != NULL) {
        acc = strtok(buff, ":");
        if(strcmp(account, acc) == 0) {
            return 1;
        }
    }
    
    return 0;
    fclose(file);
}

void list_accounts() {
    FILE* file = fopen(CREDS_FILE, "r");
    if (file == NULL) {
        perror("Error opening credential file: ");
        return;
    }
    char buff[200];
    char* acc = NULL;
    char* pass = NULL;

    while(fgets(buff, 199, file) != NULL) {
        acc = strtok(buff, ":");
        pass = strtok(NULL, "\n");
        
        printf("Acc:  %s\n", acc);
        printf("Pass: %s\n", pass);
        printf("\n");
    }
    
    fclose(file);
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