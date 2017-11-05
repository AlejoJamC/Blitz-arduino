
#include <FileIO.h>

float arr[400];
int counter = 0;

void setup() {
  // Initialize the Bridge and the Serial
  Bridge.begin();
  Serial.begin(9600);
  FileSystem.begin();

  while (!SerialUSB); // wait for Serial port to connect.
  SerialUSB.println("Filesystem Sound Recorder\n");
}


void loop() {
  SerialUSB.println("Read analog signal");
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  SerialUSB.println("ADC");
  if(counter < 400){
    arr[counter] = voltage;
    counter++; 
  } else {
    SerialUSB.println("Time to store values");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    // The FileSystem card is mounted at the following "/mnt/FileSystema1"
    File dataFile = FileSystem.open("/mnt/sda1/datalog.txt", FILE_APPEND);
  
    // if the file is available, write to it:
    if (dataFile) {
      for(int i = 0; i < 512; i++)
      {
        dataFile.println(arr[i]);
      }
      dataFile.close();
      // print to the serial port too:
      memset(arr, 0, sizeof arr);
      counter = 0;
      SerialUSB.println("Block stored");
    }
    // if the file isn't open, pop up an error:
    else {
      SerialUSB.println("error opening datalog.txt");
    }
  
    delay(3000);
    
  }
}
