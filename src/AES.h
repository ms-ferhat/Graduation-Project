#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>

#define AES_KEY_LENGTH 128
#define AES_BLOCK_SIZE 16
#define BUFFER_SIZE 1024

void AES_menu();
int AES_generate_key(unsigned char *key);
int AES_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, const unsigned char *iv, unsigned char *ciphertext);
int AES_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, const unsigned char *iv, unsigned char *plaintext);