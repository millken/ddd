#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "HttpClient.h"

static int HttpClient_connect(HttpClient* self, char *host, u_short port);
static bool HttpClient_open(HttpClient* self, const char* url);
static bool HttpClient_free(HttpClient* self);
static bool HttpClient_setDefaultHeader(HttpClient* self);
static bool HttpClient_setHeader(HttpClient* self, char* header_name, char* header_value);
static bool HttpClient_addHeader(HttpClient* self, char* header_name, char* header_value);
char * HttpClient_get(HttpClient* self, char* url);


static const HttpClient object_template = {
    .open = HttpClient_open,
    .connect = HttpClient_connect,
    .setDefaultHeader = HttpClient_setDefaultHeader,
    .setHeader = HttpClient_setHeader,
    .addHeader = HttpClient_addHeader,
    .get = HttpClient_get,
    .free = HttpClient_free,
};


HttpClient* New_HttpClient(void)
{
    //printf("%s\n", __FUNCTION__);
    HttpClient* self = malloc(sizeof(struct HttpClient));

    if (self == NULL) {
        //logger_error("Memory not enough");
        return NULL;
    }

    memcpy(self, &object_template, sizeof(HttpClient));
	self->headers = malloc(sizeof(struct http_header));
    self->setDefaultHeader(self);

    return self;
}

static int HttpClient_connect(HttpClient* self, char *host, u_short port)
{
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

char * HttpClient_get(HttpClient* self, char* url)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    int fd, n;
    snprintf(sendline, 300,
         "GET %s HTTP/1.1\r\n"
         "Host: %s\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Encoding: deflate\r\n"
         "User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0\r\n"
         "Pragma: no-cache\r\n"
         "Cache-Control: no-cache\r\n"
         "Connection: close\r\n\r\n", "/", "www.baidu.com");

    fd = self->connect(self, "www.baidu.com", 80); 
    if (fd <= 0)
    {
        fprintf(stderr, "socket connect error! %d\n", fd);
        exit(1);
    }

    write(fd, sendline, strlen(sendline));
    bzero(recvline, MAXLINE);
    while ((n = read(fd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';
        printf("%s", recvline);
        bzero(recvline, MAXLINE);
    }   


    shutdown(fd, SHUT_RDWR); 
    close(fd);    
}
static inline struct http_header *new_http_header() {
    struct http_header *header = malloc(sizeof(struct http_header));
    header->name = NULL;
    header->value = NULL;
    header->next = NULL;
    return header;
}

static bool HttpClient_setDefaultHeader(HttpClient* self)
{
    self->setHeader(self, "User-Agent", "Mozilla/5.0 (Windows NT 6.3; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0");
    self->setHeader(self, "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    self->setHeader(self, "Accept-Encoding", "deflate");
    self->setHeader(self, "Connection", "Close");

	return true;
}

static bool HttpClient_setHeader(HttpClient* self, char* header_name, char* header_value)
{
    struct http_header *header = self->headers;
    while (header != NULL) {
        if (header->name != NULL && strcmp((const char*)header->name, (const char*)header_name) == 0 ) {
            header->value = header_value;
            return true;
        }        
        if (header->next == NULL) {
            header->next = new_http_header();
            break;
        }
        header = header->next;
    }
    header = header->next;
    header->name = header_name;
    header->value = header_value;
    //printf("\nheader_name = %s,  %d\n", header_name, strlen(header_name));
    //alloc_cpy(header->name, header_name, strlen(header_name));
    //alloc_cpy(header->value, header_value, strlen(header_value));
    return true;
}

static bool HttpClient_addHeader(HttpClient* self, char* header_name, char* header_value)
{
    struct http_header *header = self->headers;
    while (header != NULL) {       
        if (header->next == NULL) {
            header->next = new_http_header();
            break;
        }
        header = header->next;
    }
    header = header->next;
    header->name = header_name;
    header->value = header_value;
    return true;
}

static bool HttpClient_open(HttpClient* self, const char* url)
{
	self->url = (char *)url;
    //printf("%s %s\n", __FUNCTION__, url);
    return true;
}

static bool HttpClient_free(HttpClient* self)
{
	if (self != NULL)free(self);
	return true;
}
