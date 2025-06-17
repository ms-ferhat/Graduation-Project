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


int Communication_inti(int Type);
int Send_Text_Message(unsigned char *message);
int Recive_Text_Message(Text_Message *message);
int Send_Voice_Message(voice_Message *message);
int Recive_Voice_Message(voice_Message *message);
#endif SECURE_COM_H


