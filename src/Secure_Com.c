#include "Secure_Com.h"


/* Start_Com function 
    1.check if trans or reciver
    2. send and recive public key 
    3. generat symicteic key and encrypt it then send it for trans
    4. send read singal after recive tsymitic key
*/

int Start_Com(int Type)
{
    unsigned char Symtric_key[AES_KEY_LENGTH];
    unsigned char Secure_key[BUFFER_SIZE];
    // Type 0 mean sender
    if(Type == 0){
        // send public key

        // recive public key form rive

        // genrete symitric key
        AES_generate_key(Symtric_key);
        // encrypt Symitric key using RSA
        RSA_encrypt(Symtric_key, Secure_key, "public_key.pem");
        // send Symitric key after encrypt
        
    }else if(Type == 1 )// Tyep 1 mean reicver
    {
        // recive public key from sender

        // send public key 
    }else{
        printf("unkown type\n");
    }

}