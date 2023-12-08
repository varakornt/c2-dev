#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // Receive and print the welcome message
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Server: %s", buffer);
    } else {
        perror("Error receiving welcome message");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Main loop to send commands to the server
    while (1) {
        printf("Enter a command (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Send the command to the server
        send(client_socket, buffer, strlen(buffer), 0);

        // Check if the user wants to exit
        if (strcmp(buffer, "exit\n") == 0 || strcmp(buffer, "quit\n") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Receive and print the command output from the server
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        } else {
            perror("Error receiving data from server");
            break;
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
