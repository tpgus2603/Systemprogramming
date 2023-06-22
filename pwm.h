#ifndef __PWM__H__
#define __PWM__H__
static int PWMExport(int pwmnum);
static int PWMEnable(int pwmnum);
static int PWMWritePeriod(int pwmnum, int value);
static int PWMWriteDutyCycle(int pwmnum, int value);


#endif
