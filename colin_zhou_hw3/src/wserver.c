#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>


char default_root[] = ".";

void* consumer(void* arg);

int *buffer;
int bufferFillIndex = 0;
int bufferUseIndex = 0;
int bufferSize = 0; //buffersize as entered in command params
int bufferCount = 0; //counter for buffersize + 1, or -1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; //condition for 
pthread_cond_t filled = PTHREAD_COND_INITIALIZER;

char *root_dir = default_root;
int port = 10000;


//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    int threads = 0;
    pthread_t masterThread;
    int masterThreadError;

    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
    	switch (c) {
        	case 'd':
        	    root_dir = optarg;
        	    break;
        	case 'p':
        	    port = atoi(optarg);
        	    break;
            case 't':
                threads = atoi(optarg);
                if (threads < 1) {
                    exit(1);
                }

                break;
            case 'b':
                bufferSize = atoi(optarg);
                if (bufferSize < 1) {
                    exit(1);
                }

                buffer = (int *)malloc(sizeof(int)*bufferSize); //initialize buffer size
                break;

        	default:
        	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	}

    pthread_t progThread[threads];

    // run out of this directory
    chdir_or_die(root_dir);

    for (int i = 0; i < threads; i++) {
        int errorCreation = pthread_create(&progThread[i], NULL, consumer, NULL); 
        if (errorCreation != 0) {
            printf("\ncan't create thread");
        }
    }

    int listen_fd = open_listen_fd_or_die(port);

    while (1) {
    	struct sockaddr_in client_addr;

    	int client_len = sizeof(client_addr);
        printf("bufferCount before accept %d\n", bufferCount);
    	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
        printf("connector file descriptor to be put in: %d\n", conn_fd);

        put(conn_fd);
    }
    return 0;
}

void Pthread_mutex_lock(pthread_mutex_t* m) {
    int rc = pthread_mutex_lock(m);
    if (rc != 0) {
        fprintf(stderr, "lock failed with return value %d\n", rc);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t* m) {
    int rc = pthread_mutex_unlock(m);
    if (rc != 0) {
        fprintf(stderr, "unlock failed with return value %d\n", rc);
    }
}

void put(int value) {

    Pthread_mutex_lock(&mutex);
    while(bufferCount == bufferSize) {
        printf("Queue full \n");
        pthread_cond_wait(&empty, &mutex);
    }

    buffer[bufferFillIndex] = value;
    bufferFillIndex = (bufferFillIndex + 1) % bufferSize;
    bufferCount++;
    printf("num of items in buffer after put: %d\n", bufferCount);

    pthread_cond_signal(&filled);
    Pthread_mutex_unlock(&mutex);

}

int get() {
    int temp = buffer[bufferUseIndex];
    bufferUseIndex = (bufferUseIndex + 1) % bufferSize;
    bufferCount--;

    printf("num of items in buffer after get: %d\n", bufferCount);
    return temp;
}

void* consumer(void* arg) {
    while (1) {
        Pthread_mutex_lock(&mutex);

        while(bufferCount == 0) {
            printf("waiting to be filled \n");
            pthread_cond_wait(&filled, &mutex);
        }

        int fd = get();
        printf("retrieved file descriptor: %d\n", fd);
        request_handle(fd);
        close_or_die(fd);
        pthread_cond_signal(&empty);
        Pthread_mutex_unlock(&mutex);
        printf("processing item in consumer: file descriptor number  %d\n",fd);
    }

   return NULL;
}
