#ifndef C51_RTC_H__
#define C51_RTC_H__

#include <ioCC2530.h>
#include "hal_types.h"

void C51_RTC_Initial(void);                     /* initialize the RTC */

void C51_RTC_EnterSleep(void);                  /* the system enter PM2 mode */

void C51_RTC_SetPeriod(uint16 sec);             /* set the period for systerm sleep */

void C51_RTC_SysPowerMode(uint8 power_mode);    /* Set the power mode with system */

#endif






