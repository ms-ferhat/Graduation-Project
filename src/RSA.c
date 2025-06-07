
#include "RSA.h"

void RSA_menu()
{
    unsigned char rsa_ciphertext[BUFFER_SIZE];
    unsigned char rsa_plaintext[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    int rsa_ciphertext_len, rsa_plaintext_len;
   int rsa_choice;

    while (1) {
        printf("###########################\n");
        printf("\nSelect an option:\n");
        printf("1. Generate RSA keys\n");
        printf("2. Encrypt data using RSA\n");
        printf("3. Decrypt data using RSA\n");
        printf("4. back\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &rsa_choice);
        getchar();  // Clear newline from input buffer
        if(rsa_choice == 4)break;
        switch (rsa_choice) {
            case 1: // Generate RSA keys
                if (generate_RSA_keys("public_key.pem", "private_key.pem")) {
                    printf("RSA keys generated successfully.\n");
                } else {
                    printf("Error generating RSA keys.\n");
                }
                break;

            case 2: // Encrypt data with RSA
                {
                    char input[BUFFER_SIZE];
                    char ciphertext_file[BUFFER_SIZE];

                    printf("Enter the plaintext message: ");
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0; // Remove newline character

                    printf("Enter the ciphertext file name: ");
                    fgets(ciphertext_file, sizeof(ciphertext_file), stdin);
                    ciphertext_file[strcspn(ciphertext_file, "\n")] = 0; // Remove newline character

                    if (encrypt_RSA((unsigned char *)input, ciphertext_file, "public_key.pem")) {
                        printf("Data encrypted successfully. Ciphertext written to %s\n", ciphertext_file);
                    } else {
                        printf("Encryption failed.\n");
                    }
                }
                break;

            case 3: // Decrypt data with RSA
                {
                    char ciphertext_file[BUFFER_SIZE];
                    unsigned char decrypted_text[BUFFER_SIZE];

                    printf("Enter the ciphertext file name: ");
                    fgets(ciphertext_file, sizeof(ciphertext_file), stdin);
                    ciphertext_file[strcspn(ciphertext_file, "\n")] = 0; // Remove newline character

                    if (decrypt_RSA(ciphertext_file, decrypted_text, "private_key.pem")) {
                        printf("Decrypted message: %s\n", decrypted_text);
                    } else {
                        printf("Decryption failed.\n");
                    }
                }
                break;

            case 4:
                exit(0);

            default:
                printf("Invalid choice. Please select again.\n");
        }
    }

    
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