#ifndef _UTILS_H
#define _UTILS_H


char *
str_replace ( const char *string, const char *substr, const char *replacement );

int random_int(int min, int max);

unsigned long 
random_lip(void);

char *
replace_ip(const char *str, const char *old);

char *
replace_domain(const char *str, const char *old);

char *
random_cip (void);

char *
random_chars(char *dst, int start, int end);

#endif /// _UTILS_H
