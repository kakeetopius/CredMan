#ifndef ARGPARSER_H
#define ARGPARSER_H

#include "commands.h"
extern struct Command commands[];

struct AddArgs {
    char *secretName;
    int flags;
};
#define ADD_FLAG_TYPE_LOGIN 0x0001
#define ADD_FLAG_TYPE_APIKEY 0x0002
#define ADD_FLAG_NOAUTO 0x0004
#define ADD_FLAG_BATCHFILE 0x0008

struct GetArgs {
    char *secretName;
    int flags;
};
#define GET_FLAG_TYPE_LOGIN 0x0001
#define GET_FLAG_TYPE_APIKEY 0x0002
#define GET_FLAG_QUIET 0x0004
#define GET_FLAG_FIELD_USERNAME 0x0008
#define GET_FLAG_FIELD_PASS 0x0010

struct ChangeArgs {
    char *secretName;
    int flags;
    char *fieldName;
};
#define CHANGE_FLAG_TYPE_LOGIN 0x0001
#define CHANGE_FLAG_TYPE_APIKEY 0x0002
#define CHANGE_FLAG_NOAUTO 0x0004
#define CHANGE_FLAG_FIELD_USERNAME 0x0008
#define CHANGE_FLAG_FIELD_PASS 0x0010
#define CHANGE_FLAG_FIELD_ACCNAME 0x0020
#define CHANGE_FLAG_MASTER 0x0040

struct DeleteArgs {
    char *secretName;
    int flags;
};
#define DELETE_FLAG_TYPE_LOGIN 0x0001
#define DELETE_FLAG_TYPE_APIKEY 0x0002

struct ListArgs {
    int flags;
};
#define LIST_FLAG_TYPE_LOGIN 0x0001
#define LIST_FLAG_TYPE_APIKEY 0x0002

int handle_input(int argc, char *argv[]);

int parse_args(int argc, char *argv[], struct Command **command);
int check_if_help_requested(char* arg);

int addArgParser(int argc, char **argv, void *arguments);
int changeArgParser(int argc, char **argv, void *arguments);
int getArgParser(int argc, char **argv, void *arguments);
int deleteArgParser(int argc, char **argv, void *arguments);
int listArgParser(int argc, char **argv, void *arguments);

void free_arguments(struct Command *command);
#endif
