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

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char* s);
void typeChar(char val);
int fd;  // seen by all subroutines

#define I2C_ADDR   0x27 // I2C device address

// Define some device constants

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE  0b00000100 // Enable bit


char* divideByTen(const char* numberString) {
    int number = atoi(numberString);  // Convert the input string to an integer
    double dividedNumber = number / 10.0;

    // Calculate the length of the divided number string
    int length = snprintf(NULL, 0, "%.1f", dividedNumber);

    char* result = malloc((length + 1) * sizeof(char));  // Allocate memory for the result string
    sprintf(result, "%.1f", dividedNumber);  // Convert the divided number to a string with one decimal place

    return result;
}



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
void sendsignal_timepass(int sock) {
    char buf[100] = "05";
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
        usleep(1000 * 1000);
    }
}

void dateprint(int *sock) {
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

        //printf("Time : %s\n",datebuffer);

        ptr1 = strtok(buffer, ",");
        ptr2 = strtok(NULL, "\n");

        //printf("Name :%s, Date :%s\n",ptr1,ptr2);
        //printf("%s, %s\n",datebuffer,ptr2);
        //printf("%d, %d\n",sizeof(datebuffer),sizeof(ptr2));

        if (strcmp(datebuffer, ptr2) == 0) {//If Date==Today, send signal
            //printf("%s's Date is near.\n",ptr1);
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

void passprint(int *sock) {
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


        //printf("Time : %s\n",datebuffer);

        ptr1 = strtok(buffer, ",");
        ptr2 = strtok(NULL, "\n"); //유통기한
        
        //food year, month, date
        fyear = atoi(strtok(ptr2,"."));
        fmonth = atoi(strtok(NULL,"."));
        fday = atoi(strtok(NULL,"."));
        //printf("Name :%s, Date :%s\n",ptr1,ptr2);
        //printf("%s, %s\n",datebuffer,ptr2);
        //printf("%d, %d\n",sizeof(datebuffer),sizeof(ptr2));

        if (year>fyear || (fyear==year && month > fmonth) || (fyear==year && fmonth==month && day>fday)) {//If Date<Today, send signal
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
void* notenoughDate(void* data) {

    int* sock = (int*)data;

    time_t currentTime;
    time_t base=0;
    struct tm* timeinfo, *previnfo;
    previnfo = localtime(&base);
    printf("prev = %d\n",previnfo->tm_year);
    int year, month, day, hours;
    int pyear, pmonth ,pday;

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

        if(year!=pyear || month!=pmonth || day!=pday){
            dateprint(sock);
            pyear=year;
            pmonth=month;
            pday=day;
        }else if(hours==18){
            dateprint(sock);
        }


        //dateprint(sock);
        sleep(1);
    }
}

void* listenActivateButton(void* data) {
    int* sock = (int*)data;
    int str_len;
    char msg[6];

    while (1) {
        str_len = read(*sock, msg, sizeof(msg));
        if (msg[0] == '!') {

            dateprint(sock);
        }
        if(msg[0]=='?'){
            passprint(sock);
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

    if (wiringPiSetup() == -1) exit(1);
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(); // setup LCD



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
    thr_id = pthread_create(&threadfordatebutton, NULL, listenActivateButton, (void*)&clnt_sock2);
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
        if (msg[0] == 't') {
            temprec = strtok(NULL, "=");
            char* tempc = divideByTen(temprec);

            lcdLoc(LINE1);
            typeln("Temperature");
            lcdLoc(LINE2);
            typeln(tempc);

            temp = atoi(temprec);
            printf("temp= %d\n", temp);
            if (temp >= 50 && temp < 80)
                sendsignal_uptemp_warn(clnt_sock2);
            if (temp > 80)
                sendsignal_uptemp_critical(clnt_sock2);
            
            // led panel show
            // if temp > 5c give signal to actuator pi
        }
        else if (msg[0] == 'p') {
            temprec = strtok(NULL, "=");
            pres = atoi(temprec);
            printf("pres= %d\n", pres);
            if (pres) {
                t = 0;
            }
            // timer reset
        }
        for(int i=0;i<6;i++){
            msg[i]='\0';
        }
        //write(clnt_sock2,msg,sizeof(msg));
    }
   
}


void typeFloat(float myFloat) {
    char buffer[20];
    sprintf(buffer, "%4.2f", myFloat);
    typeln(buffer);
}

// int to string
void typeInt(int i) {
    char array1[20];
    sprintf(array1, "%d", i);
    typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void) {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line) {
    lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val) {

    lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char* s) {

    while (*s) lcd_byte(*(s++), LCD_CHR);

}

void lcd_byte(int bits, int mode) {

    //Send byte to data pins
    // bits = the data
    // mode = 1 for data, 0 for command
    int bits_high;
    int bits_low;
    // uses the two half byte writes to LCD
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    // High bits
    wiringPiI2CReadReg8(fd, bits_high);
    lcd_toggle_enable(bits_high);

    // Low bits
    wiringPiI2CReadReg8(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits) {
    // Toggle enable pin on LCD display
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}


void lcd_init() {
    // Initialise display
    lcd_byte(0x33, LCD_CMD); // Initialise
    lcd_byte(0x32, LCD_CMD); // Initialise
    lcd_byte(0x06, LCD_CMD); // Cursor move direction
    lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
    lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
    lcd_byte(0x01, LCD_CMD); // Clear display
    delayMicroseconds(500);
}
