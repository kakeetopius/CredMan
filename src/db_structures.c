#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/db_structures.h"

DB_BIND_SET* create_bind_set() {
    DB_BIND_SET* bind_set;
    bind_set = (DB_BIND_SET*)malloc(sizeof(DB_BIND_SET));
    if (!bind_set) {
	printf("Malloc Error: Could not get bind set\n");
	return NULL;
    }
    
    bind_set->first_bind = NULL;
    bind_set->last_bind = NULL;
    return bind_set;
}

int dbstruct_insert_bind_info(DB_BIND_SET *bind_set, DB_DATA_TYPE type, void* value, int value_len) {
    if (!bind_set) {
	printf("Bind set not initialised\n");
	return -1;
    }

    DB_BIND_INFO* bind_info;
    bind_info = (DB_BIND_INFO*) malloc(sizeof(DB_BIND_INFO));
    if (!bind_info) {
	printf("Malloc error: Could not get bind info struct");
	return -1;
    }

    switch(type) {
	case DB_TYPE_INT:
	    bind_info->int_value = *(int *)value;
	    bind_info->value_len = 0;
	    break;
	case DB_TYPE_FLOAT:
	    bind_info->float_value = *(float *)value;
	    bind_info->value_len = 0;
	    break;
	case DB_TYPE_STRING:
	    bind_info->string_value = strdup((char *)value);
	    bind_info->value_len = value_len;
	    break;
	default:
	    printf("Invalid Type\n");
	    free(bind_info);
	    return -1;
    }

    bind_info->type = type;
    bind_info->next = NULL;
    bind_info->prev = NULL;

    /*---------------Inserting into bind set----------------------*/
    if (!bind_set->first_bind) {
	bind_set->first_bind = bind_info;
	bind_set->last_bind = bind_info;
	return 0;
    }
    else {
	bind_set->last_bind->next = bind_info;
	bind_info->prev = bind_set->last_bind;
	bind_set->last_bind = bind_info;
	return 0;
    }
}

void dbstruct_destroy_bind_set(DB_BIND_SET *bind_set) {
    if (!bind_set) {
	return;
    }
    
    DB_BIND_INFO* info = bind_set->first_bind;
    DB_BIND_INFO* tmp;

    if(!info) {
	free(bind_set);
	return;
    }

    while(info != NULL) {
	if (info->type == DB_TYPE_STRING) {
	    free(info->string_value);
	}

	tmp = info->next;
	free(info);
	info = tmp;
    }

    free(bind_set);
}


DB_RESULT_SET* make_result_set() {
    DB_RESULT_SET *result_set;
    result_set = (DB_RESULT_SET*) malloc(sizeof(DB_RESULT_SET));	
    if (!result_set) {
	printf("Malloc error: Could not get result set\n");
	return NULL;
    }
    
    result_set->field_metadata_first = NULL;
    result_set->field_metadata_last = NULL;
    result_set->first_row = NULL;
    result_set->last_row = NULL;
    result_set->num_of_fields = 0;
    result_set->num_of_rows = 0;

    return result_set;
}


int dbstruct_insert_field_meta(DB_RESULT_SET *result_set, char* field_name, DB_DATA_TYPE type) {
    if (!result_set) {
	printf("Unintialised result set\n");
	return -1;
    }

    DB_FIELD_META* field_meta;
    field_meta = (DB_FIELD_META*) malloc(sizeof(DB_FIELD_META));
    if(!field_meta) {
	printf("Malloc Error: Could not get field meta data structure\n");
	return -1;
    }

    field_meta->field_name = strdup(field_name);
    field_meta->field_type = type;
    field_meta->next = NULL;
    field_meta->prev = NULL;

    if (!result_set->field_metadata_first) {
	result_set->field_metadata_first = field_meta;
	result_set->field_metadata_last = field_meta;
	return 0;
    }
    else {
	result_set->field_metadata_last->next = field_meta;
	field_meta->prev = result_set->field_metadata_last;
	result_set->field_metadata_last = field_meta;
	return 0;
    }
    
}

DB_ROW* create_row() {
    DB_ROW* row;
    row = (DB_ROW *) malloc(sizeof(DB_ROW));
    if (!row) {
	printf("Malloc Error: Could not get DB row\n");
	return NULL;
    }

    row->next = NULL;
    row->prev = NULL;
    row->first_field = NULL;
    row->last_field = NULL;

    return row;
}

int dbstruct_insert_row(DB_RESULT_SET *result_set, DB_ROW *row) {
    if (!result_set) {
	printf("Unintialised result set\n");
	return -1;
    }

    if (!row) {
	printf("Uninitialised row\n");
	return -1;
    }

    if (!result_set->first_row) {
	result_set->first_row = row;
	result_set->last_row = row;
	return 0;
    }
    else {
	result_set->last_row->next = row;
	row->prev = result_set->last_row;
	result_set->last_row = row;
	return 0;
    }
}

int dbstruct_insert_field(DB_ROW *row, DB_DATA_TYPE type, void* value, int value_len) {
    if (!row) {
	printf("Unitialised row\n");
	return -1;
    }

    DB_FIELD* field = (DB_FIELD*) malloc(sizeof(DB_FIELD));
    if (!field) {
	printf("Malloc Error: Failed to initialse field for result set\n");
	return -1;
    }

    switch(type) {
	case DB_TYPE_INT:
	    field->int_value = *(int *)value;
	    field->value_len = 0;
	    break;
	case DB_TYPE_FLOAT:
	    field->float_value = *(float *)value;
	    field->value_len = 0;
	    break;
	case DB_TYPE_STRING:
	    field->string_value = strdup((char *)value);
	    field->value_len = value_len;
	    break;
	default:
	    printf("Invalid Type\n");
	    free(field);
	    return -1;
    }

    field->type = type;
    field->prev = NULL;
    field->next = NULL;

    /*---------Inserting into row--------------------*/
    if (!row->first_field) {
	row->first_field = field;
	row->last_field = field;
    }
    else {
	row->last_field->next = field;
	field->prev = row->last_field;
	row->last_field = field;
	return 0;
    }
    return 0;
}

void dbstruct_destroy_result_set(DB_RESULT_SET *result_set) {
    if (!result_set) {
	return;
    }

    /*-------------Freeing field metadata-------------------*/
    DB_FIELD_META* meta = result_set->field_metadata_first;
    if (meta) {
	DB_FIELD_META *tmp;
	while (meta != NULL) {
	    free(meta->field_name);
	    tmp = meta->next;
	    free(meta);
	    meta = tmp;
	}	
    }

    /*--------------Freeing rows-------------------------*/
    DB_ROW* row = result_set->first_row;
    if(!row) {
	free(result_set);
	return;
    }

    DB_ROW* tmp_row;
    DB_FIELD* field;
    DB_FIELD* tmp_field;
    while (row != NULL) {
	field = row->first_field;
	if(field) {
	    while (field != NULL) {
		if (field->type == DB_TYPE_STRING) {
		    free(field->string_value);
		}
		tmp_field = field->next;
		free(field);
		field = tmp_field;
	    }
	}
	tmp_row = row->next;
	free(row);
	row = tmp_row;
    }

    free(result_set);
}


