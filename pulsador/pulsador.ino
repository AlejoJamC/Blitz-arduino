int pulsador=0;                     //almacena el estado del botón

//** Programa **//
void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);        //declaramos el pin 2 como salida
  pinMode(4, INPUT);           //declaramos el pin 4 como entrada
}

void loop() {
  pulsador = digitalRead(4);   //lee el estado del botón
  if(pulsador==HIGH) {          //si el estado es pulsado
    digitalWrite(2, HIGH);       //se enciende el led
    Serial.println("LED Encendido");
  }
  else{                                   //si el estado es no pulsado
    digitalWrite(2, LOW);       //se apaga el led
    Serial.println("LED Apagado");
  }
}
