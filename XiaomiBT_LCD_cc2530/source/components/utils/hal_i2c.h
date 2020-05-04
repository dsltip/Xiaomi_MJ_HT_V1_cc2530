#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <hal_types.h>

/*********************************************************************
 * @fn      HalI2CInit
 * @brief   Initializes two-wire serial I/O bus
 * @param   void
 * @return  void
 */
void  HalI2CInit( void );

/*********************************************************************
 * @fn      HALI2CReceive
 * @brief   Receives data into a buffer from an I2C slave device
 * @param   address: address of the slave device
 * @param   buf: target array for read characters
 * @param   len: max number of characters to read
 * @return  zero when successful.
 */
int8   HalI2CReceive(uint8 address, uint8 *buf, uint16 len);

/*********************************************************************
 * @fn      HALI2CSend
 * @brief   Sends buffer contents to an I2C slave device
 * @param   address: address of the slave device
 * @param   buf - ptr to buffered data to send
 * @param   len - number of bytes in buffer
 * @return  zero when successful.
 */
int8   HalI2CSend(uint8 address, uint8 *buf, uint16 len);
void   HalI2CST(uint8 st);

#endif
