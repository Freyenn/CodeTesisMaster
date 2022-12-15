#define separador ','
String btCadena = "15,Buap_Studiantes,1234";
String timer,pass,red;


void setup() {
  Serial.begin(115200);
}

void loop() {
  // Busco la primera coma.
  
  
  btDecode(btCadena);
  
  
  delay(1000);
}

void btDecode(String Cadena){
  int inicio=0;
  int fin=Cadena.indexOf(separador,inicio);
  // El proceso se repetira hasta que no haya mas comas.
  
  // Proceso el campo.
  timer = Cadena.substring(inicio, fin); 
  Serial.println(timer); // Si es un float por ejemplo puedes usar toFloat para convertir la cadena a número.
  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);
  
  red = Cadena.substring(inicio, fin); 
  Serial.println(red);
  inicio = fin+1;
  fin = Cadena.indexOf(separador, inicio);

  pass = Cadena.substring(inicio, fin); 
  Serial.println(pass);
  }
