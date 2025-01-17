//Server :

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8888
#define MAX_CLIENTS 5

void* handle_client(void* client_socket) {
    int client_fd = *((int*)client_socket);
    char buffer[1024];

    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            // Client disconnected or encountered an error
            close(client_fd);
            printf("Client disconnected\n");
            break;
        }

        if (strcmp(buffer, "ping") == 0) {
            printf("Received 'ping' from client\n");
            send(client_fd, "pong", 4, 0);
        }
    }

    free(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("Listening failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept incoming connections
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Create a new thread to handle the client
        pthread_t client_thread;
        int* client_socket_ptr = (int*)malloc(sizeof(int));
        *client_socket_ptr = client_fd;

        if (pthread_create(&client_thread, NULL, handle_client, (void*)client_socket_ptr) != 0) {
            perror("Thread creation failed");
            close(client_fd);
            free(client_socket_ptr);
        }
    }

    // Close the server socket (this should never be reached)
    close(server_fd);
    return 0;
}

//********************************************************************************************************
//Client :

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

int main() {
    int client_fd;
    struct sockaddr_in server_addr;

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    while (1) {
        // Send "ping" to the server
        send(client_fd, "ping", 4, 0);

        // Receive the response from the server
        char buffer[1024];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            // Server disconnected or encountered an error
            printf("Server disconnected\n");
            break;
        }

        if (strcmp(buffer, "pong") == 0) {
            printf("Received 'pong' from server\n");
        }

        sleep(1);  // Send "ping" every second
    }

    // Close the client socket
    close(client_fd);
    return 0;
}







