#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <ifaddrs.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// Function to get all local IP addresses
std::vector<std::string> getLocalIPs() 
{
    std::vector<std::string> ips;
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return ips;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { // IPv4
            char host[NI_MAXHOST];
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                      host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            std::string ip(host);
            if (ip != "127.0.0.1") { // Skip localhost
                ips.push_back(ip);
            }
        }
    }

    freeifaddrs(ifaddr);
    return ips;
}

void receiveFile() 
{
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

    std::cout << "Waiting for incoming file on port " << PORT << "..." << std::endl;

    // Accept a client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return;
    }

    std::cout << "Connection from: " << inet_ntoa(client_addr.sin_addr) << std::endl;

    // First receive the filename
    char filename[256];
    int filename_size = recv(client_fd, filename, 256, 0);
    if (filename_size <= 0) {
        perror("Failed to receive filename");
        close(client_fd);
        close(server_fd);
        return;
    }
    filename[filename_size] = '\0';

    std::cout << "Receiving file: " << filename << std::endl;

    // Open file for writing
    std::ofstream out_file(filename, std::ios::binary);
    if (!out_file) {
        perror("Failed to create file");
        close(client_fd);
        close(server_fd);
        return;
    }

    // Receive file data
    ssize_t bytes_received;
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        out_file.write(buffer, bytes_received);
    }

    if (bytes_received < 0) {
        perror("Error while receiving file");
    } else {
        std::cout << "File received successfully!" << std::endl;
    }

    // Cleanup
    out_file.close();
    close(client_fd);
    close(server_fd);
}

void sendFile() 
{
    char buffer[BUFFER_SIZE];  // Buffer declaration added here
    
    // Get available IPs
    auto local_ips = getLocalIPs();
    
    // Display available IPs
    std::cout << "\nAvailable network interfaces:\n";
    for (size_t i = 0; i < local_ips.size(); ++i) {
        std::cout << i+1 << ". " << local_ips[i] << "\n";
    }
    
    // Get receiver IP
    std::string receiver_ip;
    std::cout << "\nEnter receiver IP (or select interface for broadcast): ";
    std::cin >> receiver_ip;
    
    // If user entered a number, use corresponding IP
    if (isdigit(receiver_ip[0])) {
        int choice = stoi(receiver_ip);
        if (choice > 0 && choice <= local_ips.size()) {
            // Create broadcast address
            std::string base_ip = local_ips[choice-1];
            size_t last_dot = base_ip.rfind('.');
            if (last_dot != std::string::npos) {
                std::string broadcast_ip = base_ip.substr(0, last_dot) + ".255";
                std::cout << "Using broadcast address: " << broadcast_ip << "\n";
                receiver_ip = broadcast_ip;
            }
        }
    }
    
    // Get filename to send
    std::string filename;
    std::cout << "Enter file to send: ";
    std::cin >> filename;

    // Check if file exists
    std::ifstream in_file(filename, std::ios::binary);
    if (!in_file) {
        perror("File not found");
        return;
    }

    // Create socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, receiver_ip.c_str(), &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return;
    }

    // Connect to receiver
    std::cout << "Attempting to connect to " << receiver_ip << "..." << std::endl;
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    std::cout << "Connected to " << receiver_ip << std::endl;

    // First send the filename
    send(sock, filename.c_str(), filename.size(), 0);

    // Send file data
    while (!in_file.eof()) {
        in_file.read(buffer, BUFFER_SIZE);
        ssize_t bytes_sent = send(sock, buffer, in_file.gcount(), 0);
        if (bytes_sent < 0) {
            perror("Send failed");
            break;
        }
    }

    std::cout << "File sent successfully!" << std::endl;

    // Cleanup
    in_file.close();
    close(sock);
}
