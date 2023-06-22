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
//Open Door Api
//Middle Buzzer Sound + ??? LED
void sendsignal_opendoor(int sock) {  
    char buf[100] = "01";
    write(sock, buf, sizeof(buf));
    printf("door is opened too long time\n");
    return;
}
//Warn Temperature Api
//Yellow LED 5~7C
void sendsignal_uptemp_warn(int sock) {
    char buf[100] = "02";
    write(sock, buf, sizeof(buf));
    printf("temperature warning\n");
    return;
}
//Critical Temperature Api
//High Buzzer Sound + Red LED 8C
void sendsignal_uptemp_critical(int sock) {
    char buf[100] = "03";
    write(sock, buf, sizeof(buf));
    printf("critical temperature\n");
    return;
}
//Date is near Api
//Green LED
void sendsignal_timewarn(int sock) { 
    char buf[100] = "04";
    write(sock, buf, sizeof(buf));
    return;
}

//Timer Thread for Open Door Api
int t = 0;
void* timer(void* data) {
    int* sock = (int*)data;
    while (1) {
        t++;
        if (t == 30) { //If Door is open during 30 sec, send signal
            t = 0;
            sendsignal_opendoor(*sock);
        }
        printf("%d\n", t);
        usleep(1000*1000);
    }
}

//Date is near Thread
void* notenoughDate(void* data) {
    int* sock = (int*)data;
    FILE* file_pointer;
    file_pointer = fopen("/home/user/date.txt", "r");
    char buffer[32];

    char* ptr1, *ptr2;

    int year, month, day;
    char cyear[10], cmonth[10], cday[10];
    time_t t;
    struct tm* tm;
    char comparedate[10] = {0,};
    char* doubledot = ".";

    while (1) {

        fgets(buffer, 32, file_pointer);
        if (feof(file_pointer) != 0) {
            fclose(file_pointer);
            sleep(60 * 60 * 24);
            file_pointer = fopen("/home/pi/date.txt", "r");
            continue;
        }

        t = time(NULL);
        tm = localtime(&t);
        year = tm->tm_year + 1900;
        month = tm->tm_mon + 1;
        day = tm->tm_mday;
        char datebuffer[10];

        snprintf(datebuffer,sizeof(datebuffer),"%d.%d.%d",year,month,day);

        //printf("Time : %s\n",datebuffer);
        
        ptr1 = strtok(buffer, ",");
        ptr2 = strtok(NULL, "\n");

        //printf("Name :%s, Date :%s\n",ptr1,ptr2);
        //printf("%s, %s\n",datebuffer,ptr2);
        //printf("%d, %d\n",sizeof(datebuffer),sizeof(ptr2));

        if (strcmp(datebuffer,ptr2)==0) {//If Date==Today, send signal
            //printf("%s's Date is near.\n",ptr1);
            sendsignal_timewarn(*sock);
            usleep(500000);
            write(*sock, ptr1, 16);
            sleep(5);
            datebuffer[0] = '\0';
            ptr1[0] = '\0';
            ptr2[0] = '\0';
        }
    }
}

int main(int argc, char* argv[]) {
    pthread_t p_thread, threadfordate;
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
    if (listen(serv_sock, 2) == -1)
        error_handling("listen() error");
    //client connecting (sensor)
    if (clnt_sock1 < 0) {

        clnt_addr_size = sizeof(clnt_addr1);

        clnt_sock1 = accept(serv_sock, (struct sockaddr*)&clnt_addr1, &clnt_addr_size);

        if (clnt_sock1 == -1)
            error_handling("accept() error");
    }
    //client conection (actuator)
    if (clnt_sock2 < 0) {

       clnt_addr_size = sizeof(clnt_addr2);

        clnt_sock2 = accept(serv_sock, (struct sockaddr*)&clnt_addr2, &clnt_addr_size);

       if (clnt_sock2 == -1)
           error_handling("accept() error");
    }

    thr_id = pthread_create(&p_thread, NULL, timer, (void*)&clnt_sock2);
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
    thr_id = pthread_create(&threadfordate, NULL, notenoughDate, (void*)&clnt_sock2);
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
    //main logic - Reading Temperature, Pressure from Sensor
    while (1)
    {

        str_len = read(clnt_sock1, msg, sizeof(msg));
        printf("%s\n", msg);

        if (str_len == -1)
            error_handling("read() error");

        char* temprec;
        int temp, pres;
        strtok(msg, "=");
        if (msg[0] == 't') {
            temprec = strtok(NULL, "=");

            temp = atoi(temprec);
            printf("temp= %d\n", temp);
            if(temp>=300&&temp<320)
                sendsignal_uptemp_warn(clnt_sock2);
            if(temp>320)
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
    //pthread_join(p_thread, (void**)&status);
    //pthread_join(threadfordate, (void**)&status);
}

