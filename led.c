#include"gpio.h"
#include"led.h"

int ledOn(int pin) {
    if (GPIOExport(pin) == -1)
        return(-1);
    usleep(1000 * 10);
    if (GPIODirection(pin, 1) == -1)
        return(-1);
    if (GPIOWrite(pin, 1) == -1)
        return(-1);
    usleep(5000 * 1000);
    if (GPIOWrite(pin, 0) == -1)
        return(-1);
    if (GPIOUnexpot(pin) == -1)
        return(-1);
}
