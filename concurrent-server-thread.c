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

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connection_count_lock = PTHREAD_MUTEX_INITIALIZER;
int active_connections = 0;

#define BUFFER_SIZE 256
#define MAX_THREADS 100

typedef struct {
    int portno;
    FILE *filePointer;
} ThreadArg;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

long int compute_factorial(const char *buffer) {
    int x = atoi(buffer);
    long int result = 1;
    for (int i = 1; i <= x; i++) {
        result *= i;
    }
    return result;
}

void writeToFile(FILE *filePointer, const char *output) {
    if (filePointer != NULL) {
        pthread_mutex_lock(&file_lock);
        fputs(output, filePointer);
        fputs("\n", filePointer);
        fflush(filePointer);  // Ensure immediate write to disk
        pthread_mutex_unlock(&file_lock);
    }
}

void *threadFunc(void *arg) {
    ThreadArg *threadArg = (ThreadArg *)arg;
    int portno = threadArg->portno;
    FILE *filePointer = threadArg->filePointer;

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        pthread_mutex_lock(&connection_count_lock);
        active_connections++;
        pthread_mutex_unlock(&connection_count_lock);

        printf("New connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
            if (n <= 0) break;

            printf("Received: %s\n", buffer);
            long int fact = compute_factorial(buffer);

            char output[BUFFER_SIZE];
            snprintf(output, BUFFER_SIZE, "(%s:%d) %s! = %ld", 
                     inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer, fact);
            writeToFile(filePointer, output);

            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%ld", fact);
            n = write(newsockfd, buffer, strlen(buffer));
            if (n < 0) error("ERROR writing to socket");
        }

        close(newsockfd);
        pthread_mutex_lock(&connection_count_lock);
        active_connections--;
        pthread_mutex_unlock(&connection_count_lock);
        printf("Connection closed. Active connections: %d\n", active_connections);
    }

    close(sockfd);
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
    int portno = 8080;

    FILE *filePointer = fopen("output.txt", "a");
    if (filePointer == NULL) error("ERROR opening file");

    for (int i = 0; i < n_threads; i++) {
        thread_args[i].portno = portno + i;
        thread_args[i].filePointer = filePointer;
        if (pthread_create(&threads[i], NULL, threadFunc, &thread_args[i]) != 0)
            error("ERROR creating thread");
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(filePointer);
    return 0;
}