#include <Wire.h>
#include "DFRobot_SHT20.h"
#include <ESP32Time.h>


/**
 * Hardware Connections:
 * -VCC = 3.3V
 * -GND = GND
 * -SDA = ESP32 GPIO21 
 * -SCL = ESP32 GPIO22
 */
ESP32Time rtc; 
DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

void setup()
{
  Wire.begin(21,22);
  Serial.begin(115200);
  rtc.setTime(15,00,12,8,3,2022);
  // Init SHT20 Sensor
  sht20.initSHT20();
  delay(100);
  Serial.println("Sensor init finish!");

  /**
   * Check the current status information of SHT20
   * Status information: End of battery, Heater enabled, Disable OTP reload
   * Check result: yes, no
   */
  sht20.checkSHT20();
}

void loop()
{
  /**
   * Read the measured data of air humidity
   * Return the measured air humidity data of float type, unit: %
   */
  float humd = sht20.readHumidity();

  /**
   * Read the measured temp data
   * Return the measured temp data of float type, unit: C
   */
  float temp = sht20.readTemperature();

  Serial.print(fecha());
  Serial.print(" Temperatura Ambiental:");
  Serial.print(temp, 1);   // Only print one decimal place
  Serial.print("C");
  Serial.print(" Humedad Ambiental:");
  Serial.print(humd, 1);   // Only print one decimal place
  Serial.print("%");
  Serial.println();

  delay(1000);
}

String fecha(){
  return String(rtc.getDay()) + "/" + String(rtc.getMonth()+1) + "/" + String(rtc.getYear()) + " " +String(rtc.getTime());
 }
