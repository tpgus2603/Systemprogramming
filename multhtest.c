
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>

//Error handling
void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}


int main(int argc, char* argv[]) {



    int str_len; //read_len
    //socket 1
    int serv_sock, clnt_sock1 = -1;
    struct sockaddr_in serv_addr, clnt_addr1;
    //socket 2
    int clnt_sock2 = -1;
    struct sockaddr_in clnt_addr2;

    socklen_t clnt_addr_size;
    //read buffer
    char msg[100];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
    }
    //socket connect
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    //server socket binding
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    //client connecting (sensor)
    if (clnt_sock1 < 0) {

        clnt_addr_size = sizeof(clnt_addr1);

        clnt_sock1 = accept(serv_sock, (struct sockaddr*)&clnt_addr1, &clnt_addr_size);

        if (clnt_sock1 == -1)
            error_handling("accept() error");
    }
    //client conection (actuator)
    /*if (clnt_sock2 < 0) {

        clnt_addr_size = sizeof(clnt_addr2);

        clnt_sock2 = accept(serv_sock, (struct sockaddr*)&clnt_addr2, &clnt_addr_size);

        if (clnt_sock2 == -1)
            error_handling("accept() error");
    }*/

    //main logic
    while (1)
    {
        //get info from client(sensor)
        str_len = read(clnt_sock1, msg, sizeof(msg));
        printf("%s\n", msg);
        //inaccurate info
        if (str_len == -1)
            error_handling("read() error");
        // temp t=12
        // pressure p=500
        char* temprec;//parsing 
        int temp, pres;//parsing - integer
        strtok(msg, "=");
        if (msg[0] == 't') {
            temprec = strtok(NULL, "=");
            temp = atoi(temprec);
            // led panel show
            // if temp > 5c give signal to actuator pi
        }
        else if (msg[0] == 'p') {
            temprec = strtok(NULL, "=");
            pres = atoi(temprec);
            // timer reset
        }
        printf("%s\n", temprec);
        //write(clnt_sock2,msg,sizeof(msg));
        usleep(1000000);

    }
}

