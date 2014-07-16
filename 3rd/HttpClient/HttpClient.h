#ifndef _HttpClient_H
#define _HttpClient_H
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

//typedef char * string;

typedef struct HttpClient HttpClient;
/*
typedef enum {
    false,
    true
} bool;
*/
typedef struct
{
    char *name;
    char *value;
} Header;

HttpClient* New_HttpClient(void);

struct HttpClient {
	char *url;
	char *method;
	char *path;
	char *host;
	char *user_agent;
	char *header[20];
		
	bool (*open)(HttpClient* self, const char* url);
	int (*connect)(HttpClient* self, char *host, u_short port);
	bool (*setDefaultHeader)(HttpClient* self, const char* header);
	bool (*setMaxRedirects)(HttpClient* self, int n);
	bool (*free)(HttpClient* self);
};


#endif
