#include <stdio.h>
#include <string.h>

#include "../includes/db_access.h"
#include "../includes/error_messages.h"

sqlite3 *open_db_con() {
    /*Test connection to test if database exists*/
    sqlite3 *db_test_con = NULL;
    /*Variable for return codes.*/
    int status;

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
	return NULL;
    }

    // Close the read only database handle and open with read/write.
    sqlite3_close_v2(db_test_con);

    sqlite3 *db_con = NULL;
    status = sqlite3_open(DB_FILE, &db_con);
    if (status != SQLITE_OK)
	printf("Error opening database\n");

    status = delete_account_from_db(db_con, "test8"); 
    if (status != 0)
	return NULL;
    printf("Successfull!!\n");
    return db_con;
}

int delete_account_from_db(sqlite3* db, char* acc_name) {
    int status;
    sqlite3_stmt *pstmt = NULL;
    char *query = "DELETE FROM account WHERE acc_name = ?;";

    status = sqlite3_prepare_v2(db, query, -1, &pstmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_bind_text(pstmt, 1, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_step(pstmt);
    if (status != SQLITE_DONE) {
	printf("Error: %s", sqlite3_errmsg(db));
	return -1;
    }
    int affected_rows = sqlite3_changes(db);
    
    if(affected_rows != 1) {
	return -1;
    }

    sqlite3_finalize(pstmt);
    return 0;
}

int update_db_field(sqlite3 *db, enum db_fields toUpdate, char *acc_name, char *new_field_value) {
    char* query;
    sqlite3_stmt* pStmt;
    int status;
    switch(toUpdate) {
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
	return -1;
    }

    status = sqlite3_bind_text(pStmt, 1, new_field_value, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_bind_text(pStmt, 2, acc_name , -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_step(pStmt);
    if (status != SQLITE_DONE) {
	printf("Error: %s", sqlite3_errmsg(db));
	return -1;
    }
    int affected_rows = sqlite3_changes(db);
    
    if(affected_rows != 1) {
	return -1;
    }

    sqlite3_finalize(pStmt);
    return 0;
}

int get_account_by_name(sqlite3 *db, char *acc_name, struct account *acc) {
    if (!acc) {
	return -1;
    }
    int status;
    sqlite3_stmt *pstmt = NULL;
    char *query = "SELECT * FROM account WHERE acc_name = ?;";

    status = sqlite3_prepare_v2(db, query, -1, &pstmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_bind_text(pstmt, 1, acc_name, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db));
	return -1;
    }

    status = sqlite3_step(pstmt);
    if (status != SQLITE_ROW) {
	printf("Account %s does not exist\n", acc_name);
	return -1;
    }
    acc->name = (char *)sqlite3_column_text(pstmt, 0);
    acc->username = (char *)sqlite3_column_text(pstmt, 1);
    acc->password = (char *)sqlite3_column_text(pstmt, 2);

    sqlite3_finalize(pstmt);
    return 0;
}

int get_all_credentials(sqlite3 *db, struct account_list *acc_list) {
    if (!db) {
	return -1;
    }
    if (!acc_list) {
	return -1;
    }

    const char *query = "SELECT * FROM account;";
    sqlite3_stmt *pre_stmt = NULL;
    int status = 0;

    status = sqlite3_prepare_v2(db, query, -1, &pre_stmt, NULL);
    if (status != SQLITE_OK) {
	printf("Error: %s", sqlite3_errmsg(db));
	return -1;
    }

    while ((status = sqlite3_step(pre_stmt)) == SQLITE_ROW) {
	unsigned const char *acc_name = sqlite3_column_text(pre_stmt, 1);
	unsigned const char *user_name = sqlite3_column_text(pre_stmt, 2);
	unsigned const char *pass = sqlite3_column_text(pre_stmt, 3);
	int insert_status = insert_acc(acc_list, (char *)acc_name, (char *)pass, (char *)user_name);
	if (insert_status != 0) {
	    return insert_status;
	}
    }

    if (status != SQLITE_DONE) {
	printf("An error occured: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pre_stmt);

    return 0;
}

int create_new_database() {
    int status;
    char pass[64];
    status = get_password(pass, 63, "Enter Master Password to be used for encryption. Make sure to remember it", 1);
    pass[63] = '\0';

    if (status != 0) {
	return status;
    }

    const char *create_query =
	"CREATE TABLE account ("
	" acc_id INTEGER PRIMARY KEY AUTOINCREMENT,"
	" acc_name VARCHAR(100) NOT NULL UNIQUE,"
	" user_name VARCHAR(100) NOT NULL,"
	" password VARCHAR(256) NOT NULL"
	");";

    char pragma_stmt[100];
    snprintf(pragma_stmt, 100, "PRAGMA key = \'%s\';", pass);

    sqlite3 *db_con = NULL;
    status = sqlite3_open(DB_FILE, &db_con);
    if (status != SQLITE_OK) {
	printf("Error: %s\n", sqlite3_errmsg(db_con));
	return status;
    }

    char *errmsg = NULL;

    status = sqlite3_exec(db_con, pragma_stmt, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	sqlite3_close(db_con);
	return status;
    }

    status = sqlite3_exec(db_con, create_query, NULL, NULL, &errmsg);
    if (status != SQLITE_OK) {
	printf("Error executing query: %s\n", errmsg == NULL ? "" : errmsg);
	if (errmsg) {
	    sqlite3_free(errmsg);
	}
	sqlite3_close(db_con);
	return status;
    }

    if (errmsg)
	sqlite3_free(errmsg);

    printf("New Credential Database Created Successfully at location: %s\n", DB_FILE);
    printf("To add credentials \n%s\n", ADD_MESSAGE);
    printf("Use cman help for more information\n");

    return 0;
}

/*Function to get master password from user which is also essential for
 * encryption*/
int get_password(char *buff, int buff_len, const char *prompt, int confirm) {

    struct termios oldt; // to store old terminal settings
    // Remove echo when typing input.
    set_secure_input(&oldt);

    char temp_buff[64];
    printf("%s: ", prompt);
    fgets(temp_buff, sizeof(temp_buff), stdin);
    printf("\n");

    // if some text remained in stdin
    if (strchr(temp_buff, '\n') == NULL) {
	printf("Limit Password to 64 characters\n");
	flush_stdin();
	remove_secure_input(&oldt);
	return -1;
    } else {
	/*removing new line character*/
	int new_line_pos = strcspn(temp_buff, "\n");
	temp_buff[new_line_pos] = '\0';

	if (strlen(temp_buff) == 0) {
	    printf("Master Password Can't be Empty\n");
	    remove_secure_input(&oldt);
	    return -1;
	}

	if (confirm == 1) {
	    printf("Enter Password again to confirm: ");
	    char temp_buff2[64];
	    fgets(temp_buff2, sizeof(temp_buff2), stdin);
	    printf("\n");
	    // if input is too long again
	    if (strchr(temp_buff2, '\n') == NULL) {
		flush_stdin();
		printf("Passwords don't match\n");
		remove_secure_input(&oldt);
		return -1;
	    } else {
		temp_buff2[strcspn(temp_buff2, "\n")] = '\0';
		if (strcmp(temp_buff, temp_buff2) != 0) {
		    printf("Passwords don't match\n");
		    remove_secure_input(&oldt);
		    return -1;
		}
		snprintf(buff, buff_len, "%s", temp_buff);
	    }
	}
    }

    printf("\n");
    remove_secure_input(&oldt);
    return 0;
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
