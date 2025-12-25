#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db/general.h"
#include "util/errors.h"
#include "util/input.h"
#include "util/passwd_gen.h"

char DB_FILE[256];
char *CRED_FILE_NAME = ".creds.db";
char DB_BACKUP[256];

sqlite3 *open_db_con() {
    /*Test connection to test if database exists*/
    sqlite3 *db_test_con = NULL;
    /*Variable for return codes.*/
    int status;

    /*---Getting db path from environment variables----*/
    char *db_file_name = getenv("CMAN_DBFILE");
    if (db_file_name) {
	snprintf(DB_FILE, sizeof(DB_FILE), "%s", db_file_name);
    } else {
	char *home = getenv("HOME");
	if (!home) {
	    printf("Could not determine home path from environment variables\n");
	    return NULL;
	}
	snprintf(DB_FILE, sizeof(DB_FILE), "%s/%s", home, CRED_FILE_NAME);
    }

    status = sqlite3_open_v2(DB_FILE, &db_test_con, SQLITE_OPEN_READONLY, 0);
    if (status == SQLITE_CANTOPEN) {
	char choice[3];
	char *prompt = "Can't Find Credential Database\nDo you want to initialise it(y/n)?";

	status = get_user_input(choice, sizeof(choice), prompt, 0, 0);

	if (status != SUCCESS_OP) {
	    return NULL;
	}

	if (strcmp(choice, "y") == 0) {
	    status = create_new_database();
	    if (status != SUCCESS_OP) {
		return NULL;
	    }
	}
	// return NULL to tell main to stop execution.
	return NULL;
    }

    // Close the read only database handle and open with read/write.
    sqlite3_close_v2(db_test_con);

    sqlite3 *db_con = NULL;
    status = sqlite3_open(DB_FILE, &db_con);
    if (status != SQLITE_OK) {
	printf("Error opening database\n");
	return NULL;
    }
    // decrypting the database before returning handle.
    status = decrypt_db(db_con);
    if (status != SQLITE_OK) {
	return NULL;
    }

    return db_con;
}

int decrypt_db(sqlite3 *db) {
    int status;
    char pass[CRED_BUFF_LEN];
    status = get_user_input(pass, CRED_BUFF_LEN, "Enter Master Password", 0, 1);
    pass[CRED_BUFF_LEN - 1] = '\0';
    if (status != 0) {
	return GENERAL_ERROR;
    }

    char pragma_stmt[CRED_BUFF_LEN + 32];
    snprintf(pragma_stmt, CRED_BUFF_LEN + 32, "PRAGMA key = \'%s\';", pass);

    char *errmsg = NULL;

    status = sqlite3_exec(db, pragma_stmt, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	return SQLITE_RELATED_ERROR;
    }

    // verifying if password given was correct.
    const char *query = "SELECT COUNT(*) FROM sqlite_master";
    status = sqlite3_exec(db, query, NULL, NULL, &errmsg);

    if (status == SQLITE_NOTADB) {
	printf("Could not decrypt Database\n");
	printf("Check the master password and try again\n");
	return WRONG_MASTER_PASSWORD;
    } else if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	return SQLITE_RELATED_ERROR;
    }

    if (errmsg) {
	sqlite3_free(errmsg);
    }

    return SUCCESS_OP;
}

int change_db_master_password(sqlite3 *db) {
    int status;
    char pass[CRED_BUFF_LEN];
    status = get_user_input(pass, CRED_BUFF_LEN, "Enter New Master Password. Make sure to remember it", 1, 1);
    pass[CRED_BUFF_LEN - 1] = '\0';
    if (status != 0) {
	return GENERAL_ERROR;
    }

    char pragma_stmt[CRED_BUFF_LEN + 32];
    snprintf(pragma_stmt, CRED_BUFF_LEN + 32, "PRAGMA rekey = \'%s\';", pass);

    char *errmsg = NULL;

    status = sqlite3_exec(db, pragma_stmt, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	return SQLITE_RELATED_ERROR;
    }

    if (errmsg) {
	sqlite3_free(errmsg);
    }

    return SUCCESS_OP;
}

int create_new_database() {
    int status;
    char pass[CRED_BUFF_LEN];
    status = get_user_input(pass, CRED_BUFF_LEN, "Enter Master Password to be used for encryption. Make sure to remember it", 1, 1);
    pass[CRED_BUFF_LEN - 1] = '\0';

    if (status != 0) {
	return GENERAL_ERROR;
    }

    const char *create_query =
	"CREATE TABLE account ("
	" acc_id INTEGER PRIMARY KEY AUTOINCREMENT,"
	" acc_name VARCHAR(100) NOT NULL UNIQUE,"
	" user_name VARCHAR(100) NOT NULL,"
	" password VARCHAR(256) NOT NULL"
	");"
	"CREATE TABLE api_keys ("
	"api_id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"api_name VARCHAR(100) NOT NULL,"
	"service VARCHAR(100) NOT NULL,"
	"user_name VARCHAR(100) NOT NULL,"
	"api_key VARCHAR(256) NOT NULL"
	");";

    char pragma_stmt[CRED_BUFF_LEN + 32];
    snprintf(pragma_stmt, CRED_BUFF_LEN + 32, "PRAGMA key = \'%s\';", pass);

    sqlite3 *db_con = NULL;
    status = sqlite3_open(DB_FILE, &db_con);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db_con));
	return SQLITE_RELATED_ERROR;
    }

    char *errmsg = NULL;

    status = sqlite3_exec(db_con, pragma_stmt, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	sqlite3_close(db_con);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_exec(db_con, create_query, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	sqlite3_close(db_con);
	return SQLITE_RELATED_ERROR;
    }

    if (errmsg)
	sqlite3_free(errmsg);

    printf("New Credential Database Created Successfully at location: %s\n", DB_FILE);
    printf("To add credentials \n\n%s\n", ADD_MESSAGE);
    printf("Use cman help for more information\n");

    return SUCCESS_OP;
}
