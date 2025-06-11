#include <iostream>
#include <RF24/RF24.h>
#include "rf24_init.h"
#include "rf24_send.h"

#define PIN_CE 17
#define PIN_CSN 0

using namespace std;

int main() {
    RF24 radio(PIN_CE, PIN_CSN);
    initRadio(radio);

    string msg = "Hi Ahmed Elshazly!";
    while (true) {
        sendData(radio, msg);
        delay(1000);
    }

    return 0;
}