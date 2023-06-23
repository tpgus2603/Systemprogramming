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
#include"pwm.h"
#include"lcd.h"
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
#define BTNPERIOD 400000

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

int fd;  // seen by all subroutines

void signalHandler() {
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



//냉장고 문열림이랑 온도비정상이랑 스피커 출력을 같게 하면 됨.
void buzzerOn() {
    PWMWriteDutyCycle(BUZ, 150000);
}
void buzzerOff() {
    PWMWriteDutyCycle(BUZ, 0);
}
void buzzerPattern1(int i) { //냉장고 문열림 부저  1초간 5번 
    printf("buzpattern\n");
    while (i--) {
        buzzerOn();
        usleep(500*1000); //0.5초
        buzzerOff();
        usleep(500*1000);
    }
}

void buzzerPattern2(int i) {   //온도 높은 경우 부저 삐이삐이삐이삐이삐이 10번
    printf("buzpatter2\n");
    while (i--) {
        buzzerOn();
        usleep(250 * 1000); //0.25초 
        buzzerOff();
        usleep(250 * 1000);
    }
}

void ledOn(int led) {
    if (-1 == GPIOWrite(led, 1)) {
        printf("error on ledon!");
        exit(0);
    }

}
void ledOff(int led) {
    if (-1 == GPIOWrite(led, 0)) {
        printf("error on ledon!");
        exit(0);
    }
}

void* buzthread1() {  // 문열림 부저 워커쓰레드
    printf("buzthread\n");
    buzzerPattern1(3);
}

void* buzthread2() {  //온도 8도이상 부저 쓰레드
    printf("buzthread2\n");
    buzzerPattern2(10);
}

void* ledthread(void* data) {
    printf("ledthread\n");
    int* led = (int*)data;

    ledOn(*led);
    sleep(5);
    ledOff(*led);
}

void send_datesignal(int* sock) {
    char sig[100] = "!";
    write(*sock, sig, sizeof(sig));
}
void send_passsignal(int* sock) {
    char sig[100] = "?";
    write(*sock, sig, sizeof(sig));
}

void* btnthread(void* data) {  //버튼 쓰레드 워커
    int* sock = (int*)data;
    do {
        if (-1 == GPIOWrite(BOUT, 1))
            exit(1);
        if (-1 == GPIOWrite(B2OUT, 1))
            exit(1);
        if (GPIORead(BIN) == 0) {  //1번버튼
            printf("button clicked\n");
            send_datesignal(sock);
            usleep(1000000);
        }
        if (GPIORead(B2IN) == 0) {  //2번 버튼
            printf("button clicked 2\n");
            send_passsignal(sock);
            usleep(1000000);
        }
    } while (1);
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

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    PWMExport(BUZ);
    PWMWritePeriod(BUZ, BTNPERIOD); 
    PWMWriteDutyCycle(BUZ, 0);
    PWMEnable(0);

    if (-1 == GPIOExport(LED1) || -1 == GPIOExport(LED2) || -1 == GPIOExport(LED3) || -1 == GPIOExport(LED4))
        return(1);
    if (-1 == GPIOExport(BIN) || -1 == GPIOExport(BOUT) || -1 == GPIOExport(B2IN) || -1 == GPIOExport(B2OUT))
        return(1);
    usleep(500000);
    if (-1 == GPIODirection(LED1, OUT) || -1 == GPIODirection(LED2, OUT) || -1 == GPIODirection(LED3, OUT) || -1 == GPIODirection(LED4, OUT))
        return(2);
    if (-1 == GPIODirection(BIN, IN) || -1 == GPIODirection(BOUT, OUT) || -1 == GPIODirection(B2IN, IN) || -1 == GPIODirection(B2OUT, OUT))
        return(2);

    if (wiringPiSetup() == -1) exit(1);
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(fd); // lcd초기화

    if (-1 == GPIOWrite(LED1, 0) || -1 == GPIOWrite(LED2, 0) || -1 == GPIOWrite(LED3, 0) || -1 == GPIOWrite(LED4, 0))
        return(3);


    pthread_t pthread[7];
    pthread_t btn;
    int thr_id;
    int status;

    thr_id = pthread_create(&btn, NULL, btnthread, (void*)&sock);
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }

    while (1) {

        printf("Waiting....\n");
        str_len = read(sock, msg, sizeof(msg));
        printf("%s\n", msg);

        usleep(100000);

        if (msg[0] == '0') {
            if (msg[1] == '1') {// 서버에서 보낸 시그널이 문열린경우의 시그널인 경우
                int led = LED1;

                thr_id = pthread_create(&pthread[0], NULL, buzthread1, NULL);
                printf("Door is open\n");
                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }
                thr_id = pthread_create(&pthread[1], NULL, ledthread, (void*)&led);
                printf("thr_id:%d\n", thr_id);
                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

            }

            else if (msg[1] == '2') {//// 서버에서 보낸 시그널이 온도 상승인 경우의 시그널인 경우
                int led = LED2;
                printf("Temp is warning\n");

                thr_id = pthread_create(&pthread[3], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

            }
            else if (msg[1] == '3') {//// 서버에서 보낸 시그널이 온도 급상승 경우의 시그널인 경우
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



            }
            else if (msg[1] == '4') {// 서버에서 보낸 시그널이 유효기간이 1일 남은 품목 출력 시그널인경우
                int led = LED4;
                printf("Date is near\n");
                thr_id = pthread_create(&pthread[6], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

                char name[16];
                str_len = read(sock, name, sizeof(name));
                printf("Food name = %s\n", name);
                lcdStart(LINE1, fd,"Deadline Foods"); //lcd에 표시 
                lcdStart(LINE2, fd, name);  
                delay(2000);
                ClrLcd(fd);
                name[0] = '\0';
            }
            else if (msg[1] == '5') {// 서버에서 보낸 시그널이 유효기간이 지난 남은 품목 출력 시그널인경우
                int led = LED4;
                printf("Date is passed.\n");
                thr_id = pthread_create(&pthread[6], NULL, ledthread, (void*)&led);

                if (thr_id < 0) {
                    perror("thread create error : ");
                    exit(0);
                }

                char name[16];
                str_len = read(sock, name, sizeof(name));

                lcdStart(LINE1, fd, "Expired Foods"); //lcd에 표시
                lcdStart(LINE2, fd, name);
                lcdLoc(LINE1,fd);
     
                delay(2000);
                ClrLcd(fd);
                name[0] = '\0';
            }
        }
       
        }
    }


