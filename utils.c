#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include "utils.h"


char *
str_replace ( const char *string, const char *substr, const char *replacement )
{
  char *tok = NULL;
  char *newstr = NULL;
  char *oldstr = NULL;
  char *head = NULL;
 
  /* if either substr or replacement is NULL, duplicate string a let caller handle it */
  if ( substr == NULL || replacement == NULL ) return strdup (string);
  newstr = strdup (string);
  head = newstr;
  while ( (tok = strstr ( head, substr ))){
    oldstr = newstr;
    newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
    /*failed to alloc mem, free old string and return NULL */
    if ( newstr == NULL ){
      free (oldstr);
      return NULL;
    }
    memcpy ( newstr, oldstr, tok - oldstr );
    memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
    memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
    memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
    /* move back head right after the last replacement */
    head = newstr + (tok - oldstr) + strlen( replacement );
    free (oldstr);
  }
  return newstr;
}


int random_int(int min, int max)
{
	srandom( time(0)+clock()+random() ); 
	unsigned int s_seed = 214013 * rand() + 2531011;
	return min+(s_seed ^ s_seed>>15)%(max-min+1);
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

