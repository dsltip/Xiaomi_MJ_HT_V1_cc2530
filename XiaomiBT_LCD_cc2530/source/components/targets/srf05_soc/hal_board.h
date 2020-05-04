/***********************************************************************************

  Filename:     hal_board.h

  Description:  SmartRF05 board with CCxxxxEM.

***********************************************************************************/

#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include "ioCC2530.h"
#include "hal_cc8051.h"
#include "hal_defs.h"
#include "hal_mcu.h"

/***********************************************************************************
 * CONSTANTS
 */

// Board properties
#define BOARD_NAME                     "E18"
#define NUM_LEDS                        2
#define NUM_BUTTONS                     1
#define NUM_POTS                        1

// Clock
#define BSP_CONFIG_CLOCK_MHZ                32

// LEDs
#define HAL_BOARD_IO_LED_1_PORT        2   // Blue
#define HAL_BOARD_IO_LED_1_PIN         1
#define HAL_BOARD_IO_LED_2_PORT        0   // Red
#define HAL_BOARD_IO_LED_2_PIN         0

// Buttons
#define HAL_BOARD_IO_BTN_1_PORT        1   // Button S1
#define HAL_BOARD_IO_BTN_1_PIN         0

// Debounce
#define HAL_DEBOUNCE(expr)    { int i; for (i=0; i<500; i++) { if (!(expr)) i = 0; } }

/*********************************************宏定义*********************************************/
// LED操作函数， SET = 关闭， CLR = 打开， TGL = 翻转 
#define HAL_LED_SET_1()         MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_SET_2()         MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)

#define HAL_LED_CLR_1()         MCU_IO_SET_LOW(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_CLR_2()         MCU_IO_SET_LOW(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)

#define HAL_LED_TGL_1()         MCU_IO_TGL(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_TGL_2()         MCU_IO_TGL(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)

// 按键操作函数，当按键按下，读取电平为低电平
#define HAL_BUTTON_1_PUSHED()   (!MCU_IO_GET(HAL_BOARD_IO_BTN_1_PORT, HAL_BOARD_IO_BTN_1_PIN))

// HAL processing not required for this board
#define HAL_PROCESS()

/***********************************************************************************
 * FUNCTION PROTOTYPES
 */
void halBoardInit(void);

#endif
