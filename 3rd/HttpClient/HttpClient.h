#ifndef _HttpClient_H
#define _HttpClient_H
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

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
struct http_header {
    char *name, *value;
    struct http_header *next;
};

HttpClient* New_HttpClient(void);

struct HttpClient {
	char *url;
	char *method;
	char *path;
	char *host;
	char *user_agent;
	struct http_header *headers;
		
	bool (*open)(HttpClient* self, const char* url);
	int (*connect)(HttpClient* self, char *host, u_short port);
	bool (*setDefaultHeader)(HttpClient* self);
	bool (*setHeader)(HttpClient* self, char* header_name, char* header_value);
	bool (*addHeader)(HttpClient* self, char* header_name, char* header_value);
	bool (*setMaxRedirects)(HttpClient* self, int n);
	char* (*get)(HttpClient* self, char* url);
	bool (*free)(HttpClient* self);
};


/*
//https://github.com/dexgeh/webserver-libev-httpparser/blob/master/include/server_eh.h
static inline void delete_http_header(struct http_header *header) {
    if (header->name != NULL) free(header->name);
    if (header->value != NULL) free(header->value);
    free(header);
}

static inline void delete_http_request(struct http_request *request) {
    if (request->url != NULL) free(request->url);
    if (request->body != NULL) free(request->body);
    struct http_header *header = request->headers;
    while (header != NULL) {
        struct http_header *to_delete = header;
        header = header->next;
        delete_http_header(to_delete);
    }
    free(request);
}

static inline struct http_header *add_http_header(struct http_request *request) {
    struct http_header *header = request->headers;
    while (header != NULL) {
        if (header->next == NULL) {
            header->next = new_http_header();
            return header->next;
        }
        header = header->next;
    }
    request->headers = new_http_header();
    return request->headers;
}

*/

#endif
