#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Error codes for the server responses
#define SUCCESS 0
#define COMMAND_EXECUTION_ERROR 1
#define INVALID_COMMAND_ERROR 2

// Structure for server response
struct ServerResponse {
    int error_code;
    char message[BUFFER_SIZE];
};

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Send a welcome message to the client
    const char *welcome_message = "Welcome to the enhanced shell server!\n";
    send(client_socket, welcome_message, strlen(welcome_message), 0);

    while (1) {
        // Receive data from the client
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            perror("Error receiving data from client");
            break;
        }

        // Null-terminate the received data
        buffer[bytes_received] = '\0';

        // Check if the client wants to exit
        if (strcmp(buffer, "exit\n") == 0 || strcmp(buffer, "quit\n") == 0) {
            printf("Client requested to exit. Closing connection.\n");
            break;
        }

        // Execute the command received from the client and send the output back
        FILE *fp = popen(buffer, "r");
        if (fp == NULL) {
            perror("Error executing command");
            // Send an error response to the client
            struct ServerResponse response;
            response.error_code = COMMAND_EXECUTION_ERROR;
            snprintf(response.message, sizeof(response.message), "Error executing command: %s\n", buffer);
            send(client_socket, &response, sizeof(response), 0);
        } else {
            // Read the command output and send it to the client
            struct ServerResponse response;
            response.error_code = SUCCESS;
            while (fgets(response.message, sizeof(response.message), fp) != NULL) {
                send(client_socket, &response, sizeof(response), 0);
            }
            // Close the command output file pointer
            pclose(fp);
        }
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    while (1) {
        // Accept a connection from a client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Handle the client in a separate function
        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}

