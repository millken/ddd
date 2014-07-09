#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"


extern struct configuration *config;


static int parse_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    
    //printf("section:%s,name:%s, value:%s\n", section, name, value);
    if (MATCH("", "daemon")) {
	    pconfig->daemon = strdup(value);
    } else if (MATCH("dns", "active")) {
        pconfig->dns_active = strdup(value);
    } else if (MATCH("dns", "threads")) {
    	pconfig->dns_threads = atoi(value);
    } else if (MATCH("dns", "mode")) {
    	pconfig->dns_mode = atoi(value);
    } else if (MATCH("dns", "type")) {
    	pconfig->dns_type = strdup(value);
    } else if (MATCH("dns", "domain")) {
    	pconfig->dns_domain = strdup(value);
    } else if (MATCH("dns", "sourceip")) {
    	pconfig->dns_sourceip = strdup(value);
    } else if (MATCH("dns", "targetip")) {
    	pconfig->dns_targetip = strdup(value);
    } else if (MATCH("dns", "file")) {
    	pconfig->dns_file = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

void parse_config()
{
 if (ini_parse("config.ini", parse_handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
		exit( 1 );
    }
    
}
