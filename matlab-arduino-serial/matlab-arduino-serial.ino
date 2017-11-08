void setup()
{
  Serial.begin(9600); //abrir puerto serial y estblecer la velocidad en baudios (bps)
  //abrimos los pines como puertos de salida//
  pinMode(LedPin13, OUTPUT);
  pinMode(LedPin10, OUTPUT);
  pinMode(LedPin7, OUTPUT);
  pinMode(LedPin3, OUTPUT);
}
void loop()
{
  if(Serial.available() > 0)
  {
    int i;
    int j;
    for(i = 0; i < 4; i = i + 1)
    {
      for(j = 0; j < 3; j = j + 1)
      {
        incomingbyte[i][j] = Serial.read();   // lee el byte entrante
        Serial.println(incomingbyte[i][j]);
        }
      }
      int tiempo[4];
      int g;
      for(g = 0; g < 4; g = g + 1)
      {
        tiempo[g] = (incomingbyte[g][0] - 48)*100 + (incomingbyte[g][1] - 48)*10 + (incomingbyte[g][2] - 48);
        Serial.println(tiempo[g]);
      }
      digitalWrite(LedPin13, HIGH);//encender el LED
      delay(tiempo[0]*10);
      digitalWrite(LedPin13, LOW);//apagar el LED
      digitalWrite(LedPin10, HIGH);
      delay(tiempo[1]*10);
      digitalWrite(LedPin10, LOW);
      digitalWrite(LedPin7, HIGH);
      delay(tiempo[2]*10);
      digitalWrite(LedPin7, LOW);
      digitalWrite(LedPin3, HIGH);
      delay(tiempo[3]*10);
      digitalWrite(LedPin3, LOW);
    }
  }
