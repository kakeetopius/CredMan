#ifndef SECURE_H 
#define SECURE_H



int encrypt_key(char* pass);
void encrypt_line(int key, char* buff);
void decrypt_line(int key, char* buff);



#endif
