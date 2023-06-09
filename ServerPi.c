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
void sendsignal_opendoor(int sock) {  //문이 열린경우 액츄에이터에 보냄 중간 경고음과 led등 
    char buf[1] = "1";//opendoor api
    //write(sock, buf, sizeof(buf));
    printf("door is opened too long time\n");
    return;
}
void sendsignal_uptemp_warn(int sock) { //온도가 5~8도 사이로 올라간경우 등색 led
    char buf[1] = "2";//warn api
    //write(sock, buf, sizeof(buf));
    printf("temperature warning\n");
    return;
}
void sendsignal_uptemp_critical(int sock) {  //온도가 8도이상 올라간 경우 적색 led+ 높은 경고음
    char buf[1] = "3";//critical api
    //write(sock, buf, sizeof(buf));
    printf("critical temperature\n");
    return;
}
void sendsignal_timeout(int sock) {  //유효기간 ??
    char buf[1] = "4";//opendoor api
    write(sock, buf, sizeof(buf));
    return;
}

void sendsignal_timewarn(int sock) { //
    char buf[1] = "5";//opendoor api
    write(sock, buf, sizeof(buf));
    return;
}

int t = 0;
void* timer(void* data) {
    int* sock = (int*)data;
    while (1) {
        t++;
        if (t == 30) {
            t = 0;
            sendsignal_opendoor(*sock);
        }
        printf("%d\n", t);
        usleep(1000*1000);
    }
}
int main(int argc, char* argv[]) {

    pthread_t p_thread;
    int thr_id;
    int status;


    int str_len; //read_len
    //socket 1
    int serv_sock, clnt_sock1 = -1;
    struct sockaddr_in serv_addr, clnt_addr1;
    //socket 2
    int clnt_sock2 = -1;
    struct sockaddr_in clnt_addr2;

    socklen_t clnt_addr_size;
    //read buffer
    char msg[6];

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

    thr_id = pthread_create(&p_thread, NULL, timer, (void*)&clnt_sock2);
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
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
            //lcd패널에 온도실시간  보내기 구현 해야함 
            temp = atoi(temprec);
            printf("temp= %d\n", temp);
            if(temp>=50&&temp<230)
                sendsignal_uptemp_warn(clnt_sock2);
            if(temp>250)
                sendsignal_uptemp_critical(clnt_sock2);
            // led panel show
            // if temp > 5c give signal to actuator pi
        }
        else if (msg[0] == 'p') {
            temprec = strtok(NULL, "=");

            pres = atoi(temprec);
            printf("pres= %d\n", pres);
            if (pres > 600) {
                t = 0;
            }
            // timer reset
        }
        //write(clnt_sock2,msg,sizeof(msg));
    }
    pthread_join(p_thread, (void**)&status);
}

