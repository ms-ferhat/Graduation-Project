#ifndef TRANSFER_H
#define TRANSFER_H

#include <stdint.h>
#include <stddef.h>  
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define FILENAME_SIZE 256

typedef void (*transfer_callback)(uint64_t bytes_transferred, uint64_t total_bytes);

// Basic file transfer functions
bool send_file(const char *receiver_ip, const char *filename);
bool receive_file(char *filename);

// Extended file transfer with progress callback and chunk size control
bool send_file_ex(const char *receiver_ip, const char *filename, 
                 transfer_callback callback, uint64_t chunk_size);
bool receive_file_ex(char *filename, size_t filename_buf_size, 
                    transfer_callback callback, uint64_t chunk_size);

// String transfer functions
bool send_string(const char *receiver_ip, const char *message);
char *receive_string(void);

#endif // TRANSFER_H
