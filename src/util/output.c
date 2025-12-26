#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util/errors.h"
#include "util/argparser.h"

int print_help(char *subcommand) {
    if (strcmp(subcommand, "add") == 0) {
	printf("%s", ADD_MESSAGE);
    } else if (strcmp(subcommand, "change") == 0) {
	printf("%s", CHANGE_MESSAGE);
    } else if (strcmp(subcommand, "delete") == 0) {
	printf("%s", DELETE_MESSAGE);
    } else if (strcmp(subcommand, "get") == 0) {
	printf("%s", GET_MESSAGE);
    } else if (strcmp(subcommand, "ls") == 0) {
	printf("%s", LS_MESSAGE);
    } else {
	printf("Unknown subcommand: %s\n", subcommand);
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }

    return 0;
}

void print_result(char *fieldname, char *value) {
    if (!fieldname || !value) {
	return;
    }
    int istty = isatty(STDIN_FILENO);
    int quiet = 0;
    if (strcmp(current_command->name, "get") == 0) {
        struct GetArgs *args = (struct GetArgs*)current_command->arguments;
	if (args->flags & GET_FLAG_QUIET) {
	    quiet = 1;
	}
    }
    printf("%s%s%s", istty && !quiet ? fieldname : "", value, istty && !quiet ? "\n" : "");
}
