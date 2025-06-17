#include "message.h"

int Setup_Secure_Com()
{
    // Generate and Store AES Session Key
    unsigned char temp_key[AES_KEY_LENGTH];
    if (AES_generate_key(temp_key) != 0) {
        fprintf(stderr, "Error generating AES key\n");
        return -1;
    }
    Set_Session_Key(temp_key);
    printf("AES Session Key generated and set successfully.\n");


    // Generate RSA keys
    if (RSA_generate_keys("public_key.pem", "private_key.pem") != 0) {
        fprintf(stderr, "Error generating RSA keys\n");
        return -1;
    }
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

int Communication_inti(int Type)
{
    unsigned char session_key[AES_KEY_LENGTH];
    // Type 0 mean sender
    if(Type == 0){
        // send public key

        // recive public key form receiver

        // get Symitric key
        Get_Session_Key(session_key);
        // encrypt Symitric key using RSA public key( received)
        
        if( RSA_encrypt(session_key, "encrypted_session_key.txt", "public_key.pem") != 0) {
            fprintf(stderr, "Error encrypting session key\n");
            return -1;
        }
        printf("Session key encrypted successfully.\n");
        // send Symitric key after encrypt
        
    }else if(Type == 1 )// Tyep 1 mean reicver
    {
        // recive public key from sender

        // send public key 

        // recive Symitric key after encrypt

        // decrypt Symitric key using RSA private key
        if( RSA_decryt("encrypted_session_key.txt", session_key, "private_key.pem") != 0) {
            fprintf(stderr, "Error decrypting session key\n");
            return -1;
        }
        printf("Session key decrypted successfully.\n");
        // set Symitric key
        Set_Session_Key(session_key);
        printf("Session key set successfully.\n");
    }else{
        printf("unkown type\n");
    }

}

int Send_Text_Message(unsigned char *message)
{
    Text_Message text_message;
    unsigned char session_key[AES_KEY_LENGTH];
    // load the session key
    Get_Session_Key(session_key);
    // Prepare the text message
    // Encrypt data using AES
    if (AES_encrypt(message, strlen((char *)message), session_key, text_message.data) != 0) {
        fprintf(stderr, "Error encrypting text message\n");
        return -1;
    }
    // calculate hash of encypted message
    sha256_string((char *)text_message.data, text_message.hash);
   

    // Send the message (implementation of sending is not shown here)
    // ...

    return 0; // Return success

}
int Recive_Text_Message(Text_Message *message)
{

    unsigned char session_key[AES_KEY_LENGTH];
    // load the session key
    Get_Session_Key(session_key);
    
    // Recive message 
    // (implementation of receiving is not shown here)

    
    // Decrypt data using AES
    unsigned char decrypted_message[1024];
    int decrypted_length = AES_decrypt(message->data, strlen((char *)message->data), session_key, decrypted_message);
    if (decrypted_length < 0) {
        fprintf(stderr, "Error decrypting text message\n");
        return -1;
    }
    
    // Verify the hash of the decrypted message
    unsigned char hash[65];
    sha256_string((char *)decrypted_message, hash);
    if (strcmp((char *)hash, (char *)message->hash) != 0) {
        fprintf(stderr, "Hash mismatch: message integrity compromised\n");
        return -1;
    }

    
    // ...

    return 0; // Return success
}
int Send_Voice_Message(voice_Message *message);
int Recive_Voice_Message(voice_Message *message);