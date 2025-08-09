#include <stdio.h>
#include "../includes/database.h"
#include <string.h>

int main(void) {
    
    MYSQL* mysql_handle = set_up_db_connection();
    
    if(!mysql_handle) {
	return -1;
    }
    char* query = "SELECT * FROM Account WHERE user_name = ? ";
    char name[100] = "pius";
    int name_len = sizeof(name);

    DB_BIND_SET* set = dbstruct_create_bind_set();
    if (!set) {
	close_db_connection(mysql_handle);
	return -1;
    }
    dbstruct_insert_bind_info(set, DB_TYPE_STRING, name, name_len);

    DB_RESULT_SET* result_set = dbstruct_make_result_set();
    if (result_set == NULL) {
	printf("Result set error\n");
	return -1;
    }

    int status;
    status = query_database(mysql_handle, query, set, 1, result_set, QUERY_AND_GET_RESULTS);
    printf("Affected Rows: %d\n", status);

    printf("Printing Result Set-------------------------\n");
    dbstruct_print_result_set(result_set);
    
    dbstruct_destroy_bind_set(set);
    dbstruct_destroy_result_set(result_set);
    close_db_connection(mysql_handle);
}
