#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"


extern struct configuration *config;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("protocol", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        pconfig->name = strdup(value);
    } else if (MATCH("user", "active")) {
        pconfig->email = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

void parse_config()
{
 if (ini_parse("config.ini", handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
		exit( 1 );
    }
    
}
