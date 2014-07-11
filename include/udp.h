#include <sys/socket.h>
#include <netinet/ip.h>
#define __FAVOR_BSD
#define _USE_BSD
#include <netinet/udp.h> // struct udp

int
udp_send();

void udp_worker();