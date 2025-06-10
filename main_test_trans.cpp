#include "tranceiver.h"

int main() {
    std::cout << "Smart File Transfer\n";
    std::cout << "1. Wait to receive a file\n";
    std::cout << "2. Send a file\n";
    std::cout << "Choice: ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 1) {
        receiveFile();
    } else if (choice == 2) {
        sendFile();
    } else {
        std::cout << "Invalid choice!" << std::endl;
    }
    
    return 0;
}