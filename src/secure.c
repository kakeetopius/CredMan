#include <stdio.h>
#include <string.h>
#include "../includes/secure.h"
#include "../includes/main.h"


int encrypt_key(char* pass) {
    int string_length = strlen(pass);
    int sum = 0;
    
    for(int i = 0; i < string_length; i++) {
        sum += pass[i];
    }

    return sum;
}

void encrypt_file(int key, FILE* file) {
    FILE* temp = fopen(TEMPFILE, "w");

    if (temp == NULL){  
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

    fclose(temp);
}

void decrypt_file(int key, FILE* file) {
    FILE* temp = fopen(TEMPFILE, "w");


    if (temp == NULL){  
        perror("Error opening file");
        return;
    } 

    int offset = key % 128;
    int str_length;
    char buff[100];

    while(fgets(buff, 99, file)) {
        str_length = strlen(buff);
        for (int i = 0; i < str_length; i++) {
            buff[i] = buff[i] - offset;
        }
        fprintf(temp, "%s", buff);
    }
    fclose(temp);
}