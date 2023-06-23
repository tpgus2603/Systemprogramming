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
#include <time.h>
#include <pthread.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include"lcd.h"


#define I2C_ADDR   0x27 // I2C device address


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


int fd;  // seen by all subroutines

#define I2C_ADDR   0x27 // I2C device address

// Define some device constants

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On
// LCD_BACKLIGHT = 0x00  # Off

char* divideByTen(const char* numberString) {  //온도 값을 소수점 1자리까지의 문자열로 전환하는 코드
    int number = atoi(numberString); 
    double dividedNumber = number / 10.0;

    
    int length = snprintf(NULL, 0, "%.1f", dividedNumber);

    char* result = malloc((length + 1) * sizeof(char)); 
    sprintf(result, "%.1f", dividedNumber); 

    return result;
}



//Error handling
void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
//Open Door Api

void sendsignal_opendoor(int sock) { //문열린 경우 시그널 01을 액츄에이터에 보냄
    char buf[100] = "01";
    write(sock, buf, sizeof(buf));
    printf("door is opened too long time\n");
    return;
}

void sendsignal_uptemp_warn(int sock) {//온도값이 5~7.9도인경우  경우 시그널 02을 액츄에이터에 보냄
    char buf[100] = "02";
    write(sock, buf, sizeof(buf));
    printf("temperature warning\n");
    return;
}

void sendsignal_uptemp_critical(int sock) {//온도값이 5~7.9도인경우  경우 시그널 03을 액츄에이터에 보냄
    char buf[100] = "03";
    write(sock, buf, sizeof(buf));
    printf("critical temperature\n");
    return;
}

void sendsignal_timewarn(int sock) { //유효기간이 1일남은 품목에 관한 시그널 04을 액츄에이터에 보냄
    char buf[100] = "04";
    write(sock, buf, sizeof(buf));
    return;
}
void sendsignal_timepass(int sock) {   //유효기간이 지난 품목에 관할 시그널 05를 액츄에이터에 보냄
    char buf[100] = "05";
    write(sock, buf, sizeof(buf));
    return;
}


int t = 0;
void* timer(void* data) {  //문열림 감지를 위한 타이머 쓰레드 워커
    int* sock = (int*)data;
    while (1) {
        t++;
        if (t == 30) { //시간(압력600미만인 시간) 30초가 경과하면 문열림 시그널 보냄 
            t = 0;
            sendsignal_opendoor(*sock);
        }
        printf("%d\n", t);
        usleep(1000 * 1000);
    }
}

void dateprint(int* sock) {   //data.txt에 저장한 유효기간 텍스트를 읽고 서버파이내 시간과 시간 비교 유효기간 당일 품목 관련 
    FILE* file_pointer;
    file_pointer = fopen("/home/user/date.txt", "r");
    char buffer[32];

    int year, month, day;
    char cyear[10], cmonth[10], cday[10];
    time_t t;
    struct tm* tm;
    char comparedate[10] = { 0, };
    char* doubledot = ".";

    char* ptr1, * ptr2;

    while (1) {

        fgets(buffer, 32, file_pointer);
        if (feof(file_pointer) != 0) {
            fclose(file_pointer);
            return;
        }

        t = time(NULL);
        tm = localtime(&t);
        year = tm->tm_year + 1900;
        month = tm->tm_mon + 1;
        day = tm->tm_mday;
        char datebuffer[10];

        snprintf(datebuffer, sizeof(datebuffer), "%d.%d.%d", year, month, day);

        ptr1 = strtok(buffer, ",");
        ptr2 = strtok(NULL, "\n");
;
        if (strcmp(datebuffer, ptr2) == 0) {//If Date==Today, send signal
            sendsignal_timewarn(*sock);
            usleep(500000);
            write(*sock, ptr1, 16);
            sleep(2);
            datebuffer[0] = '\0';
            ptr1[0] = '\0';
            ptr2[0] = '\0';
        }
    }
}

void passprint(int* sock) {//data.txt에 저장한 유효기간 텍스트를 읽고 서버파이내 시간과 시간 비교 유효기간 지난 품목 관련 
    FILE* file_pointer;
    file_pointer = fopen("/home/user/date.txt", "r");
    char buffer[32];

    int year, month, day;
    int fyear, fmonth, fday;
    char cyear[10], cmonth[10], cday[10];
    time_t t;
    struct tm* tm;
    char comparedate[10] = { 0, };
    char* doubledot = ".";

    char* ptr1, * ptr2;

    while (1) {

        fgets(buffer, 32, file_pointer);
        if (feof(file_pointer) != 0) {
            fclose(file_pointer);
            return;
        }

        t = time(NULL);
        tm = localtime(&t);
        year = tm->tm_year + 1900;
        month = tm->tm_mon + 1;
        day = tm->tm_mday;


      

        ptr1 = strtok(buffer, ",");
        ptr2 = strtok(NULL, "\n"); //유통기한

        //food year, month, date
        fyear = atoi(strtok(ptr2, "."));
        fmonth = atoi(strtok(NULL, "."));
        fday = atoi(strtok(NULL, "."));
        

        if (year > fyear || (fyear == year && month > fmonth) || (fyear == year && fmonth == month && day > fday)) {//If Date<Today, send signal
            //printf("%s's Date is near.\n",ptr1);
            sendsignal_timepass(*sock);
            usleep(500000);
            write(*sock, ptr1, 16);
            sleep(2);

            ptr1[0] = '\0';
            ptr2[0] = '\0';
        }
    }
}


//Date is near Thread in 24hours
void* notenoughDate(void* data) { //품목의 유효기간이 24시간 이내인 경우를 위한 쓰레드 워커

    int* sock = (int*)data;

    time_t currentTime;
    time_t base = 0;
    struct tm* timeinfo, * previnfo;
    previnfo = localtime(&base);
    printf("prev = %d\n", previnfo->tm_year);
    int year, month, day, hours;
    int pyear, pmonth, pday;

    pyear = previnfo->tm_year + 1900;
    pmonth = previnfo->tm_mon + 1;
    pday = previnfo->tm_mday;
    while (1) {
        currentTime = time(NULL);
        timeinfo = localtime(&currentTime);

        year = timeinfo->tm_year + 1900;
        month = timeinfo->tm_mon + 1;
        day = timeinfo->tm_mday;
        hours = timeinfo->tm_hour;

        if (year != pyear || month != pmonth || day != pday) {
            dateprint(sock);
            pyear = year;
            pmonth = month;
            pday = day;
        }
        else if (hours == 18) {
            dateprint(sock);
        }
        sleep(1);
    }
}

void* listenActivateButton(void* data) { //버튼에 관련된 값을 액츄에이터로부터 입력받고 1번과 2번에 따라 다른 시그널을 액츄에이터에 보냄
    int* sock = (int*)data;
    int str_len;
    char msg[6];

    while (1) {
        str_len = read(*sock, msg, sizeof(msg));
        if (msg[0] == '!') { //1번 버튼이 눌린경우

            dateprint(sock);  //유효기간 당일 품목 출력
        }
        if (msg[0] == '?') {  //2번 버튼이 눌린경우
            passprint(sock);  //유효기간 지난 품목 출력
        }
    }


}

int main(int argc, char* argv[]) {
    pthread_t p_thread, threadfordate, threadfordatebutton;
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
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); //서버소켓 생성 

    if (serv_sock == -1)
        error_handling("socket() error");

    //바인드과정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
   
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 2) == -1)
        error_handling("listen() error");
    //센서 클라이언트와 연결
    if (clnt_sock1 < 0) {

        clnt_addr_size = sizeof(clnt_addr1);

        clnt_sock1 = accept(serv_sock, (struct sockaddr*)&clnt_addr1, &clnt_addr_size);

        if (clnt_sock1 == -1)
            error_handling("accept() error");
    }
    //액츄에이터 클라이언트와 연결 
    if (clnt_sock2 < 0) {

        clnt_addr_size = sizeof(clnt_addr2);

        clnt_sock2 = accept(serv_sock, (struct sockaddr*)&clnt_addr2, &clnt_addr_size);

        if (clnt_sock2 == -1)
            error_handling("accept() error");
    }

    if (wiringPiSetup() == -1) exit(1);
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(fd); // setup LCD



    thr_id = pthread_create(&p_thread, NULL, timer, (void*)&clnt_sock2); //타이머 쓰레드 생성
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
    thr_id = pthread_create(&threadfordate, NULL, notenoughDate, (void*)&clnt_sock2);   //유효기간 관련 데이터 읽는 쓰레드 생성
    if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
    }
    thr_id = pthread_create(&threadfordatebutton, NULL, listenActivateButton, (void*)&clnt_sock2); //버튼에 관한 데이터 읽는 쓰레드 생성
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
        int temp, pres;
        strtok(msg, "=");
        char* temprec;
        if (msg[0] == 't') { //액츄에이터로 온도 플래그가 붙은 문자열을 수신한경우
            temprec = strtok(NULL, "=");
            char* tempc = divideByTen(temprec);

            lcdStart(LINE1, fd, "Temperature"); //lcd화면에 표시
            lcdStart(LINE1, fd, tempc);
           
            temp = atoi(temprec);
            printf("temp= %d\n", temp);
            if (temp >= 50 && temp < 80) //온도가 5도에서 8도 미만인경우 
                sleep(1);
            sendsignal_uptemp_warn(clnt_sock2);
            if (temp >=80) { //온도가 8도 이상인 경우
                sleep(1);
                sendsignal_uptemp_critical(clnt_sock2);
            }
        }
        else if (msg[0] == 'p') {//액츄에이터로 압력 플래그가 붙은 문자열을 수신한경우
            temprec = strtok(NULL, "=");
            pres = atoi(temprec);
            printf("pres= %d\n", pres);
            if (pres) {  //압력이 넘어오면 문열림 타이머 리셋(문열림으로 판단)
                t = 0;
            }
        }
        for (int i = 0; i < 6; i++) {
            msg[i] = '\0';
        }
    }

}


