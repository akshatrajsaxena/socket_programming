#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

#define DATA_BUFFER 500
#define MAX_CONNECTIONS 10
#define PORTNO 8080

long int compute_factorial(char buffer[DATA_BUFFER]){
    int x = atoi(buffer);
    long int result = 1;
    for(int i=1; i<=x; i++){
        result *= i;
    }
    return result;
}

void writeToFile(FILE *filePointer, char output[DATA_BUFFER]){
    if (filePointer == NULL){
        // printf("Unable to write the output to the file output.txt\n");
    }
    else{
        if(strlen(output) > 0){
            fputs(output, filePointer);
            fputs("\n", filePointer);
        }
        // printf("Added output to file\n");
    }
}

int create_tcp_server_socket(){
    struct sockaddr_in saddr;
    int fd, ret_val;

    /* Create a TCP socket */
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd==-1){
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        return -1;
    }
    printf("Created a socket with fd: %d\n", fd);

    /* Initialize the socket address structure*/
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORTNO);
    saddr.sin_addr.s_addr = INADDR_ANY;

    /* Bind the socket to port 7001 on the local host*/
    ret_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if(ret_val!=0){
        fprintf(stderr, "bind failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }

    /* listen for incoming connections */
    ret_val = listen(fd, MAX_CONNECTIONS+1);
    if(ret_val!=0){
        fprintf(stderr, "listen failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

int main(){
    double time_spent = 0.0;
    clock_t begin = clock();

    fd_set read_fd_set;
    struct sockaddr_in new_addr;
    int server_fd, new_fd, ret_val, i;
    socklen_t addrlen;
    char buf[DATA_BUFFER];
    int all_connections[MAX_CONNECTIONS+1];
    int k = 0;

    /* Get the socket server fd */
    server_fd = create_tcp_server_socket();
    if(server_fd==-1){
        fprintf(stderr, "Failed to create a server\n");
        return -1;
    }

    /* Initialize all connections and set the first entry to server fd*/
    for(i=0; i<=MAX_CONNECTIONS; i++){
        all_connections[i] = -1;
    }
    all_connections[0] = server_fd;

    while(1){
        FD_ZERO(&read_fd_set);
        /* Sert the fd_set before passing it to the select call */
        for(i=0; i<=MAX_CONNECTIONS; i++){
            if(all_connections[i]>=0){
                FD_SET(all_connections[i], &read_fd_set);
            }
        }

        /* Invoke select() and then wait */
        ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        /* select() woke up. Identify the fd that has events */
        if(ret_val>=0){
            // printf("Select returned with %d\n", ret_val);
            /* Check if the fd with event is the server fd*/
            if(FD_ISSET(server_fd, &read_fd_set)){
                /* Accept the new connection */
                // printf("Returned fd is %d (server's fd)\n", server_fd);
                new_fd = accept(server_fd, (struct sockaddr*)&new_addr, &addrlen);
                if(new_fd>=0){
                    printf("Accepted a new connection with fd: %d\n\n", new_fd);
                    for(i=0; i<=MAX_CONNECTIONS; i++){
                        if(all_connections[i]<0){
                            all_connections[i] = new_fd;
                            break;
                        }
                    }
                }
                else{
                    fprintf(stderr, "accept failed [%s]\n", strerror(errno));
                }
                ret_val--;
                if(!ret_val){
                    continue;
                }
            }
            /* Check if the fd with event is a non-server fd*/
            for(i=1; i<=MAX_CONNECTIONS; i++){
                if((all_connections[i]>0)&&(FD_ISSET(all_connections[i], &read_fd_set))){
                    /* read incoming data */
                    // printf("Returned fd is %d [index, i: %d]\n", all_connections[i], i);
                    bzero(buf, DATA_BUFFER);
                    ret_val = recv(all_connections[i], buf, DATA_BUFFER, 0);
                    if(ret_val==0){
                        printf("Closing connection for fd: %d\n\n", all_connections[i]);
                        close(all_connections[i]);
                        all_connections[i] = -1; /* Connection is now closed */
                        k++;
                    }
                    if(ret_val>0){
                        // For writing output to the file - output.txt
                        FILE* filePointer = NULL;
                        // comment below line to disable the write operation to the file
                        filePointer = fopen("output.txt", "a");

                        printf("Received data (len %d bytes, fd: %d): %s\n", ret_val, all_connections[i], buf);

                        /* reply to client */
                        long int fact = compute_factorial(buf);

                        char str[INET_ADDRSTRLEN];
                        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&new_addr;
                        struct in_addr ipAddr = pV4Addr->sin_addr;
                        inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
                        int client_port = ntohs(new_addr.sin_port);

                        // printf("%s %d %ld\n", str, client_port, fact);
                        char output[DATA_BUFFER];
                        sprintf(output, "(%s:%d) %s! = %ld", str, client_port, buf, fact);
                        // sprintf(output, "(:) %s! = %ld", buf, fact);
                        writeToFile(filePointer, output);

                        bzero(buf, DATA_BUFFER);
                        sprintf(buf, "%ld", fact);
                        int n = write(all_connections[i], buf, strlen(buf));
                        if(n<0){
                            error("ERROR writing to socket");
                        }
                        else{
                            printf("Message sent: %s\n\n", buf);
                        }
                        // close the file pointer
                        if(filePointer!=NULL){
                            fclose(filePointer);
                            // printf("File closed!\n");
                        }
                    }
                    if(ret_val==-1){
                        printf("recv() failed for fd: %d [%s]\n", all_connections[i], strerror(errno));
                        break;
                    }
                }
                ret_val--;
                if(!ret_val){
                    continue;
                }
            }
        }
        if(k==MAX_CONNECTIONS){
            break;
        }
    }

    /* Close all the sockets */
    for(i=0; i<=MAX_CONNECTIONS; i++){
        if(all_connections[i]>0){
            close(all_connections[i]);
        }
    }

    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", time_spent);

    return 0;
}