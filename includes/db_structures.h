#ifndef DB_STRUCTUES_H
#define DB_STRUCTUES_H

#define SERVER_CONFIG_FILE  "/home/pius/Dev/Projects/Cred_Manager/config/server.conf"

#define DB_CONFIG_FILE      "/home/pius/Dev/Projects/Cred_Manager/config/db.conf"


/*---------------STRUCTS FOR QUERYING AND RECEIVING RESULTS-----*/

/*-----------DATABASE DETAILS STRUCT------------*/
typedef struct db_info {
    char* host;
    char* user;
    char* passwd;
    char* dbname;
    int port;
} DB_INFO;

/*---------Enum for data types used in this project----------------------*/
typedef enum db_data_type {
    DB_TYPE_INT,
    DB_TYPE_STRING,
    DB_TYPE_FLOAT
} DB_DATA_TYPE;

/*------------Wrapper structure to hold the binding structures---------------*/
typedef struct db_bind_set {
    struct db_bind_st *first_bind;
    struct db_bind_st *last_bind;
} DB_BIND_SET;

/*------------Structure for specifiying bind values when querying with prepared statements-----------------*/
typedef struct db_bind_st {
    enum db_data_type type;
    union {
	int int_value;
	float float_value;
	char* string_value;
    }; 
    int value_len;
    struct db_bind_st *prev;
    struct db_bind_st *next;
} DB_BIND_INFO;

/*-----------Structure for result set after querying---------------------------*/
typedef struct db_result_set {
    int num_of_fields;
    int num_of_rows;
    struct db_field_meta *field_metadata_first;
    struct db_field_meta *field_metadata_last;
    struct db_row *first_row;
    struct db_row *last_row;
} DB_RESULT_SET;

/*------------Structure for the field metadata in a result set------------------*/
typedef struct db_field_meta {
    char* field_name;
    enum db_data_type field_type;
    struct db_field_meta *next;
    struct db_field_meta *prev;
} DB_FIELD_META;


/*-----------Structure for a single row in a result_set-------------------------*/
typedef struct db_row {
    struct db_field *first_field;
    struct db_field *last_field;
    struct db_row *next;
    struct db_row *prev;
} DB_ROW;


/*------------Structure for a column/field in a result set-----------------------*/
typedef struct db_field {
    enum db_data_type type;
    union {
	int int_value;
	float float_value;
	char* string_value;
    }; 
    int value_len;
    struct db_field *next;
    struct db_field *prev;
} DB_FIELD;


/*--------------Function APIs to work with the above structures----------------*/

/*Function allocates memory for a bind set and returns a pointer to it*/
/*The bind set must freed after wards by passing it dbstruct_destry_bind_set*/
DB_BIND_SET* dbstruct_create_bind_set();

/*Function allocates memory for a result set and returns a pointer to it*/
/*The result_set structure must be freed afterwards by passing it to dbstruct_destroy_result_set*/
DB_RESULT_SET* dbstruct_make_result_set();

/*Function allocates memory for a row to use in a result set and returns a pointer to it*/
/*The row structure is automatically freed by a call to dbstruct_destroy_result_set with the result set
where it belongs*/
DB_ROW* dbstruct_create_row();

/*Function inserts binding information for a parameter within a query that needs to be prepared into a
 *DB_BIND_SET to pass to query_database() function*/
/*
 * Paramters:
 * 1. bind_set: a pointer to a struct DB_BIND_SET created by dbstruct_create_bind_set() function to which the binding information should be added.
 * 2. type: the type of data to be inserted. The options are memebers of DB_DATA_TYPE
 * 3. value: is a buffer containing the data to be bound
 * NOTE: for string buffers it is better to not use string literals but use stack based buffers/arrays or dynamically allocated strings that are properly null terminated.
 * 4. value_len: the length of the buffer value to be inserted. It is only useful for DB_DATA_TYPE DB_TYPE_STRING when the value is a string and it should correspond to the buffer size and not necessarily the string length itslef.. For the rest it is not considered and a 0 may be passed.
 *
 * Return Values:
 * 1.  0 on success
 * 2.  a number less than 0 on error.
 */
int dbstruct_insert_bind_info(DB_BIND_SET *bind_set, DB_DATA_TYPE type, void* value, int value_len);

/*Function inserts meta data information about fields into a result set. Only used inside the database.c module and is not applicable any where else
 * Paramters:
 * 1. result_set: A pointer to struct DB_RESULT_SET to which the metadata is to be added
 * 2. field_name: The name of the field to add into the result_set metadata
 * 3. type: the data type of the field to be added. Options are the members of DB_DATA_TYPE
 *
 * Return Values:
 * 1. 0 on success
 * 2. a number less than 0 on error
 */
int dbstruct_insert_field_meta(DB_RESULT_SET *result_set, char* field_name, DB_DATA_TYPE type);

/*Function inserts a row into a result_set. Only used inside the database.c module and is not applicable anywhere else
 * Parameters:
 * 1. result_set: A pointer to struct DB_RESULT_SET to add the row
 * 2. row: A struct for the row to add. Row should have been initialised with dbstruct_create_row and should have been populated already with dbstruct_insert_field()
 *
 * Return Values:
 * 1. 0 on success
 * 2. a number less than 0 on error
 * */
int dbstruct_insert_row(DB_RESULT_SET *result_set, DB_ROW *row);

/*Function inserts a field information into a row of type DB_ROW. Only used in database.c module and is not applicable else where.
 * Paramters:
 * 1. row: A pointer to a structure of type DB_ROW representing the row in which to insert the field
 * 2. type: The data type of the value of the field being added. Options are memebers of DB_DATA_TYPE
 * 3. value: a pointer of the buffer where the value to add is stored
 * 4. value_len: the size in bytes of the buffer and is only applicable for type DB_TYPE_STRING for other types a 0 can suffice.
 *
 * Return Values:
 * 1. 0 on success
 * 2. a number less than 0 on error*/
int dbstruct_insert_field(DB_ROW *row, DB_DATA_TYPE type, void* value, int value_len);

/*Function frees up memory used up by a bind_set
 * Parameters:
 * 1. bind_set: A pointer to the bind_set of type DB_BIND_SET to free*/
void dbstruct_destroy_bind_set(DB_BIND_SET *bind_set);


/*Function frees up all memory used up by a result set including all its field metadata and rows
 * Parameters:
 * 1. result_set: The result set to free*/
void dbstruct_destroy_result_set(DB_RESULT_SET *result_set);


/*Function prints out information in a result set, useful for debugging or logging
 * Parameters:
 * 1. result_set: the result set to print out*/
void dbstruct_print_result_set(DB_RESULT_SET* result_set);

/*Function retrieves database information from the configuration file db.conf
 * Return Value:
 * A pointer to struct DB_INFO containing information retrieved from the configuration file
 * The struct should be freed afterwards with a call to free_dbinfo() function*/
DB_INFO* get_dbinfo();

/*Function frees up memory utilised by a DB_INFO struct that contained configuration details*/
void free_dbinfo(DB_INFO* db_info);
#endif
