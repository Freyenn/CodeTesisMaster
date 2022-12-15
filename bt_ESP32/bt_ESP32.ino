/*
  Program to control LED (ON/OFF) from ESP32 using Serial Bluetooth
  by Daniel Carrasco -> https://www.electrosoftcloud.com/
*/
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#define LED 26
#define push_b 34 

bool pEstado =false; 
BluetoothSerial BT; // Objeto Bluetooth

void setup() {
  //Definiciones para pines de entrada Salida Digitales
  pinMode(push_b,INPUT);
  pinMode (LED, OUTPUT); //Led BUILT-IN
   Serial.begin(115200);
  /* If no name is given, default 'ESP32' is applied */
  /* If you want to give your own name to ESP32 Bluetooth device, then */
  /* specify the name as an argument SerialBT.begin("myESP32Bluetooth"); */
 
  
}

void loop() {
  if (digitalRead(push_b)==HIGH){
  pEstado = !pEstado;
    if (pEstado == true){
      BT.begin();
      Serial.println("Bluetooth Started! Ready to pair...");
      }
    else {
      BT.end();
      Serial.println("Bluetooth closed");
      }
  delay(200);
  
  }
  if (pEstado==true){
    
      digitalWrite(LED, LOW);
        if (Serial.available())
      {
        BT.write(Serial.read());
      }
      if (BT.available())
      {
        Serial.write(BT.read());
      }
      delay(20);
  }else{
    digitalWrite(LED,HIGH);
    }

  
  
}
