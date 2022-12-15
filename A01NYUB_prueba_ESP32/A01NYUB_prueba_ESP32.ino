
#include <SoftwareSerial.h>

#define RXD2 16
#define TXD2 17
#define MYPORT_TX  (12)
#define MYPORT_RX  (13)
unsigned char data[4]={};
float distance;
#define push_b 34 
SoftwareSerial myPort;
void setup()
{
 Serial.begin(115200);
 myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX);
 pinMode(push_b,INPUT);
}

void loop()
{
 // if (digitalRead(push_b)==HIGH){
    readDistance();
    if (digitalRead(push_b)==HIGH){
        Serial.print("distance=");
           distance = distance/10;
           Serial.print(distance);
           Serial.println("cm");
    }
}

void readDistance(){
do{
     for(int i=0;i<4;i++)
     {
       data[i]=myPort.read();
     }
  }while(myPort.read()==0xff);

  myPort.flush();

  if(data[0]==0xff)
    {
      int sum;
      sum=(data[0]+data[1]+data[2])&0x00FF;
      if(sum==data[3])
      {
        distance=(data[1]<<8)+data[2];
        if(distance>280)
          {
           //Serial.print("distance=");
           //distance = distance/10;
           //Serial.print(distance);
           //Serial.println("cm");
          }else 
              {
                //Serial.println("Below the lower limit");        
              }
      }//else //Serial.println("ERROR");
     }
     delay(150);
}
