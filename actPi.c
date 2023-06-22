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
#include <pthread.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "gpio.h"
#include"lcd.h"
#include"led.h"
#include"pwm.h"

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
#define BIN 20
#define BOUT 21


#define LINE1  0x80 // lcd첫번째 줄 주소
#define LINE2  0xC0 // lcd두번째 줄 주소 
#define I2C_ADDR   0x27 // I2C device


int fd;  // seen by all subroutines
void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}


//냉장고 문열림이랑 온도비정상 스피커 다르게  온도 비정상은 강하게 삐삐삐
//냉장고 문열림은 중간 삐삐삐
void buzzerOn(int power) {
    PWMWriteDutyCycle(BUZ, power);
}
void buzzerOff() {
    PWMWriteDutyCycle(BUZ, 0);
}
void buzzerPattern1(int i) {   //문열린 경우 스피커 중간세기 삐삐삐
    printf("buzpatter1\n");
    while (i--) {
        buzzerOn(1000*100);
        usleep(500*1000); //0.5초간 
        buzzerOff();
        usleep(500*1000);
    }
}
void buzzerPattern2(int i) {   //온도 높은 경우 스피커 강한세기 삐이삐이삐이삐이삐이 5번
    printf("buzpatter2\n");
    while (i--) {
        buzzerOn(1000 * 200);
        usleep(1000 * 1000); //1초간 
        buzzerOff();
        usleep(1000 * 1000);
    }
}

void* buzthread1() {
    printf("buzthread1\n");
    buzzerPattern1(3);
}
void* buzthread2() {
    printf("buzthread2\n");
    buzzerPattern2(3);
}

void* ledthread(void* data) {
    printf("ledthread\n");
    int* led = (int*)data;
    ledOn(*led);
 
}

int main(int argc, char* argv[]) {
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
    PWMWritePeriod(BUZ, 200000); // lower number, higher sound!!
    PWMWriteDutyCycle(BUZ, 0);
    PWMEnable(0);

    pthread_t pthread[7];
    int thr_id;
    int status;

    if (wiringPiSetup() == -1) exit(1);  //lcd설정 
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(); 

    while (1) {

        printf("Waiting....\n");
        str_len = read(sock, msg, sizeof(msg));
        printf("%s\n", msg);
        usleep(100000);

        if (msg[0] == '0') {
            if (msg[1] == '1') {//문열린 경우 
                int led = LED1;

                thr_id = pthread_create(&pthread[0], NULL, buzthread1, NULL);
                printf("Door is open\n");
                if (thr_id < 0) error_handling("thread create error:");
                thr_id = pthread_create(&pthread[1], NULL, ledthread, (void*)&led);
                printf("thr_id:%d\n", thr_id);
                if (thr_id < 0) error_handling("thread create error:");
                   
            }
            else if (msg[1] == '2') {//Temparature is warn
                int led = LED2;
                printf("Temp is warning\n");

                if (thr_id < 0) error_handling("thread create error:");
                  
                thr_id = pthread_create(&pthread[3], NULL, ledthread, (void*)&led);

                if (thr_id < 0) error_handling("thread create error:");
                  

            }
            else if (msg[1] == '3') {//온도가 비정상적으로 높은 경우 
                int led = LED3;
                printf("Temp is critical\n");
                thr_id = pthread_create(&pthread[4], NULL, buzthread2, NULL);

                if (thr_id < 0) error_handling("thread create error:");
                  
                thr_id = pthread_create(&pthread[5], NULL, ledthread, (void*)&led);

                if (thr_id < 0) if (thr_id < 0) error_handling("thread create error:");

            }
            else if (msg[1] == '4') {
                int led = LED4;
                printf("Date is near\n");
                thr_id = pthread_create(&pthread[6], NULL, ledthread, (void*)&led);

                if (thr_id < 0) error_handling("thread create error:");
                char name[16];
                str_len = read(sock, name, sizeof(name));
                lcdStart(LINE1, "Data is near...");
                lcdStart(LINE2, name);
                delay(5000);
                ClrLcd();
                name[0] = '\0';
            }
        }
        else {//유통기한 출력 부분

       /*pwmWrite(BUZ, 1000000 / 262 / 2);
       delay(3000);*/
        }
    }

}


