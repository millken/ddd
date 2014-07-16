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

int HttpClient_connect(HttpClient* self, char *host, u_short port);
static bool HttpClient_open(HttpClient* self, const char* url);
static bool HttpClient_free(HttpClient* self);
static bool HttpClient_setDefaultHeader(HttpClient* self, const char* header);

static const HttpClient object_template = {
    .open = HttpClient_open,
    .connect = HttpClient_connect,
    .setDefaultHeader = HttpClient_setDefaultHeader,
    .free = HttpClient_free,
};

HttpClient* New_HttpClient(void)
{
    printf("%s", __FUNCTION__);
    HttpClient* self = malloc(sizeof(HttpClient));

    if (self == NULL) {
        //logger_error("Memory not enough");
        return NULL;
    }

    memcpy(self, &object_template, sizeof(HttpClient));
	//self->header = NULL;

    return self;
}

int HttpClient_connect(HttpClient* self, char *host, u_short port)
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

static bool HttpClient_setDefaultHeader(HttpClient* self, const char* header)
{
	self->header[0] = (char*) malloc(32 * sizeof(char));
    strcpy(self->header[0], "What?");
	printf("sizeof(self->headers) = %d\n", sizeof(self->header));
	//self->headers[sizeof(self->headers)] = (char *)header;
	return true;
}

static bool HttpClient_open(HttpClient* self, const char* url)
{
	self->url = (char *)url;
    printf("%s %s", __FUNCTION__, url);
    return true;
}

static bool HttpClient_free(HttpClient* self)
{
	if (self != NULL)free(self);
	return true;
}
