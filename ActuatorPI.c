#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "gpio.h"

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1


// GPIO
#define BUZ 0
#define LED1 17
#define LED2 27
#define LED3 22
#define LED4 23
#define BIN 6
#define BOUT 13
#define B2IN 19
#define B2OUT 26
#define BTNPERIOD 200000

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

void signalHandler(){
    GPIOUnexport(LED1);
    GPIOUnexport(LED2);
    GPIOUnexport(LED3);
    GPIOUnexport(LED4);
    GPIOUnexport(BIN);
    GPIOUnexport(BOUT);
    GPIOUnexport(B2IN);
    GPIOUnexport(B2OUT);
    exit(1);
}
void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

static int PWMExport(int pwmnum) {
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    int bytes_written;
    int fd;

    fd = open("/sys/class/pwm/pwmchip0/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in unexport!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, bytes_written);
    close(fd);

    sleep(1);
    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in export!\n");
        return(-1);
    }
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, bytes_written);
    close(fd);
    sleep(1);
    return(0);
}

static int PWMEnable(int pwmnum) {
    static const char s_unenable_str[] = "0";
    static const char s_enable_str[] = "1";
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm%d/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_unenable_str, strlen(s_unenable_str));
    close(fd);

    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);
    return(0);
}

static int PWMWritePeriod(int pwmnum, int value) {
    char s_values_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in period!\n");
        return(-1);
    }
    byte = snprintf(s_values_str, VALUE_MAX, "%d", value);
    if (-1 == write(fd, s_values_str, byte)) {
        fprintf(stderr, "Failed to write value in period!\n");
        close(fd);
        return(-1);
    }
    close(fd);
    return(0);
}
static int PWMWriteDutyCycle(int pwmnum, int value) {
    char path[VALUE_MAX];
    char s_values_str[VALUE_MAX];
    int fd, byte;

    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open in duty_cycle!\n");
        return(-1);
    }

    byte = snprintf(s_values_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_values_str, byte)) {
        fprintf(stderr, "Failed to write value! in duty_cycle\n");
        close(fd);
        return(-1);
    }

    close(fd);
    return(0);
}

//냉장고 문열림이랑 온도비정상이랑 스피커 출력을 같게 하면 됨.
void buzzerOn(){
    PWMWriteDutyCycle(BUZ,100000);
}
void buzzerOff(){
    PWMWriteDutyCycle(BUZ,0);
}
void buzzerPattern1(int i){
    printf("buzpattern\n");
    while(i--){
        buzzerOn();
        usleep(100000);
        buzzerOff();
        usleep(100000);
    }   
}

void buzzerPattern2(int i) {   //온도 높은 경우 스피커 강한세기 삐이삐이삐이삐이삐이 5번
    printf("buzpatter2\n");
    while (i--) {
        buzzerOn();
        usleep(500 * 1000); //1초간 
        buzzerOff();
        usleep(500 * 1000);
    }
}

void ledOn(int led){
    if (-1 == GPIOWrite(led, 1)){
        printf("error on ledon!");
        exit(0);
    }
        
}
void ledOff(int led){
    if (-1 == GPIOWrite(led, 0)){
        printf("error on ledon!");
        exit(0);
    }
}

void *buzthread1(){
    printf("buzthread\n");
    buzzerPattern1(5);    
}

void* buzthread2() {
    printf("buzthread2\n");
    buzzerPattern2(3);
}

void *ledthread(void* data){
    printf("ledthread\n");
    int *led =(int*)data;

    ledOn(*led);
    sleep(5);
    ledOff(*led);
}

void send_datesignal(int *sock){
    char sig[100] = "!";
    write(*sock, sig, sizeof(sig));
}
void send_passsignal(int *sock){
    char sig[100] = "?";
    write(*sock, sig, sizeof(sig));
}

void *btnthread(void* data){
    int *sock=(int*)data;
    do{
        if(-1==GPIOWrite(BOUT,1))
            exit(1);
        if(-1==GPIOWrite(B2OUT,1))
            exit(1);
        if(GPIORead(BIN)==0){
            printf("button clicked\n");
            send_datesignal(sock);
            usleep(1000000);
        }
        if(GPIORead(B2IN)==0){
            printf("button clicked 2\n");
            send_passsignal(sock);
            usleep(1000000);
        }
    }while(1);
}

void (*breakCapture)(int);


int main(int argc, char* argv[]) {

    setsid();
    umask(0);

    breakCapture = signal(SIGINT, signalHandler);

    int sock;
    struct sockaddr_in serv_addr;
    int str_len;
    char msg[100];

    if(argc!=3){
        printf("Usage : %s <IP> <port>\n",argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));  
        
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("connect() error");
    
    PWMExport(BUZ);
    PWMWritePeriod(BUZ, BTNPERIOD); // lower number, higher sound!!
    PWMWriteDutyCycle(BUZ, 0);
    PWMEnable(0);

    if (-1 == GPIOExport(LED1) || -1 == GPIOExport(LED2) || -1 == GPIOExport(LED3) || -1 == GPIOExport(LED4))
        return(1);
    if (-1 == GPIOExport(BIN) || -1 == GPIOExport(BOUT)||-1 == GPIOExport(B2IN) || -1 == GPIOExport(B2OUT))
        return(1);
    usleep(500000);
    if (-1 == GPIODirection(LED1, OUT) || -1 == GPIODirection(LED2, OUT) || -1 == GPIODirection(LED3, OUT) || -1 == GPIODirection(LED4, OUT))
        return(2);
    if (-1 == GPIODirection(BIN, IN) || -1 == GPIODirection(BOUT, OUT)||-1 == GPIODirection(B2IN, IN) || -1 == GPIODirection(B2OUT, OUT))
        return(2);

    if (wiringPiSetup() == -1) exit(1);
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(); // setup LCD

    if (-1 == GPIOWrite(LED1, 0) || -1 == GPIOWrite(LED2, 0) || -1 == GPIOWrite(LED3, 0) || -1 == GPIOWrite(LED4, 0))
        return(3);

    
    pthread_t pthread[7];
    pthread_t btn;
    int thr_id;
    int status;

    thr_id = pthread_create(&btn, NULL, btnthread,(void*)&sock);
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
    
    while (1) {

        printf("Waiting....\n");
        str_len = read(sock, msg, sizeof(msg));
        printf("%s\n", msg);
        
        usleep(100000);

        if(msg[0]=='0'){
            if(msg[1]=='1'){//Door is open
                int led = LED1;

                thr_id = pthread_create(&pthread[0], NULL, buzthread1, NULL);
                printf("Door is open\n");
                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }
                thr_id = pthread_create(&pthread[1], NULL, ledthread, (void*)&led);
                printf("thr_id:%d\n",thr_id);
                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

            }

            else if(msg[1]=='2'){//Temparature is warn
                int led = LED2;
                printf("Temp is warning\n");

                thr_id = pthread_create(&pthread[3], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

            }else if(msg[1]=='3'){//
                int led = LED3;
                printf("Temp is critical\n");
                thr_id = pthread_create(&pthread[4], NULL, buzthread2, NULL);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }
                thr_id = pthread_create(&pthread[5], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }


                
            }else if(msg[1]=='4'){
                int led = LED4;
                printf("Date is near\n");
                thr_id = pthread_create(&pthread[6], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

                char name[16];
                str_len = read(sock, name, sizeof(name));

                lcdLoc(LINE1);
                typeln("Deadline Foods");
                lcdLoc(LINE2);
                typeln(name);
                delay(2000);
                ClrLcd();
                name[0]='\0';
            }else if(msg[1]=='5'){
                int led = LED4;
                printf("Date is passed.\n");
                thr_id = pthread_create(&pthread[6], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

                char name[16];
                str_len = read(sock, name, sizeof(name));

                lcdLoc(LINE1);
                typeln("Expired Foods");
                lcdLoc(LINE2);
                typeln(name);
                delay(2000);
                ClrLcd();
                name[0]='\0';
            }
        }else{//유통기한 출력 부분
            
        /*pwmWrite(BUZ, 1000000 / 262 / 2);
        delay(3000);*/
        }
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
