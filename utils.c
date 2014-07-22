#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa report error, if no
#include <sys/timeb.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include "utils.h"


unsigned short
ip_sum (unsigned short *addr, int len)
{
  register int nleft = len;
  register unsigned short *w = addr;
  register int sum = 0;
  unsigned short answer = 0;

  while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
  if (nleft == 1)
    {
      *(unsigned char *) (&answer) = *(unsigned char *) w;
      sum += answer;
    }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

unsigned short
cksum (unsigned short * buf, int nwords)
{

  unsigned long sum;

  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

//http://creativeandcritical.net/str-replace-c/
char *replace_str(const char *str, const char *old, const char *new)
{
  char *ret, *r;
  const char *p, *q;
  size_t oldlen = strlen(old);
  size_t count, retlen, newlen = strlen(new);

  if (oldlen != newlen) {
    for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
      count++;
    /* this is undefined if p - str > PTRDIFF_MAX */
    retlen = p - str + strlen(p) + count * (newlen - oldlen);
  } else
    retlen = strlen(str);

  if ((ret = malloc(retlen + 1)) == NULL)
    return NULL;

  for (r = ret, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen) {
    /* this is undefined if q - p > PTRDIFF_MAX */
    ptrdiff_t l = q - p;
    memcpy(r, p, l);
    r += l;
    memcpy(r, new, newlen);
    r += newlen;
  }
  strcpy(r, p);

  return ret;
}

int random_int(int min, int max)
{
  /* Seed number for rand() */
  struct timeb t;
  ftime(&t);
  srand((unsigned int) 1000 * t.time + t.millitm + random());
  unsigned int s_seed = 214013 * rand() + 2531011;
  return min+(s_seed ^ s_seed>>15)%(max-min+1);
}

unsigned long 
random_lip(void)
{
	char convi[16];
	sprintf (convi, "%d.%d.%d.%d", random_int(1, 254), random_int(1, 254), random_int(1, 254), random_int(1, 254));
	return inet_addr (convi);
}

char *
replace_ip(const char *str, const char *old)
{
  char *ret, *r;
  const char *p, *q;
  size_t oldlen = strlen(old);
  size_t count, retlen, newlen = 4;
  
  char ip[3];
  for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
    count++;
  /* this is undefined if p - str > PTRDIFF_MAX */
  retlen = p - str + strlen(p) + count * (newlen - oldlen);  
  if ((ret = malloc(retlen + 1)) == NULL)
    return NULL;

  for (r = ret, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen) {
    /* this is undefined if q - p > PTRDIFF_MAX */
    ptrdiff_t l = q - p;
    memcpy(r, p, l);
    r += l;
    bzero(ip, newlen);
    sprintf(ip, "%d", random_int(1, 255));
    newlen = strlen(ip);
    memcpy(r, ip, newlen);
    r += newlen;
  }
  strcpy(r, p);  
 return ret;  
}

char *
replace_domain(const char *str, const char *old)
{
  char *ret, *r;
  const char *p, *q;
  size_t oldlen = strlen(old);
  size_t count, retlen, newlen = 26;
  
  char s[25];
  for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
    count++;
  /* this is undefined if p - str > PTRDIFF_MAX */
  retlen = p - str + strlen(p) + count * (newlen - oldlen);  
  if ((ret = malloc(retlen + 1)) == NULL)
    return NULL;

  for (r = ret, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen) {
    /* this is undefined if q - p > PTRDIFF_MAX */
    ptrdiff_t l = q - p;
    memcpy(r, p, l);
    r += l;
    bzero(s, newlen);
    random_chars(s, 5, 16);
    newlen = strlen(s);
    memcpy(r, s, newlen);
    r += newlen;
  }
  strcpy(r, p);  
 return ret;  
}

char *
random_cip (void)
{
  struct in_addr hax0r;
  hax0r.s_addr = random_lip();
  return (inet_ntoa(hax0r));
}

char *
random_chars(char *dst, int start, int end)
{
	srandom( time(0)+clock()+random() ); //生成更好的随机数
    static const char allowable_chars[] = "1234567890abcdefhijklnmopqrstuwxyz";
    int i, r;
    int size = rand()%(end - start + 1) + start;/*n为a~b之间的随机数*/
    for (i = 0; i< size; i++) {
        r = (int)((double)rand() / ((double)RAND_MAX + 1) * (sizeof(allowable_chars) -1 ));
        dst[i] = allowable_chars[r];
    }
    dst[i] = '\0';

    return dst;
}

