#include "ioCC2530.h"
//#include "zcomdef.h"
#include "hal_i2c.h"

#define STATIC static

#if !defined HAL_I2C_RETRY_CNT
#define HAL_I2C_RETRY_CNT  3
#endif

// the default cofiguration below uses P1.6 for SDA and P0.0 for SCL.
// change these as needed.
//P1.0 SCL
//P1.1 SDA

#ifndef OCM_CLK_PORT
  #define OCM_CLK_PORT  1
#endif

#ifndef OCM_DATA_PORT
  #define OCM_DATA_PORT   1
#endif

#ifndef OCM_CLK_PIN
  #define OCM_CLK_PIN   0
#endif

#ifndef OCM_DATA_PIN
  #define OCM_DATA_PIN    1
#endif

// General I/O definitions
#define IO_GIO  0  // General purpose I/O
#define IO_PER  1  // Peripheral function
#define IO_IN   0  // Input pin
#define IO_OUT  1  // Output pin
#define IO_PUD  0  // Pullup/pulldn input
#define IO_TRI  1  // Tri-state input
#define IO_PUP  0  // Pull-up input pin
#define IO_PDN  1  // Pull-down input pin

#define OCM_ADDRESS  (0xA0)
#define OCM_READ     (0x01)
#define OCM_WRITE    (0x00)
#define SMB_ACK      (0)
#define SMB_NAK      (1)
#define SEND_STOP    (0)
#define NOSEND_STOP  (1)
#define SEND_START   (0)
#define NOSEND_START (1)

// device specific as to where the 17th address bit goes...

// *************************   MACROS   ************************************
#undef P

  /* I/O PORT CONFIGURATION */
#define CAT1(x,y) x##y  // Concatenates 2 strings
#define CAT2(x,y) CAT1(x,y)  // Forces evaluation of CAT1

// OCM port I/O defintions
// Builds I/O port name: PNAME(1,INP) ==> P1INP
#define PNAME(y,z) CAT2(P,CAT2(y,z))
// Builds I/O bit name: BNAME(1,2) ==> P1_2
#define BNAME(port,pin) CAT2(CAT2(P,port),CAT2(_,pin))

// OCM port I/O defintions
#define OCM_SCL BNAME(OCM_CLK_PORT, OCM_CLK_PIN)
#define OCM_SDA BNAME(OCM_DATA_PORT, OCM_DATA_PIN)

#define IO_DIR_PORT_PIN(port, pin, dir) \
{\
  if ( dir == IO_OUT ) \
    PNAME(port,DIR) |= (1<<(pin)); \
  else \
    PNAME(port,DIR) &= ~(1<<(pin)); \
}

#define OCM_DATA_HIGH()\
{ \
  IO_DIR_PORT_PIN(OCM_DATA_PORT, OCM_DATA_PIN, IO_IN); \
}

#define OCM_DATA_LOW() \
{ \
  IO_DIR_PORT_PIN(OCM_DATA_PORT, OCM_DATA_PIN, IO_OUT); \
  OCM_SDA = 0;\
}

#define IO_FUNC_PORT_PIN(port, pin, func) \
{ \
  if( port < 2 ) \
  { \
    if ( func == IO_PER ) \
      PNAME(port,SEL) |= (1<<(pin)); \
    else \
      PNAME(port,SEL) &= ~(1<<(pin)); \
  } \
  else \
  { \
    if ( func == IO_PER ) \
      P2SEL |= (1<<(pin>>1)); \
    else \
      P2SEL &= ~(1<<(pin>>1)); \
  } \
}

#define IO_IMODE_PORT_PIN(port, pin, mode) \
{ \
  if ( mode == IO_TRI ) \
    PNAME(port,INP) |= (1<<(pin)); \
  else \
    PNAME(port,INP) &= ~(1<<(pin)); \
}

#define IO_PUD_PORT(port, dir) \
{ \
  if ( dir == IO_PDN ) \
    P2INP |= (1<<(port+5)); \
  else \
    P2INP &= ~(1<<(port+5)); \
}

STATIC void   hali2cSend( uint8 *buffer, uint16 len, uint8 sendStart, uint8 sendStop );
STATIC _Bool  hali2cSendByte( uint8 dByte );
STATIC void   hali2cWrite( uint8 dBit );
STATIC void   hali2cClock( uint8 dir );
STATIC void   hali2cStart( void );
STATIC void   hali2cStop( void );
STATIC void   hali2cReceive( uint8 address, uint8 *buffer, uint16 len );
STATIC uint8  hali2cReceiveByte( void );
STATIC _Bool  hali2cRead( void );
STATIC void   hali2cSendDeviceAddress(uint8 address);


STATIC __near_func void   hali2cWait( uint8 );

STATIC uint8 s_xmemIsInit;

/*********************************************************************
 * @fn      HalI2CInit
 * @brief   Initializes two-wire serial I/O bus
 * @param   void
 * @return  void
 */
void HalI2CInit( void )
{
  if (!s_xmemIsInit)  {
    s_xmemIsInit = 1;

    // Set port pins as inputs
    IO_DIR_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_IN );
    IO_DIR_PORT_PIN( OCM_DATA_PORT, OCM_DATA_PIN, IO_IN );

    // Set for general I/O operation
    IO_FUNC_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_GIO );
    IO_FUNC_PORT_PIN( OCM_DATA_PORT, OCM_DATA_PIN, IO_GIO );

    // Set I/O mode for pull-up/pull-down
    IO_IMODE_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_PUD );
    IO_IMODE_PORT_PIN( OCM_DATA_PORT, OCM_DATA_PIN, IO_PUD );

    // Set pins to pull-up
    IO_PUD_PORT( OCM_CLK_PORT, IO_PUP );
    IO_PUD_PORT( OCM_DATA_PORT, IO_PUP );
  }
}

int8 HalI2CReceive(uint8 address, uint8 *buf, uint16 len)
{
  hali2cReceive(address, buf, len);

  return 0;
}

int8 HalI2CSend(uint8 address, uint8 *buf, uint16 len)
{
  // begin the write sequence with the address byte
  hali2cSendDeviceAddress(address);
  hali2cSend(buf, len, NOSEND_START, SEND_STOP);

  return 0;
}
/*********************************************************************
 * @fn      hali2cSend
 * @brief   Sends buffer contents to SM-Bus device
 * @param   buffer - ptr to buffered data to send
 * @param   len - number of bytes in buffer
 * @param   sendStart - whether or not to send start condition.
 * @param   sendStop - whether or not to send stop condition.
 * @return  void
 */
STATIC void hali2cSend( uint8 *buffer, uint16 len, uint8 sendStart, uint8 sendStop )
{
  uint16 i;
  uint8 retry = HAL_I2C_RETRY_CNT;

  if (!len)  {
    return;
  }

  if (sendStart == SEND_START)  {
    hali2cStart();
  }

  for ( i = 0; i < len; i++ )
  {
    do {
      if ( hali2cSendByte( buffer[i] ) )  // takes care of ack polling
      {
        break;
      }
    } while ( --retry );
  }

  if (sendStop == SEND_STOP) {
    hali2cStop();
  }
}

/*********************************************************************
 * @fn      hali2cSendByte
 * @brief   Serialize and send one byte to SM-Bus device
 * @param   dByte - data byte to send
 * @return  ACK status - 0=none, 1=received
 */
STATIC _Bool hali2cSendByte( uint8 dByte )
{
  uint8 i;

  for ( i = 0; i < 8; i++ )
  {
    // Send the MSB
    hali2cWrite( dByte & 0x80 );
    // Next bit into MSB
    dByte <<= 1;
  }
  // need clock low so if the SDA transitions on the next statement the
  // slave doesn't stop. Also give opportunity for slave to set SDA
  hali2cClock( 0 );
  OCM_DATA_HIGH(); // set to input to receive ack...
  hali2cClock( 1 );
  hali2cWait(1);

  return ( !OCM_SDA );  // Return ACK status
}

/*********************************************************************
 * @fn      hali2cWrite
 * @brief   Send one bit to SM-Bus device
 * @param   dBit - data bit to clock onto SM-Bus
 * @return  void
 */
STATIC void hali2cWrite( uint8 dBit )
{
  hali2cClock( 0 );
  hali2cWait(1);
  if ( dBit )
  {
    OCM_DATA_HIGH();
  }
  else
  {
    OCM_DATA_LOW();
  }

  hali2cClock( 1 );
  hali2cWait(1);
}

/*********************************************************************
 * @fn      hali2cClock
 * @brief   Clocks the SM-Bus. If a negative edge is going out, the
 *          I/O pin is set as an output and driven low. If a positive
 *          edge is going out, the pin is set as an input and the pin
 *          pull-up drives the line high. This way, the slave device
 *          can hold the node low if longer setup time is desired.
 * @param   dir - clock line direction
 * @return  void
 */
STATIC void hali2cClock( uint8 dir )
{
  if ( dir )
  {
    IO_DIR_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_IN );
  }
  else
  {
    IO_DIR_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_OUT );
    OCM_SCL = 0;
  }
  hali2cWait(1);
}

/*********************************************************************
 * @fn      hali2cStart
 * @brief   Initiates SM-Bus communication. Makes sure that both the
 *          clock and data lines of the SM-Bus are high. Then the data
 *          line is set high and clock line is set low to start I/O.
 * @param   void
 * @return  void
 */
STATIC void hali2cStart( void )
{
  uint8 retry = HAL_I2C_RETRY_CNT;

  // set SCL to input but with pull-up. if slave is pulling down it will stay down.
  hali2cClock(1);

  do {
    // wait for slave to release clock line...
    if (OCM_SCL)  // wait until the line is high...
    {
      break;
    }
    hali2cWait(1);
  } while ( --retry );

  // SCL low to set SDA high so the transition will be correct.
  hali2cClock(0);
  OCM_DATA_HIGH();  // SDA high
  hali2cClock(1);      // set up for transition
  hali2cWait(1);
  OCM_DATA_LOW();   // start

  hali2cWait(1);
  hali2cClock( 0 );
}

/*********************************************************************
 * @fn      hali2cStop
 * @brief   Terminates SM-Bus communication. Waits unitl the data line
 *          is low and the clock line is high. Then sets the data line
 *          high, keeping the clock line high to stop I/O.
 * @param   void
 * @return  void
 */
STATIC void hali2cStop( void )
{
  // Wait for clock high and data low
  hali2cClock(0);
  OCM_DATA_LOW();  // force low with SCL low
  hali2cWait(1);

  hali2cClock( 1 );
  OCM_DATA_HIGH(); // stop condition
  hali2cWait(1);

}

/*********************************************************************
 * @fn      hali2cWait
 * @brief   Wastes a an amount of time.
 * @param   count: down count in busy-wait
 * @return  void
 */
STATIC __near_func void hali2cWait( uint8 count )
{
  //count+=1;
  while ( count-- );
}

/*********************************************************************
 * @fn      hali2cReceiveByte
 * @brief   Read the 8 data bits.
 * @param   void
 * @return  character read
 */
STATIC uint8 hali2cReceiveByte()
{
  int8 i, rval = 0;

  for (i=7; i>=0; --i)  {
    if (hali2cRead())  {
      rval |= 1<<i;
    }
  }

  return rval;
}
/**************************************************************************************************
**************************************************************************************************/
/*********************************************************************
 * @fn      hali2cReceive
 * @brief   reads data into a buffer
 * @param   address: linear address on part from which to read
 * @param   buffer: target array for read characters
 * @param   len: max number of characters to read
 * @return  void
 */
STATIC void hali2cReceive( uint8 address, uint8 *buffer, uint16 len )
{
  //uint8  ch;
  uint16 i;

  if (!len)  {
    return;
  }

  hali2cSendDeviceAddress(address);

  //ch = OCM_ADDRESS_BYTE(0, OCM_READ);
  //hali2cSend(&ch, 1, SEND_START, NOSEND_STOP);

  for ( i = 0; i < len-1; i++ )
  {
    // SCL may be high. set SCL low. If SDA goes high when input
    // mode is set the slave won't see a STOP
    hali2cClock(0);
    OCM_DATA_HIGH();

    buffer[i] = hali2cReceiveByte();
    hali2cWrite(SMB_ACK);           // write leaves SCL high
  }

  // condition SDA one more time...
  hali2cClock(0);
  OCM_DATA_HIGH();
  buffer[i] = hali2cReceiveByte();
  hali2cWrite(SMB_NAK);

  hali2cStop();
}

/*********************************************************************
 * @fn      hali2cRead
 * @brief   Toggle the clock line to let the slave set the data line.
 *          Then read the data line.
 * @param   void
 * @return  TRUE if bit read is 1 else FALSE
 */
STATIC _Bool hali2cRead( void )
{
  // SCL low to let slave set SDA. SCL high for SDA
  // valid and then get bit
  hali2cClock( 0 );
  hali2cWait(1);
  hali2cClock( 1 );
  hali2cWait(1);

  return OCM_SDA;
}

/*********************************************************************
 * @fn      hali2cSendDeviceAddress
 * @brief   Send onlythe device address. Do ack polling
 *
 * @param   void
 * @return  none
 */
STATIC void hali2cSendDeviceAddress(uint8 address)
{
  uint8 retry = HAL_I2C_RETRY_CNT;

  do {
    hali2cStart();
    if (hali2cSendByte(address))  // do ack polling...
    {
      break;
    }
  } while ( --retry );
}

void HalI2CST(uint8 st){
    IO_DIR_PORT_PIN( OCM_CLK_PORT, OCM_CLK_PIN, IO_OUT );
    OCM_SCL = st;
    IO_DIR_PORT_PIN(OCM_DATA_PORT, OCM_DATA_PIN, IO_OUT); 
    OCM_SDA = st;
}
