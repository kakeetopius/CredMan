#ifndef API_OBJ_H
#define API_OBJ_H

typedef struct api_node {
    char *name;
    char *service;
    char *username;
    char *key;
    struct api_node *next;
    struct api_node *prev;
} api_node, *Api_node;

typedef struct api_list {
    struct api_node *head;
    struct api_node *tail;
} api_list, *Api_list;

typedef struct api_key {
    char *name;
    char *service;
    char *username;
    char *key;
} api_key, *Api_Key;

void destroyApiList(Api_list lst);
int insert_apinode(Api_list lst, char *name, char *uname, char *service, char *key);
Api_list createApiList();
#endif
