#include "transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdbool.h>

bool send_file(const char *receiver_ip, const char *filename) {
    char buffer[BUFFER_SIZE];
    
    // Check if file exists
    FILE *in_file = fopen(filename, "rb");
    if (!in_file) {
        perror("File not found");
        return false;
    }

    // Create socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        fclose(in_file);
        return false;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, receiver_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        fclose(in_file);
        return false;
    }

    // Connect to receiver
    printf("Attempting to connect to %s...\n", receiver_ip);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        fclose(in_file);
        return false;
    }

    printf("Connected to %s\n", receiver_ip);

    // First send filename length
    uint8_t filename_len = strlen(filename);
    if (send(sock, &filename_len,1,0) < 0) {
        perror("Failed to send filename length");
        close(sock);
        fclose(in_file);
        return false;
    }

    //  send the filename
    if (send(sock, filename, strlen(filename), 0) < 0) {
        perror("Failed to send filename");
        close(sock);
        fclose(in_file);
        return false;
    }

    // Send file data
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in_file)) > 0) {
        if (send(sock, buffer, bytes_read, 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    printf("File sent successfully!\n");

    // Cleanup
    fclose(in_file);
    close(sock);
    return true;
}

bool receive_file(char *filename) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return false;
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        return false;
    }

    printf("Waiting for incoming file on port %d...\n", PORT);

    // Accept a client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return false;
    }

    printf("Connection from: %s\n", inet_ntoa(client_addr.sin_addr));
    // First receive the filename length
    uint8_t filename_len; 
    int r = recv(client_fd, &filename_len, sizeof(filename_len), 0);
    if (r != sizeof(filename_len)) {
        perror("Failed to receive filename length");
        return false;
    }
    if (filename_len == 0 || filename_len >= FILENAME_SIZE) {
    fprintf(stderr, "Invalid filename length: %u\n", filename_len);
    return false;
    }
    // First receive the filename
    int total_received = 0;
    while (total_received < filename_len) {
        int bytes = recv(client_fd, filename + total_received, filename_len - total_received, 0);
        if (bytes <= 0) {
            perror("Failed to receive complete filename");
            return false;
        }
        total_received += bytes;
    }
    filename[filename_len] = '\0'; // Ensure null-terminated string
    printf("Receiving file: %s\n", filename);

    // Open file for writing
    FILE *out_file = fopen(filename, "wb");
    if (!out_file) {
        perror("Failed to create file");
        close(client_fd);
        close(server_fd);
        return false;
    }

    // Receive file data
    ssize_t bytes_received;
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        if (fwrite(buffer, 1, bytes_received, out_file) != bytes_received) {
            perror("Write failed");
            break;
        }
    }

    if (bytes_received < 0) {
        perror("Error while receiving file");
    } else {
        printf("File received successfully!\n");
    }

    // Cleanup
    fclose(out_file);
    close(client_fd);
    close(server_fd);
    return true;
}

bool send_string(const char *receiver_ip, const char *message) {
    // Create socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, receiver_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return false;
    }

    // Connect to receiver
    printf("Attempting to connect to %s...\n", receiver_ip);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return false;
    }

    printf("Connected to %s\n", receiver_ip);

    // Send the string
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Failed to send string");
        close(sock);
        return false;
    }

    printf("String sent successfully!\n");
    close(sock);
    return true;
}

char *receive_string(void) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        return NULL;
    }

    printf("Waiting for incoming string on port %d...\n", PORT);

    // Accept a client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return NULL;
    }

    printf("Connection from: %s\n", inet_ntoa(client_addr.sin_addr));

    // Receive the string
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive string");
        close(client_fd);
        close(server_fd);
        return NULL;
    }
    buffer[bytes_received] = '\0';

    printf("Received string: %s\n", buffer);

    // Cleanup
    close(client_fd);
    close(server_fd);

    // Return a copy of the string
    char *received_str = malloc(bytes_received + 1);
    if (received_str) {
        strcpy(received_str, buffer);
    }
    return received_str;
}