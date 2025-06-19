#include "message.h"
#include "transfer.h"
int Communication_Setup()
{
    // Generate and Store AES Session Key
    unsigned char temp_key[AES_KEY_LENGTH];
    AES_generate_key(temp_key);
    
    Set_Session_Key(temp_key);
    printf("AES Session Key generated and set successfully.\n");


    // Generate RSA keys
    RSA_generate_keys("public_key.pem", "private_key.pem");
    printf("RSA keys generated successfully.\n");


    // Additional setup can be done here if needed
    return 0;
}


/* Start_Com function 
    1. check if trans or reciver
    2. send and recive public key 
    3. generat symicteic key and encrypt it then send it for trans
    4. send read singal after recive tsymitic key
*/

int Communication_inti(NodeType node_type,const char *receiver_ip)
{
    unsigned char session_key[AES_KEY_LENGTH];
    char *filename;
    // Type 0 mean sender
    if(node_type == SENDER) {
        // send Connection request to receiver
        if (!send_string(receiver_ip, "1")) {
            fprintf(stderr, "Error sending connection request\n");
            return -1;
        }
        // wait for public key from receiver
        
        receive_file(filename);
       
        // get Symitric key
        Get_Session_Key(session_key);
        // encrypt Symitric key using RSA public key( received)
        
        RSA_encrypt(session_key, "encrypted_session_key.txt", "public_key.pem");
        printf("Session key encrypted successfully.\n");
        // send Symitric key after encrypt
        send_file(receiver_ip, "encrypted_session_key.txt");
        printf("Encrypted session key sent successfully.\n");
        
    }else if(node_type == RECEIVER )
    {
        // receive connection request from sender
        char *buffer = receive_string();

        // Check if the connection request is valid
        if (buffer == NULL || strcmp(buffer, "1") != 0) {
            fprintf(stderr, "Invalid connection request\n");
            return -1;
        }
        free(buffer); // Free the received buffer
        printf("Connection request received successfully.\n");
        // send public key to sender
        send_file(receiver_ip, "public_key.pem");
        printf("Public key sent successfully.\n");
        // recive Symitric key after encrypt
        receive_file(filename);
        printf("Encrypted session key received successfully.\n");
        // decrypt Symitric key using RSA private key
        RSA_decrypt("encrypted_session_key.txt", session_key, "private_key.pem");
        printf("Session key decrypted successfully.\n");
        // set Symitric key
        Set_Session_Key(session_key);
        printf("Session key set successfully.\n");
    }else{
        printf("unkown type\n");
    }

}

int Send_Text_Message(unsigned char *message, unsigned char *receiver_ip)
{
    unsigned char cipher_message[BUFFER_SIZE];
    unsigned char session_key[AES_KEY_LENGTH];
    // load the session key
    Get_Session_Key(session_key);
    // Prepare the text message
    // Encrypt data using AES
    AES_encrypt(message, strlen((char *)message), session_key, cipher_message);
     
    // calculate hash of encypted message
    //sha256_string((char *)text_message.data, text_message.hash);
   
    printf("Cipher message: %s\n", cipher_message);
    // Send the message (implementation of sending is not shown here)
    send_string(receiver_ip, cipher_message);

    return 0; // Return success

}
int Recive_Text_Message(unsigned char *decrypted_message)
{
    unsigned char session_key[AES_KEY_LENGTH];
    // load the session key
    Get_Session_Key(session_key);
    

    // Recive message 
    // (implementation of receiving is not shown here)
    unsigned char *re_string = receive_string();
    printf("Cipher message: %s\n", re_string);

    if (re_string == NULL) {
        fprintf(stderr, "Error receiving text message\n");
    }
    
    // Decrypt data using AES
    AES_decrypt(re_string, strlen((char *)re_string), session_key, decrypted_message);
    
    decrypted_message[strlen((char *)re_string)] = '\0'; // Null-terminate the decrypted message
    
}
int Send_Voice_Message(unsigned char *filename, unsigned char *receiver_ip)
{
  
    unsigned char session_key[AES_KEY_LENGTH];
    unsigned char message_buffer[BUFFER_SIZE];

    // load the session key
    Get_Session_Key(session_key);

    //read the voice file int buffer
    FILE *file = fopen((const char *)filename, "rb");
    if (!file) {
        fprintf(stderr, "Error opening voice file\n");
        return -1;
    }
    size_t bytes_read = fread(message_buffer, 1, BUFFER_SIZE, file);
    fclose(file);
    if (bytes_read == 0) {
        fprintf(stderr, "Error reading voice file or file is empty\n");
        return -1;
    }
    // Encrypt the voice message using AES
    unsigned char encrypted_message[BUFFER_SIZE];
    AES_encrypt(message_buffer, bytes_read, session_key, encrypted_message);
    printf("Encrypted voice message: %s\n", encrypted_message);    
    // Calculate the hash of the encrypted message
    
    // save the encrypted message to a file
    FILE *encrypted_file = fopen("encrypted_voice_message.bin", "wb");
    if (!encrypted_file) {
        fprintf(stderr, "Error opening file to save encrypted voice message\n");
        return -1;
    }
    fwrite(encrypted_message, 1, strlen(encrypted_message), encrypted_file);
    fclose(encrypted_file);

    // send encrypted voice message file
    send_file(receiver_ip, "encrypted_voice_message.bin");
    printf("Voice message sent successfully.\n");

}
int Recive_Voice_Message(char *filename)
{
    // This function will receive the encrypted voice message file
    unsigned char session_key[AES_KEY_LENGTH];
    unsigned char encrypted_message[BUFFER_SIZE];  

    // load the session key
    Get_Session_Key(session_key);

    // receive the encrypted voice message file
    receive_file(filename);
    printf("Voice message received successfully.\n");

    // read the encrypted voice message from the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error opening received voice file\n");
        return -1;  
    }
    size_t bytes_read = fread(encrypted_message, 1, BUFFER_SIZE, file);
    fclose(file);
    if (bytes_read == 0) {
        fprintf(stderr, "Error reading received voice file or file is empty\n");
        return -1;  
    }
    printf("Encrypted voice message: %s\n", encrypted_message);    


    // Decrypt the voice message using AES
    unsigned char decrypted_voice_message[BUFFER_SIZE];
    AES_decrypt(encrypted_message, bytes_read, session_key, decrypted_voice_message);

    // Save the decrypted voice message to a new file
    FILE *decrypted_file = fopen("decrypted_voice_message.bin", "wb");
    if (!decrypted_file) {
        fprintf(stderr, "Error opening file to save decrypted voice message\n");
        return -1;  
    }
    fwrite(decrypted_voice_message, 1, strlen(decrypted_voice_message), decrypted_file);
    fclose(decrypted_file);
    
}