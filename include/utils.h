#ifndef _UTILS_H
#define _UTILS_H

char *
str_replace ( const char *string, const char *substr, const char *replacement );

int random_int(int min, int max);

char *
random_chars(char *dst, int start, int end);

#endif /// _UTILS_H
