#include <stdio.h>
#include <string.h>

void clean_line(char *str, int len);

int main(void) {
    FILE *file = fopen("dout.txt", "r");
    if (!file) {
        printf("Error opening file\n");
        return -1;
    }

    FILE *out = fopen("creds.txt", "w");
    if (!file) {
        printf("Error printing file\n");
        return -1;
    }

    char buff[100];

    while (fgets(buff, 100, file)) {
        clean_line(buff, strlen(buff));
        fprintf(out, "%s\n", buff);
    }
}

void clean_line(char *str, int len) {
    for (int i = 0; i < len; i++) {
        if (str[i] == '\r')
            str[i] = '\n';
        if (str[i] == '\n')
            str[i] = '\0';
    }
}
