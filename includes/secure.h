#ifndef SECURE_H 
#define SECURE_H

#ifdef _WIN32
#define BACKUP "C:\\Log\\wcred.txt"
#define TEMPFILE "C:\\Log\\temp.txt"
#else
#define BACKUP "/home/kapila/.creds.txt"
#define TEMPFILE "/home/kapila/Windows_Files/temp.txt"
#endif

char* get_key(char* pass);
int encrypt_file(char* filename);
int decrypt_file(char* filename);
void set_key(const unsigned char* the_key);
void set_iv(const unsigned char* the_iv);

#endif