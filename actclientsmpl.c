#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>

#include <wiringPiI2C.h>
#include <wiringPi.h>

#include "gpio.h"

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1

// GPIO
#define BUZ 18
#define LED1 17
#define LED2 27
#define LED3 22
#define LED4 23

// Define some device parameters
#define I2C_ADDR   0x27 // I2C device address

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE  0b00000100 // Enable bit

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char* s);
void typeChar(char val);
int fd;  // seen by all subroutines


void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[]) {
    /*int sock;
    struct sockaddr_in serv_addr;
    int str_len;
    char msg[100];*/

    if (-1 == GPIOExport(LED1) || -1 == GPIOExport(LED2) || -1 == GPIOExport(LED3) || -1 == GPIOExport(LED4))
        return(1);
    if (-1 == GPIODirection(LED1, OUT) || -1 == GPIODirection(LED2, OUT) || -1 == GPIODirection(LED3, OUT) || -1 == GPIODirection(LED4, OUT))
        return(2);

    if (wiringPiSetup() == -1) exit(1);

    fd = wiringPiI2CSetup(I2C_ADDR);

    printf("fd = %d \n", fd);

    lcd_init(); // setup LCD

    /*wiringPiSetupGpio();
    pinMode(BUZ, PWM_OUTPUT);

    pwmSetClock(19);
    pwmSetMode(PWM_MODE_MS);

    pwmSetRange(1000000 / 262);*/

    if (-1 == GPIOWrite(LED1, 1) || -1 == GPIOWrite(LED2, 1) || -1 == GPIOWrite(LED3, 1) || -1 == GPIOWrite(LED4, 1))
        return(3);
    char array1[] = "Hello world!";
    char array2[] = "Smart Class!";


    while (1) {
        printf("hihi\n", fd);
        lcdLoc(LINE1);
        typeln("Using wiringPi");
        lcdLoc(LINE2);
        typeln("kjpark editor.");


        /*pwmWrite(BUZ, 1000000 / 262 / 2);
        delay(3000);*/
    }


    //if (argc != 3) {
    //    printf("Usage : %s <IP> <port>\n", argv[0]);
    //    exit(1);
    //}

    //sock = socket(PF_INET, SOCK_STREAM, 0);

    //if (sock == -1)
    //    error_handling("socket() error");

    //memset(&serv_addr, 0, sizeof(serv_addr));
    //serv_addr.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //serv_addr.sin_port = htons(atoi(argv[2]));

    //if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    //    error_handling("connect() error");
    ////read write
    //while (1) {
    //    str_len = read(sock, msg, sizeof(msg));
    //    printf("%s\n", msg);
    //    usleep(100000);
    //}
}

// float to string
void typeFloat(float myFloat) {
    char buffer[20];
    sprintf(buffer, "%4.2f", myFloat);
    typeln(buffer);
}

// int to string
void typeInt(int i) {
    char array1[20];
    sprintf(array1, "%d", i);
    typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void) {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line) {
    lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val) {

    lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char* s) {

    while (*s) lcd_byte(*(s++), LCD_CHR);

}

void lcd_byte(int bits, int mode) {

    //Send byte to data pins
    // bits = the data
    // mode = 1 for data, 0 for command
    int bits_high;
    int bits_low;
    // uses the two half byte writes to LCD
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    // High bits
    wiringPiI2CReadReg8(fd, bits_high);
    lcd_toggle_enable(bits_high);

    // Low bits
    wiringPiI2CReadReg8(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits) {
    // Toggle enable pin on LCD display
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}


void lcd_init() {
    // Initialise display
    lcd_byte(0x33, LCD_CMD); // Initialise
    lcd_byte(0x32, LCD_CMD); // Initialise
    lcd_byte(0x06, LCD_CMD); // Cursor move direction
    lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
    lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
    lcd_byte(0x01, LCD_CMD); // Clear display
    delayMicroseconds(500);
}
