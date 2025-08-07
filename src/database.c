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

void get_results() {
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

    int field_num = mysql_num_fields(res_meta);
    printf("Number of fields: %d\n", field_num);


    /*--------------Results--------------------*/
    unsigned long real_lengths[6] = {0};
    MYSQL_BIND result_bind[6] = {0};
    int results_size = sizeof(result_bind)/sizeof(result_bind[0]);

    for (int i = 0; i < results_size; i++) {
	result_bind[i].buffer = 0;
	result_bind[i].buffer_length = 0;
	result_bind[i].length = &real_lengths[i];
    }

    status = mysql_stmt_bind_result(stmt, result_bind);
    if (status != 0) {
	printf("Mysql Error: %s\n", mysql_stmt_error(stmt));
	return;
    }

    printf("Results bound\n");

    printf("Fetching Ok\n");

    MYSQL_FIELD* field = NULL;
    while((field = mysql_fetch_field(res_meta))) {
	printf("%s\t\t", field->name);
    }
    printf("\n");

    while(mysql_stmt_fetch(stmt) != MYSQL_NO_DATA){
	for (int i = 0; i < results_size; i++) {
	    if (real_lengths[i] > 0) {
		char* buff = malloc(real_lengths[i]);
		result_bind[0].buffer = buff;
		result_bind[0].buffer_length = real_lengths[i];
		mysql_stmt_fetch_column(stmt, result_bind, i, 0);
		printf("%*s\t\t", (int)real_lengths[i], buff);
	    }
	}
	printf("\n");
    }

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

