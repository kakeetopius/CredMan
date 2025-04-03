#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include "secure.h"


int handle_input(int argc, char* argv[], char* input_buff, int buff_size);
void create_pass(char* pass_for);
void list_accounts();
void split(char* tosplit, char* arr1, int arr1_size, char* arr2, int arr2_size, char delimiter);
int account_exists(char* account);
void get_pass(char* passwd_for);
void change_pass(char* passwd_for);
void get_pass_string(char* buff, int buff_size);
void delete_account(char* account);


#ifdef _WIN32
    #define CREDS_FILE "C:\\Documents\\creds.txt"
#else
    #define CREDS_FILE "/home/kapila/Windows_Files/Documents/creds.txt"
#endif

/*----------input flags----------*/
#define n_flag 0x01  //00000001
#define s_flag 0x02  //00000010
#define c_flag 0x04  //00000100
#define l_flag 0x08  //00001000
#define d_flag 0x10  //00010000

int main(int argc, char* argv[]) {
    /*Removing echo on terminal*/
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag = ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);


    char pass[20];
    printf("Enter Master password: \n");
    scanf("%19s", pass);
    /*Restoring echo on terminal*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    char* hash = get_key(pass);
    char* env = getenv("CMP");
    if(env == NULL) {
        printf("Cannot get hash\n");
        return -1;
    }
    if(strcmp(env, hash) != 0) {
        printf("Wrong Password\n");
        exit(-1);
    }

    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};

    memcpy(key, hash, 16);
    memcpy(iv, (hash+17), 16);

    set_key(key);
    set_iv(iv);
    int stat;

    stat = decrypt_file(CREDS_FILE);
    if (stat != 0) {
        printf("Error decrypting\n");
        return 0;
    }

    char input_buff[100];
    memset(input_buff, 0, 100);
    int flags = handle_input(argc, argv, input_buff, 99);
    
    if(flags == -1) {
        return -1;
    }
    else if(flags & n_flag) {
        create_pass(input_buff);
    }
    else if(flags & l_flag) {
        list_accounts();
    }
    else if(flags & s_flag) {
        get_pass(input_buff);
    }
    else if (flags & c_flag) {
        #ifdef _WIN32
            printf("Cannot change password on Windows\n");
            return 0;
        #else
        char option[10];
        printf("Are you sure you want to change password for %s: ", input_buff);
        scanf("%s", option);
        if(strcmp(option, "yes") == 0)
            change_pass(input_buff);
        else 
            printf("Not changed\n");
        #endif
    }
    else if(flags & d_flag) {
        #ifdef _WIN32
            printf("Cannot delete account on Windows\n");
            return 0;
        #else
        char option[10] = {0};
        printf("Are you sure you want to delete this account: ");
        scanf("%s", option);
        if(strcmp(option, "yes") == 0) {
            delete_account(input_buff);
        }
        else{
            printf("Not Deleted\n");
        }
        #endif
    }
    
    stat = encrypt_file(CREDS_FILE);
    if(stat != 0) {
        printf("Error encrypting\n");
    }
    return 0;
}

void change_pass(char* pass_for) {
    if(account_exists(pass_for) == 0) {
        printf("Account doesn't exist\n");
        return;
    }

    FILE* temp;
    FILE* file = fopen(CREDS_FILE, "r");
    
    #ifdef _WIN32
        temp = fopen("temp.txt", "a+");
    #else
        char* temp_file = "/home/kapila/Windows_Files/temp.txt";
        temp = fopen(temp_file, "a+");
    #endif

    if(file == NULL || temp == NULL) {
        perror("Error opening file: ");
        return;
    }

    char buff[100];
    char acc[50];
    char pass[50];

    while(fgets(buff, 100, file) != NULL) {
        split(buff, acc, 50, pass, 50, ':');
        if(strcmp(acc, pass_for) == 0) {
            continue;
        }
        fprintf(temp, "%s:%s", acc, pass);
    }

    
    char passwd[16];

    get_pass_string(passwd, 16);

    fprintf(temp, "%s:%s\n", pass_for, passwd);
    printf("New Password: %s\n", passwd);
    fclose(temp);
    fclose(file);

    #ifdef _WIN32
        unlink(CREDS_FILE);
        if(rename("temp.txt", CREDS_FILE) != 0) {
            perror("Error changing file: ");
            return;
        }
    #else
        if(rename(temp_file, CREDS_FILE) != 0) {
            perror("Error changing file: ");
            return;
        }
        unlink(temp_file);
    #endif
    
}

void delete_account(char* account) {
    if(account_exists(account) == 0) {
        printf("Account doesn't exist\n");
        return;
    }

    FILE* file = fopen(CREDS_FILE, "r");
    FILE* temp;

    #ifdef _WIN32
        temp = fopen("temp.txt", "a+");
    #else
        char* temp_file = "/home/kapila/Windows_Files/temp.txt";
        temp = fopen(temp_file, "a+");
    #endif


    if(file == NULL || temp == NULL) {
        perror("Error opening file: ");
        return;
    }

    char buff[100];
    char acc[50];
    char pass[50];

    while(fgets(buff, 100, file) != NULL) {
        split(buff, acc, 50, pass, 50, ':');
        if(strcmp(acc, account) == 0) {
            continue;
        }
        fprintf(temp, "%s:%s", acc, pass);
    }

    fclose(temp);
    fclose(file);

    #ifdef _WIN32
        if(rename("temp.txt", CREDS_FILE) != 0) {
            perror("Error changing file: ");
            return;
        }
        remove(CREDS_FILE);
    #else
    if(rename(temp_file, CREDS_FILE) != 0) {
        perror("Error changing file: ");
        return;
    }
    unlink(temp_file);
    #endif
    
    printf("Account Deleted\n");
}

void create_pass(char* pass_for) {
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
    char passwd[16];

    get_pass_string(passwd, 16);
    printf("New Password for %s: %s\n", pass_for, passwd);
    fprintf(file, "%s:%s\n", pass_for, passwd);
    fclose(file);
}

void get_pass(char* pass_for) {
    char buff[100];
    char acc[50];
    char pass[50];

    FILE* file = fopen(CREDS_FILE, "r");
    if (file == NULL) {
        perror("Error opening credential file: ");
        return;
    }
    
    while(fgets(buff, 100, file) != NULL) {
        split(buff, acc, 50, pass, 50, ':');
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
    char acc[50];
    char pass[50];

    while(fgets(buff, 199, file) != NULL) {
        split(buff, acc, 50, pass, 50, ':');
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
    char acc[50];
    char pass[50];

    while(fgets(buff, 199, file) != NULL) {
        split(buff, acc, 50, pass, 50, ':');
        printf("Acc:  %s\n", acc);
        printf("Pass: %s\n", pass);
    }
    fclose(file);
}

void split(char* tosplit, char* arr1, int arr1_size, char* arr2, int arr2_size, char delimiter) {
    int string_len = strlen(tosplit);
    memset(arr1, 0, arr1_size);
    memset(arr2,0, arr2_size);
    char* p = strchr(tosplit, delimiter);
    if(p == NULL) {
        return;
    }

    int delimiter_pos = p - tosplit;
    int i;
    for(i = 0; i < delimiter_pos; i++) {
        if (i == arr1_size) break;
        arr1[i] = tosplit[i];
    } 
    if (i == arr1_size) 
        arr1[i-1] = '\0';
    else 
        arr1[i] = '\0';

    i = 0;
    for (int j = delimiter_pos+1; j<string_len; j++,i++) {
        if(i == arr2_size) break;
        arr2[i] = tosplit[j];
    }
    if(i == arr2_size)
        arr2[i-1] = '\0';
    else
        arr2[i] = '\0';
}

int handle_input(int argc, char* argv[], char* input_buff, int buff_size) {
    
    char* help_message = "Usage: ./cred_manager -n <account> to generate new password for given account\n"
                         "OR:    ./cred_manager -s <account> to search and get password for given account\n"
                         "OR:    ./cred_manager -c <account> to change password for given account\n"
                         "OR:    ./cred_manager -l to list all accounts and passwords\n"
                         "OR:    ./cred_manager -h to display this message\n"
                         "OR:    ./cred_manager -d <account>to delete account given";

                            
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
                case 'h': printf("%s", help_message); return -1;
                default: printf("Unknown option: %c\n%s", *(p+1), help_message); return -1;
            }
        }
    }


    if (flags != c_flag && flags!= s_flag && flags != n_flag && flags != l_flag && flags != d_flag) {
        printf("%s", help_message);
        return -1;
    }
    else if(flags & l_flag) {
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