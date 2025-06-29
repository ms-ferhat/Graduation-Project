#include "encryption.h"



static char Session_Key[AES_KEY_LENGTH] = {
    0x60, 0x3d, 0xeb, 0x10,
    0x15, 0xca, 0x71, 0xbe,
    0x2b, 0x73, 0xae, 0xf0,
    0x85, 0x7d, 0x77, 0x81
};
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



// Generate RSA key pair and save to files
int RSA_generate_keys(const char *public_key_path, const char *private_key_path) {
    int ret = 0;
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    FILE *pub_file = NULL, *priv_file = NULL;

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx || EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_LENGTH) <= 0 ||
        EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "RSA key generation failed.\n");
        goto cleanup;
    }

    // Write private key
    priv_file = fopen(private_key_path, "wb");
    if (!priv_file || !PEM_write_PrivateKey(priv_file, pkey, NULL, NULL, 0, NULL, NULL)) {
        fprintf(stderr, "Error writing private key.\n");
        goto cleanup;
    }

    // Write public key
    pub_file = fopen(public_key_path, "wb");
    if (!pub_file || !PEM_write_PUBKEY(pub_file, pkey)) {
        fprintf(stderr, "Error writing public key.\n");
        goto cleanup;
    }

    ret = 1;

cleanup:
    if (pub_file) fclose(pub_file);
    if (priv_file) fclose(priv_file);
    if (pkey) EVP_PKEY_free(pkey);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return ret;
}

// Encrypt plaintext using RSA public key
int RSA_encrypt(const unsigned char *plaintext, const char *ciphertext_file, const char *public_key_path) {
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    FILE *pub_file = fopen(public_key_path, "rb");
    unsigned char *ciphertext = NULL;
    size_t outlen;

    if (!pub_file) {
        fprintf(stderr, "Failed to open public key file.\n");
        return 0;
    }

    pkey = PEM_read_PUBKEY(pub_file, NULL, NULL, NULL);
    fclose(pub_file);
    if (!pkey) {
        fprintf(stderr, "Failed to load public key.\n");
        return 0;
    }

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        fprintf(stderr, "RSA encryption context init failed.\n");
        goto cleanup;
    }

    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, plaintext, strlen((char *)plaintext)) <= 0) {
        fprintf(stderr, "Encryption size calculation failed.\n");
        goto cleanup;
    }

    ciphertext = OPENSSL_malloc(outlen);
    if (!ciphertext) goto cleanup;

    if (EVP_PKEY_encrypt(ctx, ciphertext, &outlen, plaintext, strlen((char *)plaintext)) <= 0) {
        fprintf(stderr, "Encryption failed.\n");
        goto cleanup;
    }

    FILE *ct_file = fopen(ciphertext_file, "wb");
    if (!ct_file || fwrite(ciphertext, 1, outlen, ct_file) != outlen) {
        fprintf(stderr, "Writing ciphertext failed.\n");
        if (ct_file) fclose(ct_file);
        goto cleanup;
    }
    fclose(ct_file);
    ret = 1;

cleanup:
    if (ciphertext) OPENSSL_free(ciphertext);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    return ret;
}

// Decrypt ciphertext using RSA private key
int RSA_decrypt(const char *ciphertext_file, unsigned char *plaintext, const char *private_key_path) {
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    FILE *priv_file = fopen(private_key_path, "rb");
    unsigned char *ciphertext = NULL;
    size_t ciphertext_len, outlen;

    if (!priv_file) {
        fprintf(stderr, "Failed to open private key file.\n");
        return 0;
    }

    pkey = PEM_read_PrivateKey(priv_file, NULL, NULL, NULL);
    fclose(priv_file);
    if (!pkey) {
        fprintf(stderr, "Failed to load private key.\n");
        return 0;
    }

    FILE *ct_file = fopen(ciphertext_file, "rb");
    if (!ct_file) {
        fprintf(stderr, "Error opening ciphertext file.\n");
        goto cleanup;
    }

    fseek(ct_file, 0, SEEK_END);
    ciphertext_len = ftell(ct_file);
    fseek(ct_file, 0, SEEK_SET);

    ciphertext = OPENSSL_malloc(ciphertext_len);
    if (!ciphertext || fread(ciphertext, 1, ciphertext_len, ct_file) != ciphertext_len) {
        fprintf(stderr, "Error reading ciphertext.\n");
        fclose(ct_file);
        goto cleanup;
    }
    fclose(ct_file);

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        fprintf(stderr, "RSA decryption context init failed.\n");
        goto cleanup;
    }

    // Determine output length
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, ciphertext, ciphertext_len) <= 0) {
        fprintf(stderr, "Decryption size calculation failed.\n");
        goto cleanup;
    }

    if (EVP_PKEY_decrypt(ctx, plaintext, &outlen, ciphertext, ciphertext_len) <= 0) {
        fprintf(stderr, "Decryption failed.\n");
        goto cleanup;
    }

    plaintext[outlen] = '\0';
    ret = 1;

cleanup:
    if (ciphertext) OPENSSL_free(ciphertext);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    return ret;
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
