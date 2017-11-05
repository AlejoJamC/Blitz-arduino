/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = 0;

int counter = 0;
int limitTime = 8000;

File audioFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  //Serial.print("Initializing SD card...");//------------------------1

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {//------------------------2
    //Serial.println("Card failed, or not present");//------------------------3
    // don't do anything more:
    //return;//------------------------4
  }

  Serial.println("card initialized.");

   // open a new file and immediately close it:
  Serial.println("Creating audio.txt...");
  audioFile = SD.open("audio.txt", FILE_WRITE);
  audioFile.close();
  // Check to see if the file exists:
  if (SD.exists("audio.txt")) {
    Serial.println("audio.txt exists.");
  } else {
    Serial.println("audio.txt doesn't exist.");
  }
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";

  // read the sensor and send it to the serial monitor:
  //-----------------------------------
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(A0);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits

  //  delay(500);        // delay in between reads for stability
  //----------------------------------
  int sensor = analogRead(A0);
  float sensorvalue;

  sensorvalue = sensor * 5.0 / 1023.0;// This is the voltage reading

  float Ave = (((average - 512) / 1023.0) * 5.0) * 1000 / .124;
  dataString = String(sensorvalue);

  Serial.println(dataString);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  while(counter < limitTime){
    File audioFile1 = SD.open("audio.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (audioFile1) {
      audioFile1.println(dataString);
      audioFile1.close();
      //delay(500);
      // print to the serial port too:
      //    Serial.println(dataString);
      Serial.println(counter);
      counter++;
    }
    // if the file isn't open, pop up an error:
    else {
      //Serial.println("error opening audio.txt");
    } 
  } 
}

