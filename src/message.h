#ifndef message_h
#define message_h
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include "encryption.h"


typedef struct 
{
    /* data */
    unsigned char data[1024]; // Buffer for the message
    unsigned char hash[AES_KEY_LENGTH+1]; // Hash of the message
}Text_Message;

typedef struct 
{
    /* data */
    unsigned char filename[20]; // Name of the file
    unsigned char filehash[AES_KEY_LENGTH+1]; // Hash of the file
}voice_Message;

// enum to define node(sender, receiver) types
typedef enum {
    SENDER = 0,
    RECEIVER = 1
} NodeType;

int Communication_Setup();
int Communication_inti(NodeType node_type, const char *receiver_ip);
int Send_Text_Message(unsigned char *message, unsigned char *receiver_ip);
int Recive_Text_Message(unsigned char *decrypted_message);
int Send_Voice_Message(unsigned char *filename, unsigned char *receiver_ip);
int Recive_Voice_Message(char *filename);
#endif SECURE_COM_H


