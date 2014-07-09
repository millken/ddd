#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{

	if(getuid() != 0) {
		printf("You must be running as root!\n");
		return 1;
	}
		
   parse_config();
   start_worker();

    return 0;
}

void usage(char *str);
void usage(char *str)
{
	printf("%s\n config.ini\n", str);
	exit(0);
}
