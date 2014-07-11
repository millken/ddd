#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "config.h"


extern struct Configuration *config;

void daemonize_init()
{
    int i;
    if(getppid()==1) // parent pid ==1, the init process
        return; /* already a daemon */
    i=fork();
    if (i<0)
        exit(1); /* fork error */
    if (i>0)
        exit(0); /* parent exits */
    /* child (daemon) continues */
    setsid(); /* obtain a new process group */

    for (i=getdtablesize();i>=0;--i)
        close(i); /* close all descriptors */
    i=open("/dev/null",O_RDWR);
    dup(i);
    dup(i); /* handle standart I/O */

    umask(027); /* set newly created file permissions */

    //chdir(WORK_DIR); /* change running directory */

    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);

    //the following signals have been captured in forwarder.c
    //signal(SIGHUP,signal_handler); /* catch hangup signal */
    //signal(SIGTERM,signal_handler); /* catch kill signal */
    //signal(SIGINT,signal_handler); /* catch kill signal */
}

int bool_value(char *value)
{
    if (!strcasecmp(value, "yes") || !strcasecmp(value, "true")) return TRUE;
    return FALSE;
}

int config_set_default( Configuration * config)
{
    if( config == NULL)
        return -1;

    memset(config, 0, sizeof (Configuration));
    config->daemon = FALSE;
    
    return 0;
}
static int parse_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    Configuration* pconfig = (Configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    
    //printf("section:%s,name:%s, value:%s\n", section, name, value);
    if (MATCH("", "daemon")) {
        pconfig->daemon = bool_value((char *)value);
    } else if (MATCH("dns", "active")) {
        pconfig->dns_active = bool_value((char *)value);
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
    /*udp config*/
    } else if (MATCH("udp", "active")) {
        pconfig->udp_active = bool_value((char *)value);        
    } else if (MATCH("udp", "source-ip")) {
        pconfig->udp_sourceip = strdup(value);        
    } else if (MATCH("udp", "target-ip")) {
        pconfig->udp_targetip = strdup(value); 
    } else if (MATCH("udp", "source-port")) {
        pconfig->udp_sourceport = atoi(value);
    } else if (MATCH("udp", "target-port")) {
        pconfig->udp_targetport = atoi(value);                               
    } else if (MATCH("udp", "package-size")) {
        pconfig->udp_pkgsize = atoi(value);       
    } else if (MATCH("udp", "sleep-time")) {
        pconfig->udp_sleeptime = atoi(value);             
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
