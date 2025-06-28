#ifndef ENCRYPTION_H
#define ENCRYPTION_H
 
// Include necessary OpenSSL headers
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>
#include "transfer.h"

// Define constants for AES encryption
#define AES_KEY_LENGTH 128
#define AES_BLOCK_SIZE 16
// Define constants for RSA encryption
#define RSA_KEY_LENGTH 1024



// Function prototypes for AES encryption and decryption
int AES_generate_key(unsigned char *key);
void Set_Session_Key(unsigned char *key );
void Get_Session_Key(unsigned char *key);
int AES_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *ciphertext);
int AES_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *plaintext);
int AES_Encrypt_file(const char *in_filename, const char *out_filename, const unsigned char *key);
int AES_Decrypt_file(const char *in_filename, const char *out_filename, const unsigned char *key);


// Function prototypes for RSA encryption and decryption
int RSA_generate_keys(const char *public_key_path,const char *private_key_path);
int RSA_encrypt(const unsigned char *plaintext, const char *ciphertext_file, const char *public_key_path);
int RSA_decrypt(const char *ciphertext_file, unsigned char *plaintext, const char *private_key_path);

// Function prototypes for SHA-256 hashing
void sha256_string(const char *str, char outputBuffer[65]);
void sha256_file(const char *filename, char outputBuffer[65]);

#endif // ENCRYPTION_H