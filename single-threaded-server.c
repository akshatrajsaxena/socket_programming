#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>  
#include <arpa/inet.h>

#define MAX_CONNECTIONS 10

// Function to print error messages
void error(char *msg){
    perror(msg);
    exit(1);
}

long int compute_factorial(char buffer[256]){
    int x = atoi(buffer);
    long int result = 1;
    for(int i=1; i<=x; i++){
        result *= i;
    }
    return result;
}

void writeToFile(FILE *filePointer, char output[100]){
    if (filePointer == NULL){
        // printf("Unable to write the output to the file output.txt\n");
        return; // Added return to prevent further execution
    }
    if(strlen(output) > 0){
        fputs(output, filePointer);
        fputs("\n", filePointer);
        // printf("Added output to file\n");
    }
}

int main(int argc, char *argv[]){
    double time_spent = 0.0;
    clock_t begin = clock();

    // For writing output to the file - output.txt
    FILE *filePointer = fopen("output.txt", "a");
    if (filePointer == NULL) {
        error("ERROR opening file");
    }

    int sockfd, newsockfd;
    int portno = 8080;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, k=0;

    /* Create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        error("ERROR opening socket");
    }

    /* Filling in details */
    bzero((char *)&serv_addr, sizeof(serv_addr)); // Sets all values to null
    serv_addr.sin_family = AF_INET; // For IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // All IP address allowed
    serv_addr.sin_port = htons(portno); // To handle bytes ordering

    /* Bind socket with the IP and port */
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0){
        error("ERROR on binding");
    }

    /* Listen for incoming connection requests */
    listen(sockfd, MAX_CONNECTIONS);
    clilen = sizeof(cli_addr);

    while(1){
        /* Accept a new connection */
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if(newsockfd < 0){
            error("ERROR on accept");
        }

        /* Read message from client */
        printf("Connected\n\n");
        while(1){
            /* Code */
            bzero(buffer, 256);
            n = read(newsockfd, buffer, 255);
            if(n < 0){
                error("ERROR reading from socket");
            } else if(n == 0){
                printf("Client disconnected.\n");
                break; // Client disconnected
            } else {
                printf("Client message: %s\n", buffer);
            }

            /* Reply to client */
            long int fact = compute_factorial(buffer);

            char str[INET_ADDRSTRLEN];
            struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&cli_addr;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
            int client_port = ntohs(cli_addr.sin_port);
            
            char output[100];
            sprintf(output, "(%s:%d) %s! = %ld", str, client_port, buffer, fact);
            writeToFile(filePointer, output);

            bzero(buffer, 256);
            sprintf(buffer, "%ld", fact);
            n = write(newsockfd, buffer, strlen(buffer));
            if(n < 0){
                error("ERROR writing to socket");
            } else {
                printf("Message sent: %s\n\n", buffer);
            }
        }
        close(newsockfd); // Close the connection socket after finishing with it
        k++;
        if(k == MAX_CONNECTIONS){
            break; // Limit the number of connections
        }
    }

    // Close the file pointer
    fclose(filePointer);
    printf("File closed!\n");

    // Close the listening socket
    close(sockfd);
    printf("Listening socket closed!\n");

    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", time_spent);

    return 0;
}

