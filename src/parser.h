#ifndef L_PARSER
#define L_PARSER

#include <string.h>
#include <stdbool.h>

typedef struct {
    char *pname;
    char *pvalue;
} http_param, *phttp_param;

typedef struct node_list_param node_list_param, *pnode_list_param;

struct node_list_param {
	pnode_list_param next;
	http_param value;
};

typedef struct {
	pnode_list_param next;
	pnode_list_param last;
	size_t len;
} list_param;

//extern const struct http_param l_error_param;

bool paramComp(const http_param p,const http_param d);
int init_param(char * key, char * value, phttp_param p);
int destroy_param(phttp_param p);
list_param parser(char *buffer, size_t s);

list_param list_param_init();
pnode_list_param list_param_addlast(list_param * obj, http_param val);
pnode_list_param list_param_at(list_param * obj, int index);
int list_param_free(list_param * obj);
#endif
