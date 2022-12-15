#include <Preferences.h>

#define LED 2

String ssid = "";
String password =  "";
int temporizador,com;
Preferences preferences;

void setup() {
  pinMode (LED, OUTPUT);
  digitalWrite(LED, LOW);
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  
  getConf();
  saveConf("BUAP_Estudiantes","f85ac21de4",5,1);
  getConf();

  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void saveConf(String ssid_conf,String pass_conf, int temp_conf,int com_conf){
  preferences.begin("config", false);
  preferences.putString("SSID",ssid_conf);
  preferences.putString("Password",pass_conf);
  preferences.putInt("Temporizador",temp_conf);
  preferences.putInt("Comunicacion",com_conf);

  Serial.println("Configuración Guardada");
  preferences.end();
  }

void getConf(){
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
