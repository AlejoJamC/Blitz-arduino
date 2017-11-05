bool status = false;
int pulsador = 0;

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(4, INPUT);

}

void loop() {
  pulsador = digitalRead(4);
  if(pulsador == HIGH){
    status = !status;
  }

  if(status){
    digitalWrite(2, HIGH);       //se enciende el led
    Serial.println("LED Encendido");
  }else{
    digitalWrite(2, LOW);       //se apaga el led
    Serial.println("LED Apagado");
  }
}
