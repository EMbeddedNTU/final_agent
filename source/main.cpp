#include "mbed.h"

int main()
{
    DigitalOut led(LED1);

    while (true) {
        // check recive signal, put into event queue
        // make an event despatch system

        led = true;
        ThisThread::sleep_for(500ms);
    }
}
