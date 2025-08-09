#ifndef DB_STRUCTUES_H
#define DB_STRUCTUES_H
/*---------------STRUCTS FOR QUERYING AND RECEIVING RESULTS-----*/

/*---------Enum for data types used in this project----------------------*/
typedef enum db_data_type {
    DB_TYPE_INT,
    DB_TYPE_STRING,
    DB_TYPE_FLOAT
} DB_DATA_TYPE;

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
DB_BIND_SET* create_bind_set();
DB_RESULT_SET* make_result_set();
DB_ROW* create_row();

int dbstruct_insert_bind_info(DB_BIND_SET *bind_set, DB_DATA_TYPE type, void* value, int value_len);
void dbstruct_destroy_bind_set(DB_BIND_SET *bind_set);

int dbstruct_insert_field_meta(DB_RESULT_SET *result_set, char* field_name, DB_DATA_TYPE type);

int dbstruct_insert_row(DB_RESULT_SET *result_set, DB_ROW *row);

int dbstruct_insert_field(DB_ROW *row, DB_DATA_TYPE type, void* value, int value_len);

void dbstruct_destroy_result_set(DB_RESULT_SET *result_set);

void dbstruct_print_result_set(DB_RESULT_SET* result_set);

#endif
