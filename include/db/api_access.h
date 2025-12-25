#ifndef API_ACCESS_H
#define API_ACCESS_H

#include "db/general.h"
#include "objects/api_obj.h"

enum apikey_db_fields {
    DB_API_NAME,
    DB_API_SERVICE,
    DB_API_USERNAME,
    DB_API_KEY,
};

int check_apikey_exists(sqlite3 *db, char *api_name);

int add_apikey_to_db(sqlite3 *db, Api_Key key);

int delete_apikey_from_db(sqlite3 *db, char *key_name);

int update_api_db_field(sqlite3 *db, enum apikey_db_fields toUpdate, char *api_name, char *new_field_value);

int get_db_apikey_by_name(sqlite3 *db, char *api_name, Api_Key key);

int get_all_db_apikeys(sqlite3 *db, struct api_list *api_list);

#endif
