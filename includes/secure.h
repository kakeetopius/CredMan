#ifndef SECURE_H 
#define SECURE_H

#include <stdio.h>

int encrypt_key(char* pass);
void decrypt_file(int key, FILE* file);
void encrypt_file(int key, FILE* file);



#endif