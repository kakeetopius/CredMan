#ifndef DATABASE_H
#define DATABASE_H

#include "../includes/db_structures.h"
#include <mysql/mysql.h>


/*-------------------------OPTIONS FOR THE query_database FUNCTION-----------------*/
typedef enum {
    QUERY_ONLY,
    QUERY_AND_GET_RESULTS
}QUERY_OPTIONS;


MYSQL* set_up_db_connection();
/*Function sets up connection to a mysql database by first reading the configuration file and then makes the connection.
 * Parameters:
 *None

 * Returns:
 * NULL on error
 * A mysql handle or MYSQL* pointer on success.
 *
 * Should always call close_db_connection after use.
 * */


/*Function closes connection to a msyql database.*/
void close_db_connection(MYSQL* mysql_handle);


/*Function is used as a generic function to execute any query to a mysql database
 *
 * Parameters:
 * 1. mysql: A mysql handle returned from a call to the function set_up_db_connection(). It must be freed by user afterwards with a call to close_db_connection().
 *
 * 2. query: The sql query to send to the database. If it contains any variable information it is best to use pprepared statement with '?' as place holders instead
 *
 * 3. bind_set: A bind set containing parameters to bind if the query contains placeholders '?' ie when a prepared statement is used. If the query does not have any parameters that need binding just pass NULL.
 * The struct DB_BIND_SET can be obtained by calling dbstruct_create_bind_set() and then providing the actual parameter values using the function dbstruct_insert_bind_info()
 *
 4. num_of_binds: If a bind set is provided when using prepared statements that need binding parameters, num_of_binds should be the number of paramters that need binding. If no bind set is provided, 0 should be passed to signal that no binding needed.
 *
 * 5. results: If the statement that is to be executed is expected to return some results for example SELECT statements, then results should be a pointer to a struct DB_RESULT_SET which the function will populate with the results from the database for usage later. 
 * This struct can be obtained by calling the dbstruct_make_result_set(). Note that this struct should be freed after using it by passing it to dbstruct_destroy_result_set(). 
 * If no results are expected then user should pass NULL.
 *
 * 6. options: This parameter specifies the type of query that is going to be executed. The options are memebers of QUERY_OPTIONS enum.
 * QUERY_ONLY option should be passed if the user expects no result set.
 * QUERY_AND_GET_RESULTS option should be passed if the user expects some results from the database and the results parameter should point to a valid struct DB_RESULT_SET obtanined using the function dbstruct_make_result_set()*/
int query_database(MYSQL* msyql, char* query, DB_BIND_SET* bind_set, int num_of_binds, DB_RESULT_SET* results, QUERY_OPTIONS options);
#endif
