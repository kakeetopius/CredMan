#include <stdio.h>
#include <string.h>

int encrypt_key(char* pass);
void encrypt_file(int key);
void decrypt_file(int key);

int main(void) {
    char pass[30];
    printf("Enter the master password: ");
    scanf("%29s", pass);
    int enkey = encrypt_key(pass);
    decrypt_file(enkey);
    FILE* test = fopen("../creds.txt", "r");
}

int encrypt_key(char* pass) {
    int string_length = strlen(pass);
    int sum = 0;
    
    for(int i = 0; i < string_length; i++) {
        sum += pass[i];
    }

    return sum;
}

void encrypt_file(int key) {
    FILE* file = fopen("../creds.txt", "r");
    FILE* temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL){  
        perror("Error opening file");
        return;
    } 

    int offset = key % 128;
    char buff[100];
    int str_length;
    while(fgets(buff, 99, file) != NULL) {
        str_length = strlen(buff);
        for(int i = 0; i < str_length; i++) {
            buff[i] = buff[i] + offset;
        }
        fprintf(temp, "%s", buff);
    }

    fclose(file);
    fclose(temp);
}

void decrypt_file(int key) {
    FILE* temp = fopen("temp.txt", "r");
    FILE* file = fopen("temp.text", "w");

    if (file == NULL || temp == NULL){  
        perror("Error opening file");
        return;
    } 

    int offset = key % 128;
    int str_length;
    char buff[100];

    while(fgets(buff, 99, temp)) {
        str_length = strlen(buff);
        for (int i = 0; i < str_length; i++) {
            buff[i] = buff[i] - offset;
        }
        fprintf(file, "%s", buff);
    }

    fclose(file);
    fclose(temp);


}