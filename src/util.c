#include "../includes/util.h"
#include "../includes/db_access.h"
#include "../includes/error_messages.h"

#include <string.h>
#include <stdio.h>

int print_help(char* subcommand) {
    if (strcmp(subcommand, "add") == 0) {
	printf("%s", ADD_MESSAGE);
    }
    else if (strcmp(subcommand, "change") == 0) {
	printf("%s", CHANGE_MESSAGE);
    }
    else if (strcmp(subcommand, "delete") == 0) {
	printf("%s", DELETE_MESSAGE);
    } 
    else if (strcmp(subcommand, "search") == 0) {
	printf("%s", SEARCH_MESSAGE);
    }
    else if (strcmp(subcommand, "ls") == 0) {
	printf("%s", LS_MESSAGE);
    }
    else {
	printf("Unknown subcommand: %s\n", subcommand);
	printf("%s", GENERAL_MESSAGE);
	return -1;
    }
    
    return 0;
}

int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret) {

    struct termios oldt; // to store old terminal settings
    // Remove echo when typing input.
    if (secret == 1)
	set_secure_input(&oldt);

    char temp_buff[64];
    printf("%s: ", prompt);
    fgets(temp_buff, sizeof(temp_buff), stdin);
    printf("\n");
    int temp_buff_strlen;

    // if some text remained in stdin
    if (strchr(temp_buff, '\n') == NULL) {
	printf("Limit input to %d characters\n", buff_len);
	flush_stdin();
	if (secret == 1)
	    remove_secure_input(&oldt);
	return GENERAL_ERROR;
    } else {
	/*removing new line character*/
	int new_line_pos = strcspn(temp_buff, "\n");
	temp_buff[new_line_pos] = '\0';
	temp_buff_strlen = strlen(temp_buff);

	if (temp_buff_strlen == 0) {
	    printf("Input can't be Empty\n");
	    if (secret == 1)
		remove_secure_input(&oldt);
	    return GENERAL_ERROR;
	}

	if (confirm == 1) {
	    printf("Enter again to confirm: ");
	    char temp_buff2[64];
	    fgets(temp_buff2, sizeof(temp_buff2), stdin);
	    printf("\n");
	    // if input is too long the second time
	    if (strchr(temp_buff2, '\n') == NULL) {
		flush_stdin();
		printf("Inputs don't match\n");
		if (secret == 1)
		    remove_secure_input(&oldt);
		return GENERAL_ERROR;
	    } else {
		temp_buff2[strcspn(temp_buff2, "\n")] = '\0';
		if (strcmp(temp_buff, temp_buff2) != 0) {
		    printf("Inputs don't match\n");
		    if (secret == 1)
			remove_secure_input(&oldt);
		    return GENERAL_ERROR;
		}
	    }
	}
    }

    if (secret == 1)
	remove_secure_input(&oldt);

    if (buff_len < temp_buff_strlen) {
	printf("Limit input to %d characters\n", buff_len);
	return GENERAL_ERROR;
    }
    snprintf(buff, buff_len, "%s", temp_buff);

    return SUCCESS_OP;
}

void set_secure_input(struct termios *oldt) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    // Get current console mode
    GetConsoleMode(hStdin, &mode);

    // Disable echo input
    DWORD newMode = mode & ~ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, newMode);
#else

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, oldt) != 0) {
	perror("tcgetattr");
    }

    struct termios newt;
    // Make a copy and modify it: turn off echo
    newt = *oldt;
    newt.c_lflag &= ~ECHO;

    // Apply the new settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
	perror("tcsetattr");
    }
#endif
}

void remove_secure_input(struct termios *oldt) {
#ifdef _WIN32
    // Restore original console mode
    SetConsoleMode(hStdin, mode);
#else
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, oldt);
#endif
}

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
	;
}
