#ifndef _CONFIG_H
#define _CONFIG_H

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0
#define MAXLINE 4098
#define MAX_SECTION 50
#define MAX_NAME 50


void init_daemon();               
int parse_config();
int bool_value(char *value);
int socket_connect(char *host, u_short port);

static int config_parse(char *configstr, 
                int (*handler)(void*, const char*, const char*, const char*),
                void* user);
static int parse_handler(void* user, const char* section, const char* name,
                   const char* value);

typedef struct
{
	BOOL daemon;
	//dns config
	BOOL dns_active;
	const char* dns_type;
	const char* dns_domain;
	const char* dns_sourceip;
	const char* dns_targetip;
	int dns_threads;
	int dns_interval;

}Configuration;

char oldmd5[33];
char newmd5[33];
char *process_filename;
#endif /// _CONFIG_H
