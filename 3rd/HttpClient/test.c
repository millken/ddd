#include <stdio.h>
#include "HttpClient.h"

int main(int argc, char* args[]) {
	struct HttpClient *ht = New_HttpClient();
	ht->open(ht, "xxxxxxxx");
	
	ht->setDefaultHeader(ht, "Host: test.com");
	
	printf("url=%s\n", ht->url);
	
	printf("header=%d, header[0]=%s\n", sizeof(ht->header), ht->header[0]);
	ht->free(ht);
	return 0;
}
