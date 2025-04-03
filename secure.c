#include <openssl/evp.h>
#include <openssl/rand.h>
#include <crypt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SALT_SIZE 22
#define AES_BLOCK_SIZE 16
#define CHUNK_SIZE 20

char* get_key(char* pass);

int main(void) {
    char pass[20];
    printf("Enter Master password: ");
    scanf("%19s", pass);

    char* hash = get_key(pass);
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};

    memcpy(key, (hash+10), 16);
    memcpy(iv, (hash+24), 16);

}

char* get_key(char* pass) {
    char c_slt[SALT_SIZE];

    /*Getting salt*/
    for(int i = 0, j = 87; i < SALT_SIZE; i++, j--) {
        c_slt[i] = j;
    }
    c_slt[SALT_SIZE-1] = '\0';

    struct crypt_data cpd = {0};
    /*Generating salt*/
    char* slt = crypt_gensalt_rn 
                (
                    "$2b$",
                    12,
                    c_slt,
                    SALT_SIZE,
                    cpd.setting,
                    CRYPT_OUTPUT_SIZE
                );

    if (slt == NULL) {
        printf("Error generating salt\n");
        return NULL;
    }
    
    /*creating hash*/
    snprintf(cpd.input, sizeof(cpd.input), "%s", pass);

    char* hsh = crypt_r 
                (
                    cpd.input,
                    cpd.setting,
                    &cpd
                );

    if(hsh == NULL) {
        printf("Error generating hash\n");
        return NULL;
    }

    return strdup(cpd.output);
}