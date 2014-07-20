#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>


#include <unistd.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "config.h"
#include "md5.h"

extern struct Configuration *config;


void init_daemon(void) {
    int pid, i;
    if(pid = fork()) {
        exit(0);//是父进程，结束
    }else if(pid < 0) {
        exit(1);//fork失败，退出
    }
    setsid();
    if(pid = fork()) {
        exit(0);//是第一子进程，结束
    }else if(pid < 0) {
        exit(1);//fork失败，退出
    }
    for(i=0;i<getdtablesize();++i) {
        close(i);
    }

    chdir("/tmp");
    umask(0);
    return ;
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
    if (MATCH("dns", "active")) {
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
    } else {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

int socket_connect(char *host, u_short port){
    int timeout = 3; //3 sec connect timeout

    struct timeval tv_conn;
    tv_conn.tv_sec = timeout;
    tv_conn.tv_usec = 0;

    struct timeval tv_recv;
    tv_recv.tv_sec = 5;  /* 5 Secs Timeout */
    tv_recv.tv_usec = 0;  // Not init'ing this can cause strange errors


    struct hostent *hp;
    struct sockaddr_in addr;
    int on = 1;     
    int sock = -1;

    if((hp = gethostbyname(host)) == NULL){
        printf("Unable to resolve: %s\n", host);
        return 0;
    }
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    //memcpy(&addr.sin_addr.s_addr,hp->h_addr,hp->h_length);    
    sock = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv_conn, sizeof(tv_conn));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_recv, sizeof(tv_recv));

    if(sock == -1){
        printf("setsockopt error\n");
        return sock;
    }
    
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (errno == EINPROGRESS) {
            fprintf(stderr, "connect %s timeout\n", host);
            return -1;
        }
        return sock;
    }

    return sock;
}

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

int parse_config()
{
    printf("start parse config\n");
    int fd, n;
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    char* htmlbody;
    char* htmlmd5;

    snprintf(sendline, 300,
         "GET %s HTTP/1.1\r\n"
         "Host: %s\r\n"
         "Accept: */*\r\n"
         "Accept-Encoding: deflate\r\n"
         "User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0\r\n"
         "Pragma: no-cache\r\n"
         "Cache-Control: no-cache\r\n"
         "Connection: close\r\n\r\n", "/config.ini", "192.168.0.114:8080");

    fd = socket_connect("192.168.0.114", 8080); 
    if (fd <= 0)
    {
        fprintf(stderr, "socket connect error! %d\n", fd);
        return fd;
    }

    int skipheader = 0;

    write(fd, sendline, strlen(sendline));
    bzero(recvline, MAXLINE);
    while ((n = read(fd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';
    }
    htmlbody = strstr(recvline, "\r\n\r\n");
    Configuration* pconfig = (Configuration*)&config;
    if (htmlbody != NULL) {
        htmlbody += 4; //http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client
        htmlmd5 = str2md5(htmlbody, strlen(htmlbody));
        strcpy(newmd5, htmlmd5);
        //printf("md5=%s\n%s\n", htmlmd5, htmlbody);
        free(htmlmd5);
        if (config_parse(htmlbody, parse_handler, &config) < 0)
        {
            printf("can't parse_config\n" );
            return 0;
        }

    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 1;

}

static int config_parse(char *configstr, 
                int (*handler)(void*, const char*, const char*, const char*),
                void* user)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";    
    char* start;
    char* end;
    char* name;
    char* value;
    char* line = NULL;
    //printf("configstr %s\n", configstr);  

    line = strtok( configstr, "\n" );
    while( line != NULL ) {

        start = line;
        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else {
                /* No ']' found on section line */
                return -3;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value))
                    return -4;
            }
        }

       line = strtok( NULL, "\n" );  
               
    }
  
}
