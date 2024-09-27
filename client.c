#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8005




// Client task function executed by each thread
void *client_task(void *arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *hello = "Hello from client";
    char buffer[1024] = {0};

    pthread_t this_id = pthread_self();  // Getting thread ID in C (using pthread_self())

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Thread %ld: Socket creation error\n", (long)this_id);
        return NULL;
    }

    // Set server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Thread %ld: Invalid address / Address not supported\n", (long)this_id);
        return NULL;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Thread %ld: Connection failed\n", (long)this_id);
        return NULL;
    }

    // Send a message to the server (if needed)
    // send(sock, hello, strlen(hello), 0);
    // printf("Thread %ld: Hello message sent\n", (long)this_id);

    // Read response from server
    read(sock, buffer, 1024);
    printf("Thread %ld: Message received: %s\n", (long)this_id, buffer);

    // Close the socket
    close(sock);
    return NULL;
}


int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_clients>\n", argv[0]);
        return -1;
    }

    // Number of clients specified as command-line argument
    int num_clients = atoi(argv[1]);

    // Allocate memory for the threads
    pthread_t *threads = (pthread_t *)malloc(num_clients * sizeof(pthread_t));

    // Create the specified number of threads
    for (int i = 0; i < num_clients; i++) {
        pthread_create(&threads[i], NULL, client_task, NULL);
    }

    // Join the threads to ensure the main function waits for their completion
    for (int i = 0; i < num_clients; i++) {
        pthread_join(threads[i], NULL);
    }

    // Free allocated memory
    free(threads);

    return 0;
}
