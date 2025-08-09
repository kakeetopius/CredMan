#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mariadb/mysql.h>
#include <stdbool.h>

#include "../includes/database.h"

/*---------Some global variables used by different functions in module----------*/
MYSQL* mysql;

int main(void) {
    DB_INFO* db = get_dbinfo();

    if (!db) {
	printf("Could not get database credentials\n");
	return -1;
    }
    
    mysql = mysql_init(NULL);

    if (!mysql) {
	printf("Mysql handle could not be created\n");
	return -1;
    }

    MYSQL* mysql_status = mysql_real_connect(mysql, db->host, db->user, db->passwd, db->dbname, db->port, NULL, 0);

    if (!mysql_status) {
	printf("Error connecting to mysql Database: %s\n", mysql_error(mysql));
	return -1;
    }

    printf("Connection successfully made to Mysql Database\n");
    char* query = "SELECT * FROM Client_sync";
    

    DB_RESULT_SET* result_set = dbstruct_make_result_set();
    if (result_set == NULL) {
	printf("Result set error\n");
	return -1;
    }
    int status;

    status = query_database(query, NULL, 0, result_set, QUERY_AND_GET_RESULTS);
    printf("Affected Rows: %d\n", status);

    printf("Printing Result Set-------------------------\n");
    dbstruct_print_result_set(result_set);
    
    dbstruct_destroy_result_set(result_set);
    mysql_close(mysql);
    free_dbinfo(db);
}

int query_database(char* query, DB_BIND_SET* bind_set, int num_of_binds, DB_RESULT_SET* result_set, QUERY_OPTIONS options) {
    /*-------------Statement Handler--------------------*/

    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (!stmt) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	return -1;
    }

    /*---------------Statement Preparation-------------*/
    int status = mysql_stmt_prepare(stmt, query, strlen(query));
    unsigned long param_count = mysql_stmt_param_count(stmt);

    if (status != 0) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	mysql_stmt_close(stmt);
	return -1;
    }
    if (num_of_binds != param_count) {
	printf("Bind paramters count mismatch\n");
	mysql_stmt_close(stmt);
	return -1;
    }
    printf("Statment prepared well\n");

    MYSQL_BIND* bind;
    /*---------------Statement Binding-----------------*/
    if (num_of_binds > 0) {
	bind = (MYSQL_BIND*) calloc(num_of_binds, sizeof(MYSQL_BIND)); 
	if (!bind) {
	    printf("Calloc Error: could not get bind structure\n");
	    mysql_stmt_close(stmt);
	    return -1;
	}

	int bind_count = 0;
	DB_BIND_INFO* bind_info = bind_set->first_bind;
	while (bind_info != NULL) {
	    if (bind_count >= num_of_binds) {
		break;
	    }
	    switch(bind_info->type) {
		case DB_TYPE_INT:
		    bind[bind_count].is_unsigned = true;
		    bind[bind_count].is_null = 0;
		    bind[bind_count].buffer_type = MYSQL_TYPE_LONG;
		    bind[bind_count].buffer = &bind_info->int_value;
		    bind[bind_count].length = 0;
		    break;
		case DB_TYPE_FLOAT:
		    bind[bind_count].length = 0;
		    bind[bind_count].is_null = 0;
		    bind[bind_count].buffer_type = MYSQL_TYPE_FLOAT;
		    bind[bind_count].buffer = &bind_info->float_value;
		    break;
		case DB_TYPE_STRING:
		    bind[bind_count].length = (unsigned long*)&bind_info->value_len;
		    bind[bind_count].is_null = 0;
		    bind[bind_count].buffer_type = MYSQL_TYPE_STRING;
		    bind[bind_count].buffer = (char*)bind_info->string_value;
		    bind[bind_count].buffer_length = (unsigned long)bind_info->value_len;
		    break;
	    }
	    
	    bind_info = bind_info->next;
	    bind_count++;
	}

	status = mysql_stmt_bind_param(stmt, bind);
	if (status != 0) {
	    printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	    mysql_stmt_close(stmt);
	    if (num_of_binds > 0) free(bind);
	    return -1;
	}
	printf("Binding done well\n");
    }


    /*----------------Statement Execution------------*/
    status = mysql_stmt_execute(stmt);
    if (status != 0) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	if (num_of_binds > 0) 	free(bind);
	mysql_stmt_close(stmt);
	return -1;
    }

    printf("Statement executed\n");

    /*---------If no RESULT SET expected--------------------------*/
    if (options == QUERY_ONLY) {
	int affected_rows = mysql_affected_rows(mysql);
	if (num_of_binds > 0) free(bind);
	mysql_stmt_close(stmt);
	return affected_rows;
    }

    /*----------------RESULT SET----------------------------*/

    /*---------------Statement metadata--------------------*/
    MYSQL_RES* res_meta = mysql_stmt_result_metadata(stmt);
    if (!res_meta) {
	printf("No result metadata\n");
	mysql_stmt_close(stmt);
	if (num_of_binds > 0) 	free(bind);
	return -1;
    }

    result_set->num_of_fields = (int) mysql_num_fields(res_meta);


    MYSQL_FIELD* res_meta_field = mysql_fetch_fields(res_meta);
    if (!res_meta_field) {
	printf("No data collected\n");
	if (num_of_binds > 0) 	free(bind);
	mysql_free_result(res_meta);
	mysql_stmt_close(stmt);
	return -1;
    }

    for (int i = 0; i < result_set->num_of_fields; i++) {
	DB_DATA_TYPE db_type;
	switch(res_meta_field[i].type) {
	    case MYSQL_TYPE_LONG:		
		db_type = DB_TYPE_INT;
		break;
	    case MYSQL_TYPE_FLOAT:
	    case MYSQL_TYPE_DOUBLE:
		db_type = DB_TYPE_FLOAT;
		break;
	    case MYSQL_TYPE_STRING:
	    case MYSQL_TYPE_VARCHAR:
	    case MYSQL_TYPE_VAR_STRING:
		db_type = DB_TYPE_STRING;
		break;
	    default:
		continue;
	}

	dbstruct_insert_field_meta(result_set, res_meta_field[i].name, db_type);

    }
    
    /*--------------------------MYSQL Result Set Binding------------------------------*/
    MYSQL_BIND* res_bind;
    res_bind = (MYSQL_BIND*) calloc(result_set->num_of_fields, sizeof(MYSQL_BIND));
    
    unsigned long* lengths = calloc(result_set->num_of_fields , sizeof(unsigned long));

    my_bool* is_null = calloc(result_set->num_of_fields, sizeof(my_bool));

    if (!res_bind || !lengths || !is_null) {
	printf("Calloc Error: Could not get result bind info\n");
    	if (num_of_binds > 0) 	free(bind);
	mysql_free_result(res_meta);
	mysql_stmt_close(stmt);
    }
    
    printf("Result Bind structures set up successfully\n");
    
    /*------------------Setting up mysql bind for the different fields------------------*/
    DB_FIELD_META* field_meta = result_set->field_metadata_first;
    int int_value;
    float float_value;
    
    for (int i = 0; i < result_set->num_of_fields; i++) {
	if (!field_meta) {
	    break;
	}
	switch (field_meta->field_type) {
	    case DB_TYPE_INT:
		res_bind[i].buffer_type = MYSQL_TYPE_LONG;
		res_bind[i].buffer = (char* )&int_value;
		res_bind[i].is_null = &is_null[i];
		res_bind[i].length = 0;
		res_bind[i].buffer_length = 0;
		break;
	    case DB_TYPE_FLOAT:
		res_bind[i].buffer_type = MYSQL_TYPE_FLOAT;
		res_bind[i].buffer = (char* )&float_value;
		res_bind[i].is_null = &is_null[i];
		res_bind[i].length = 0;
		res_bind[i].buffer_length = 0;
		break;
	    case DB_TYPE_STRING:
		res_bind[i].buffer = 0;
		res_bind[i].buffer_length = 0;
		res_bind[i].is_null = &is_null[i];
		res_bind[i].length = &lengths[i];
		break;
	}
	field_meta = field_meta->next;
    }

    status = mysql_stmt_bind_result(stmt, res_bind);
    if (status != 0) {
	printf("Could not bind results\n");
	if (num_of_binds > 0) 	free(bind);
	free(res_bind);
	free(lengths);
	free(is_null);
	mysql_free_result(res_meta);
	mysql_stmt_close(stmt);
    }
    printf("Result buffers bound successfully\n");
    
    int num_rows = 0;
    while(1) {
	status = mysql_stmt_fetch(stmt);
	if (status == 1 || status == MYSQL_NO_DATA) {
	    break;
	}
	DB_ROW* row = dbstruct_create_row();

	for (int i = 0; i < result_set->num_of_fields; i++) {
	    if (res_bind[i].buffer_type == MYSQL_TYPE_LONG) {
		status = mysql_stmt_fetch_column(stmt, &res_bind[i], i, 0);
		if (status != 0) {
		    continue;
		} 
		dbstruct_insert_field(row, DB_TYPE_INT, res_bind[i].buffer, 0);
	    }
	    else if (res_bind[i].buffer_type == MYSQL_TYPE_FLOAT || res_bind[i].buffer_type == MYSQL_TYPE_DOUBLE) {
		status = mysql_stmt_fetch_column(stmt, &res_bind[i], i, 0);
		if (status != 0) {
		    continue;
		} 
		dbstruct_insert_field(row, DB_TYPE_FLOAT, res_bind[i].buffer, 0);
	    }
	    else {    //for strings
		char* buff = calloc(1, lengths[i]);
		if (!buff) {
		    printf("Calloc Error\n");
		    continue;
		}
		res_bind[i].buffer = (char *)buff;
		res_bind[i].buffer_length = lengths[i];

		status = mysql_stmt_fetch_column(stmt, &res_bind[i], i, 0);
		if (status != 0) {
		    continue;
		    free(buff);
		} 
		dbstruct_insert_field(row, DB_TYPE_STRING, res_bind[i].buffer, 0);
		free(buff);
    	    }
	
	}
	
	dbstruct_insert_row(result_set, row);
	num_rows++;
    }
    
    result_set->num_of_rows = num_rows;


    free(res_bind);
    free(lengths);
    free(is_null);
    if (num_of_binds > 0)
	free(bind);
    mysql_free_result(res_meta);
    mysql_stmt_close(stmt);
    return num_rows;
}

DB_INFO* get_dbinfo() {
    FILE* cred_file = fopen(DB_CONFIG_FILE, "r"); 
    if (!cred_file) {
	perror("Error opening credential file ");
	return NULL;
    }
    
    DB_INFO* db_info = (DB_INFO*) malloc(sizeof(DB_INFO));
    db_info->user = NULL;
    db_info->host = NULL;
    db_info->passwd = NULL;
    db_info->dbname = NULL;
    db_info->port = 3306;

    char buff[100] = {0};
    char* saveptr;

    while(fgets(buff, sizeof(buff), cred_file)) {
	if (strcmp(buff, "\n") == 0) {     //if empty line
	    continue;
	}
	
	char* info_name = strtok_r(buff, ":", &saveptr);

	if (!info_name) {
	    printf("Error parsing credential file\n");
	    printf("Syntax Error\n");
	    fclose(cred_file);
	    return NULL;
	}

	char* info_value = strtok_r(NULL, ":", &saveptr);

	if (!info_value) {
	    printf("Error parsing credential file\n");
	    printf("Syntax Error\n");
	    fclose(cred_file);
	    return NULL;
	}

	/*---Removing new line character if found-----*/
	int newline_pos = strcspn(info_value, "\n");
	info_value[newline_pos] = '\0';

	if (strcmp(info_name, "host") == 0) {
	    db_info->host = strdup(info_value);
	}
	else if (strcmp(info_name, "uname") == 0) {
	    db_info->user = strdup(info_value);
	}
	else if (strcmp(info_name, "passwd") == 0) {
	    db_info->passwd = strdup(info_value);
	}
	else if (strcmp(info_name, "db") == 0) {
	    db_info->dbname = strdup(info_value);
	}
	else if (strcmp(info_name, "port") == 0) {
	    db_info->port = atoi(info_value);
	}
	else { 
	    printf("Config File error. Unknown field: %s\n", info_name);
	    free_dbinfo(db_info);
	    fclose(cred_file);
	    return NULL;
	}
    }
    
    if (!db_info->host || !db_info->user || !db_info->dbname || !db_info->passwd) {
	printf("Config File error. Missing some crucial field(s)\n");
	free_dbinfo(db_info);
	return NULL;
    }

    fclose(cred_file);
    return db_info;
} 

void free_dbinfo(DB_INFO *db_info) {
    if (!db_info) {
	return;
    } 

    if (db_info->host) {
	free(db_info->host);
    }
    if (db_info->passwd) {
	free(db_info->passwd);
    }
    if (db_info->user) {
	free(db_info->user);
    }
    if (db_info->dbname) {
	free(db_info->dbname);
    }

    free(db_info);
} 



