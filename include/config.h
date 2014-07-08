#ifndef _CONFIG_H
#define _CONFIG_H

#define DDD_VERSION "v0.01"

static int handler(void* user, const char* section, const char* name,
                   const char* value);
               
void parse_config();
  
typedef struct 
{
    int version;
    const char* name;
    const char* email;
}configuration;


#endif /// _CONFIG_H
