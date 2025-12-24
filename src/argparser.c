#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparser.h"
#include "error_messages.h"
#include "util.h"

struct Command commands[] = {
    {.name = "add", .argparser = addArgParser, .Run = runAdd},
    {.name = "get", .argparser = getArgParser, .Run = runGet},
    {.name = "change", .argparser = changeArgParser, .Run = runChange},
    {.name = "ls", .argparser = listArgParser, .Run = runList},
    {.name = "delete", .argparser = deleteArgParser, .Run = runDelete},
};

int num_of_commands = sizeof(commands) / sizeof(commands[0]);

int parse_args(int argc, char *argv[], struct Command **command) {
    if (argc < 2) {
	printf("%s", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }
    if (check_if_help_requested(argv[1]) == USER_REQUESTED_HELP) {
	printf("%s", GENERAL_MESSAGE);
	return USER_REQUESTED_HELP;
    }

    int sub_command_found = 0;
    struct Command *subcommand = NULL;
    for (int i = 0; i < num_of_commands; i++) {
	if (strcmp(argv[1], commands[i].name) == 0) {
	    sub_command_found = 1;
	    subcommand = &commands[i];
	}
    }
    if (!sub_command_found) {
	printf("Unknown command: %s\n", argv[1]);
	printf("%s\n", GENERAL_MESSAGE);
	return GENERAL_ERROR;
    }
    if (!subcommand) {
	return GENERAL_ERROR;
    }

    // initalising argument struct based on subcommand.
    void *arguments = NULL;
    if (strcmp(subcommand->name, "add") == 0) {
	arguments = malloc(sizeof(struct AddArgs));
    } else if (strcmp(subcommand->name, "get") == 0) {
	arguments = malloc(sizeof(struct GetArgs));
    } else if (strcmp(subcommand->name, "change") == 0) {
	arguments = malloc(sizeof(struct ChangeArgs));
    } else if (strcmp(subcommand->name, "ls") == 0) {
	arguments = malloc(sizeof(struct ListArgs));
    } else if (strcmp(subcommand->name, "delete") == 0) {
	arguments = malloc(sizeof(struct DeleteArgs));
    } else {
	return GENERAL_ERROR;
    }

    int status = subcommand->argparser(argc, argv, arguments);
    if (status != SUCCESS_OP) {
	return status;
    }
    subcommand->arguments = arguments;

    *command = subcommand; // initalise main's pointer to point to the correct command stuct.
    return SUCCESS_OP;
}

void free_arguments(struct Command *command) {
    if (!command) {
	return;
    }
    if (command->arguments) {
	free(command->arguments);
    }
}

int check_if_help_requested(char *arg) {
    char *help_args[] = {"-h", "--help", "help"};
    int help_args_num = sizeof(help_args) / sizeof(help_args[0]);
    for (int i = 0; i < help_args_num; i++) {
	if (strcmp(arg, help_args[i]) == 0) {
	    return USER_REQUESTED_HELP;
	}
    }
    return GENERAL_ERROR;
}

bool strings_match(char *str1, char *str2) {
    if (!str1 || !str2) {
	return false;
    }
    if (strcmp(str1, str2) == 0) {
	return true;
    } else {
	return false;
    }
}

int addArgParser(int argc, char **argv, void *arguments) {
    if (!arguments) {
	return GENERAL_ERROR;
    }
    if (argc < 3) {
	printf("%s", ADD_MESSAGE);
	return GENERAL_ERROR;
    }

    struct AddArgs *args = (struct AddArgs *)arguments;

    if (check_if_help_requested(argv[2]) == USER_REQUESTED_HELP) {
	printf("%s", ADD_MESSAGE);
	return USER_REQUESTED_HELP;
    }
    args->flags = 0;
    args->secretName = argv[2];

    int i = 3;
    while (i < argc) {
	if (argv[i][0] == '-') {
	    // if argument.
	    if (strings_match(argv[i], "-b") || strings_match(argv[i], "--batch")) {
		args->flags = args->flags | ADD_FLAG_BATCHFILE;

		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "-t") || strings_match(argv[i], "--type")) {
		if (argc == i + 1) {
		    printf("No type given. Use cman add -h for more information\n");
		    return GENERAL_ERROR;
		}
		char *type = argv[i + 1];
		if (strings_match(type, "login")) {
		    args->flags = args->flags | ADD_FLAG_TYPE_LOGIN;
		} else if (strings_match(type, "api_key")) {
		    args->flags = args->flags | ADD_FLAG_TYPE_APIKEY;
		} else {
		    printf("Unknown type: %s. Use cman add -h for more information.\n", type);
		    return GENERAL_ERROR;
		}
		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "--no-auto")) {
		args->flags = args->flags | ADD_FLAG_NOAUTO;
	    } else {
		printf("Unknown option: %s. Use cman add -h for more information.s\n", argv[i]);
		return GENERAL_ERROR;
	    }
	} else {
	    printf("Unknown option: %s. Use cman add -h for more information\n", argv[i]);
	    return GENERAL_ERROR;
	}
	i++;
    }

    return SUCCESS_OP;
}

int getArgParser(int argc, char **argv, void *arguments) {
    if (!arguments) {
	return GENERAL_ERROR;
    }
    if (argc < 3) {
	printf("%s", GET_MESSAGE);
	return GENERAL_ERROR;
    }

    struct GetArgs *args = (struct GetArgs *)arguments;

    if (check_if_help_requested(argv[2]) == USER_REQUESTED_HELP) {
	printf("%s", GET_MESSAGE);
	return USER_REQUESTED_HELP;
    }
    args->flags = 0;
    args->secretName = argv[2];

    int i = 3; // start after secretName
    while (i < argc) {
	if (argv[i][0] == '-') {
	    // if argument.
	    if (strings_match(argv[i], "-f") || strings_match(argv[i], "--field")) {
		if (argc == i + 1) {
		    // if this is the last argument in argument list.
		    printf("No field name given. Use cman get -h for more information\n");
		    return GENERAL_ERROR;
		}
		char *field = argv[i + 1];
		if (strings_match(field, "uname")) {
		    args->flags = args->flags | GET_FLAG_FIELD_USERNAME;
		} else if (strings_match(field, "pass")) {
		    args->flags = args->flags | GET_FLAG_FIELD_PASS;
		} else {
		    printf("Unknown field type: %s. Use cman get -h for more information.\n", field);
		    return GENERAL_ERROR;
		}

		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "-t") || strings_match(argv[i], "--type")) {
		if (argc == i + 1) {
		    printf("No type given. Use cman get -h for more information\n");
		    return GENERAL_ERROR;
		}
		char *type = argv[i + 1];
		if (strings_match(type, "login")) {
		    args->flags = args->flags | GET_FLAG_TYPE_LOGIN;
		} else if (strings_match(type, "api_key")) {
		    args->flags = args->flags | GET_FLAG_TYPE_APIKEY;
		} else {
		    printf("Unknown type: %s. Use cman get -h for more information.\n", type);
		    return GENERAL_ERROR;
		}
		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "-q") || strings_match(argv[i], "--quiet")) {
		args->flags = args->flags | GET_FLAG_QUIET;
	    } else {
		printf("Unknown option: %s. Use cman get -h for more information.\n", argv[i]);
		return GENERAL_ERROR;
	    }
	} else {
	    printf("Unknown option: %s. Use cman get -h for more information.\n", argv[i]);
	    return GENERAL_ERROR;
	}
	i++;
    }

    return SUCCESS_OP;
}

int changeArgParser(int argc, char **argv, void *arguments) {
    if (!arguments) {
	return GENERAL_ERROR;
    }
    if (argc < 3) {
	printf("%s", CHANGE_MESSAGE);
	return GENERAL_ERROR;
    }

    struct ChangeArgs *args = (struct ChangeArgs *)arguments;
    if (strings_match(argv[2], "--master")) {
	args->flags = args->flags | CHANGE_FLAG_MASTER;
	return SUCCESS_OP;
    }

    if (check_if_help_requested(argv[2]) == USER_REQUESTED_HELP) {
	printf("%s", CHANGE_MESSAGE);
	return USER_REQUESTED_HELP;
    }
    args->flags = 0;
    args->secretName = argv[2];

    int i = 3; // start after secretName
    while (i < argc) {
	if (argv[i][0] == '-') {
	    // if argument.
	    if (strings_match(argv[i], "-f") || strings_match(argv[i], "--field")) {
		if (argc == i + 1) {
		    // if this is the last argument in argument list.
		    printf("No field name given. Use cman change -h for more information\n");
		    return GENERAL_ERROR;
		}

		char *field = argv[i + 1];
		if (strings_match(field, "pass")) {
		    args->flags = args->flags | CHANGE_FLAG_FIELD_PASS;
		} else if (strings_match(field, "uname")) {
		    args->flags = args->flags | CHANGE_FLAG_FIELD_USERNAME;
		} else if (strings_match(field, "accname")) {
		    args->flags = args->flags | CHANGE_FLAG_FIELD_ACCNAME;
		} else {
		    printf("Unknown field type: %s. Use cman change -h for more information\n.", field);
		    return GENERAL_ERROR;
		}

		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "-t") || strings_match(argv[i], "--type")) {
		if (argc == i + 1) {
		    printf("No type given. Use cman change -h for more information\n");
		    return GENERAL_ERROR;
		}
		char *type = argv[i + 1];
		if (strings_match(type, "login")) {
		    args->flags = args->flags | CHANGE_FLAG_TYPE_LOGIN;
		} else if (strings_match(type, "api_key")) {
		    args->flags = args->flags | CHANGE_FLAG_TYPE_APIKEY;
		} else {
		    printf("Unknown type: %s. Use cman change -h for more information\n", type);
		    return GENERAL_ERROR;
		}
		i = i + 2; // skip checking the next argument.
		continue;
	    } else if (strings_match(argv[i], "--no-auto")) {
		args->flags = args->flags | CHANGE_FLAG_NOAUTO;
	    } else if (strings_match(argv[i], "--master")) {
		args->flags = args->flags | CHANGE_FLAG_MASTER;
	    } else {
		printf("Unknown option: %s. Use cman change -h for more information\n", argv[i]);
		return GENERAL_ERROR;
	    }
	} else {
	    printf("Unknown option: %s. Use cman change -h for more information.\n", argv[i]);
	    return GENERAL_ERROR;
	}
	i++;
    }

    return SUCCESS_OP;
}

int listArgParser(int argc, char **argv, void *arguments) {
    if (!arguments) {
	return GENERAL_ERROR;
    }

    struct ListArgs *args = (struct ListArgs *)arguments;

    args->flags = 0;

    if (argc < 3) {
	return SUCCESS_OP;
    }

    if (check_if_help_requested(argv[2]) == USER_REQUESTED_HELP) {
	printf("%s", LS_MESSAGE);
	return USER_REQUESTED_HELP;
    }
    int i = 2;
    while (i < argc) {
	if (argv[i][0] == '-') {
	    // if argument.
	    printf("Unknown Option: %s. Use cman ls -h for more information.\n", argv[i]);
	    return GENERAL_ERROR;
	} else {
	    printf("Unknown option: %s. Use cman ls -h for more information.\n", argv[i]);
	    return GENERAL_ERROR;
	}
	i++;
    }

    return SUCCESS_OP;
}

int deleteArgParser(int argc, char **argv, void *arguments) {
    if (!arguments) {
	return GENERAL_ERROR;
    }

    if (argc < 3) {
	printf("%s\n", DELETE_MESSAGE);
	return GENERAL_ERROR;
    }

    struct DeleteArgs *args = (struct DeleteArgs *)arguments;

    args->flags = 0;

    if (check_if_help_requested(argv[2]) == USER_REQUESTED_HELP) {
	printf("%s", DELETE_MESSAGE);
	return USER_REQUESTED_HELP;
    }
    args->secretName = argv[2];
    int i = 3;
    while (i < argc) {
	if (strings_match(argv[i], "-t") || strings_match(argv[i], "--type")) {
	    if (argc == i + 1) {
		printf("No type given. Use cman delete -h for more information\n");
		return GENERAL_ERROR;
	    }
	    char *type = argv[i + 1];
	    if (strings_match(type, "login")) {
		args->flags = args->flags | DELETE_FLAG_TYPE_LOGIN;
	    } else if (strings_match(type, "api_key")) {
		args->flags = args->flags | DELETE_FLAG_TYPE_APIKEY;
	    } else {
		printf("Unknown type: %s. Use cman delete -h for more information.\n", type);
		return GENERAL_ERROR;
	    }
	    i = i + 2; // skip checking the next argument.
	    continue;
	} else {
	    printf("Unknown option: %s. Use cman delete -h for more information.\n", argv[i]);
	    return GENERAL_ERROR;
	}
	i++;
    }

    return SUCCESS_OP;
}
