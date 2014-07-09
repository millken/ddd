#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "config.h"
#include "worker.h"
#include "dns.h"
#include "utils.h"

 
configuration config;


int fork_process(void (*func)())
{
    int ret = fork();

    if(ret == 0) {
        func();
    }

    if(ret > 0) {
    }

    return ret;
}


int new_thread_p(void *func, void *i)
{
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if(pthread_create(&thread, &attr, func, i)) {
        return 0;
    }

    return 1;
}

void start_worker()
{
   if (strcasecmp(config.dns_active, "true") == 0) {
 		printf("dns child\n");
		fork_process(dns_master);
   }
   printf("Config loaded : daemon=%s, dns.active=%s,dns.threads=%d, dns.domain=%s \n",
        config.daemon, config.dns_active, config.dns_threads, config.dns_domain);
	while(1) {
		sleep(1);
	}
}

void dns_master()
{
	int i;
	for (i=0; i < config.dns_threads; i++) {
		new_thread_p(dns_send1, 0);
	}
	printf("dns master start\n");
}

void dns_worker()
{
	printf("dns worker .%s\n", config.dns_domain);
}



