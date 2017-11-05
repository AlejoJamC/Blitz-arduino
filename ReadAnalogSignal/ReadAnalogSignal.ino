float arr[512];
int counter = 0;
void setup() {
  Serial.begin(9600);

}

void loop() {
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  if(counter < 512){
    arr[counter] = voltage;
    counter++; 
    Serial.println(voltage);
    Serial.println(counter);
  } else{
    for(int i = 0; i < 512; i++)
    {
      Serial.print("Objeto ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(arr[i]);
    }
    delay(3000);
  }
  
  
  
  
}
