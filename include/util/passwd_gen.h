#ifndef PASSWD_H
#define PASSWD_H

/*---------CREDEENTIAL BUFFER SIZE DEFINITIONS---------*/
#define PASSWORD_LENGTH 16
#define CRED_BUFF_LEN 128
#define BATCH_FILE_LINE_LEN ((CRED_BUFF_LEN) * 3 + 3)

int gen_pass_string(char *buff, int buff_size);

#endif
