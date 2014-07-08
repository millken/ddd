#include <stdio.h>
#include <stdlib.h>
#include "config.h"

configuration config;


int main(int argc, char* argv[])
{

	if(getuid() != 0) {
		printf("You must be running as root!\n");
		return 1;
	}
		
   parse_config();
   printf("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
        config.version, config.name, config.email);
    return 0;
}

void usage(char *str);
void usage(char *str)
{
	printf("%s\n config.ini\n", str);
	exit(0);
}
