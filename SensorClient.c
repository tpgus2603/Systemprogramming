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

#define PIN1    17
static const char *DEVICE = "/dev/spidev0.0";


void error_handling(char* message) { //에러처리 코드 
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* pressure_worker(void* sock){
     int socket_fd=(int)sock;
    int p_value;
    int d_fd = open(DEVICE, O_RDWR);
    while(1){
    char buf[5];
    p_value=pressure_sensor(d_fd);
    if(p_value>600){
    printf("%d\n",p_value);
    sprintf(buf,"p=%d",p_value);
    write(socket_fd, &buf, sizeof(strlen(buf)));
    usleep(1000*1000);
    }
    }
    close(d_fd);
    // while(1){
    //   int p_value;
    //   char buf[20];
    //   p_value=600;
    //   sprintf(buf,"p=%d",p_value);
    //   write(socket_fd, buf, strlen(buf));
    //  usleep(1000*1000);

    // }

    
}

void *tem_worker(void *sock){
    int socket_fd=(int)sock;
    while(1){
    int t_value;
    char buf[5];
    t_value=tempStart(PIN1);
    sprintf(buf,"t=%d",t_value);
    printf("%d \n",t_value);
    write(socket_fd, buf, strlen(buf));

    usleep(5000*1000);
    }


}

int main(int argc, char* argv[]) {

	pthread_t temp_thd,pressure_thd;
	int status;

	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
 
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);
 
	if (sock == -1)
		error_handling("socket() error");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
  printf("HI");
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	//--------------------------------------------------------------------------


  int thr_id=0;
	thr_id =pthread_create(&temp_thd, NULL, tem_worker, (void*)sock);
  if(thr_id<0)
    error_handling("thread() error");
	thr_id=pthread_create(&pressure_thd, NULL, pressure_worker, (void*)sock);
  if(thr_id<0)
   error_handling("thread() error");

  
  pthread_join( pressure_thd, NULL );


	return 0;
}
