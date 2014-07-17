#ifndef _HttpClient_H
#define _HttpClient_H
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <sys/types.h>
#include "http_parser.h"

#define MAXLINE 4096

#define alloc_cpy(dest, src, len) \
	dest = malloc(len + 1);\
	memcpy(dest, src, len);\
	dest[len] = '\0';

//typedef char * string;

typedef struct HttpClient HttpClient;
/*
typedef enum {
    false,
    true
} bool;
*/
typedef struct http_url {
    char *scheme, *host, *user, *pass, *path, *query, *fragment;
    char *port;
}http_url;

typedef struct http_header {
    char *name, *value;
    struct http_header *next;
}http_header;

struct request {
    int method;
    const char *host;
    const char *path;
    int port;
    http_header *headers;
    const char *body;
} ;

//https://github.com/georgi/hitpoint/blob/master/hitpoint.h
typedef struct http_response {
    int fd;
    int status;
    int complete;
    int headers_complete;
    int header_state;
    http_header *headers;
    char *body;
    unsigned int pos;
    unsigned int content_length;
} http_response;

HttpClient* New_HttpClient(void);

struct HttpClient {
	char *url;
	char *method;
	char *path;
	char *host;
	char *user_agent;
    http_url *urls;
	http_header *headers;
    http_response *response;
		
	bool (*open)(HttpClient* self, const char* url);
	int (*connect)(HttpClient* self, char *host, u_short port);
	bool (*setDefaultHeader)(HttpClient* self);
	bool (*setHeader)(HttpClient* self, char* header_name, char* header_value);
	bool (*addHeader)(HttpClient* self, char* header_name, char* header_value);
    struct http_url *(*parse_url)(HttpClient* self, char *url);
	bool (*setMaxRedirects)(HttpClient* self, int n);
	char* (*get)(HttpClient* self, char* url);
    char* (*format_request)(HttpClient* self);
	bool (*free)(HttpClient* self);
};

char *aprintf(char **s, const char *fmt, ...);
static char *extract_url_part(char *, struct http_parser_url *, enum http_parser_url_fields);
static int http_read_headers(http_parser *parser, http_response *response);

#endif
