#ifndef DB_ACCESS_H
#define DB_ACCESS_H

#include <sqlcipher/sqlite3.h>

#include "../includes/account.h"

#ifndef _WIN32
#define DB_FILE "/home/pius/.creds.db"
#define DB_BACKUP "/home/pius/.creds_backup.db"
#endif

#define NON_AFFECTED_ERROR 11
#define SQLITE_RELATED_ERROR 12
#define DB_ROW_EXISTS 13
#define DB_ROW_NX 14
#define WRONG_MASTER_PASSWORD 15

/*-----------This enum is utilised by the update_db_field() to indicate which field is being updated in the database----------*/
enum db_fields {
    DB_ACC_NAME,
    DB_USER_NAME,
    DB_ACC_PASSWORD
};

/*
 * open_db_con() will invoke sqlite3_open() to get a database handler for database with name DB_FILE
 *
 * On success it return a database handler of type sqlite3*
 * On failure it returns NULL
 *
*/
sqlite3* open_db_con();

/*
 * create_new_database() will be invoked automatically when open_db_con() is called and the DB_FILE doesn't exist on 
 * the user's filesystem. It will prompt the user to create a new database and initialse it with a master passowrd
 * also propmpted from the user.
 *
 * Returns:
 * SUCCESS_OP on success.
 * GENERAL_ERROR if an error is reported when getting user input.
 * SQLITE_RELATED_ERROR if sqlite3 reports back and error.
 */
int create_new_database();

/*
 * get_all_credentials() is used to return all credentials in a database and initialse them in a struct account_list linked list
 *
 * Paramters:
 * 1. db: A handle to an open and decrypted db.
 * 2, acc_list: A pointer to an already initialsed account_list
 *
 * Returns:
 * SUCCESS_OP: on success
 * SQLITE_RELATED_ERROR: if sqlite3 reports an error.
 * GENERAL_ERROR: otherwise eg if db or acc_list are NULL.
 */
int get_all_credentials(sqlite3 *db, struct account_list *acc_list);

/*
 * get_account_by_name() is used to search return a single credential in a database and initialse them in a struct account object
 *
 * Paramters:
 * 1. db: A handle to an open and decrypted db.
 * 2. acc_name The name of the account to search for in the database.
 * 3. acc: An already initialsed pointer to a struct account.
 *
 * Note: The char* pointers inside the acc struct will point to heap memory allocated by malloc so the caller must call free()
 * on all three strings to ensure no memory leaks.
 *
 * Returns:
 * SUCCESS_OP: on success
 * SQLITE_RELATED_ERROR: if sqlite3 reports an error.
 * GENERAL_ERROR: otherwise eg if db or acc are NULL.
 */
int get_account_by_name(sqlite3* db, char* acc_name, struct account *acc);

/*
 * update_db_field() is used to update a field about an account in the database,
 *
 * Paramters:
 * 1. db: A handle to an open and decrypted database.
 * 2. toUpdate: A value from enum db_fields to indicate which field is being updated.
 * 3. acc_name: The name of the account to be modified.
 * 4. new_field_value: The new value to update.
 *
 * Returns:
 * SUCCESS_OP: on success
 * NON_AFFECTED_ERROR: If nothing in the database was changed.
 * SQLITE_RELATED_ERROR: if sqlite3 reports any other error.
 * GENERAL_ERROR: otherwise eg if db or acc are NULL.
 */
int update_db_field(sqlite3 *db, enum db_fields toUpdate, char *acc_name, char *new_field_value);

/*
 * delete_account_from_db() is used to delete an account from the database.
 *
 * Paramters:
 * 1. db: A handle to an open and decrypted db.
 * 2. acc_name The name of the account to delete in the database.
 *
 * Returns:
 * SUCCESS_OP: on success
 * NON_AFFECTED_ERROR: If nothing in the database was changed.
 * SQLITE_RELATED_ERROR: if sqlite3 reports any other error.
 * GENERAL_ERROR: otherwise eg if db or acc are NULL.
 */
int delete_account_from_db(sqlite3* db, char* acc_name);

/*
 * add_account_from_db() is used to add an account from the database.
 *
 * Paramters:
 * 1. db: A handle to an open and decrypted db.
 * 2. acc: An initialsed Account object containing the information about the account to be added in the database 
 *
 * Returns:
 * SUCCESS_OP: on success
 * NON_AFFECTED_ERROR: If nothing in the database was changed.
 * SQLITE_RELATED_ERROR: if sqlite3 reports an other error.
 * GENERAL_ERROR: otherwise eg if db or acc are NULL.
 */
int add_account_to_db(sqlite3 *db, Account acc);

/*
 *decrypt_db() is called internally by the open_db_con() to decrypt the database associated with the handle *db. It prompts the user for the master password
 * and confirms if the database was actually decrypted with the master password.
 * 
 * Returns:
 * SUCCESS_OP: on success
 * WRONG_MASTER_PASSWORD: if the master password could not decrypt the database.
 * SQLITE_RELATED_ERROR: if sqlite reported back an error.
 * GENERAL_ERROR: otherwise.
*/
int decrypt_db(sqlite3* db);


/*
* change_db_master_password() is used to change the master password for the database associated with *db. It prompts the user
* for the new master password to be used.
*
 * Returns:
 * SUCCESS_OP: on success
 * SQLITE_RELATED_ERROR: if sqlite3 reports an error.
 * GENERAL_ERROR: otherwise eg if db or acc are NULL.
 */
int change_db_master_password(sqlite3 *db);

/*
* check_account_exists()  is used to find out credentials for account with name acc_name exist in the database.
*
* Returns:
* DB_ROW_EXISTS: If the account exists in the database.
* DB_ROW_NX: If the account doesn't exist.
* SQLITE_RELATED_ERROR: If sqlite3 reports an error.
* GENERAL_ERROR: otherwise
*/
int check_account_exists(sqlite3* db, char* acc_name);


#endif



