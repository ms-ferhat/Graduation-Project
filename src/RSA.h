#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>

#define RSA_KEY_LENGTH 1024
#define BUFFER_SIZE 256


void RSA_menu();
int RSA_generate_keys(char *public_key_path, char *private_key_path);
int RSA_encrypt(const unsigned char *plaintext, const char *ciphertext_file, const char *public_key_path);
int RAS_decrypt(const char *ciphertext_file, unsigned char *plaintext, const char *private_key_path);