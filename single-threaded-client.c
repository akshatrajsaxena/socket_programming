#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Function to print error messages
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, n;
    int portno = 8080;

    struct sockaddr_in serv_addr;

    char buffer[256];

    // Create socket, get sockfd handle
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Filling server details
    bzero((char *)&serv_addr, sizeof(serv_addr)); // sets all values to null
    serv_addr.sin_family = AF_INET; // for IPv4
    serv_addr.sin_port = htons(portno); // to handle bytes ordering
    serv_addr.sin_addr.s_addr = INADDR_ANY; // connect to localhost (server running on the same machine)

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        error("ERROR connecting");
    }

    // Read message from server
    printf("Connected to server\n\n");
    for (int i = 1; i <= 20; i++) {
        bzero(buffer, 256);
        sprintf(buffer, "%d", i);
        printf("Message sent: %s\n", buffer);

        // Message to server
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
        }

        n = read(sockfd, buffer, 255);
        if (n < 0) {
            error("ERROR reading from socket");
        } else if (n == 0) {
            break; // Server closed connection
        } else {
            printf("Server's response: %s\n\n", buffer);
        }
    }

    // Close the connected socket
    close(sockfd);
    printf("Connected socket closed!\n");

    return 0;
}

