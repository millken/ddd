#include <stdio.h>
#include "HttpClient.h"

int main(int argc, char* args[]) {
	struct HttpClient *ht = New_HttpClient();
	char *body;
	int i = 0;
	for (i = 0; i < 10; ++i)
	{
		ht = New_HttpClient();
		body = ht->get(ht, args[1]);
		if (ht->getError(ht) != NULL && body != NULL) {
			printf("%d\n", strlen(body));
		}
		ht->free(ht);
	}
	exit(1);
	//ht->setHeader(ht, "Host", "google.com");
	ht->setHeader(ht, "Server", "gws");
	ht->setHeader(ht, "User-Agent", "gws");
	ht->addHeader(ht, "User-Agent", "gws");
	body = ht->get(ht, args[1]);

	if (ht->getError(ht) != NULL) {
		printf("Err: %s\n", ht->getError(ht));
		exit(1);
	}

	printf("urls.scheme=%s,urls.host=%s,urls.port=%s,urls.path=%s\n", ht->urls->scheme, ht->urls->host, ht->urls->port, ht->urls->path);

    struct http_header *header = ht->headers;
    while (header->next != NULL) {
        header = header->next;
        //printf("header[%s: %s]\n", header->name, header->value);
    }

    header = ht->response->headers;
    while (header->next != NULL) {
        header = header->next;
        printf("header[%s: %s]\n", header->name, header->value);
    }   
    printf("%d\n", ht->response->status);
    printf("%d=%d\n", strlen(body), ht->response->pos);
	//printf("header=%d, header[0]=%s\n", sizeof(ht->header), ht->header[0]);
	ht->free(ht);
	return 0;
}
