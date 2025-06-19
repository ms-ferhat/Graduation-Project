#ifndef TRANSFER_H
#define TRANSFER_H

#include <stdbool.h>

#define PORT 5000
#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256

// Function prototypes
bool send_file(const char *receiver_ip, const char *filename);
bool receive_file(char *filename);
bool send_string(const char *receiver_ip, const char *message);
char *receive_string(void);

#endif // TRANSFER_H