#include <stdio.h>
#include <string.h>

void decrypt_file(int key, FILE *file);
int encrypt_key(char *pass);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./decrypt <file>\n");
        return -1;
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Error opening file ");
        return -1;
    }

    char pass[30];
    printf("Enter key: ");
    scanf("%s", pass);
    if (file == NULL) {
        perror("Error opening file ");
        return -1;
    }

    decrypt_file(encrypt_key(pass), file);
}

void decrypt_file(int key, FILE *file) {
    FILE *temp = fopen("dout.txt", "w");

    if (temp == NULL) {
        perror("Error opening file");
        return;
    }

    int offset = key % 128;
    int str_length;
    char buff[100];

    while (fgets(buff, 99, file)) {
        str_length = strlen(buff);
        for (int i = 0; i < str_length; i++) {
            if (buff[i] == '\n')
                continue;
            buff[i] = buff[i] - offset;
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
