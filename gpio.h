#define DIRECTION_MAX 35 
#define _MAX 3 
#define VALUE_MAX 30 

#define IN 0 
#define OUT 1 
#define LOW 0 
#define HIGH 1 

extern  int GPIOExport(int pin);
extern  int GPIOUnexport(int pin);
extern  int GPIODirection(int pin, int dir);
extern  int GPIORead(int pin);
extern  int GPIOWrite(int pin, int value);
