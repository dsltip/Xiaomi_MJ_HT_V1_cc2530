#ifndef C51_GPIO_H__
#define C51_GPIO_H__

#include <ioCC2530.h>
#include "hal_types.h"

#define IO_MAX          7       // the max number of io need to detective

// P0.1！！P2.0     P0.2！！P1.7    P0.3！！P1.6   LEDB！！P0.0  LEDR！！P1.1
// P0.4！！P1.5     P0.5！！P1.4    P0.6！！P1.3   P0.7！！P1.2  KEY！！P1.0
#define KEY             P1_0
#define LED_B           P0_0
#define LED_R           P1_1

// the output IO define
#define IO_OUT0         P0_1    
#define IO_OUT1         P0_2
#define IO_OUT2         P0_3
#define IO_OUT3         P0_4
#define IO_OUT4         P0_5
#define IO_OUT5         P0_6
#define IO_OUT6         P0_7

// the input IO define
#define IO_IN0          P2_0    
#define IO_IN1          P1_7
#define IO_IN2          P1_6
#define IO_IN3          P1_5
#define IO_IN4          P1_4
#define IO_IN5          P1_3
#define IO_IN6          P1_2

// the function of handle the LED
#define LED_B_ON()      LED_B = 0
#define LED_B_OFF()     LED_B = 1

#define LED_R_ON()      LED_R = 0;
#define LED_R_OFF()     LED_R = 1;

// the function define
void C51_GPIO_initial(void);                /* initialize the gpios */    

uint8 C51_GPIO_ioDetective(void);           /* detect all io right or not */

uint8 C51_GPIO_ioSumInput(void);            /* calculate the value of all io level */

void C51_GPIO_OffDetective(void);           /* turn off detective GPIO and drop power */

void C51_GPIO_ioSetOutAllHigh(void);        /* Set all output io is high level */

uint8 C51_GPIO_ioCheckInput(uint8 ioNum);   /* check the level right or not */

void C51_GPIO_ioSetOutput(uint8 ioNum);     /* Set the low level for output io base input number */

#endif






