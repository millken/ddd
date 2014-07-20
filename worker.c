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

 
Configuration config;


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
   //new_thread_p(config_worker, 0);
   
    while(1) {
        parse_config();
        if (strlen(oldmd5) == 0 && strlen(newmd5) != 0)
        {
            new_thread_p(dns_worker, 0);
        }
        //config.oldmd5 = newmd5;
        printf("oldmd5=%s, newmd5=%s,dns_domain=%s\n", oldmd5, newmd5, config.dns_domain);
        if (strlen(oldmd5) != 0 && strcmp(oldmd5, newmd5) != 0) {
            printf("fork new child process\n");
            exit(1);
        }
        sleep(5);
    }    
}

void config_worker()
{
    while(1) 
    {
  	  parse_config();
      sleep(5);
	}
}


void dns_worker()
{
    sleep(1);
    printf("dns_worker\n");
    strcpy(oldmd5, newmd5);
    //printf("oldmd5=%s, newmd5=%s,dns_domain=%s\n", config.oldmd5, newmd5, config.dns_domain);
    if (config.dns_active)
    {
        dns_send();
    }
}



