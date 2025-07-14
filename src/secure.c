#include <string.h>
#include "../includes/secure.h"

int encrypt_key(char* pass) {
    int string_length = strlen(pass);
    int sum = 0;
    
    for(int i = 0; i < string_length; i++) {
        if (pass[i] == '\n') continue;
        sum += pass[i];
    }
    return sum;
}


void encrypt_line(int key, char* buff) {
    int offset = key % 128;
    int str_length = strlen(buff);

    for(int i = 0; i < str_length; i++) {
        if (buff[i] == '\n') continue;
        buff[i] = buff[i] + offset;
    }
}

void decrypt_line(int key, char* buff) {
    int offset = key % 128;
    int str_length = strlen(buff);

    for (int i = 0; i < str_length; i++) {
        if (buff[i] == '\n') continue;
        buff[i] = buff[i] - offset;
    }
}
