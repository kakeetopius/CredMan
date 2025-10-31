/* 
 * This source file contains all logic that is concerned 
 * with interfacing with the sqlite3 library which 
 * in turn controls the sqlite3 database
 */

#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "../includes/db_access.h"
#include "../includes/error_messages.h"
#include "../includes/util.h"

char DB_FILE[100];
char* CRED_FILE_NAME = ".creds.db";
char DB_BACKUP[100];


sqlite3 *open_db_con() {
    /*Test connection to test if database exists*/
    sqlite3 *db_test_con = NULL;
    /*Variable for return codes.*/
    int status;
    
    char* home = getenv("HOME");
    if (!home) {
	printf("Could not determine home path from environment variables\n");
	return NULL;
    }

    snprintf(DB_FILE, 100, "%s/%s", home, CRED_FILE_NAME);

    /*---Getting home path from environment variables----*/
    status = sqlite3_open_v2(DB_FILE, &db_test_con, SQLITE_OPEN_READONLY, 0);
    if (status == SQLITE_CANTOPEN) {
	char choice[3];
	printf("Can't Find the Specified Database\n");
	printf("Do you want to initialise a new one(y/n): ");

	fgets(choice, sizeof(choice), stdin);
	// check if some remaining stuff in stdin
	if (strchr(choice, '\n') == NULL) {
	    flush_stdin();
	} else {
	    // remove \n
	    choice[strcspn(choice, "\n")] = '\0';
	}

	if (strcmp(choice, "y") == 0) {
	    status = create_new_database();
	    return NULL;
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
    //decrypting the database before returning handle.
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
    }
    else if (status != SQLITE_OK) {
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

int check_account_exists(sqlite3* db, char* acc_name) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT EXISTS(SELECT 1 FROM account WHERE acc_name = ?);";
    int status;

    status = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    sqlite3_bind_text(stmt, 1, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
	exists = sqlite3_column_int(stmt, 0);  
    }

    return exists == 1 ? DB_ROW_EXISTS : DB_ROW_NX;

    sqlite3_finalize(stmt);
}

int add_account_to_db(sqlite3 *db, Account acc) {
    if (!db) {
	printf("Database handle is NULL\n");
	return GENERAL_ERROR;
    }
    if (!acc) {
	printf("Account object is NULL\n");
	return GENERAL_ERROR;
    }

    int status;
    sqlite3_stmt *pStmt = NULL;
    char *query = "INSERT INTO account(acc_name, user_name, password) VALUES (?, ?, ?);";

    status = sqlite3_prepare_v2(db, query, -1, &pStmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pStmt, 1, acc->name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pStmt, 2, acc->username, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pStmt, 3, acc->password, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_step(pStmt);
    if (status != SQLITE_DONE) {
	printf("Error: %s", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }
    int affected_rows = sqlite3_changes(db);

    sqlite3_finalize(pStmt);
    if (affected_rows != 1) {
	return NON_AFFECTED_ERROR;
    }

    return SUCCESS_OP;
}

int delete_account_from_db(sqlite3 *db, char *acc_name) {
    int status;
    sqlite3_stmt *pstmt = NULL;
    char *query = "DELETE FROM account WHERE acc_name = ?;";

    status = sqlite3_prepare_v2(db, query, -1, &pstmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pstmt, 1, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pstmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_step(pstmt);
    if (status != SQLITE_DONE) {
	printf("Error: %s", sqlite3_errmsg(db));
	sqlite3_finalize(pstmt);
	return SQLITE_RELATED_ERROR;
    }
    int affected_rows = sqlite3_changes(db);

    sqlite3_finalize(pstmt);
    if (affected_rows != 1) {
	return NON_AFFECTED_ERROR;
    }

    return SUCCESS_OP;
}

int update_db_field(sqlite3 *db, enum db_fields toUpdate, char *acc_name, char *new_field_value) {
    char *query;
    sqlite3_stmt *pStmt;
    int status;
    switch (toUpdate) {
    case DB_ACC_NAME:
	query = "UPDATE account SET acc_name = ? WHERE acc_name = ?;";
	break;
    case DB_USER_NAME:
	query = "UPDATE account SET user_name = ? WHERE acc_name = ?;";
	break;
    case DB_ACC_PASSWORD:
	query = "UPDATE account SET password = ? WHERE acc_name = ?;";
	break;
    }

    status = sqlite3_prepare_v2(db, query, -1, &pStmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pStmt, 1, new_field_value, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pStmt, 2, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_step(pStmt);
    if (status != SQLITE_DONE) {
	printf("Error: %s", sqlite3_errmsg(db));
	sqlite3_finalize(pStmt);
	return SQLITE_RELATED_ERROR;
    }
    int affected_rows = sqlite3_changes(db);

    sqlite3_finalize(pStmt);
    if (affected_rows != 1) {
	return NON_AFFECTED_ERROR;
    }

    return SUCCESS_OP;
}

int get_account_by_name(sqlite3 *db, char *acc_name, struct account *acc) {
    if (!acc) {
	return GENERAL_ERROR;
    }
    int status;
    sqlite3_stmt *pstmt = NULL;
    char *query = "SELECT acc_name, user_name, password FROM account WHERE acc_name = ?;";

    status = sqlite3_prepare_v2(db, query, -1, &pstmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_bind_text(pstmt, 1, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pstmt);
	return SQLITE_RELATED_ERROR;
    }

    status = sqlite3_step(pstmt);
    if (status != SQLITE_ROW) {
	printf("Account %s does not exist\n", acc_name);
	sqlite3_finalize(pstmt);
	return SQLITE_RELATED_ERROR;
    }

    const unsigned char* a_name = sqlite3_column_text(pstmt, 0);
    const unsigned char* username = sqlite3_column_text(pstmt, 1);
    const unsigned char* pass = sqlite3_column_text(pstmt, 2);

    acc->name = strdup((char*)a_name);
    acc->username = strdup((char*)username);
    acc->password = strdup((char*)pass);

    sqlite3_finalize(pstmt);
    return SUCCESS_OP;
}

int get_all_credentials(sqlite3 *db, struct account_list *acc_list) {
    if (!db) {
	return GENERAL_ERROR;
    }
    if (!acc_list) {
	return GENERAL_ERROR;
    }

    const char *query = "SELECT acc_name, user_name, password FROM account;";
    sqlite3_stmt *pre_stmt = NULL;
    int status = 0;

    status = sqlite3_prepare_v2(db, query, -1, &pre_stmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s", sqlite3_errmsg(db));
	return SQLITE_RELATED_ERROR;
    }

    while ((status = sqlite3_step(pre_stmt)) == SQLITE_ROW) {
	unsigned const char *acc_name = sqlite3_column_text(pre_stmt, 0);
	unsigned const char *user_name = sqlite3_column_text(pre_stmt, 1);
	unsigned const char *pass = sqlite3_column_text(pre_stmt, 2);
	int insert_status = insert_acc(acc_list, (char *)acc_name, (char *)pass, (char *)user_name);
	if (insert_status != 0) {
	    sqlite3_finalize(pre_stmt);
	    return GENERAL_ERROR;
	}
    }

    if (status != SQLITE_DONE) {
	printf("An error occured: %s\n", sqlite3_errmsg(db));
	sqlite3_finalize(pre_stmt);
	return SQLITE_RELATED_ERROR;
    }

    sqlite3_finalize(pre_stmt);

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
    printf("To add credentials \n%s\n", ADD_MESSAGE);
    printf("Use cman help for more information\n");

    return SUCCESS_OP;
}

