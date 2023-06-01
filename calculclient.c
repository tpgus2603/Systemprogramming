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

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 40

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

void main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;

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
    
    FILE *file_pointer;
    file_pointer = fopen("/home/pi/date.txt","r");
    char buffer[32];
    char response[3];
    
    while(1){
        char *buff = fgets(buffer,32,file_pointer);
        if(feof(file_pointer) != 0){
            break;
        }
        printf("%s\n",buffer);
        write(sock,buffer,sizeof(buffer));
        usleep(1000000);
    }
    fclose(file_pointer);








}
