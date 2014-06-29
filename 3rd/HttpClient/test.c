#include <stdio.h>
#include "HttpClient.h"

int main(int argc, char* args[]) {
	struct HttpClient *ht = New_HttpClient();
	ht->open(ht, "xxxxxxxx");
	
	ht->setHeader(ht, "Host", "google.com");
	ht->setHeader(ht, "Server", "gws");
	ht->setHeader(ht, "User-Agent", "gws");
	ht->addHeader(ht, "User-Agent", "gws");
	ht->get(ht, "www.baidu.com");

	printf("url=%s\n", ht->url);

    struct http_header *header = ht->headers;
    while (header->next != NULL) {
        header = header->next;
        printf("header[%s: %s]\n", header->name, header->value);
    }

	//printf("header=%d, header[0]=%s\n", sizeof(ht->header), ht->header[0]);
	ht->free(ht);
	return 0;
}
