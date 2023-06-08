#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define VALUE_MAX 45
#define DIRECTION_MAX 45
#define buz 0

static int PWMExport(int pwmnum){
   #define BUFFER_MAX 3
   char buffer[BUFFER_MAX];
   int bytes_written;
   int fd;
   
   fd = open("/sys/class/pwm/pwmchip0/unexport",O_WRONLY);
   if(-1==fd){
      fprintf(stderr, "Failed to open in unexport!\n");
      return(-1);
   }
   
   bytes_written = snprintf(buffer, BUFFER_MAX,"%d",pwmnum);
   write(fd,buffer,bytes_written);
   close(fd);
   
   sleep(1);
   fd = open("/sys/class/pwm/pwmchip0/export",O_WRONLY);
   if(-1==fd){
      fprintf(stderr,"Failed to open in export!\n");
      return(-1);
   }
   bytes_written = snprintf(buffer, BUFFER_MAX,"%d",pwmnum);
   write(fd,buffer,bytes_written);
   close(fd);
   sleep(1);
   return(0);
}

static int PWMEnable(int pwmnum){
   static const char s_unenable_str[] = "0";
   static const char s_enable_str[] = "1";
   char path[DIRECTION_MAX];
   int fd;
   
   snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm%d/enable",pwmnum);
   fd = open(path,O_WRONLY);
   if(-1==fd){
      fprintf(stderr,"Failed to open in enable!\n");
      return -1;
   }
   
   write(fd,s_unenable_str,strlen(s_unenable_str));
   close(fd);
   
   fd=open(path,O_WRONLY);
   if(-1==fd){
      fprintf(stderr,"Failed to open in enable!\n");
      return -1;
   }
   
   write(fd,s_enable_str,strlen(s_enable_str));
   close(fd);
   return(0);
}

static int PWMWritePeriod(int pwmnum, int value){
   char s_values_str[VALUE_MAX];
   char path[VALUE_MAX];
   int fd,byte;
   
   snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/period",pwmnum);
   fd = open(path,O_WRONLY);
   if(-1==fd){
      fprintf(stderr, "Failed to open in period!\n");
      return(-1);
   }
   byte=snprintf(s_values_str,VALUE_MAX,"%d",value);
   if(-1==write(fd,s_values_str,byte)){
      fprintf(stderr,"Failed to write value in period!\n");
      close(fd);
      return(-1);
   }
   close(fd);
   return(0);
}
static int PWMWriteDutyCycle(int pwmnum, int value){
   char path[VALUE_MAX];
   char s_values_str[VALUE_MAX];
   int fd,byte;
   
   snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle",pwmnum);
   fd = open(path, O_WRONLY);
   if(-1==fd){
      fprintf(stderr, "Failed to open in duty_cycle!\n");
      return(-1);
   }
   
   byte = snprintf(s_values_str,VALUE_MAX,"%d",value);
   
   if(-1==write(fd,s_values_str,byte)){
      fprintf(stderr, "Failed to write value! in duty_cycle\n");
      close(fd);
      return(-1);
   }
   
   close(fd);
   return(0);
}

int main(void){

    PWMExport(buz);
    PWMWritePeriod(buz, 2000000); // lower number, higher sound!!
    PWMWriteDutyCycle(buz,0);
    PWMEnable(0);

    int i=0;
    while(1){
        PWMWriteDutyCycle(buz,10000);
    }
    
}