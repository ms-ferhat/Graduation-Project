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
#include <ctype.h>  // Added for isdigit()
#include "tranceiver.h"

#define PORT 5000
#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256
#define MAX_IPS 10

// Function to get all local IP addresses
void getLocalIPs(char ips[][16], int *count) {
    struct ifaddrs *ifaddr, *ifa;
    *count = 0;
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { // IPv4
            char host[NI_MAXHOST];
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                      host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (strcmp(host, "127.0.0.1") != 0) { // Skip localhost
                strncpy(ips[*count], host, 15);
                ips[*count][15] = '\0';
                (*count)++;
                if (*count >= MAX_IPS) break;
            }
        }
    }
    freeifaddrs(ifaddr);
}

void receiveFile() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return;
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        return;
    }

    printf("Waiting for incoming file on port %d...\n", PORT);

    // Accept a client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return;
    }

    printf("Connection from: %s\n", inet_ntoa(client_addr.sin_addr));

    // First receive the filename
    char filename[FILENAME_SIZE];
    int filename_size = recv(client_fd, filename, FILENAME_SIZE, 0);
    if (filename_size <= 0) {
        perror("Failed to receive filename");
        close(client_fd);
        close(server_fd);
        return;
    }
    filename[filename_size] = '\0';

    printf("Receiving file: %s\n", filename);

    // Open file for writing
    FILE *out_file = fopen(filename, "wb");
    if (!out_file) {
        perror("Failed to create file");
        close(client_fd);
        close(server_fd);
        return;
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
}

void sendFile() {
    char buffer[BUFFER_SIZE];
    char ips[MAX_IPS][16];
    int ip_count = 0;
    
    // Get available IPs
    getLocalIPs(ips, &ip_count);
    
    // Display available IPs
    printf("\nAvailable network interfaces:\n");
    for (int i = 0; i < ip_count; ++i) {
        printf("%d. %s\n", i+1, ips[i]);
    }
    
    // Get receiver IP
    char receiver_ip[16];
    printf("\nEnter receiver IP (or select interface for broadcast): ");
    if (scanf("%15s", receiver_ip) != 1) {
        printf("Invalid input\n");
        return;
    }
    
    // If user entered a number, use corresponding IP
    if (isdigit(receiver_ip[0])) {
        int choice = atoi(receiver_ip);
        if (choice > 0 && choice <= ip_count) {
            // Create broadcast address
            char *last_dot = strrchr(ips[choice-1], '.');
            if (last_dot != NULL) {
                char broadcast_ip[16];
                strncpy(broadcast_ip, ips[choice-1], last_dot - ips[choice-1]);
                strcat(broadcast_ip, ".255");
                printf("Using broadcast address: %s\n", broadcast_ip);
                strncpy(receiver_ip, broadcast_ip, 15);
                receiver_ip[15] = '\0';
            }
        }
    }
    
    // Get filename to send
    char filename[FILENAME_SIZE];
    printf("Enter file to send: ");
    if (scanf("%255s", filename) != 1) {
        printf("Invalid filename\n");
        return;
    }

    // Check if file exists
    FILE *in_file = fopen(filename, "rb");
    if (!in_file) {
        perror("File not found");
        return;
    }

    // Create socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        fclose(in_file);
        return;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, receiver_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        fclose(in_file);
        return;
    }

    // Connect to receiver
    printf("Attempting to connect to %s...\n", receiver_ip);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        fclose(in_file);
        return;
    }

    printf("Connected to %s\n", receiver_ip);

    // First send the filename
    if (send(sock, filename, strlen(filename), 0) < 0) {
        perror("Failed to send filename");
        close(sock);
        fclose(in_file);
        return;
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
}

