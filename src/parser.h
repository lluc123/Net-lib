#ifndef L_PARSER
#define L_PARSER

#include <string.h>
#include <stdbool.h>

struct http_param {
    char *pname;
    char *pvalue;
};

extern const struct http_param l_error_param;

bool paramIsNotNull(const struct http_param p);
bool paramComp(const struct http_param p,const struct http_param d);
int init_param(char * key, char * value, struct http_param* p);
int destroy_param(struct http_param p);
int clear_param(struct http_param* p);
struct http_param* parser(char *buffer, size_t s, char separator, char setter);
#endif