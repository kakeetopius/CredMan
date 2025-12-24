#include <stdio.h>
#include <string.h>

#include "error_messages.h"
#include "util.h"

int handle_input(int argc, char *argv[]) {
    if (argc < 2) {
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }
    char *help_args[] = {"-h", "--help", "help"};
    int help_args_num = sizeof(help_args) / sizeof(help_args[0]);
    for (int i = 0; i < help_args_num; i++) {
	if (strcmp(argv[1], help_args[i]) == 0) {
	    printf("%s", GENERAL_MESSAGE);
	    return USER_REQUESTED_HELP;
	}
    }
    int status = 0; 

    char *command = argv[1];

    /*------------Dispatch table for subcommands------------*/
    struct sub_command dispatch[] = {
	{"ls", list_accounts},
	{"add", add_acc},
	{"get", get_details},
	{"change", change_details},
	{"delete", delete_account},
	{NULL, NULL}};

    int dispatch_size = sizeof(dispatch) / sizeof(dispatch[0]);

    for (int i = 0; i < dispatch_size; i++) {
	if (dispatch[i].name == NULL) { /*If the end of the dispatch array is reached and no matches*/
	    printf("Unknown Command: %s\n", command);
	    printf("%s", GENERAL_MESSAGE);
	    return GENERAL_ERROR;
	} else if (strcmp(command, dispatch[i].name) == 0) {
	    break;
	}
    }

    return status;
}
