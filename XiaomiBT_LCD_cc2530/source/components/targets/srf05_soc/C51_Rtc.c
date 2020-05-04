
#include "c51_rtc.h"

/**************************************************************************************************
*Function   :   C51_RTC_SysPowerMode() => Set the power mode with system
*Input      :   power_mode,   0-3, the mode will be set, PM0-PM3
**************************************************************************************************/
void C51_RTC_SysPowerMode(uint8 power_mode) 
{ 
    // Set system enter sleep mode and wake up for interrupt 
    if (power_mode < 4)     { SLEEPCMD |= power_mode; PCON = 0x01; }     
    else                    { PCON = 0x00; }
}

/**************************************************************************************************
*Function   :   C51_RTC_Initial() => initialize the RTC
**************************************************************************************************/
void C51_RTC_Initial(void) 
{ 
    ST2 = 0x00; 
    ST1 = 0x00; 
    ST0 = 0x00;

    STIE = 1;   // enable ST interrupt  0,disable; 1, enable
    STIF = 0;   // clear the flag of interrupt
    EA = 1;     // enable all interrupt
}

/**************************************************************************************************
*Function   : C51_RTC_SetPeriod() => set the period for systerm sleep
*Input      : sec,  how many second the system will be wake up
**************************************************************************************************/
void C51_RTC_SetPeriod(uint16 sec) 
{ 
    uint32 SleepTimer = 0;
    //uint32 delta = (uint32)sec << 15;
    SleepTimer |= ST0;
    SleepTimer |= (uint32)ST1 << 8;
    SleepTimer |= (uint32)ST2 << 16;
    
    SleepTimer += (uint32)sec << 15;    // = sec * 32768
    SleepTimer -= 94;
    ST2 = (uint8) (SleepTimer >> 16);
    ST1 = (uint8) (SleepTimer >> 8);
    ST0 = (uint8) (SleepTimer);
}

/**************************************************************************************************
*Function   : C51_RTC_EnterSleep() => the system enter PM2 mode 
**************************************************************************************************/
void C51_RTC_EnterSleep(void)
{
    C51_RTC_SetPeriod(2);       // set the time of sleep
    C51_RTC_SysPowerMode(2);    // make the system enter PM2 mode
}


/**************************************************************************************************
*Function   :   ST_ISR() => the interrupt service function for ST
*Declare    :   #program vector is interrupt vector
**************************************************************************************************/
#pragma vector = ST_VECTOR 
__interrupt void ST_ISR(void) 
{ 
    STIF = 0;                   // clear the flag of interrupt
    C51_RTC_SysPowerMode(4);    // system enter normal work mode
} 
