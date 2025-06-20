#include "encryption.h"



static char Session_Key[AES_KEY_LENGTH] = {0};
// Function to generate AES key and IV
int AES_generate_key(unsigned char *key) {
    if (!RAND_bytes(key, AES_KEY_LENGTH / 8)) {
        fprintf(stderr, "Error generating random bytes for key or IV.\n");
        return 0;
    }
    return 1;
}
void Set_Session_Key(unsigned char *key )
{
    // This function can be used to set the session key in a secure manner.
    strncpy(Session_Key, key, AES_KEY_LENGTH - 1);
}
void Get_Session_Key(unsigned char *key)
{
    // This function retrieves the session key.
    strncpy(key, Session_Key, AES_KEY_LENGTH - 1);
    key[AES_KEY_LENGTH - 1] = '\0'; // Ensure null termination
}

// AES encryption function
int AES_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;

    // Initialize the encryption operation with AES-256-CBC
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Perform the encryption operation
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize the encryption
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// AES decryption function
int AES_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;

    // Initialize the decryption operation with AES-256-CBC
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Perform the decryption operation
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    // Finalize the decryption
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}




// AES file encryption function
int AES_Encrypt_file(const char *in_filename, const char *out_filename, const unsigned char *key) {
    FILE *in_file = fopen(in_filename, "rb");
    FILE *out_file = fopen(out_filename, "wb");
    if (!in_file || !out_file) {
        if (in_file) fclose(in_file);
        if (out_file) fclose(out_file);
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    unsigned char in_buf[BUFFER_SIZE];
    unsigned char out_buf[BUFFER_SIZE + AES_BLOCK_SIZE];
    int in_len, out_len;

    while ((in_len = fread(in_buf, 1, BUFFER_SIZE, in_file)) > 0) {
        if (1 != EVP_EncryptUpdate(ctx, out_buf, &out_len, in_buf, in_len)) {
            EVP_CIPHER_CTX_free(ctx);
            fclose(in_file);
            fclose(out_file);
            return -1;
        }
        fwrite(out_buf, 1, out_len, out_file);
    }

    if (1 != EVP_EncryptFinal_ex(ctx, out_buf, &out_len)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(in_file);
        fclose(out_file);
        return -1;
    }
    fwrite(out_buf, 1, out_len, out_file);

    EVP_CIPHER_CTX_free(ctx);
    fclose(in_file);
    fclose(out_file);

    return 0;
}
// AES file decryption function
int AES_Decrypt_file(const char *in_filename, const char *out_filename, const unsigned char *key) {
    FILE *in_file = fopen(in_filename, "rb");
    FILE *out_file = fopen(out_filename, "wb");
    if (!in_file || !out_file) {
        if (in_file) fclose(in_file);
        if (out_file) fclose(out_file);
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    unsigned char in_buf[BUFFER_SIZE];
    unsigned char out_buf[BUFFER_SIZE + AES_BLOCK_SIZE];
    int in_len, out_len;

    while ((in_len = fread(in_buf, 1, BUFFER_SIZE, in_file)) > 0) {
        if (1 != EVP_DecryptUpdate(ctx, out_buf, &out_len, in_buf, in_len)) {
            EVP_CIPHER_CTX_free(ctx);
            fclose(in_file);
            fclose(out_file);
            return -1;
        }
        fwrite(out_buf, 1, out_len, out_file);
    }

    if (1 != EVP_DecryptFinal_ex(ctx, out_buf, &out_len)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(in_file);
        fclose(out_file);
        return -1;
    }
    fwrite(out_buf, 1, out_len, out_file);

    EVP_CIPHER_CTX_free(ctx);
    fclose(in_file);
    fclose(out_file);

    return 0;
}

// Function to generate RSA keys
int RSA_generate_keys(char *public_key_path, char *private_key_path) {
    RSA *rsa = RSA_generate_key(RSA_KEY_LENGTH, RSA_F4, NULL, NULL);
    if (!rsa) {
        fprintf(stderr, "Error generating RSA keys.\n");
        return 0;
    }

    // Save the public key
    FILE *pub_file = fopen(public_key_path, "wb");
    if (pub_file == NULL || !PEM_write_RSAPublicKey(pub_file, rsa)) {
        fprintf(stderr, "Error saving public key.\n");
        RSA_free(rsa);
        return 0;
    }
    fclose(pub_file);

    // Save the private key
    FILE *priv_file = fopen(private_key_path, "wb");
    if (priv_file == NULL || !PEM_write_RSAPrivateKey(priv_file, rsa, NULL, NULL, 0, NULL, NULL)) {
        fprintf(stderr, "Error saving private key.\n");
        RSA_free(rsa);
        return 0;
    }
    fclose(priv_file);

    RSA_free(rsa);
    return 1;
}

// RSA encryption function
int RSA_encrypt(const unsigned char *plaintext, const char *ciphertext_file, const char *public_key_path) {
    RSA *rsa = RSA_new();
    FILE *pub_file = fopen(public_key_path, "rb");
    if (!pub_file || !PEM_read_RSAPublicKey(pub_file, &rsa, NULL, NULL)) {
        fprintf(stderr, "Error loading public key.\n");
        return 0;
    }
    fclose(pub_file);

    // Encrypt plaintext
    unsigned char ciphertext[RSA_size(rsa)];
    int result = RSA_public_encrypt(strlen((const char *)plaintext), plaintext, ciphertext, rsa, RSA_PKCS1_OAEP_PADDING);
    
    if (result < 0) {
        RSA_free(rsa);
        return 0; // Encryption failed
    }

    // Write ciphertext to the file
    FILE *ct_file = fopen(ciphertext_file, "wb");
    if (!ct_file) {
        fprintf(stderr, "Error opening ciphertext file for writing.\n");
        RSA_free(rsa);
        return 0;
    }
    fwrite(ciphertext, 1, result, ct_file);
    fclose(ct_file);

    RSA_free(rsa);
    return 1; // Successful encryption
}
// RSA decryption function
int RSA_decrypt(const char *ciphertext_file, unsigned char *plaintext, const char *private_key_path) 
{
    RSA *rsa = RSA_new();
    FILE *priv_file = fopen(private_key_path, "rb");
    if (!priv_file || !PEM_read_RSAPrivateKey(priv_file, &rsa, NULL, NULL)) {
        fprintf(stderr, "Error loading private key.\n");
        return 0;
    }
    fclose(priv_file);

    // Read ciphertext from the file
    unsigned char ciphertext[RSA_size(rsa)];
    FILE *ct_file = fopen(ciphertext_file, "rb");
    if (!ct_file) {
        fprintf(stderr, "Error opening ciphertext file.\n");
        RSA_free(rsa);
        return 0;
    }

    int ciphertext_len = fread(ciphertext, 1, RSA_size(rsa), ct_file);
    fclose(ct_file);

    // Decrypt ciphertext
    int result = RSA_private_decrypt(ciphertext_len, ciphertext, plaintext, rsa, RSA_PKCS1_OAEP_PADDING);

    if (result < 0) {
        RSA_free(rsa);
        return 0; // Decryption failed
    }

    plaintext[result] = '\0'; // Null-terminate the plaintext
    RSA_free(rsa);
    return 1; // Successful decryption
}

// Compute SHA256 hash of a string
void sha256_string(const char *str, char outputBuffer[65]) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, str, strlen(str));
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    for (unsigned int i = 0; i < hash_len; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    outputBuffer[hash_len * 2] = '\0';
}

// Compute SHA256 hash of a file
void sha256_file(const char *filename, char outputBuffer[65]) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    unsigned char buf[4096];
    size_t bytesRead;
    FILE *file = fopen(filename, "rb");

    if (!file) {
        fprintf(stderr, "Could not open file %s\n", filename);
        outputBuffer[0] = '\0';
        EVP_MD_CTX_free(ctx);
        return;
    }

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    while ((bytesRead = fread(buf, 1, sizeof(buf), file)) > 0)
        EVP_DigestUpdate(ctx, buf, bytesRead);
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    fclose(file);

    for (unsigned int i = 0; i < hash_len; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    outputBuffer[hash_len * 2] = '\0';
}