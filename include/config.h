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
	int dns_threads;
	int dns_mode;

	//udp config
	BOOL udp_active;
	const char* udp_sourceip;
	const char* udp_targetip;
	int udp_sourceport;
	int udp_targetport;
	int udp_pkgsize;
	int udp_sleeptime;
	int udp_threads;

}Configuration;


#endif /// _CONFIG_H
