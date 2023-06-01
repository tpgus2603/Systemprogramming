
#include <stdio.h>
#include <wiringPi.h>
#define buzzer_pin 18

int main(void) 
{

    wiringPiSetupGpio();
    pinMode(buzzer_pin, PWM_OUTPUT);

    pwmSetClock(19);
    pwmSetMode(PWM_MODE_MS);F

    pwmSetRange(1000000/262);
    pwmWrite(buzzer_pin, 1000000/262/2);
    delay(3000);

    pwmWrite(buzzer_pin,0);
}

// softTone.h를 사용한 코드

// #include <stdio.h>
// #include <wiringPi.h>
// #include <softTone.h>

// const int pinPiezo = 18;

// const int aMelody[8] = {131,147,165,175,196,220,247,262};

// int main(void)
// {
// 	wiringPiSetupGpio();

// 	softToneCreate(pinPiezo);

// 	for (i = 0 ; i < 8 ; ++i)

//     {

//       printf ("%3d\n", i) ;

//       softToneWrite (pinPiezo, aMelody[i]) ;

//       delay (500) ;

//     }
// }
