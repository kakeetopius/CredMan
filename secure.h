#ifndef SECURE_H
#define SECURE_H

char* get_key(char* pass);
int encrypt_file(char* filename);
int decrypt_file(char* filename);
void set_key(const unsigned char* the_key);
void set_iv(const unsigned char* the_iv);

#endif