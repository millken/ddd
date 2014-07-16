//http://www.cnitblog.com/zouzheng/archive/2010/11/25/71711.html
//http://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
//http://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/
//https://github.com/noumia/cc_inflate/tree/master/test 
//https://github.com/choobin/obfuslate
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
 
#define MAXLINE 4096

int main(int argc, char *argv[]){
    int fd, n;
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    if(argc < 3){
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1); 
    }
    snprintf(sendline, 300,
         "GET %s HTTP/1.1\r\n"
         "Host: %s\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Encoding: gzip, deflate\r\n"
         "User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0\r\n"
         "Pragma: no-cache\r\n"
         "Cache-Control: no-cache\r\n"
         "Connection: close\r\n\r\n", "/config.ini", argv[1]);

    fd = socket_connect(argv[1], atoi(argv[2])); 
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

    return 0;
}
