#include "tranceiver.h"

int main() {
    printf("Smart File Transfer\n");
    printf("1. Wait to receive a file\n");
    printf("2. Send a file\n");
    printf("Choice: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice\n");
        return 1;
    }
    
    if (choice == 1) {
        receiveFile();
    } else if (choice == 2) {
        sendFile();
    } else {
        printf("Invalid choice!\n");
    }
    
    return 0;
}