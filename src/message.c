#include "message.h"
#include "encryption.h"
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
    unsigned char encrypted_file[FILENAME_SIZE]="encrypted_voice_message.bin";

    // load the session key
    Get_Session_Key(session_key);

    // encrypt voice message using AES
    AES_Encrypt_file(filename, encrypted_file, session_key);
    printf("Voice message encrypted successfully.\n");
    //send encrypted voice message file
    send_file_ex(receiver_ip, encrypted_file,NULL,0);
    
    printf("Voice message sent successfully.\n");

}
int Recive_Voice_Message(char *filename)
{
    // This function will receive the encrypted voice message file
    unsigned char session_key[AES_KEY_LENGTH];
    unsigned char decrypt_filename[FILENAME_SIZE]= "voice_message.wav";

    // load the session key
    Get_Session_Key(session_key);
    // receive encrypted voice message file
    receive_file_ex(filename, FILENAME_SIZE,NULL,0);
    printf("Voice message received successfully.\n");
    
    // decrypt voice message using AES
    AES_Decrypt_file(filename, decrypt_filename, session_key);
    printf("Voice message decrypted successfully.\n");

    // sign decrypted filename to return filename
    strncpy(filename, decrypt_filename, strlen(decrypt_filename) - 1);
    return 0;
}

int handle_receiving(MessageType message_type,char *filename)
{
    // define var to hold received message type
    
    message_type = (MessageType)(*receive_string();)
    if (received_message_type == MSG_TYPE_TEXT) {
        unsigned char decrypted_message[BUFFER_SIZE];
        Recive_Text_Message(decrypted_message);
        printf("Received text message: %s\n", decrypted_message);
        // show recived message in UI
        pass_recieved_message(MSG_TYPE_TEXT, NULL, (char **)&decrypted_message);
    } else if (received_message_type == MSG_TYPE_VOICE) {
        Recive_Voice_Message((char *)filename);
        printf("Received voice message saved to: %s\n", filename);
        // show recived message in UI
        pass_recieved_message(MSG_TYPE_VOICE, filename, NULL);
    } else {
        fprintf(stderr, "Unknown message type received.\n");
        return -1; // Error handling for unknown message type
    }
    return 0; // Return success
}