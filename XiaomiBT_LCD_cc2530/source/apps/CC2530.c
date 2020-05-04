/*************************  LCD MONITOR-SENSOR SLEEPING************************************/
#include <hal_led.h>
#include <hal_assert.h>
#include <hal_board.h>
#include <hal_int.h>
#include "hal_mcu.h"
#include "hal_button.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "flash.h"
#include "c51_gpio.h"
#include "c51_rtc.h"
#include "hal_i2c.h"

#define Set(ADDRESS,BIT) (ADDRESS |= (1<<BIT))
#define Clr(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT))

/************************************************************************************/
//#define UART //!!!!!!!!!!!!!!!!!!!!!!!!!!11
//#define LED //!!!!!!!!!!!!!!!!!!!!!!!!!!11
//MAP
//0 - Type(hi), SendCnt(lo)
//1 - Temp
//2 - voltage
//3 - Humidity
//4 - PERIOD

#define MY_TYPE             0x00   // Monitor device temp/hum
#define MY_ADDR             0xDEF0  // 
#define MAS_ADDR            0xBEEF  // 
#define PERIOD 179 //<30 means minutes

//#define RF_CHANNEL          25      // 2.4 GHz RF channel 11 - 26 
#define PAN_ID              0x2007  // PAN ID

#define SEND_LENGTH        5       // 
#define MAS_LENGTH         5       // 
#define RF_PKT_MAX_SIZE     30      // 
#define BIN8(s7,s6,s5,s4,s3,s2,s1,s0) (uint8)( s0 * 0x01 + s1 * 0x02 + s2 * 0x04 + s3 * 0x08 + s4 * 0x10 + s5 * 0x20 + s6 * 0x40 + s7 * 0x80 )
//LCD control pins
#define LCD_SCL  2 //P1.2
#define LCD_SDA  3 //P1.3
#define LCD_CSB  4 //P1.4
#define LCD_INHB 5 //P1.5

#define DATA_SEND_GAP 10 //micros
#define LCD_CSB_HIGH_HLD 1
#define LCD_CSB_LOW_HLD  1
#define DATA_MAX 18
#define LCD_CMD_ICSET   0xEA //0b11101010
#define LCD_CMD_ICSET2  0xE8 //0b11101000
#define LCD_CMD_BLKCTL2 0xF0 //0b11110000 //(P10 - 00 - Blink off)
#define LCD_CMD_DISCTL  0xB8 //0b10111000 //(P43-11-53Hz,P2-0-FInvers,P10-00-Save1)
#define LCD_CMD_ADSET   0x00 //0b00000000
#define LCD_CMD_APCTL2  0xFC //0b11111100 //(normal,normal)
#define LCD_CMD_MODSET  0xC8 //0b11001000//(P3 - 1 - Disp on, P2-0-1/3 bias)
#define LCD_CMD_MODSET2 0xC0 //0b11000000//(P3 - 0 - Disp off, P2-0-1/3 bias)

static uint8 data[18];
static uint8 CMD_DISCTL;
//LCD segment digits( 6 placeholders * 10digits * 4(rigsters used by each digit) )
uint8 bb[240] = {2,7,223,192,2,0,7,192,2,7,189,192,2,5,191,192,2,4,231,192,2,5,255,128,2,7,255,128,2,0,143,192,2,7,255,192,2,5,255,192,4,61,252,128,4,0,116,128,
4,55,220,128,4,23,252,128,4,14,244,128,4,31,248,128,4,63,248,128,4,0,252,128,4,63,252,128,4,31,252,128,5,1,125,244,5,0,0,244,5,1,63,212,5,1,47,244,5,0,106,244,5,1,111,116,
5,1,127,116,5,0,12,244,5,1,127,244,5,1,111,244,7,0,125,252,7,0,0,124,7,0,123,220,7,0,91,252,7,0,78,124,7,0,95,248,7,0,127,248,7,0,8,252,7,0,127,252,7,0,95,252,9,3,223,200,
9,0,7,72,9,3,125,200,9,1,127,200,9,0,239,72,9,1,255,136,9,3,255,136,9,0,15,200,9,3,255,200,9,1,255,200,11,23,223,64,11,0,15,64,11,19,253,64,11,18,255,64,11,6,175,64,11,22,247,
64,11,23,247,64,11,0,207,64,11,23,255,64,11,22,255,64};

/************************************************************************************/
static basicRfCfg_t basicRfConfig;
static uint8    pRxData[RF_PKT_MAX_SIZE];               // 
static uint8    pTxData[SEND_LENGTH] = { 0,0,0,0,0 };   // 0,sendcnt,volt,0,period
static uint8    i2cdata1[6] = { 0,0,0,0,0,0};
static uint8    i2cdata2[2] = { 0,0};   
static uint8   SendCnt = 0;                            // 
static uint8   RecvCnt = 0;                            // 
static uint8   tcnt;

static uint8   SWcnt;
static uint8   SW; // Switch state
static uint8   SW_; // Switch state prev
static int16   TemperatureOut=0;
static int16   CO2Out=0;
static int16   TemperatureIn=0;
static int16   TemperatureOut_=0;
static int16   TemperatureIn_=0;
static uint8 period;
static uint8 flash_buf[4];
static uint8 RF_CHANNEL;

//static char Txdata[20];
/**************************************************************************************/
static void RF_SendPacket(uint8 ack);        // 
static void RF_RecvPacket(void);        // 
static void RF_Initial(uint8 rxe);
static void draw(uint8 p,uint8 n);
static void draw_up(int16 v);
static void draw_down(int16 v);
static void sendData(uint8 data, uint8 half);

//static uint16 getTemperature(void);
static uint16 getADC(uint8 reg);
static void InitT3();

#ifdef UART
static void InitUART(void);           //
static void UartSend_String(char *Data,int len);
static void UartSend_Char(char Data);
#endif
#pragma location = "SLEEP_CODE"  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void SetSleepMode(void);
/*************************************************************************************************
*************************************************************************************************/
/*************************************************************************************************
*************************************************************************************************/

void main(void)
{
  uint8   ack;
  uint8 i,half;
  uint16 TemperatureRaw;
  uint16 RelHumidityRaw;    
  float _TemperatureCeil;
  float _RelHumidity; 
  //halBoardInit();                                         // 
  //halMcuInit(); //Set the clock sources ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  myMcuInit();
  // internal temp sensor
  //TR0=0X01;         //set '1' to connectthe temperature sensorto the SOC_ADC.
  //ATEST=0X01;       // Enables the temperature sensor

  C51_RTC_Initial(); 
  flash_page_read(0,14,0,flash_buf,4);//bank,page,offset b0p2 0x1000
  period = flash_buf[0];
  if((period<10)||(period==255))period=PERIOD;
  RF_CHANNEL = flash_buf[1];
  if((RF_CHANNEL<11)||(RF_CHANNEL>25))RF_CHANNEL = 25;
  
  if (halRfInit() == FAILED)      { HAL_ASSERT(FALSE); }  //
        
  RF_Initial(0);   

  pTxData[0] = MY_TYPE;
  pTxData[4] = period;
  // if (!C51_GPIO_ioDetective())    { halLedClear(2); }     //
#ifdef UART
   InitUART();
   UartSend_Char(period);
   UartSend_Char(RF_CHANNEL);
#endif
  InitT3();
  //P1.2(input),P1.3(output)

  P1IFG = 0;         // clear all interrupt flags for P1
  /*P1DIR &= ~BV(2);   // set bit as input P1.2
  P1DIR |= BV(3);   // set bit as output P1.3
  P1INP  = 1 << 2  ; // tri state(no pull up down) ///////////////!!!!!!!!!!!!!!!!!! uncomment this and comment next line
  P2INP |= 0x40;//   select P1 pulldown mode //                                          uncomment
  P1 = 0x00; */              // All pins on port 1 to low 

  for(i = 0; i < DATA_MAX; i++) data[i] = 0;
//  draw_up(100);
  
    /* Pin mode set */
  P1DIR |= 0x1C; //P1.2-4 - output
  halMcuWaitMs(100);
  /* Power ON sequence */
  P1DIR |= 0x20; //P1.5 - output
  Clr(P1,LCD_INHB);//digitalWrite(LCD_INHB, 0);
  halMcuWaitMs(150);//milliseconds
  Set(P1,LCD_INHB);//digitalWrite(LCD_INHB, 1);
  halMcuWaitUs(150);//microseconds
  Set(P1,LCD_CSB);//digitalWrite(LCD_CSB,  1);
  halMcuWaitUs(LCD_CSB_HIGH_HLD);//delayMicroseconds(LCD_CSB_HIGH_HLD);//1(min 50ns)
   
 /* Initialize sequence */
  Clr(P1,LCD_CSB);
  halMcuWaitUs(LCD_CSB_LOW_HLD); //1
  sendData(LCD_CMD_ICSET, 0); //0b11101010 -soft reset(P2 -MSB0, P1-reset, P0 - Int clock)
  sendData(LCD_CMD_MODSET2, 0);//0b11000000(P3 - 0 - Disp off, P2-0-1/3 bias)
  sendData(LCD_CMD_ADSET, 0); //0b00000000
  for(i = 0; i < DATA_MAX; i++) { //DATA_MAX = 18
        half = (i == DATA_MAX - 1) ? 1 : 0;
        sendData(data[i], half);
    }
  Set(P1,LCD_CSB);
  halMcuWaitUs(LCD_CSB_HIGH_HLD);

/* DISPON sequence */
  Clr(P1,LCD_CSB);
  halMcuWaitUs(LCD_CSB_LOW_HLD);
  sendData(LCD_CMD_DISCTL, 0); //0b10100111(P43-00-80Hz,P2-1-Invers,P10-11-HighPower)
  sendData(LCD_CMD_BLKCTL2, 0); //0b11110000(P10 - 00 - Blink off)
  sendData(LCD_CMD_APCTL2, 0);  //0b11111100(normal,normal)
  sendData(LCD_CMD_MODSET, 0);  //0b11001000(P3 - 1 - Disp on, P2-0-1/3 bias)
  Set(P1,LCD_CSB);
  halMcuWaitUs(LCD_CSB_HIGH_HLD);
  //shalMcuWaitMs(10);
  Clr(P1DIR,5); //pinMode(LCD_INHB, INPUT);
   
  
  CMD_DISCTL = LCD_CMD_DISCTL;
  SW = 0;
  SW_ = 0;
  SWcnt=0;

  halIntOn();
  HalI2CInit(); //Init I2C for SHT3x
  i2cdata2[0]=0x2C;
  i2cdata2[1]=0x10;
  while (1)//////////////////////////////////////////////////////////////////!!!!!!!!!!
    {
      HalI2CReceive(0x89, i2cdata1, 6);
      TemperatureRaw   = (i2cdata1[0]<<8)+i2cdata1[1];
      RelHumidityRaw   = (i2cdata1[3]<<8)+i2cdata1[4];
      _TemperatureCeil = ((float) TemperatureRaw) * 0.00267033 - 45.;
      _RelHumidity = ((float) RelHumidityRaw) * 0.0015259;
      pTxData[1] = (_TemperatureCeil-10)*10;
      pTxData[3] = _RelHumidity;
      TemperatureIn =  _TemperatureCeil*10;
      //halMcuWaitUs(LCD_CSB_HIGH_HLD);
      
#ifdef UART
      UartSend_Char(pTxData[1]);      
      UartSend_Char(pTxData[3]);
#endif 
     
       RF_SendPacket(0);
       halRfReceiveOn();//wait answer from master(server)  
       tcnt = 0;
       while(tcnt < 40){///47 - 1.5ms 31.25kHz
            RF_RecvPacket();
          }
       halRfReceiveOff();
     //update LCD with changed data
     if((TemperatureIn != TemperatureIn_)||(TemperatureOut != TemperatureOut_)){
      for(i = 0; i < 14; i++) data[i] = 0;
      Set(data[7],1);
      Set(data[7],0);
      draw_up(TemperatureOut);
      if(CO2Out!=0)draw_down(CO2Out); else draw_down(TemperatureIn);
      Clr(P1,LCD_CSB);
      halMcuWaitUs(LCD_CSB_LOW_HLD); //1
      sendData(CMD_DISCTL, 0); //0b10100111(P43-00-80Hz,P2-1-Invers,P10-11-HighPower)
      sendData(LCD_CMD_BLKCTL2, 0); //0b11110000(P10 - 00 - Blink off)
      sendData(LCD_CMD_APCTL2, 0);  //0b11111100(normal,normal)
      sendData(LCD_CMD_MODSET, 0);  //0b11001000(P3 - 1 - Disp on, P2-0-1/3 bias)
      sendData(LCD_CMD_ADSET, 0); //0b00000000
      for(i = 0; i < 14/*DATA_MAX*/; i++) { //DATA_MAX = 18
            half = (i == DATA_MAX - 1) ? 1 : 0;
            sendData(data[i], half);
        }
      Set(P1,LCD_CSB);
      TemperatureIn_ = TemperatureIn;
      TemperatureOut_ = TemperatureOut;
      };//!=
          
      HalI2CSend(0x88, i2cdata2, 2);
      
      C51_RTC_SetPeriod(period);       // set the time of sleep
      //C51_RTC_SetPeriod(10);       // set the time of sleep
      //C51_RTC_SysPowerMode(2);    // make the system enter PM2 mode
      SLEEPCMD |= 2;
      //halIntOff();
      T3IE = 0; 
      SetSleepMode();
      halIntOn();

       RF_Initial(0);     //
       T3IE = 1; //enable timer 3 int*/
    }//while
}

static void draw(uint8 p,uint8 n){//draw digit 'n' on place 'p'
    uint8 c = p*40+n*4;
    uint8 r = bb[c];
    c++;
    data[r]|=bb[c];r++;c++;
    data[r]|=bb[c];r++;c++;
    data[r]|=bb[c];
}

void draw_up(int16 v){ //draw upper number
//-99.9..999.9(9999)
uint8 point=0;
int16 n=v;
if(v<0){//draw minus
  data[3]|=0x24;
  n=v*-1;

  if((n>99)&&(n<200)){//-19.9..-9.9(100..199)
    n = n - 100;
    draw(0,1);
  }

if(n>99){//no need point
    n= n / 10;
  } 
  else 
  { //need point
    point = 1;
    data[5]|=0x2;
  } 
  if(n>9){//draw 2 digs
    draw(2,n%10);
    draw(1,n/10);
  } else
  {
    if(point>0)draw(1,0);
    draw(2,n);//draw one digit
  }
  
}//draw minus
else {//>=0
  if(n>999){//no need point
    n= n / 10;
  } 
  else 
  { //need point
    data[5]|=0x2;
    point = 1;
  } 
  if(n>99){//draw 3 digs
    draw(2,n%10);
    n=n/10;
    draw(1,n%10);
    draw(0,n/10);
      } else 
  if(n>9){//draw 2 digs
    draw(2,n%10);
    draw(1,n/10);
  } else 
  {
    if(point>0)draw(1,0);
    draw(2,n);//draw one digit
  }
}//>0
};
void draw_down(int16 v){//draw bottom number
//-99.9..999.9(9999)
uint8 point=0;
int16 n=v;
if(v<0){
  data[8]|=0x42;
  n=v*-1;

  if((n>99)&&(n<200)){//-19.9..-9.9(100..199)
    n = n - 100;
    draw(3,1);
  }

  
if(n>99){//no need point
    n= n / 10;
  } 
  else 
  { //need point
    point = 1;
    data[11]|=0x20;
  } 
  if(n>9){//draw 2 digs
    draw(5,n%10);
    draw(4,n/10);
  } else
  {
    if(point>0)draw(4,0);
    draw(5,n);//draw one digit
  }
  
}//draw minus
else {//>=0
  if(n>999){//no need point
    n= n / 10;
  } 
  else 
  { //need point
    data[11]|=0x20;
    point = 1;
  } 
  if(n>99){//draw 3 digs
    draw(5,n%10);
    n=n/10;
    draw(4,n%10);
    draw(3,n/10);
      } else 
  if(n>9){//draw 2 digs
    draw(5,n%10);
    draw(4,n/10);
  } else 
  {
    if(point>0)draw(4,0);
    draw(5,n);//draw one digit
  }
}//>0
};
static void sendData(uint8 data, uint8 half) { //send Register to LCD   
    uint8 bits[8];
    int MAX = (half == 1) ? 3 : 7;
    int i = 0;

    bits[7] = (data & 1) >> 0;
    bits[6] = (data & 2) >> 1;
    bits[5] = (data & 4) >> 2;
    bits[4] = (data & 8) >> 3;
    bits[3] = (data & 16) >> 4;
    bits[2] = (data & 32) >> 5;
    bits[1] = (data & 64) >> 6;
    bits[0] = (data & 128) >> 7;

    for (i=0; i <= MAX; i++) {
       if(bits[i]>0)Set(P1,LCD_SDA); else Clr(P1,LCD_SDA);//digitalWrite(LCD_SDA, bits[i]);//Sets the GPIO level, on or off. 
       //halMcuWaitUs(DATA_SEND_GAP);
       Set(P1,LCD_SCL);
       halMcuWaitUs(DATA_SEND_GAP);//delayMicroseconds(DATA_SEND_GAP);
       Clr(P1,LCD_SCL);
       //halMcuWaitUs(DATA_SEND_GAP);
      }
}

static void RF_Initial(uint8 rxe)
{
//	if (RX == mode)     { basicRfConfig.myAddr = RX_ADDR; } 
//	else                { basicRfConfig.myAddr = TX_ADDR; }    

    basicRfConfig.myAddr = MY_ADDR;
    basicRfConfig.panId = PAN_ID;           // 
    basicRfConfig.channel = RF_CHANNEL;     // 
    basicRfConfig.ackRequest = FALSE;        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    
    if (basicRfInit(&basicRfConfig) == FAILED)      { HAL_ASSERT(FALSE); }
          
    halRfSetTxPower(2);                     // 4dbm
    if (rxe>0)basicRfReceiveOn(); else basicRfReceiveOff();
}



/*************************************************************************************************
*************************************************************************************************/
static void RF_SendPacket(uint8 ack)
{ 
  uint16 sens_t;
  uint8   SendState;
  if ((SendCnt & 3) == 2){
    sens_t = getADC(BIN8(0,0,0,1,1,1,1,1));//Refrence internal= 1.15V ; Resolution= 9bit; ref = VDD/3;
    if (sens_t<256)sens_t = 0; else sens_t-=256;//for battery
    pTxData[2] = sens_t & 0xFF; //bat voltage
  };
  pTxData[0] = MY_TYPE + (SendCnt & 0x0F);

  SendCnt++;
   
  if(ack==FALSE)goto  sendnoack;/////////////////////////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!
  SendState  = basicRfSendPacket(MAS_ADDR, pTxData, SEND_LENGTH,TRUE);
  if(SendState == FAILED){
    //pTxData[3] += 16;
    halMcuWaitMs(25);
 sendnoack:
   basicRfSendPacket(MAS_ADDR, pTxData, SEND_LENGTH,FALSE);
  };
}

/*************************************************************************************************
*************************************************************************************************/
static void RF_RecvPacket(void)
{
    uint8 length = 0;
    
    if(basicRfPacketIsReady()){
      tcnt=155;
      if ((length=basicRfReceive(pRxData, RF_PKT_MAX_SIZE, NULL)) > 0) 
      {
          basicRfReceiveOff();
          if ((MAS_LENGTH==length))
          {
              RecvCnt++;
              //halMcuWaitMs(100);
              if(pRxData[1]==0x76){
                TemperatureOut = (pRxData[3] << 8) + pRxData[4];
                CO2Out = (pRxData[0] << 8) + pRxData[2];
              } else 
              if(pRxData[1]==0x77){
                if(period != pRxData[2]){
                period = pRxData[2];
                pTxData[4] = period;
                flash_buf[0] = period;
                flash_page_erase(0,14);
                flash_page_write(0,14,0,flash_buf,4);//bank page off
                }
              } else if (pRxData[1]==0x78){
                if(RF_CHANNEL != pRxData[2]){
                if((pRxData[2]>10)&&(pRxData[2]<26))RF_CHANNEL = pRxData[2];
                flash_buf[1] = RF_CHANNEL;
                flash_page_erase(0,14);
                flash_page_write(0,14,0,flash_buf,4);//bank page off
                halRfSetChannel(RF_CHANNEL);
                }
              } else CMD_DISCTL = pRxData[1];

          }                                  
      }
    }
}
static void InitT3()
{     
      T3CTL |= 0x08 ;      //enable interrupt    
      T3IE = 1;            //IEN1.03 bit - T3 interrupt enable    
      //T3CTL|=0XE0;         //Tick Freq / 128   = 32MHz/128 = 250 kHz  
      //T3CTL|=0X60;         //Tick Freq / 8   = 32MHz/8 = 4 MHz  
      T3CTL|=0X40;         //Tick Freq / 4   = 32MHz/4 = 4 MHz  
      T3CTL &= ~0X03;      //free running 00­>0xff  65200/256=254
      T3CTL |=0X10;        //Start timer
      //EA = 1; // Enable global interrupt
}

#pragma vector = URX0_VECTOR 
  __interrupt void UART0_ISR(void) 
 { 
  uint8 temp;
  URX0IF = 0;    
  temp = U0DBUF;   
  
  U0DBUF = temp;
  while(UTX0IF == 0);
  UTX0IF = 0;
 }

#ifdef UART
static void InitUART(void)
{ 
    PERCFG = 0x00;		      //
    P0SEL = 0x0c;		      //P0_2,P0_3
    P2DIR &= ~0XC0;                   //P0 UART0

    U0CSR |= 0x80;		      //
    U0GCR |= 11;				       
    U0BAUD |= 216;		      //115200
    UTX0IF = 0;                       //UART0 TX
    U0CSR |= 0X40;                    //Enable receiver
    IEN0 |= 0x84;                     //RX interrupt

}
void UartSend_String(char *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}
static void UartSend_Char(char Data){
    U0DBUF = Data;
    while(UTX0IF == 0);
    UTX0IF = 0;
  
}
#endif

/*static uint16 getTemperature(void){ 
   
   uint16  value; 
   ADCCON3  = 0x2E;      //Source for single conv = Temp sensor ; Resolution= 10bit; ref = internal 1.25V
   ADCCON1 |= 0x30;      //ADCCON1.ST(06bit) - starts conversion
   ADCCON1 |= 0x40;      //ADCCON1.ST =1 - Starting coversion
   while(!(ADCCON1 & 0x80));  //wait for end of conversion
   value =  ADCL >> 4;                //
   value |= (((uint16)ADCH) << 4);
   return value;
   //return (value-1367.5)/4.5-5;    
}*/

static uint16 getADC(uint8 reg){ 
   
   uint16  value;
   ADCCON3  =reg;//Source for single conv = VDD5 ; Resolution= 10bit; ref = AIN2(P0_2);
   while(!(ADCCON1 & 0x80));  //wait for end of conversion
   value = (int16) (ADCL);
   value |= (int16) (ADCH << 8);
   value >>= 6; //for 9bit
   return value;
}
//*************************************************************************************************/

#pragma vector = T3_VECTOR 
 __interrupt void T3_ISR(void) // 32MHz/4 = 8Mhz / 256 = 31.25 kHz
 { 
   tcnt++;
 }
#pragma optimize=none
void SetSleepMode(void)
{
  PCON = 1;
  EA = 0;
}






