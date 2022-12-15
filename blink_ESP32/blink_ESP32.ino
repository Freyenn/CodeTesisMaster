//Paso 1
int pin_led = 25;
int push_b = 34;
bool bState = false;
//Paso 2
void setup() {
  Serial.begin(115200);
  pinMode(pin_led,OUTPUT);
  pinMode(push_b,INPUT);
}

//Paso 3
void loop() {
    int push_state = digitalRead(push_b);
    
    if (push_state== HIGH) bState = !bState;


    if (bState) digitalWrite(pin_led,HIGH);
    else digitalWrite(pin_led,LOW);

delay(200);
    
}
