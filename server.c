#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define PORT 8005
#define MAX_CLIENTS 100
#define MAX_PROC_NAME 256
#define MAX_PATH 512  // Increased from 256
#define MAX_BUFFER 1024
#define MAX_RESPONSE 1024  // Increased from 512

// Structure to hold process information
struct proc_info {
    char name[MAX_PROC_NAME];
    int pid;
    long user_time;
    long kernel_time;
};

// Function to read CPU usage of a process from /proc/[pid]/stat
void get_process_info(struct proc_info *proc, int pid) {
    char path[MAX_PATH], buffer[MAX_BUFFER];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        sscanf(buffer, "%d (%[^)]) %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %ld %ld",
               &proc->pid, proc->name, &proc->user_time, &proc->kernel_time);
    }
    close(fd);
}

// Function to find the top two CPU-consuming processes
void find_top_two_processes(struct proc_info *top_two) {
    DIR *dir;
    struct dirent *entry;
    struct proc_info temp;

    top_two[0].user_time = top_two[1].user_time = 0;
    top_two[0].kernel_time = top_two[1].kernel_time = 0;

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir failed");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            char proc_dir[MAX_PATH];
            if (snprintf(proc_dir, sizeof(proc_dir), "/proc/%s", entry->d_name) >= sizeof(proc_dir)) {
                // Path was truncated, skip this entry
                continue;
            }

            DIR *subdir = opendir(proc_dir);
            if (subdir != NULL) {
                closedir(subdir);
                get_process_info(&temp, pid);

                long total_time = temp.user_time + temp.kernel_time;
                if (total_time > (top_two[0].user_time + top_two[0].kernel_time)) {
                    top_two[1] = top_two[0];
                    top_two[0] = temp;
                } else if (total_time > (top_two[1].user_time + top_two[1].kernel_time)) {
                    top_two[1] = temp;
                }
            }
        }
    }
    closedir(dir);
}

// Client handler function
void* handle_client(void* arg) {
    intptr_t client_socket = (intptr_t)arg;

    struct proc_info top_two[2];
    find_top_two_processes(top_two);

    char response[MAX_RESPONSE];
    int written = snprintf(response, sizeof(response),
             "Top CPU Processes:\n1. %.200s (PID: %d) - User: %ld, Kernel: %ld\n2. %.200s (PID: %d) - User: %ld, Kernel: %ld\n",
             top_two[0].name, top_two[0].pid, top_two[0].user_time, top_two[0].kernel_time,
             top_two[1].name, top_two[1].pid, top_two[1].user_time, top_two[1].kernel_time);

    if (written >= sizeof(response)) {
        // Response was truncated, handle appropriately (e.g., log a warning)
        fprintf(stderr, "Warning: Response truncated\n");
    }

    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Prepare address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept incoming connections
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)(intptr_t)new_socket) != 0) {
            perror("Thread creation failed");
            close(new_socket);
        } else {
            pthread_detach(thread_id);  // Detach the thread for independent execution
        }
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
