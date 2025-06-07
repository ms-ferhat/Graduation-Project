#include "AES.h"

void AES_menu()
{
    unsigned char key[AES_KEY_LENGTH / 8];
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char ciphertext[BUFFER_SIZE];
    unsigned char plaintext[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    int ciphertext_len, plaintext_len;

    int aes_choice;
    while (1) {
        printf("###########################\n");
        printf("Select an option:\n");
        printf("1. Generate AES key and IV\n");
        printf("2. Encrypt data\n");
        printf("3. Decrypt data\n");
        printf("4. Back\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &aes_choice);
        getchar();  // Clear newline from input buffer
        if(aes_choice == 4)break;
        switch (aes_choice) {
            case 1:
                if (generate_key(key, iv)) {
                    printf("Generated AES Key: ");
                    for (int i = 0; i < AES_KEY_LENGTH / 8; i++) {
                        printf("%02x", key[i]);
                    }
                    printf("\nGenerated IV: ");
                    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
                        printf("%02x", iv[i]);
                    }
                    printf("\n");
                } else {
                    printf("Error generating key or IV.\n");
                }
                break;

            case 2:
                printf("Enter the AES key (hex format): ");
                fgets(input, sizeof(input), stdin);
                for (int i = 0; i < AES_KEY_LENGTH / 8; i++) {
                    sscanf(input + 2 * i, "%2hhx", &key[i]);
                }
                
                printf("Enter the IV (hex format): ");
                fgets(input, sizeof(input), stdin);
                for (int i = 0; i < AES_BLOCK_SIZE; i++) {
                    sscanf(input + 2 * i, "%2hhx", &iv[i]);
                }

                printf("Enter the plaintext message: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;  // Remove the newline character

                ciphertext_len = encrypt_AES((unsigned char *)input, strlen((char *)input), key, iv, ciphertext);
                if (ciphertext_len >= 0) {
                    printf("Ciphertext: ");
                    for (int i = 0; i < ciphertext_len; i++) {
                        printf("%02x", ciphertext[i]);
                    }
                    printf("\n");
                } else {
                    printf("Encryption failed.\n");
                }
                break;

            case 3:
                printf("Enter the AES key (hex format): ");
                fgets(input, sizeof(input), stdin);
                for (int i = 0; i < AES_KEY_LENGTH / 8; i++) {
                    sscanf(input + 2 * i, "%2hhx", &key[i]);
                }

                printf("Enter the IV (hex format): ");
                fgets(input, sizeof(input), stdin);
                for (int i = 0; i < AES_BLOCK_SIZE; i++) {
                    sscanf(input + 2 * i, "%2hhx", &iv[i]);
                }

                printf("Enter the ciphertext (hex format): ");
                fgets(input, sizeof(input), stdin);
                int ciphertext_len_input = strlen(input) / 2;
                for (int i = 0; i < ciphertext_len_input; i++) {
                    sscanf(input + 2 * i, "%2hhx", &ciphertext[i]);
                }

                plaintext_len = decrypt_AES(ciphertext, ciphertext_len_input, key, iv, plaintext);
                if (plaintext_len >= 0) {
                    plaintext[plaintext_len] = '\0'; // Null-terminate the plaintext
                    printf("Decrypted message: %s\n", plaintext);
                } else {
                    printf("Decryption failed.\n");
                }
                break;

            case 4:
                exit(0);

            default:
                printf("Invalid choice. Please select again.\n");
        }
    }

}


// Function to generate AES key and IV
int AES_generate_key(unsigned char *key) {
    if (!RAND_bytes(key, AES_KEY_LENGTH / 8)) {
        fprintf(stderr, "Error generating random bytes for key or IV.\n");
        return 0;
    }
    return 1;
}

// AES encryption function
int AES_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, const unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;

    // Initialize the encryption operation with AES-256-CBC
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
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
int AES_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, const unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;

    // Initialize the decryption operation with AES-256-CBC
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
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
