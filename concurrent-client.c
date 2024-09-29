#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 256
#define MAX_THREADS 100
#define NUM_REQUESTS 20

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int portno;
    int thread_id;
} ThreadArg;

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void *threadFunc(void *arg) {
    ThreadArg *threadArg = (ThreadArg *)arg;
    int portno = threadArg->portno;
    int thread_id = threadArg->thread_id;

    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Assuming server is on localhost
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting client");

    pthread_mutex_lock(&print_lock);
    printf("Thread %d connected to port %d\n", thread_id, portno);
    pthread_mutex_unlock(&print_lock);

    for (int i = 1; i <= NUM_REQUESTS; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%d", i);

        pthread_mutex_lock(&print_lock);
        printf("Thread %d sending: %s\n", thread_id, buffer);
        pthread_mutex_unlock(&print_lock);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");

        memset(buffer, 0, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) error("ERROR reading from socket");

        pthread_mutex_lock(&print_lock);
        printf("Thread %d received: %s\n", thread_id, buffer);
        pthread_mutex_unlock(&print_lock);
    }

    close(sockfd);
    pthread_mutex_lock(&print_lock);
    printf("Thread %d: Connection closed\n", thread_id);
    pthread_mutex_unlock(&print_lock);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        exit(1);
    }

    int n_threads = atoi(argv[1]);
    if (n_threads <= 0 || n_threads > MAX_THREADS) {
        fprintf(stderr, "Invalid number of threads. Must be between 1 and %d.\n", MAX_THREADS);
        exit(1);
    }

    pthread_t threads[MAX_THREADS];
    ThreadArg thread_args[MAX_THREADS];
    int base_port = 8080;

    clock_t start = clock();

    for (int i = 0; i < n_threads; i++) {
        thread_args[i].portno = base_port + i;
        thread_args[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, threadFunc, &thread_args[i]) != 0)
            error("ERROR creating thread");
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("All threads completed. Total CPU time: %f seconds\n", cpu_time_used);

    return 0;
}