#include <ESP32Time.h>
#define RXD2 17
#define TXD2 16
#define separador ','
#define pwr_key 32                                                                    
ESP32Time rtc; 
String message="";
String host="54.243.5.115";
String json="{\"data\":\"hola\"}";
char ATCOMBUFF [500];
char data;

void setup() {
  // put your setup code here, to run once:
  pinMode(pwr_key,OUTPUT);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  digitalWrite(pwr_key,LOW);
}

void loop() {
  
  if(Serial.available()){
    data=Serial.read();
   message="";
    
   if(data=='A')
    {
      Serial2.print("AT\r"); //Echo enable
      delay(500);
      while(Serial2.available())
      {
        data=(char)Serial2.read();
        message = message + data;
        //Serial.write((char)data);
      }
      Serial.println(message);
    }

    if(data=='B')
    {
      Serial2.print("AT+CGPSINFO\r"); //Echo enable
      delay(500);
      while(Serial2.available())
      {
        data=(char)Serial2.read();
        message = message + data;
        //Serial.write((char)data);
      }
      
      Serial.println(message);
      rtcSincro4G(message);
    }

    if(data=='D')
    {
      Serial.println("Encendiendo 4G");
      digitalWrite(pwr_key,HIGH);
      delay(500);
      digitalWrite(pwr_key,LOW);
    }
    if(data=='F')
    {
      Serial2.print("AT+CPOF\r"); //Echo enable
      delay(500);
      while(Serial2.available())
      {
        data=(char)Serial2.read();
        message = message + data;
        //Serial.write((char)data);
      }
      
      Serial.println(message);
      }
      

    if(data=='E'){
      Serial2.print("\032"); //Echo enable
      delay(500);
      while(Serial2.available())
      {
        data=(char)Serial2.read();
        message = message + data;
        //Serial.write((char)data);
      }
      
      Serial.println(message);
      }

    if(data=='C')
    {
      senDataCuatroG(json);
    }
    
  }
}
///Envio de datos por 4G
void senDataCuatroG(String json){
  message ="";
  //Envio de datos HTTP POST 4G
      sprintf(ATCOMBUFF,"AT+CHTTPACT=\"%s\",5000\r\n",host.c_str());
      Serial2.print(ATCOMBUFF);
      delay(500);
      while(message.indexOf("CHTTPACT: REQUEST")==-1){
          while(Serial2.available())
          {
            data=(char)Serial2.read();
            message = message + data;
            //Serial.write((char)data);
          }
          
      }
      Serial.println(message);
      
      if(message.indexOf("CHTTPACT: REQUEST")>-1){
        Serial.println("OK Funciona");
        message ="";
        sprintf(ATCOMBUFF,"POST http://%s:5000/prueba HTTP/1.1\r\nHost: %s:5000\r\nContent-Type: application/json\r\nContent-Length: %i\r\n\r\n%s\r\n\032\r\n",host.c_str(),host.c_str(),json.length(),json.c_str());
        Serial2.print(ATCOMBUFF); //Echo enable
        delay(500);
        while(Serial2.available())
          {
            data=(char)Serial2.read();
            message = message + data;
            //Serial.write((char)data);
          }
          Serial.println(message);
        }
  }
///Decodificaciòn de los datos obtenidos por BT//////////////
void rtcSincro4G(String Cadena){
  int inicio=0;
  int fin=Cadena.indexOf(separador,inicio);
  String aux,date,tm;
  int year,mes,dia,hr,mn,sg;
  aux = Cadena.substring(inicio, fin); 
  
  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);  
  aux = Cadena.substring(inicio, fin); 
  
  
  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);
  aux = Cadena.substring(inicio, fin); 
  

  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);
  aux = Cadena.substring(inicio, fin); 
  

  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);
  date = Cadena.substring(inicio, fin); 
  //Serial.println(date);
  dia = date.substring(0,2).toInt();
  mes = date.substring(2,4).toInt();
  year = date.substring(4,6).toInt()+2000;
  //Serial.println(year);Serial.println(mes);Serial.println(dia);

  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);
  tm = Cadena.substring(inicio, fin); 
  //Serial.println(tm);
  hr = tm.substring(0,2).toInt()-6;
  mn = tm.substring(2,4).toInt();
  sg = tm.substring(4,6).toInt();
  //Serial.println(hr);Serial.println(mn);Serial.println(sg);


   rtc.setTime(sg, mn, hr, dia, mes,year);
   Serial.println(rtc.getTime("%d/%m/%Y %H:%M:%S"));
  
  }

  String fecha(){
  return String(rtc.getDay()) + "/" + String(rtc.getMonth()+1) + "/" + String(rtc.getYear()) + " " +String(rtc.getTime());
 }
 
