
#include "c51_gpio.h"

/**************************************************************************************************
*Function   :   C51_GPIO_initial() => initialize the gpios
*Declare    :   the io direction config as follow
                P0.1！！P2.0     P0.2！！P1.7    P0.3！！P1.6   LEDB！！P0.0  LEDR！！P1.1
                P0.4！！P1.5     P0.5！！P1.4    P0.6！！P1.3   P0.7！！P1.2  KEY！！P1.0
**************************************************************************************************/
void C51_GPIO_initial(void)
{
    // Set the direction of GPIOs
    P0DIR = 0xFF;       // 1111 1111    P0 output
    P1DIR = 0x02;       // 0000 0010    P1.1 output, else input
    P2DIR = 0x00;       // 0000 0000    P2 input
    
    LED_R_OFF();        // turn off the red led
    LED_B_OFF();        // turn off the blue led
}

/**************************************************************************************************
*Function   :   C51_GPIO_OffDetective() => turn off detective GPIO and drop power
*Declare    :   the io direction config as follow
                P0.1！！P2.0     P0.2！！P1.7    P0.3！！P1.6   LEDB！！P0.0  LEDR！！P1.1
                P0.4！！P1.5     P0.5！！P1.4    P0.6！！P1.3   P0.7！！P1.2  KEY！！P1.0
**************************************************************************************************/
void C51_GPIO_OffDetective(void)
{
    P0DIR = 0x01;       // 0000 0001    P0.0output, else input
    P1DIR = 0x02;       // 0000 0010    P1.1output, else input
    P2DIR = 0x00;       // 0000 0000    P2 input
}

/**************************************************************************************************
*Function   : C51_GPIO_ioDetective() => detect all io right or not
*Output     : the result of ckeck   0, check error; else, check right 
**************************************************************************************************/
uint8 C51_GPIO_ioDetective(void)
{
    uint8 i = 0;

    for (i=0; i<IO_MAX; i++)
    {
        C51_GPIO_ioSetOutput(i);
        if (!C51_GPIO_ioCheckInput(i))      { break; }
    }
    
    if (IO_MAX == i)        { return 1; }

    return 0;   
}

/**************************************************************************************************
*Function   : C51_GPIO_ioSumInput() => calculate the value of all io level
*Output     : the result of calculate
**************************************************************************************************/
uint8 C51_GPIO_ioSumInput(void)
{
    uint8 tmp = IO_IN0;
    
    tmp = tmp + IO_IN1;
    tmp = tmp + IO_IN2;
    tmp = tmp + IO_IN3;
    tmp = tmp + IO_IN4;
    tmp = tmp + IO_IN5;
    tmp = tmp + IO_IN6;

    return (tmp);      
}

/**************************************************************************************************
*Function   : C51_GPIO_ioCheckInput() => check the level right or not
*Input      : ioNum,    0-6, decide which io will be become low level
*Output     : the result of ckeck   0, check error; else, check right 
**************************************************************************************************/
uint8 C51_GPIO_ioCheckInput(uint8 ioNum)
{
    uint8 error = 0;

    // parameter check, the input number must Less than the maximum
    if (ioNum > IO_MAX)                 { return 0; }     
    
    // Add all value what is read input io, and the sum must be equal 6
    if (0x06 != C51_GPIO_ioSumInput())  { return 0; }

    // judge the level is right or not base on input number
    switch (ioNum)                  
    {
        case 0:     if (0 == IO_IN0)    { error = 1; } break;
        case 1:     if (0 == IO_IN1)    { error = 1; } break;
        case 2:     if (0 == IO_IN2)    { error = 1; } break;    
        case 3:     if (0 == IO_IN3)    { error = 1; } break;   
        case 4:     if (0 == IO_IN4)    { error = 1; } break;   
        case 5:     if (0 == IO_IN5)    { error = 1; } break;   
        case 6:     if (0 == IO_IN6)    { error = 1; } break;
        default:    break;
    } 
    
    return (error);      
}

/**************************************************************************************************
*Function   : C51_GPIO_ioSetOutAllHigh() => Set all output io is high level 
**************************************************************************************************/
void C51_GPIO_ioSetOutAllHigh(void)
{
    IO_OUT0 = 1;
    IO_OUT1 = 1;
    IO_OUT2 = 1;
    IO_OUT3 = 1;
    IO_OUT4 = 1;
    IO_OUT5 = 1;
    IO_OUT6 = 1;        
}

/**************************************************************************************************
*Function   : C51_GPIO_ioSetOutput() => Set the low level for output io base input number 
*Input      : ioNum,    0-6, decide which io will be output low level
**************************************************************************************************/
void C51_GPIO_ioSetOutput(uint8 ioNum)
{
    // parameter check, the input number must Less than the maximum
    if (ioNum > IO_MAX)     { return; }     
    
    C51_GPIO_ioSetOutAllHigh();     // set all output io is high level
    
    // set the number of io output low level base on input number
    switch (ioNum)                  
    {
        case 0:     IO_OUT0 = 0; break;
        case 1:     IO_OUT1 = 0; break;
        case 2:     IO_OUT2 = 0; break;    
        case 3:     IO_OUT3 = 0; break;    
        case 4:     IO_OUT4 = 0; break;    
        case 5:     IO_OUT5 = 0; break;    
        case 6:     IO_OUT6 = 0; break;
        default:    break;
    }       
}
