#ifndef _CONFIG_H
#define _CONFIG_H

#define DDD_VERSION "v0.01"

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0

static int handler(void* user, const char* section, const char* name,
                   const char* value);

void daemonize_init();               
void parse_config();
int bool_value(char *value);
  
typedef struct
{
	BOOL daemon;
	
	//dns config
	BOOL dns_active;
	const char* dns_type;
	const char* dns_domain;
	const char* dns_sourceip;
	const char* dns_targetip;
	const char* dns_file;
	int dns_threads, dns_mode;
		
}Configuration;


#endif /// _CONFIG_H
