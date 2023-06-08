#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include"led.h"
#include"gpio.h"
#include"buzzer.h"
#include <pthread.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 40

#define PIN1    11
#define PIN2    13
#define PIN3    15
#define PIN4    17
#define PIN5    19
#define BUZ     0

void* led_work(void* pin){
    int pin1=(int)pin;
    ledOn(pin);
    pthread_exit(NULL);

}
void *buzzer_work(void*bnum){
    int b=(int)bnum;
    if(b==0)
        buzzerLow(BUZ);
    if(b==1)
        buzzerHigh(BUZ);
    pthread_exit(NULL);

}
void *lcd_work(){

}



void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc, char *argv[]){
    int sock;
    pthread_t thd1,thd2,thd3,thd4;
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
    //read write
    while (1) {
        str_len = read(sock, msg, 1);
        printf("%s\n", msg);
        if(strcmp(msg,"1")){
            int pnum=PIN1;
            int bnum=0;     //문열림 적색 led BUZZERLOW 
            pthread_create(&thd1,NULL,led_work,(void*)PIN1);
            pthread_create(&thd1,NULL,buzzer_work,(void*)bnum);
            
        }
        if(strcmp(msg,"2"))  //온도가 5~8도 등색 led
        {   
            int pnum=PIN2;
            pthread_create(&thd2,NULL,led_work,(void*)pnum);
        }
        
        if(strcmp(msg,"3")) //온도가 8도이상 적색 led+buzzerHIgh
        {
            int pnum=PIN3;
            int bnum=1; 
            pthread_create(&thd3,NULL,led_work,(void*)pnum);
            pthread_creadt(&thd4,NULL,buzzer_work,(void*)bnum);
        }
        
        if(strcmp(msg,"4"))   //유효기간 관련
        
        if(strcmp(msg,"5"))



        usleep(100000);
    }
}
