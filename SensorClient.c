#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include "preSensor.h"  
#include "temsor.h"


#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 256

#define PIN1    17  //온습도센서 17번 핀번호 사용
static const char* DEVICE = "/dev/spidev0.0"; //디바이스 수조


void error_handling(char* message) { //에러처리 코드 
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void* pressure_worker(void* sock) { //압력센서 처리 쓰레드 워커
    int socket_fd = (int)sock;
    int p_value;
    int d_fd = open(DEVICE, O_RDWR);
    while (1) {
        char buf[5];
        p_value = pressure_sensor(d_fd); //압력값 읽기 
        if (p_value > 600) {   //600이상인(문닫힌) 경우  플래그가 포함된 압력값 문자열 전달  
            printf("%d\n", p_value); 
            sprintf(buf, "p=%d", p_value);
            write(socket_fd, &buf, sizeof(strlen(buf))); //서버에 전달
            usleep(1000 * 1000);
        }
    }
    close(d_fd);
 
}

void* tem_worker(void* sock) {  //온도센서 처리 쓰레드 워커
    int socket_fd = (int)sock;
    while (1) {
        int t_value;
        char buf[5];
        t_value = tempStart(PIN1);  //온도값 읽기
        sprintf(buf, "t=%d", t_value);   //플래그가 포함된 온도 문자열 생성 
        printf("%d \n", t_value);
        write(socket_fd, buf, strlen(buf));  //서버에 전달

        usleep(5000 * 1000);
    }


}

int main(int argc, char* argv[]) {

    pthread_t temp_thd, pressure_thd;
    int status;

    int sock;
    struct sockaddr_in serv_addr;
    int str_len;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock = socket(AF_INET, SOCK_STREAM, 0);  //서버와 연결할 소켓 생성 

    if (sock == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    printf("HI");
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)  //연결 
        error_handling("connect() error");
    

    int thr_id = 0;
    thr_id = pthread_create(&temp_thd, NULL, tem_worker, (void*)sock);   //온도 처리 쓰레드 생성 
    if (thr_id < 0)
        error_handling("thread() error");
    thr_id = pthread_create(&pressure_thd, NULL, pressure_worker, (void*)sock);  //압력 처리 쓰레드 생성 
    if (thr_id < 0)
        error_handling("thread() error");


    pthread_join(pressure_thd, NULL);


    return 0;
}

