//
// client.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      client hostname portnumber filename
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// For testing your server, you will want to modify this client.  
// For example:
// You may want to make this multi-threaded so that you can 
// send many requests simultaneously to the server.
//
// You may also want to be able to request different URIs; 
// you may want to get more URIs from the command line 
// or read the list from a file. 
//
// When we test your server, we will be using modifications to this client.
//

#include "io_helper.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define MAXBUF (8192)

//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) {
    char buf[MAXBUF];
    char hostname[MAXBUF];
    
    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    printf("hostname\n", hostname);
    write_or_die(fd, buf, strlen(buf));
}

typedef struct _client_data {
    char *host;
    char *filename;
    int port;
} client_data;

void *single_client(void* arg) {
    client_data* t = (client_data*)(arg);
    char* host = t->host;
    char* filename = t->filename;
    int port = t->port;

    int clientfd = open_client_fd_or_die(host, port);

    client_send(clientfd, filename);
    client_print(clientfd);

    close_or_die(clientfd);
    return NULL;
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
	printf("Header: %s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
	
	// If you want to look for certain HTTP tags... 
	// int length = 0;
	//if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
	//    printf("Length = %d\n", length);
	//}
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) {
	printf("%s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
    }
}

int main(int argc, char *argv[]) {
    char *host, *filename;
    int port;
    int clientfd;
    int clientfd2;
//    int numThreads;
    printf("cclients %d\n", argc);
    
    if (argc < 4) {
	fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
	exit(1);
    }
    
    host = argv[1];
    port = atoi(argv[2]);
//    filename = argv[3];

    int concur_clients = argc - 3;
    pthread_t threadRequests[3];

    for (int i = 0; i < concur_clients; i++) {
        client_data *d = malloc(sizeof(client_data));
        if ( d == NULL) {
            printf("error");
            continue;
        }

        d->host = host;
        d->port = port;
        d->filename = argv[3 + i];
        pthread_create(&threadRequests[i], NULL, single_client, (void *)d);
    }

    for (int j = 0; j < concur_clients; j++) {
        pthread_join(threadRequests[j], NULL);
    }

    /* Open a single connection to the specified host and port */
//    clientfd = open_client_fd_or_die(host, port);
//        printf("error %s\n", strerror(errno), open_client_fd_or_die(host, port));
    if (open_client_fd(host,port) == -1) {
        printf("error open_client_fd %s\n", strerror(errno));
    }
 
//    clientfd2 = open_client_fd_or_die(host, port);

//    client_send(clientfd, filename);
////    client_send(clientfd2, filename);
//
//    client_print(clientfd);
////    client_print(clientfd2);
//
//    close_or_die(clientfd);
//    close_or_die(clientfd2);

    exit(0);
}
