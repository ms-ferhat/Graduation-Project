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
#include <inttypes.h>

// Default callback functions
static void default_send_callback(uint64_t bytes_sent, uint64_t total_bytes) {
    if (total_bytes > 0) {
        printf("Progress: %.2f%%\r", (double)bytes_sent / total_bytes * 100);
        fflush(stdout);
    }
}

static void default_receive_callback(uint64_t bytes_received, uint64_t total_bytes) {
    if (total_bytes > 0) {
        printf("Progress: %.2f%%\r", (double)bytes_received / total_bytes * 100);
        fflush(stdout);
    }
}

bool send_file_ex(const char *receiver_ip, const char *filename, 
                 transfer_callback callback, uint64_t chunk_size) {
    // Set default callback if none provided
    if (!callback) {
        callback = default_send_callback;
    }
    
    // Use default chunk size if not specified
    if (chunk_size == 0) {
        chunk_size = BUFFER_SIZE;
    }

    // Check if file exists and get size
    FILE *in_file = fopen(filename, "rb");
    if (!in_file) {
        perror("File not found");
        return false;
    }

    // Get file size
    fseek(in_file, 0, SEEK_END);
    uint64_t file_size = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

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

    // Send file metadata (filename + size)
    uint8_t filename_len = strlen(filename);
    uint32_t metadata[3] = {
        htonl(filename_len),
        htonl(file_size >> 32),     // High 32 bits of file size
        htonl(file_size & 0xFFFFFFFF) // Low 32 bits of file size
    };

    if (send(sock, metadata, sizeof(metadata), 0) != sizeof(metadata)) {
        perror("Failed to send metadata");
        close(sock);
        fclose(in_file);
        return false;
    }

    // Send the filename
    if (send(sock, filename, filename_len, 0) != filename_len) {
        perror("Failed to send filename");
        close(sock);
        fclose(in_file);
        return false;
    }

    // Send file data in chunks
    char *buffer = malloc(chunk_size);
    if (!buffer) {
        perror("Memory allocation failed");
        close(sock);
        fclose(in_file);
        return false;
    }

    uint64_t total_sent = 0;
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, chunk_size, in_file)) > 0) {
        ssize_t bytes_sent = send(sock, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Send failed");
            break;
        }
        
        total_sent += bytes_sent;
        callback(total_sent, file_size);
        
        // If we didn't send all bytes we read, try to send the rest
        if ((size_t)bytes_sent < bytes_read) {
            size_t remaining = bytes_read - bytes_sent;
            while (remaining > 0) {
                bytes_sent = send(sock, buffer + bytes_sent, remaining, 0);
                if (bytes_sent < 0) {
                    perror("Send failed");
                    break;
                }
                remaining -= bytes_sent;
                total_sent += bytes_sent;
                callback(total_sent, file_size);
            }
        }
    }

    free(buffer);
    
    if (total_sent == file_size) {
        printf("\nFile sent successfully! (%" PRIu64 " bytes)\n", total_sent);
    } else {
        printf("\nFile transfer incomplete. Sent %" PRIu64 " of %" PRIu64 " bytes\n", 
               total_sent, file_size);
    }

    // Cleanup
    fclose(in_file);
    close(sock);
    return (total_sent == file_size);
}

bool receive_file_ex(char *filename, size_t filename_buf_size, 
                    transfer_callback callback, uint64_t chunk_size) {
    // Set default callback if none provided
    if (!callback) {
        callback = default_receive_callback;
    }
    
    // Use default chunk size if not specified
    if (chunk_size == 0) {
        chunk_size = BUFFER_SIZE;
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

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

    // Receive file metadata
    uint32_t metadata[3];
    if (recv(client_fd, metadata, sizeof(metadata), MSG_WAITALL) != sizeof(metadata)) {
        perror("Failed to receive metadata");
        close(client_fd);
        close(server_fd);
        return false;
    }

    uint8_t filename_len = ntohl(metadata[0]);
    uint64_t file_size = ((uint64_t)ntohl(metadata[1]) << 32) | ntohl(metadata[2]);

    // Validate filename length
    if (filename_len == 0 || filename_len >= filename_buf_size) {
        fprintf(stderr, "Invalid filename length: %u (buffer size: %zu)\n", 
                filename_len, filename_buf_size);
        close(client_fd);
        close(server_fd);
        return false;
    }

    // Receive the filename
    if (recv(client_fd, filename, filename_len, MSG_WAITALL) != filename_len) {
        perror("Failed to receive complete filename");
        close(client_fd);
        close(server_fd);
        return false;
    }
    filename[filename_len] = '\0';

    printf("Receiving file: %s (%" PRIu64 " bytes)\n", filename, file_size);

    // Open file for writing
    FILE *out_file = fopen(filename, "wb");
    if (!out_file) {
        perror("Failed to create file");
        close(client_fd);
        close(server_fd);
        return false;
    }

    // Receive file data in chunks
    char *buffer = malloc(chunk_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(out_file);
        close(client_fd);
        close(server_fd);
        return false;
    }

    uint64_t total_received = 0;
    ssize_t bytes_received;
    
    while (total_received < file_size) {
        uint64_t remaining = file_size - total_received;
        size_t to_receive = (remaining > chunk_size) ? chunk_size : remaining;
        
        bytes_received = recv(client_fd, buffer, to_receive, 0);
        if (bytes_received <= 0) {
            perror("Error while receiving file");
            break;
        }
        
        if (fwrite(buffer, 1, bytes_received, out_file) != bytes_received) {
            perror("Write failed");
            break;
        }
        
        total_received += bytes_received;
        callback(total_received, file_size);
    }

    free(buffer);
    
    if (total_received == file_size) {
        printf("\nFile received successfully! (%" PRIu64 " bytes)\n", total_received);
    } else {
        printf("\nFile transfer incomplete. Received %" PRIu64 " of %" PRIu64 " bytes\n", 
               total_received, file_size);
    }

    // Cleanup
    fclose(out_file);
    close(client_fd);
    close(server_fd);
    return (total_received == file_size);
}

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
    if (send(sock, &filename_len, 1, 0) < 0) {
        perror("Failed to send filename length");
        close(sock);
        fclose(in_file);
        return false;
    }

    // Then send the filename
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

    // Then receive the filename
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
