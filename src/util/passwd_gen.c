#include <stdio.h>
#include <string.h>

#include "util/errors.h"
#include "util/passwd_gen.h"

int gen_pass_string(char *pass_buff, int buff_size) {
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		     "abcdefghijklmnopqrstuvwxyz"
		     "1234567890"
		     "!@#$%&^*()";
    int charset_size = sizeof(charset) - 1; // minus to not consider null terminator.

    if (!pass_buff) {
	printf("Buff is NULL\n");
	return GENERAL_ERROR;
    }

    memset(pass_buff, 0, buff_size);

#ifdef _WIN32
    srand(time(NULL));
    int random_bytes[PASSWORD_LENGTH];
    srand(time(NULL));
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	random_bytes[i] = rand() % charset_size;
    }

    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	pass_buff[i] = charset[random_bytes[i]];
    }
    return SUCCESS_OP;
#else
    FILE *urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
	perror("Error opening /dev/urandom");
	return GENERAL_ERROR;
    }

    unsigned char random[PASSWORD_LENGTH];
    int read_numbers = fread(random, sizeof(char), PASSWORD_LENGTH, urandom);

    if (read_numbers < PASSWORD_LENGTH) {
	printf("Error reading from /dev/urandom\n");
	fclose(urandom);
	return GENERAL_ERROR;
    }

    int pass_index;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
	pass_index = random[i] % charset_size;
	pass_buff[i] = charset[pass_index];
    }

    fclose(urandom);
    return SUCCESS_OP;
#endif
}
