#ifndef message_h
#define message_h
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include "ui.h"


// enum to define node(sender, receiver) types
typedef enum {
    SENDER = 0,
    RECEIVER = 1
} NodeType;



int Communication_Setup();
int Communication_inti(NodeType node_type, const char *receiver_ip);
int Send_Text_Message(const char *message, unsigned char *receiver_ip);
int Recive_Text_Message(unsigned char *decrypted_message);
int Send_Voice_Message(unsigned char *filename, unsigned char *receiver_ip);
int Recive_Voice_Message(char *filename);
//int handle_sending(MessageType message_type, const char *receiver_ip, const char *message);
int handle_receiving(char  message_type, char *filename);
#endif //SECURE_COM_H


