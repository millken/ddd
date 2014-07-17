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
#include "http_parser.h"
#include "sds.h"

static int HttpClient_connect(HttpClient* self, char *host, u_short port);
static bool HttpClient_open(HttpClient* self, const char* url);
static bool HttpClient_free(HttpClient* self);
static bool HttpClient_setDefaultHeader(HttpClient* self);
static bool HttpClient_setHeader(HttpClient* self, char* header_name, char* header_value);
static bool HttpClient_addHeader(HttpClient* self, char* header_name, char* header_value);
struct http_url * HttpClient_parse_url(HttpClient* self, char *url);
char * HttpClient_format_request(HttpClient* self);


char * HttpClient_get(HttpClient* self, char* url);


static const HttpClient object_template = {
    .open = HttpClient_open,
    .connect = HttpClient_connect,
    .setDefaultHeader = HttpClient_setDefaultHeader,
    .setHeader = HttpClient_setHeader,
    .addHeader = HttpClient_addHeader,
    .parse_url = HttpClient_parse_url,
    .format_request = HttpClient_format_request,
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
	self->urls = malloc(sizeof(struct http_url));
    self->headers = malloc(sizeof(struct http_header));
    self->response = malloc(sizeof(struct http_response));
    //self->response->headers = malloc(sizeof(struct http_header));
    self->setDefaultHeader(self);

    return self;
}

//https://github.com/wg/wrk/blob/master/src/wrk.c
struct http_url * HttpClient_parse_url(HttpClient* self, char *url)
{
    struct http_parser_url parser_url;
    struct http_url *urls;
    urls = self->urls;

    if (http_parser_parse_url(url, strlen(url), 0, &parser_url)) {
        fprintf(stderr, "invalid URL: %s\n", url);
        exit(1);
    }

    urls->scheme = extract_url_part(url, &parser_url, UF_SCHEMA);
    urls->host = extract_url_part(url, &parser_url, UF_HOST);
    urls->port = extract_url_part(url, &parser_url, UF_PORT);
    urls->path = "/"; 

    if (parser_url.field_set & (1 << UF_PATH)) {
        urls->path = &url[parser_url.field_data[UF_PATH].off];
    }
    return urls;
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

char *aprintf(char **s, const char *fmt, ...) {
    char *c = NULL;
    int n, len;
    va_list ap;
    va_start(ap, fmt);
    n = vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);
    len = *s ? strlen(*s) : 0;
    if ((*s = realloc(*s, (len + n) * sizeof(char)))) {
        c = *s + len;
        va_start(ap, fmt);
        vsnprintf(c, n, fmt, ap);
        va_end(ap);
    }
    return c;
} 

char * HttpClient_format_request(HttpClient* self)
{
    char *req = NULL;
    char *head = NULL;
    struct http_header *header = self->headers;
    while (header->next != NULL) {
        header = header->next;
        aprintf(&head, "%s: %s\r\n", header->name, header->value);
    }
    aprintf(&req, "GET %s HTTP/1.1\r\n", self->urls->path);
    if (self->urls->host) aprintf(&req, "Host: %s", self->urls->host);
    if (self->urls->port) aprintf(&req, ":%s", self->urls->port);
    if (self->urls->host) aprintf(&req, "\r\n");
    aprintf(&req, "%s\r\n", head ? head : "");
    free(head);
    return req; 
}

char * HttpClient_get(HttpClient* self, char* url)
{

    self->parse_url(self, url);
    u_short port = self->urls->port != NULL ? atoi(self->urls->port) : 80;

    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    int fd, n;
    char *request = self->format_request(self);

    self->response->fd = self->connect(self, self->urls->host, port); 
    if (self->response->fd <= 0)
    {
        fprintf(stderr, "socket connect error! %d\n", self->response->fd);
        exit(1);
    }

    http_parser parser;
    http_parser_init(&parser, HTTP_RESPONSE);
    parser.data = self->response;

    write(self->response->fd, request, strlen(request));

    if (http_read_headers(&parser, self->response) != 0) {
        printf("response error\n");
        exit(1);
        //free_response(response);
        return NULL;
    }
    int count = 0;
    self->response->body = malloc(self->response->content_length + 1);
    memset(self->response->body, 0, self->response->content_length + 1);

    char *p = self->response->body;

    while ((count = read(self->response->fd, p, 4096)) > 0) {
        self->response->pos += count;
        p += count;
    }

    close(self->response->fd);

    return self->response->body;
  
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

static int on_url(http_parser *parser, const char *at, size_t len) {
    return 0;
}
    
static int on_message_begin(http_parser *parser) {
    return 0;
}

static int on_header_field(http_parser *parser, const char *at, size_t len) {
    http_response *response = parser->data;
    http_header *header;

    if (response->header_state != 1) {
        header = malloc(sizeof(struct http_header));
        header->name = NULL;
        header->value = NULL;
        header->next = response->headers;
        response->headers = header;
    } else {
        header = response->headers;
    }

    sds s = sdsnewlen(at, len);
    header->name = header->name == NULL ? s : sdscatsds(header->name, s);
    response->header_state = 1;

    return 0;
}

static int on_header_value(http_parser *parser, const char *at, size_t len) {
    http_response *response = parser->data;
    http_header *header = response->headers;

    sds s = sdsnewlen(at, len);
    header->value = header->value == NULL ? s : sdscatsds(header->value, s);
    response->header_state = 2;
    return 0;
}

static int on_status(http_parser *parser, const char *at, size_t len) {
    http_response *response = parser->data;
    response->status = parser->status_code;
    return 0;
}

static int on_headers_complete(http_parser *parser) {
    http_response *response = parser->data;
    response->content_length = parser->content_length;
    response->headers_complete = 1;
    return 0;
}

static int on_message_complete(http_parser *parser) {
    http_response *response = parser->data;
    response->complete = 1;
    return 0;
}

http_parser_settings parser_settings = {
    .on_message_begin = on_message_begin,
    .on_url = on_url,
    .on_status = on_status,
    .on_header_field = on_header_field,
    .on_header_value = on_header_value,
    .on_headers_complete = on_headers_complete,
    .on_message_complete = on_message_complete,
};

static int http_read_headers(http_parser *parser, http_response *response)
{
    int ret;
    char buf[1];
    int fd = response->fd;

    while (response->headers_complete == 0 && (ret = read(fd, buf, sizeof(buf))) > 0) {
        if (http_parser_execute(parser, &parser_settings, buf, ret) != (size_t) ret) {
            fprintf(stderr, "parse error\n");
            return -1;
        }
    }

    return 0;
}


static char *extract_url_part(char *url, struct http_parser_url *parser_url, enum http_parser_url_fields field) {
    char *part = NULL;

    if (parser_url->field_set & (1 << field)) {
        uint16_t off = parser_url->field_data[field].off;
        uint16_t len = parser_url->field_data[field].len;
        part = malloc(len + 1 * sizeof(char));
        memcpy(part, &url[off], len);
    }

    return part;
}
