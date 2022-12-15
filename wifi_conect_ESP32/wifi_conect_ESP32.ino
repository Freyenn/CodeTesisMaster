//Librerias necesarias
#include "BluetoothSerial.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Time.h>
#include <Wire.h>
#include "DFRobot_SHT20.h"
#include <OneWire.h>
#include "time.h"
#include <Preferences.h>
#include <SoftwareSerial.h>

#define separador ','
#define DS18S20_Pin 15 //DS18S20 Signal pin on digital 2
#define push_b 34
#define verde 25
#define azul 33
#define rojo 26
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#define LED 2
#define RXD2 16
#define TXD2 17
#define MYPORT_TX  (12)
#define MYPORT_RX  (13)

char dats [100];
char ATCOMBUFF [500];
String message = "";
String host = "54.243.5.115";


int temporizador, com;
String tempo, pass, red, btCadena, comu;

String serialDisp = "a0001";
const String passDisp = "Niponas1!";
//char* ssid = "Totalplay-ECA5";
//char* password =  "ECA5A3994GHe4ZYv";
String ssid = "";
String password =  "";

// NTP server to request time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -6 * 3600;
const int   daylightOffset_sec = 0;

unsigned char data[4] = {};
float distance;
float dist_true;
float humd;
float temp;
float temperature;
bool pEstado = false;

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2
DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);
ESP32Time rtc;
BluetoothSerial BT; // Objeto Bluetooth
Preferences preferences;
SoftwareSerial ultrasonico;
//Necesarias para el timer
volatile int interruptCounter;
int totalInterruptCounter;
int minutos = 5;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void getConf() {
  preferences.begin("config", false);
  ssid = preferences.getString("SSID", "");
  password = preferences.getString("Password", "");
  temporizador = preferences.getInt("Temporizador", 10); //Default 10 Minutos
  com = preferences.getInt("Comunicacion", 0); //Default 0 == Wifi

  Serial.println("Configuración Obtenida");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(temporizador);
  Serial.println(com);
  preferences.end();

}

void saveConf(String ssid_conf, String pass_conf, int temp_conf, int com_conf) {
  preferences.begin("config", false);
  preferences.putString("SSID", ssid_conf);
  preferences.putString("Password", pass_conf);
  preferences.putInt("Temporizador", temp_conf);
  preferences.putInt("Comunicacion", com_conf);

  Serial.println("Configuración Guardada");
  preferences.end();
}

void setup() {

  //Definiciones para pines de entrada Salida Digitales
  pinMode(push_b, INPUT);
  pinMode(azul, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode (LED, OUTPUT); //Led BUILT-IN

  //Inicializaciòn de LED RGB Apagado
  digitalWrite(verde, LOW);
  digitalWrite(azul, LOW);

  //Inicializaciòn de comunicaciones UART1, UART2, I2C, etc.
  Wire.begin(21, 22);
  Serial.begin(115200);
  ultrasonico.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  sht20.initSHT20();
  delay(100);
  sht20.checkSHT20();
  //Obtener Configuración inicial de la memoria
  getConf();

  ///AGREGAR selecciòn de WIFI o 4G
  ///AGREGAR Sicronizaciòn del RTC usando el 4G


  //Sincronizar RTC con Tiempo Actual Usando WIFI
  //synchroniseWith_NTP_Time();

  //Inicializaciòn de interrupciòn por timer
  timer = timerBegin(0, 8000, true);//Prescaler de 8000
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 600000 * temporizador, true);
  timerAlarmEnable(timer);

  //Primera Lectura de limpieza
  getDatos();

}

void loop() {
  //Asigna estado de configuraciòn
  if (digitalRead(push_b) == HIGH) {

    pEstado = !pEstado;
    if (pEstado == true) {
      BT.begin();
      Serial.println("El dispositivo Bluetooth está listo para emparejar");
    } else {
      BT.end();
      Serial.println("Bluetooth cerrado");
    }
    delay(200);
  }

  if (pEstado == true) {

    digitalWrite(azul, LOW);
    digitalWrite(verde, LOW);
    digitalWrite(LED, HIGH);

    if (BT.available()) {
      btCadena = BT.readString() ;
      //Envio de datos para visualizar Conf en la APP
      if (btCadena == "1\r\n") {
        getConf();
        sprintf(dats, "%s,%s,%i,%i", ssid.c_str(), password.c_str(), temporizador, com);
        BT.println(dats);
      } else {
        btDecode(btCadena);
        saveConf(ssid, password, tempo.toInt(), comu.toInt());
        getConf();
        //Actualizar Temporizador
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 600000 * temporizador, true);
      }

      Serial.println(btCadena);
    }
    delay(20);
  } else {
    digitalWrite(LED, LOW);
    digitalWrite(azul, LOW); // Encender el LED
    digitalWrite(verde, HIGH); // Encender el LED

    //Lectura de distancia debe ser continua para no tener errores
    readDistance();

    //Envio de datos por temporizador
    if (interruptCounter > 0) {

      portENTER_CRITICAL(&timerMux);
      interruptCounter--;
      portEXIT_CRITICAL(&timerMux);

      totalInterruptCounter++;

      Serial.print("An interrupt as occurred. Total number: ");
      Serial.println(totalInterruptCounter);
      dist_true = distance / 10;
      getDatos();
      ///AGREGAR selecciòn de envio por WI-FI o 4G acontinuaciòn
      if (com == 0) {
        ////WI-FI
        Serial.println("Envio Por WIFI");
        sendData(createJson(serialDisp, passDisp, random(0, 14), random(0, 5), temp, humd, temperature, dist_true, fecha()));
      } else {
        Serial.println("Envio Por 4G");
        senDataCuatroG(createJson(serialDisp, passDisp, random(0, 14), random(0, 5), temp, humd, temperature, dist_true, fecha()));
      }
      //4G


    }
  }
  // Envio de datos al presionar boton para debug--
  /*
    if (digitalRead(push_b)==HIGH){
      dist_true = distance/10;
      getDatos();
      sendData(createJson("a0001","Niponas1!",random(0,14),random(0,5),temp,humd,temperature,dist_true,fecha()));
      }*/
}

///////////////////Inician Funciones///////////////////////////
///Envio de datos por 4G


void senDataCuatroG(String json) {
  char data;

  digitalWrite(azul, HIGH); // Encender el LED
  digitalWrite(verde, HIGH); // Encender el LED
  Serial2.print("AT\r"); //Echo enable
  delay(500);
  while (Serial2.available())
  {
    data = (char)Serial2.read();
    message = message + data;
    //Serial.write((char)data);
  }
  Serial.println(message);
  message = "";

  //Envio de datos HTTP POST 4G
  sprintf(ATCOMBUFF, "AT+CHTTPACT=\"%s\",5000\r\n", host.c_str());
  Serial2.print(ATCOMBUFF);
  delay(500);
  while (message.indexOf("CHTTPACT: REQUEST") == -1) {
    while (Serial2.available())
    {
      data = (char)Serial2.read();
      message = message + data;
      //Serial.write((char)data);
    }

  }
  Serial.println(message);

  if (message.indexOf("CHTTPACT: REQUEST") > -1) {
    Serial.println("OK Funciona");
    message = "";
    sprintf(ATCOMBUFF, "POST http://%s:5000/get_data HTTP/1.1\r\nHost: %s:5000\r\nContent-Type: application/json\r\nContent-Length: %i\r\n\r\n%s\r\n\032\r\n", host.c_str(), host.c_str(), json.length(), json.c_str());
    Serial2.print(ATCOMBUFF); //Echo enable
    delay(500);
    while (Serial2.available())
    {
      data = (char)Serial2.read();
      message = message + data;
      //Serial.write((char)data);
    }
    Serial.println(message);
  }
  digitalWrite(azul, LOW); // Encender el LED
  digitalWrite(verde, HIGH); // Encender el LED
}
///Decodificaciòn de los datos obtenidos por BT//////////////
void btDecode(String Cadena) {
  int inicio = 0;
  int fin = Cadena.indexOf(separador, inicio);
  red = Cadena.substring(inicio, fin);
  Serial.println(red); // Si es un float por ejemplo puedes usar toFloat para convertir la cadena a número.

  inicio = fin + 1;
  fin = Cadena.indexOf(separador, inicio);
  pass = Cadena.substring(inicio, fin);
  Serial.println(pass);

  inicio = fin + 1;
  fin = Cadena.indexOf(separador, inicio);
  tempo = Cadena.substring(inicio, fin);
  Serial.println(tempo);

  inicio = fin + 1;
  fin = Cadena.indexOf(separador, inicio);
  comu = Cadena.substring(inicio, fin);
  Serial.println(comu);

}

/////////Sincronizaciòn de tiempo con servidor NTP////////////////
void synchroniseWith_NTP_Time() {
  initWiFi();
  struct tm timeinfo;
  time_t now;
  digitalWrite(azul, HIGH); // Encender el LED
  digitalWrite(verde, LOW); // Encender el LED
  Serial.print("configTime uses ntpServer ");
  Serial.println(ntpServer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("synchronising time");

  while (timeinfo.tm_year + 1900 < 2000 ) {
    time(&now);                       // read the current time
    localtime_r(&now, &timeinfo);

    delay(100);
    Serial.print(".");
  }
  localtime_r(&now, &timeinfo);
  int yr = timeinfo.tm_year + 1900;
  int mt = timeinfo.tm_mon + 1;
  int dy = timeinfo.tm_mday;
  int hr = timeinfo.tm_hour;
  int mi = timeinfo.tm_min;
  int se = timeinfo.tm_sec;
  rtc.setTime(se, mi, hr, dy, mt, yr);
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.print("\n time synchronsized \n");
  digitalWrite(azul, LOW); // Encender el LED
  digitalWrite(verde, HIGH); // Encender el LED
}

/////////////////////Lectura de datos/////////////////
void getDatos() {
  humd = sht20.readHumidity();
  temp = sht20.readTemperature();
  temperature = getTemp();


  Serial.print(fecha());
  Serial.print(" Temperatura Ambiental:");
  Serial.print(temp, 1);   // Only print one decimal place
  Serial.print("C");
  Serial.print(" Humedad Ambiental:");
  Serial.print(humd, 1);   // Only print one decimal place
  Serial.print("%");
  Serial.print(" Temperatura Sol:");
  Serial.print(temperature);
  Serial.print("°C");
  Serial.print(" Distancia:");
  Serial.print(dist_true);
  Serial.print("cm");
  Serial.println();

}

void readDistance() {
  do {
    for (int i = 0; i < 4; i++)
    {
      data[i] = ultrasonico.read();
    }
  } while (ultrasonico.read() == 0xff);

  ultrasonico.flush();

  if (data[0] == 0xff)
  {
    int sum;
    sum = (data[0] + data[1] + data[2]) & 0x00FF;
    if (sum == data[3])
    {
      distance = (data[1] << 8) + data[2];
      if (distance > 280)
      {
        //Serial.print("distance=");

        //Serial.print(distance);
        //Serial.println("cm");
      }//else
      //{
      //Serial.println("Below the lower limit");
      //}
    }//else //Serial.println("ERROR");
  }
  delay(150);
}

float getTemp() {
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;

}

/////////////////////// Creacion de formato para fecha//////////////////////////
String fecha() {
  return String(rtc.getDay()) + "/" + String(rtc.getMonth() + 1) + "/" + String(rtc.getYear()) + " " + String(rtc.getTime());
}

/////////////////////// Creacion de Json para envio por HTTP POST///////////////////
String createJson(String serial, String contrasena, int ph, int ec, float tempa, float huma, float temps, float nivel, String fecha) {
  return "{\"serial\":\"" + serial + "\", \"contrasena\":\"" + contrasena + "\",\"ph\":\"" + ph +
         "\",\"ec\":\"" + ec + "\",\"temp-a\":\"" + tempa + "\",\"hum-a\":\"" +
         huma + "\",\"temp-s\":\"" + temps + "\",\"nivel\":\"" + nivel + "\",\"fecha\":\"" + fecha + "\"}";
}
/////////////////////////// Crear Conexion WiFi///////////////////////
void initWiFi() {
  
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
}

/////////////////////////////// Conexion HTTP POST para envio de datos ///////////////////////
void sendData(String dataJson) {
  digitalWrite(azul, HIGH); // Encender el LED
  digitalWrite(verde, LOW); // Encender el LED
  Serial.println("Intentando hacer POST");
  initWiFi();
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;

    //http.begin("http://jsonplaceholder.typicode.com/posts");  //Specify destination for HTTP request
    http.begin("http://54.243.5.115:5000/get_data");
    http.addHeader("Content-Type", "application/json");             //Specify content-type header
    // JSON data to send with HTTP POST
    //String httpRequestData = "{\"contrasena\":\"Niponas1!\",\"temp\":\"15\"}";
    Serial.println(dataJson);
    int httpResponseCode = http.POST(dataJson);   //Send the actual POST request

    if (httpResponseCode > 0) {

      String response = http.getString();                       //Get the response to the request

      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer

    } else {

      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);

    }

    http.end();  //Free resources

  } else {

    Serial.println("Error in WiFi connection");

  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  digitalWrite(azul, LOW); // Encender el LED
  digitalWrite(verde, HIGH); // Encender el LED
  delay(500);
}
