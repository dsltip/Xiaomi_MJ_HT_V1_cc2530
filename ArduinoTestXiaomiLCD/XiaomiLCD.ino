#include <Wire.h>
//#include "SHT3x.h"
//SHT3x Sensor;

#define LCD_SCL  2
#define LCD_SDA  3
#define LCD_CSB  4
#define LCD_INHB 5

#define Set(ADDRESS,BIT) (ADDRESS |= (1<<BIT))
#define Clr(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT))

#define DATA_SEND_GAP 9 //micros
#define LCD_CSB_HIGH_HLD 1
#define LCD_CSB_LOW_HLD  1

#define DATA_MAX 18

  #define LCD_CMD_ICSET   0b11101010
  #define LCD_CMD_ICSET2  0b11101000
  #define LCD_CMD_BLKCTL2 0b11110000 //(P10 - 00 - Blink off)
//#define LCD_CMD_DISCTL  0b10100110 //(P43-00-80Hz,P2-1-LInvers,P10-10-NormPower) - orig
  #define LCD_CMD_DISCTL  0b10111000 //(P43-11-53Hz,P2-0-FInvers,P10-00-Save1)
//#define LCD_CMD_DISCTL2 0b10100111 //(P43-00-80Hz,P2-1-LInvers,P10-11-HighPower)
  #define LCD_CMD_DISCTL2 0b10111000 //(P43-11-53Hz,P2-0-FInvers,P10-00-PSave1)
  #define LCD_CMD_ADSET   0b00000000
  #define LCD_CMD_APCTL2  0b11111100 //(normal,normal)
  #define LCD_CMD_MODSET  0b11001000//(P3 - 1 - Disp on, P2-0-1/3 bias)
  #define LCD_CMD_MODSET2 0b11000000//(P3 - 0 - Disp off, P2-0-1/3 bias)

unsigned char data[18];
int cnt=0;
unsigned char Reg=0;
unsigned char Bit=0;

/*unsigned char b00[5] = {2,3,7,223,192};
unsigned char b01[4] = {3,2,7,192};
unsigned char b02[5] = {2,3,7,189, 192};
unsigned char b03[5] = {2,3,5,  191, 192};
unsigned char b04[5] = {2,3,4,  231, 192};
unsigned char b05[5] = {2,3,5,  255, 128};
unsigned char b06[5] = {2,3,7,  255, 128};
unsigned char b07[4] = {3,2,143,192};
unsigned char b08[5] = {2,3,7,255,192};
unsigned char b09[5] = {2,3,5,255,192};
unsigned char b10[5] = {4,3,61,  252, 128};
unsigned char b16[5] = {4,3,63,  248, 128};
unsigned char b59[5] = {11,3,22,  255, 64};*/

unsigned char bb[240] = {2,7,223,192,2,0,7,192,2,7,189,192,2,5,191,192,2,4,231,192,2,5,255,128,2,7,255,128,2,0,143,192,2,7,255,192,2,5,255,192,4,61,252,128,4,0,116,128,
4,55,220,128,4,23,252,128,4,14,244,128,4,31,248,128,4,63,248,128,4,0,252,128,4,63,252,128,4,31,252,128,5,1,125,244,5,0,0,244,5,1,63,212,5,1,47,244,5,0,106,244,5,1,111,116,
5,1,127,116,5,0,12,244,5,1,127,244,5,1,111,244,7,0,125,252,7,0,0,124,7,0,123,220,7,0,91,252,7,0,78,124,7,0,95,248,7,0,127,248,7,0,8,252,7,0,127,252,7,0,95,252,9,3,223,200,
9,0,7,72,9,3,125,200,9,1,127,200,9,0,239,72,9,1,255,136,9,3,255,136,9,0,15,200,9,3,255,200,9,1,255,200,11,23,223,64,11,0,15,64,11,19,253,64,11,18,255,64,11,6,175,64,11,22,247,
64,11,23,247,64,11,0,207,64,11,23,255,64,11,22,255,64};


//#define last b59

void setup() {
  // put your setup code here, to run once:
/*float f=0;
if(isnan(f))f=5;
*/
//Sensor.Begin();
Wire.begin();
Serial.begin(115200); 
}
void draw(unsigned char p,unsigned char n){
unsigned char c = p*40+n*4;
unsigned char r = bb[c];
c++;
for(int i=0;i<3;i++){data[r]|=bb[c];r++;c++;}
}
void draw_up(int v){
//-99.9..999.9(9999)
unsigned char point=0;
int n=v;
if(v<0){
  data[3]|=0x24;
  n=v*-1;

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
void draw_down(int v){
//-99.9..999.9(9999)
unsigned char point=0;
int n=v;
if(v<0){
  data[8]|=0x42;
  n=v*-1;

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


void sendData(unsigned char data, int half) {    
    unsigned char bits[8] = {0};
    int MAX = (half == 1) ? 3 : 7;
    int i = 0;

    bits[7] = (data & 0b00000001) >> 0;
    bits[6] = (data & 0b00000010) >> 1;
    bits[5] = (data & 0b00000100) >> 2;
    bits[4] = (data & 0b00001000) >> 3;
    bits[3] = (data & 0b00010000) >> 4;
    bits[2] = (data & 0b00100000) >> 5;
    bits[1] = (data & 0b01000000) >> 6;
    bits[0] = (data & 0b10000000) >> 7;

    // for (i=MAX; i >= 0; i--) {
    for (i=0; i <= MAX; i++) {
       digitalWrite(LCD_SDA, bits[i]);//Sets the GPIO level, on or off. 
       digitalWrite(LCD_SCL, HIGH);
       delayMicroseconds(DATA_SEND_GAP);//99
       digitalWrite(LCD_SCL, LOW);
    }
}
void loop() {
  unsigned char i;
  int half = 0;
  // put your main code here, to run repeatedly:
    /* Pin mode set */
    pinMode(LCD_CSB, OUTPUT);
    pinMode(LCD_SDA, OUTPUT);
    pinMode(LCD_SCL, OUTPUT);
    delay(100);//milliseconds

    /* Power ON sequence */
    pinMode(LCD_INHB, OUTPUT);
    digitalWrite(LCD_INHB, 0);
    delay(150);////milliseconds
    digitalWrite(LCD_INHB, 1);
    delayMicroseconds(150);//microseconds
    digitalWrite(LCD_CSB,  1);
    delayMicroseconds(LCD_CSB_HIGH_HLD);//1(min 50ns)    

    /* Initialize sequence */
    digitalWrite(LCD_CSB,  0);
    delayMicroseconds(LCD_CSB_LOW_HLD); //1
    sendData(LCD_CMD_ICSET, 0); //0b11101010 -soft reset(P2 -MSB0, P1-reset, P0 - Int clock)
    sendData(LCD_CMD_MODSET2, 0);//0b11000000(P3 - 0 - Disp off, P2-0-1/3 bias)
    sendData(LCD_CMD_ADSET, 0); //0b00000000
    for(i = 0; i < DATA_MAX; i++) data[i] = 0;
    draw_up(255);
    draw_down(-155);

   for(i = 0; i < DATA_MAX; i++) { //DATA_MAX = 18
        half = (i == DATA_MAX - 1) ? 1 : 0;
        sendData(data[i], half);
        //sendData(0b00010100, half);
    }
    digitalWrite(LCD_CSB,  1);
    delayMicroseconds(LCD_CSB_HIGH_HLD);
    //delay(1000);
    /* DISPON sequence */
    digitalWrite(LCD_CSB,  0);
    delayMicroseconds(LCD_CSB_LOW_HLD);
    sendData(LCD_CMD_DISCTL2, 0); //0b10100111(P43-00-80Hz,P2-1-Invers,P10-11-HighPower)
    sendData(LCD_CMD_BLKCTL2, 0); //0b11110000(P10 - 00 - Blink off)
    sendData(LCD_CMD_APCTL2, 0);  //0b11111100(normal,normal)
    sendData(LCD_CMD_MODSET, 0);  //0b11001000(P3 - 1 - Disp on, P2-0-1/3 bias)
    digitalWrite(LCD_CSB,  1);
    delayMicroseconds(LCD_CSB_HIGH_HLD);
    delay(1000);
    pinMode(LCD_INHB, INPUT);

  
    cnt=0;
    Reg = 2;
    
  while(1){
    //if (Serial.available() > 0) {
  delay(2000);
  
  Wire.requestFrom(0x44, (uint8_t)6);
  uint32_t WaitingBeginTime = millis();
  while ((Wire.available()<6) && ((millis() - WaitingBeginTime) < 5))
      {
        //Do nothing, just wait
      }
  uint8_t datat[6];
  for (uint8_t i = 0; i<6; i++)
        {
          datat[i] = Wire.read();
        }
 uint16_t TemperatureRaw   = (datat[0]<<8)+(datat[1]<<0);
 uint16_t RelHumidityRaw   = (datat[3]<<8)+(datat[4]<<0);
 float _TemperatureCeil = ((float) TemperatureRaw) * 0.00267033 - 45.;
 float _RelHumidity = ((float) RelHumidityRaw) * 0.0015259;

  Serial.print("Temperature: ");
  Serial.println(_TemperatureCeil);
  Serial.print("Humidity: ");
  Serial.println(_RelHumidity);
      
        
       Serial.read();
       Serial.println(cnt);
       for(i = 0; i < 14; i++) data[i] = 0;
       //draw_up(_TemperatureCeil*10);
       //draw_down(_RelHumidity*10);
       draw_up(cnt);
       draw_down(cnt);
       cnt++;

        digitalWrite(LCD_CSB,  0);
        delayMicroseconds(LCD_CSB_LOW_HLD);
        //sendData(LCD_CMD_DISCTL2, 0); //0b10100111(P43-00-80Hz,P2-1-Invers,P10-11-HighPower)
        //sendData(LCD_CMD_BLKCTL2, 0); //0b11110000(P10 - 00 - Blink off)
        //sendData(LCD_CMD_APCTL2, 0);  //0b11111100(normal,normal)
        //sendData(LCD_CMD_MODSET, 0);  //0b11001000(P3 - 1 - Disp on, P2-0-1/3 bias)
        sendData(LCD_CMD_ADSET, 0); //0b00000000
        for(i = 0; i < DATA_MAX; i++) { //DATA_MAX = 18
          half = (i == DATA_MAX - 1) ? 1 : 0;
          sendData(data[i], half);
        }
        digitalWrite(LCD_CSB,  1);

    Wire.beginTransmission(0x44);
  Wire.write(0x2C);//Single_LowRep_ClockStretch
  Wire.write(0x10);
  Wire.endTransmission();
    //};//if serial 
   
    }
        /* DISPOFF sequence */
    // digitalWrite(LCD_CSB,  0);
    // delayMicroseconds(LCD_CSB_LOW_HLD);
    // sendData(LCD_CMD_MODSET2, 0);
    // delayMicroseconds(1000); // Wait for several ms

    /* Disable data transfer */
    // digitalWrite(LCD_CSB,  1);
    // delayMicroseconds(LCD_CSB_HIGH_HLD);
}
