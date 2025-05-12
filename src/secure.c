#include <openssl/evp.h>
#include <openssl/rand.h>
#include <crypt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../includes/secure.h"

#define SALT_SIZE 22
#define AES_BLOCK_SIZE 16
#define CHUNK_SIZE 20

const unsigned char* key;
const unsigned char* iv;

void set_key(const unsigned char* the_key) {
    key = the_key;
}

void set_iv(const unsigned char* the_iv) {
    iv = the_iv;
}

int encrypt_file(char* filename) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if(ctx == NULL) {
        printf("Error setting up the EVP_CTX\n");
        return -1;
    }

    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    if(cipher == NULL) {
        printf("Error setting up Cipher\n");
        return -1;
    }

    int status;
    status = EVP_EncryptInit_ex2(ctx, cipher, key, iv, NULL);
    if(status < 1) {
        printf("Error setting up encryption\n");
        return -1;
    }
   
    FILE* file = fopen(filename, "rb");
    FILE* out = fopen(TEMPFILE, "wb");
    FILE* bkp = fopen(BACKUP, "wb");

    if(file == NULL || out == NULL || bkp == NULL) {
        perror("Error opening file ");
        return -1;
    }

    unsigned char plaintext[CHUNK_SIZE];
    unsigned char ciphertext[CHUNK_SIZE + AES_BLOCK_SIZE];
    int read_bytes, cipher_len, total_len;

    while((read_bytes = fread(plaintext, 1, CHUNK_SIZE, file)) > 0) {
        EVP_EncryptUpdate(ctx, ciphertext, &cipher_len, plaintext, read_bytes);
        total_len += cipher_len;
        fwrite(ciphertext, 1, cipher_len, out);
        fwrite(ciphertext, 1, cipher_len, bkp);
    }

    EVP_EncryptFinal(ctx, ciphertext, &cipher_len);
    total_len += cipher_len;
    fwrite(ciphertext, 1, cipher_len, out);
    fwrite(ciphertext, 1, cipher_len, bkp);

    if(rename(TEMPFILE, filename) != 0) {
        perror("Error changing file: ");
        return -1;
    }

    unlink(TEMPFILE);

    fclose(file);
    fclose(out);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free((EVP_CIPHER*)cipher);

    return 0;
}

int decrypt_file(char* filename) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if(ctx == NULL) {
        printf("Error setting up the EVP_CTX\n");
        return -1;
    }

    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    if(cipher == NULL) {
        printf("Error setting up Cipher\n");
        return -1;
    }

    int status;
    status = EVP_DecryptInit_ex2(ctx, cipher, key, iv, NULL);

    if (status < 1) {
        printf("Error setting up decryption\n");
        return -1;
    }

    FILE* ifile = fopen(filename, "rb");
    FILE* ofile = fopen(TEMPFILE, "wb");
    if(ifile == NULL || ofile == NULL) {
        perror("Error opening file ");
        return 0;
    }
    
    int read_bytes, plain_len, total_len;
    unsigned char plaintext[CHUNK_SIZE];
    unsigned char ciphertext[CHUNK_SIZE];

    while((read_bytes = fread(ciphertext, 1, CHUNK_SIZE, ifile)) > 0) {
        EVP_DecryptUpdate(ctx, plaintext, &plain_len, ciphertext, read_bytes);
        total_len += plain_len;
        fwrite(plaintext, 1, plain_len, ofile);
    }

    EVP_DecryptFinal(ctx, plaintext, &plain_len);
    total_len += plain_len;
    fwrite(plaintext, 1, plain_len, ofile);

    if(rename(TEMPFILE, filename) != 0) {
        perror("Error changing file: ");
        return -1;
    }

    unlink(TEMPFILE);
    
    fclose(ofile);
    fclose(ifile);

    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free((EVP_CIPHER*)cipher);
    return 0;
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