#include <sodium.h>
#include <stdio.h>
#include <string.h>

void encrypt_file(int key, FILE *file);
int encrypt_key(char *pass);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./encrypt <file>\n");
        return -1;
    }

    FILE *file = fopen(argv[1], "rw");

    char pass[30];
    printf("Enter key: ");
    scanf("%s", pass);
    if (file == NULL) {
        perror("Error opening file ");
        return -1;
    }

    encrypt_file(encrypt_key(pass), file);
}

void encrypt_file(int key, FILE *file) {
    FILE *temp = fopen("enout.txt", "w");

    if (temp == NULL) {
        perror("Error opening file");
        return;
    }

    int offset = key % 128;
    char buff[100];
    int str_length;
    while (fgets(buff, 99, file) != NULL) {
        str_length = strlen(buff);
        for (int i = 0; i < str_length; i++) {
            if (buff[i] == '\n')
                continue;
            buff[i] = buff[i] + offset;
        }
        fprintf(temp, "%s", buff);
    }

    fclose(temp);
}

int encrypt_key(char *pass) {
    int string_length = strlen(pass);
    int sum = 0;

    for (int i = 0; i < string_length; i++) {
        sum += pass[i];
    }

    return sum;
}
