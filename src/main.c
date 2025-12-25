#include <stdio.h>

#include "db/general.h"
#include "util/argparser.h"
#include "util/errors.h"

int main(int argc, char *argv[]) {
    struct Command *cmd = NULL;

    int status = parse_args(argc, argv, &cmd);
    if (status == USER_REQUESTED_HELP) {
	return 0;
    } else if (!cmd) {
	return GENERAL_ERROR;
    } else if (status != SUCCESS_OP) {
	return GENERAL_ERROR;
    }

    sqlite3 *db_con = open_db_con();
    if (!db_con) {
	free_arguments(cmd);
	return GENERAL_ERROR;
    }

    status = cmd->Run(cmd->arguments, db_con);

    free_arguments(cmd);
    sqlite3_close(db_con);

    return status;
}
