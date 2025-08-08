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
    get_results();
    mysql_close(mysql);
    free_dbinfo(db);
}

void query_and_get_results(char* query) {
    /*-------------Statement Handler--------------------*/

    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (!stmt) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	return;
    }

    /*---------------Statement Preparation-------------*/
    char* query = "SELECT * FROM Account";
    int status = mysql_stmt_prepare(stmt, query, strlen(query));
    // unsigned long param_count = mysql_stmt_param_count(stmt);

    if (status != 0) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	return;
    }
    printf("Statment prepared well\n");

 //    /*---------------Statement Binding-----------------*/
 //    int id = 3;
	//
 //    MYSQL_BIND* bind = (MYSQL_BIND*) calloc(param_count, sizeof(MYSQL_BIND));
 //    if (!bind) {
	// printf("Calloc Error\n");
	// return;
 //    }
	//
 //    bind[0].is_unsigned = true;
 //    bind[0].is_null = (my_bool *) 0;
 //    bind[0].buffer_type = MYSQL_TYPE_LONG;
 //    bind[0].buffer = &id;
 //    
 //    status = mysql_stmt_bind_param(stmt, bind);
 //    if (status != 0) {
	// printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	// return;
 //    }
	//
 //    printf("Binding done well\n");
    /*----------------Statement Execution------------*/
    status = mysql_stmt_execute(stmt);
    if (status != 0) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	return;
    }

    printf("Statement executed\n");

    /*---------------Statement metadata--------------------*/
    MYSQL_RES* res_meta = mysql_stmt_result_metadata(stmt);
    if (!res_meta) {
	printf("No result metadata");
    }

    unsigned int num_meta_fields = mysql_num_fields(res_meta);
    printf("Results have %llu rows\n", mysql_num_rows(res_meta));
    printf("Result has %u fields\n", num_meta_fields);

    MYSQL_FIELD* res_meta_field = mysql_fetch_fields(res_meta);
    if (!res_meta_field) {
	printf("Could not get fields from result metadata\n");
	return;
    }

    MYSQL_BIND* bind_res = (MYSQL_BIND*) calloc(num_meta_fields, sizeof(MYSQL_BIND)); 
    unsigned long* lengths = (unsigned long*) calloc(num_meta_fields, sizeof(unsigned long));
    my_bool* is_null = (my_bool*) calloc(num_meta_fields, sizeof(my_bool));

    if (!bind_res || !lengths || !is_null) {
	printf("Calloc error: Could not get bind structures for result\n");
	return;
    }
    
    printf("Bind structures set up successfully\n");

    int num_res = 1;
    for (int i = 0; i < num_meta_fields; i++) {
	if (res_meta_field[i].type == MYSQL_TYPE_LONG) {
	    bind_res[i].buffer_type = MYSQL_TYPE_LONG;
	    bind_res[i].buffer = &num_res;
	    continue;
	}
	bind_res[i].buffer = 0;
	bind_res[i].buffer_length = 0;	
	bind_res[i].is_null = &is_null[i];
	bind_res[i].length = &lengths[i];
    }

    status = mysql_stmt_bind_result(stmt, bind_res);

    if (status != 0) {
	printf("Could not bind results\n");
	return;
    }
    printf("Results bound successfully\n") ;


    while(1) {
	status = mysql_stmt_fetch(stmt);
	if (status == 1 || status == MYSQL_NO_DATA) {
	    break;
	}
	
	for (int i = 0; i < num_meta_fields; i++) {
	    if (is_null[i]) {
		printf("Null Field\n");
		continue;
	    }
	    if (lengths[i] <= 0 && bind_res[i].buffer_type == MYSQL_TYPE_VAR_STRING) {
		printf("Length Zero or Less\n");
		continue;
	    }
	    

	    if (bind_res[i].buffer_type == MYSQL_TYPE_LONG) {
		status = mysql_stmt_fetch_column(stmt, &bind_res[i], i, 0);
		if (status) {
		    printf("Could not fetch column");
		}
		printf("Int: %d\n", num_res);
		continue;
	    }

	    char* buff = calloc(1,lengths[i]);
	    if (!buff) {
		printf("calloc error\n");
		continue;
	    }
	    bind_res[i].buffer = (char* )buff;
	    bind_res[i].buffer_length = lengths[i];
	    status = mysql_stmt_fetch_column(stmt, &bind_res[i], i, 0);
	    if (status != 0) {
		printf("Could not fetch column\n");
		continue;
	    }

	    switch(res_meta_field[i].type) {
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
		    printf("String: %.*s\n", (int) lengths[i], buff);
		    break;
		default:
		    printf("Can't Print\n");
		    printf("Type in Numeric Form: %d\n", res_meta_field[i].type);
		    break;
	    }

	    free(buff);
	}
	printf("\n\n");
    }
    free(bind_res);
    free(lengths);
    free(is_null);
    /*--------------Results--------------------*/
    mysql_free_result(res_meta);
    mysql_stmt_close(stmt);
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


