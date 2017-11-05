/*
fastad_lcd_dbuf.ino

Real time Analogue Sampling at 9.4 Khz using interrupts.
This is the double buffering version.
Data is captured in real time.
Data is written to two 512 byte buffers, which when full are written to a wav file.
The sample number is configerable. It is limited to 150000 bytes - around  271 minutes.
(Assuming the sd card can take the file size!)


At the end of the sample period the raw data file can be interrogated to produce a formatted text file.
The data can be displayed graphically on a serial monitor (The monitor must be in a fixed width font)
Lcd is suported - Resampling and alter sample rate buttons implemented

Acknowledgements:
Amanda Ghassaei Sept 2012
http://www.instructables.com/id/Arduino-Audio-Input/

Modified David Patterson 28.6.2104

Equipment:
  Arduino Mega 2560
  LCD Keypad Shield
    http://www.hobbytronics.co.uk/arduino-lcd-keypad-shield
  MicroSD Breakout Board Regulated with Logic Conversion V2
    http://www.hobbytronics.co.uk/microsd-card-regulated-v2?keyword=micro%20sd
  4GB Micro SD Memory Card
    http://www.hobbytronics.co.uk/4gb-microsd
  Microphone pre-amplifier- ac coupled with a potential divider to centre the voltage between Audino 0-5V rail
                            Amanda has published a circuit at the link stated above
                            I designed my own with bass, treble and volume controls
                            -there are plenty of pre-amp designs on the web
Specifications:
  >> Serial, LCD, SD and flash memory support
  Time stamp in Linux format from Serial monitor
  Serial Input can initiate new data capture- command 'again'
  Serial output of data after input request- command 'read'
    output can be stopped with command 'q'
  Basic graph plotted on serial monitor
  Raw data saved in wav compatible file
  Raw data translated to tabulated file- command 'write'
  Repeated 60S time update to eeprom
  LCD dims during time update to eeprom
  PM3 Square wave available for testing- toggle with command 'test'
  Set input 2 high to avoid wait for serial time set
  Serial set at 115200 baud
  Uno use will require the relocation of the time storage bytes (variable ememory)
  
  NB best quality audio is obtained with the input set at mid voltage- 128
  This is important!
  
  Lcd output of basic information
  Lcd adc port specified with variable lcdport
  Lcd button support:
    Select initiates a new sample
    Left alters Sample number- then up +10, down -10, left -1, right +1
 */

#include <Time.h>
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
const unsigned long DEFAULT_TIME = 1403524800; // 12am Jun 23 2014
unsigned long pctime = DEFAULT_TIME;
unsigned long timestart = 0; // used to monitor elapsed time since timestamp
/*
Online Linux Time value: http://www.onlineconversion.com/unix_time.htm
*/

// Function F puts strings into flash memory, saving ram!
#include <Flash.h>
FLASH_STRING(myline,"    0+++++++++++++++64+++++++++++++128+++++++++++++192++++++++++++255 ");

#include <EEPROM.h>
const int ememory = 4090; // uno will have to be 5 concurrent bytes located in available e-memory space

#include <SD.h>
#include <SPI.h>
// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53; // (Or any suitable pin!)
const int defaultselect =53; // mega

File myFile;
File tempfile;
char bufFile[]="/adlog/00112233.wav";
char sdfile[]= "/adlog/00112233.txt";
char filename[]="00112233.txt";
boolean hascard=false;
boolean hasdata , written, aready, writeit;
boolean pwtoggle = false;
unsigned long starttime , endtime, filesize;
float frequency;
float period , interval;

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
const int lcdlight=10; // lcd backlight control port
int lcdport=0; // adc port used by lcs for buttons

int const adport = 1; // Set the port to be used for input!
String temp;

const byte testpin=3; // pwm outpin (Only pwm pins 2-3 available with LCD screen)
const byte inpin=2; // take this high to avoid serial wait
// buf is used to store icoming data and is written to file when full
// 512 bytes is optimized for sdcard
#define BUF_SIZE 512
uint8_t bufa[BUF_SIZE];
uint8_t bufb[BUF_SIZE];
uint16_t bufcount;
byte wavheader[44];

unsigned long readings = 28672; //  initial sample size- kept small to avoid delay- enough to create data to look at
int i;
unsigned long grab,counter;

// Defines for clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Defines for setting register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// alter prescalar here 32, 64, 128
// 32 is the fastest sampling rate
byte prescalar=64;


void setup(){
  //Specimem frequency used for first reading to estimate initial sample duration
  if(prescalar==32) frequency = 38.3829;
  if(prescalar==64) frequency= 19.2300;
  if(prescalar==128) frequency= 9.6182;

  if (chipSelect != defaultselect) { pinMode(defaultselect, OUTPUT); }
Serial.begin(115200); // start serial for output
Serial.flush();
Serial.print(F("Fast A/D\n"));
lcd.begin(16, 2);
// set pw3 to output
pinMode(testpin,OUTPUT);
analogWrite(testpin, 0);
// setup lcdbacklight
pinMode(lcdlight,OUTPUT);
digitalWrite(lcdlight,HIGH);
pinMode(inpin,INPUT);
Serial.println(F("\nSet pin 2 high to avoid serial input wait"));
Serial.println(F("Toggle PWM test wave on PW3 using 'test'"));
// Ram test out of interest
Serial.print(F("Free RAM: "));
Serial.println(FreeRam());
// wavheader setup
// little endian (lowest byte 1st)
wavheader[0]='R';
wavheader[1]='I';
wavheader[2]='F';
wavheader[3]='F';
//wavheader[4] to wavheader[7] size of data + header -8
wavheader[8]='W';
wavheader[9]='A';
wavheader[10]='V';
wavheader[11]='E';
wavheader[12]='f';
wavheader[13]='m';
wavheader[14]='t';
wavheader[15]=' ';
wavheader[16]=16;
wavheader[17]=0;
wavheader[18]=0;
wavheader[19]=0;
wavheader[20]=1;
wavheader[21]=0;
wavheader[22]=1;
wavheader[23]=0;
// wavheader[24] to wavheader[27] samplerate hz
// wavheader[28] to wavheader[31] samplerate*1*1
// optional bytes can be added here
wavheader[32]=1;
wavheader[33]=0;
wavheader[34]=8;
wavheader[35]=0;
wavheader[36]='d';
wavheader[37]='a';
wavheader[38]='t';
wavheader[39]='a';
//wavheader[40] to wavheader[43] sample number

if (SD.begin(chipSelect)) {
  hascard=true;
  aready=true;
  card.init(SPI_FULL_SPEED, chipSelect);
  waitfordate(); // get time stamp
    // Create adlog sub-directory
    if (SD.exists("/adlog") == false) {
    SD.mkdir("/adlog");
    }
  hascard=fileopen();
  }
  if (hascard==false) error("SD Problem");
cli();
startad();
}

void startad(){
// Setup continuous reading of the adc port 'adport' using an interrupt

cli();//disable interrupts
//clear ADCSRA and ADCSRB registers
ADCSRA = 0;
ADCSRB = 0;
ADMUX |= adport;   //set up continuous sampling of analog pin adport 
ADMUX |= (1 << REFS0); //set reference voltage
ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only

// pre scalar to set interrupt frequency:
//ADCSRA |= (1 << ADPS2); // 16 prescalar 72Khz, but what could you do at that rate?
// 128 prescalar - 9.4 Khz sampling
if (prescalar==128) ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
//32 prescaler - 16mHz/32=500kHz - produces 37 Khz sampling
if (prescalar==32) ADCSRA |= (1 << ADPS2) | (1 << ADPS0);
// 64 prescalar produces 19.2 Khz sampling
if(prescalar==64) ADCSRA |= (1 << ADPS2) | (1 << ADPS1);

ADCSRA |= (1 << ADATE); //enable auto trigger
ADCSRA |= (1 << ADIE); 
//ADCSRA |= (1 << ADEN); //enable ADC- works fine, but so does the next line
sbi(ADCSRA,ADEN); // //enable ADC
ADCSRA |= (1 << ADSC); //start ADC measurements on interrupt
writeit=false;
starttime=millis();
sei();//enable interrupts
}  
  
// Interrupt routine *******************************************
// this is the key to the program!!
ISR(ADC_vect) {
  if (counter < readings) {
    if(aready) { // get the new value from analogue port
    bufa[bufcount]=ADCH;
    } else {
    bufb[bufcount]=ADCH;
    }
  counter++; // increment data counter
  bufcount++; // increment buffer counter
    if(bufcount==BUF_SIZE){
      if(writeit==false) {
      bufcount=0;
      aready = ! aready;
      writeit=true; // flag that a write is needed
      } else { // wait for file write to complete
      bufcount--;
      counter--;
      }
    }
  } else {
  // All data collected

  cli();//disable interrupts
  cbi(ADCSRA,ADEN); // disable ADC
  sei();//enable interrupts
  endtime=millis();
  // write the last block
    if (aready){
    tempfile.write(bufa, BUF_SIZE); // write the data block
    } else {
    // initiate block write from B
    tempfile.write(bufb, BUF_SIZE); // write the data block
    }
  Serial.println(F("Logging stopped"));
  period= endtime-starttime;
  frequency = float(readings)/period;
  interval = 1000/frequency;
  tempfile.flush();
  // update wav header
  long datacount=readings;
  long setf=long((frequency*1000)+0.55555555);
  headmod(datacount + 36,4); //set size of data +44-8
  headmod(setf, 24); //set sample rate Hz
  headmod(setf, 28); //set sample rate Hz
  headmod(datacount, 40); // set data size
  tempfile.close();
  // done   
  lcd.clear();
  lcd.print(period/1000,2);
  lcd.print(F("S"));
  lcd.setCursor(9,0);
  lcd.print(frequency,1);
  lcd.print(F("KHz"));
  lcd.setCursor(0,1);
  lcd.print(F("Agn:Sel Altr:Lft"));
  title();
  // setup lcd port- it was disabled when the other port was logging
  cli();
  ADCSRA = 0;
  ADCSRB = 0;
  ADMUX |= lcdport;
  sbi(ADCSRA,ADEN); // //enable lcdport ADC
  sei();
  hasdata=true; // flag the data presence
  }
}
// End Interupt section *******************************************

void loop(){
    if(writeit){
      if (aready){
      tempfile.write(bufb, BUF_SIZE); // write the data block
      } else {
      // initiate block write from buf A
      tempfile.write(bufa, BUF_SIZE); // write the data block
      }
    writeit=false; // the write is done!
    }
  if(hasdata==true) {  
    if (hascard == true) { //update stored time in eeprom- only needed for sd filenames
      if (millis()-timestart >=60000){// every minute 
      timestart=millis();
      digitalWrite(lcdlight,LOW);
      writeeeprom(ememory + 1, now());
      delay(250);
      digitalWrite(lcdlight,HIGH);
      }
    }
    int x=analogRead(lcdport);
      if ((x >599) && (x < 800)){
      hascard=fileopen();
        if (!hascard) error("SD Problem");
      startad();
      }
        if ((x >399) && (x < 600)){
        readings = select(readings/1024) *1024; //update value in KB bytes
                                                // change to bytes
        lcd.print(F("SampleNow="));
        lcd.print(readings/1024);
        lcd.setCursor(0,1);
        lcd.print(F("Agn:Sel Altr:Lft"));
        Serial.print(F("\nSample size now set to "));
        Serial.print(readings/1024);
        Serial.println(F(" Kb"));
        delay(200);
        }
    if (Serial.available() > 0) {
    temp=Serial.readString();
    Serial.print(F("\n > "));
    Serial.println(temp);
      if ((temp=="write")&& (!written)) filewrite(); // call write to file
      if (temp=="test") pwtoggle=togglepw(pwtoggle);
      if (temp=="read") serialout();
      if (temp=="alter") change();
      if (temp=="again"){
      hascard=fileopen();
        if (!hascard) error("SD Problem");
      startad();
      }
    }
  }
}
// End Void Loop section ******************************************

void filewrite(){
Serial.println(F("Writing to file..\n"));
  if (SD.exists(sdfile)) SD.remove(sdfile);
myFile = SD.open(sdfile, FILE_WRITE);
Serial.print(F("File "));
Serial.print(sdfile);
  if (myFile == false) {
  Serial.println(F(" failed to open"));
  } else {
  // get filename copy for lcd
    for(i=7;i<15;i++){ filename[i-7] = sdfile[i]; }
  Serial.println(F(" opened for write:"));
  myFile.println(pad(day(grab),2,"0")+"/"+pad(month(grab),2,"0")+"/"+
  String(year(grab))+" "+pad(hour(grab),2,"0")+":"+pad(minute(grab),2,"0")
  +":"+pad(second(grab),2,"0"));
  lcd.clear();
  lcd.print(F("Writing"));
  lcd.setCursor(0,1);
  lcd.print(F("Value"));
  myFile.print(F("Duration "));
  myFile.print(period,0);
  myFile.print(F(" mS Frequency "));
  myFile.print(frequency,4);
  myFile.print(F(" KHz Interval "));
  myFile.print(interval,3);
  myFile.println(F(" microS"));
  myFile.println();
  myFile.println(F("Count , Value"));
  //open the raw data file
  tempfile = SD.open(bufFile, FILE_READ);
  tempfile.seek(44);
      for (counter=0;counter<readings;counter=counter+BUF_SIZE){      
      tempfile.read(bufa, BUF_SIZE);
        for (i=0;i<BUF_SIZE;i++){
        myFile.print(pad(counter+i,6," "));
        myFile.print(F(" , "));
        myFile.println(bufa[i]);
          if(i%10240==0){ // display 1 in 10*1024
          lcd.setCursor(8,0);
          lcd.print(counter+i);
          lcd.setCursor(8,1);
          lcd.print(bufa[i]);
          lcd.print("  ");
          }
        }
      }
  myFile.flush();
  tempfile.close();
  filesize = myFile.size();
  myFile.close();
  lcd.clear();
  lcd.print(F("File finished"));
  lcd.setCursor(0,1);
  lcd.print(filename);
  Serial.print(F("File size "));
  Serial.print(filesize);
  Serial.println(F(" bytes"));
  title();
  written=true;
  }  
}

void headmod(long value, byte location){
// write four bytes for a long
tempfile.seek(location); // find the location in the file
byte tbuf[4];
tbuf[0] = value & 0xFF; // lo byte
tbuf[1] = (value >> 8) & 0xFF;
tbuf[2] = (value >> 16) & 0xFF;
tbuf[3] = (value >> 24) & 0xFF; // hi byte
tempfile.write(tbuf,4); // write the 4 byte buffer
}

void title(){
Serial.print(F("Duration "));
Serial.print(period,0);
Serial.println(F(" mS"));
Serial.print(F("Frequency "));
Serial.print(frequency,4);
Serial.print(F(" KHz , Measuring interval "));
Serial.print(interval,3);
Serial.println(F(" microS"));
Serial.println(F("\nSend 'write' to write data, 'read' to receive data,\n'again' to re-measure, 'alter' to alter samplesize,\n'test' to toggle PW3 test wave."));
}

unsigned long select(unsigned long myvar) {
// Change the sample number using the lcd buttons
int y;
lcd.clear();
delay(250); // debounce
lcd.setCursor(0,0);
lcd.print("Variable");
lcd.setCursor(0,1);
lcd.print("Button?");
  do {
  lcd.setCursor(10,0);
  lcd.print(myvar);
lcd.print("      ");  
  y = analogRead(lcdport);
  lcd.setCursor(10,1);
    if ((y < 60) && (myvar < 150000)) {
    lcd.print ("+1 "); // right
    myvar = myvar+1;
    }
    else if ((y > 59) &&(y < 200) && (myvar < 149991)) {
    lcd.print ("+10"); // up
    myvar = myvar+10;
    }
    else if ((y > 199 ) && (y < 400) && (myvar>10)) {
    lcd.print ("-10"); // down
    myvar = myvar-10;
    }
    else if ((y > 399) && (y < 600) && (myvar>1)) {
    lcd.print ("-1 "); // left
    myvar = myvar-1;
    }
    else if ((y > 599) && (y < 800)) {  // select
    lcd.clear();
    delay(300); // debounce
    return(myvar);
    }
  delay(125); // debounce
  } while ((y < 600) || (y > 799));
}

void change() {
// Change the sample number using serial port
lcd.clear();
lcd.print(F("Serial port to"));
lcd.setCursor(0,1);
lcd.print(F("alter interval"));
Serial.print(F("\nWaiting for new samplesize in Kb (50K "));
Serial.print(0.050*1024/frequency,3);
Serial.println(F(" seconds)"));
  do {  
  delay(10);
  } while (Serial.available()==0);
  long data = Serial.parseFloat();
    if ((data > 0) && (data < 150001)){
    readings = 1024 * data; // update number
    float var = float(readings)/1000;
    Serial.print(F("Sample size now "));
    Serial.print(data);
    Serial.print(F(" Kb ("));
    Serial.print(readings);
    Serial.println(F(" Bytes )"));
    Serial.print(F("Projected interval "));
    float estimate = var / frequency;
    Serial.print(estimate,3);
    Serial.println(F(" S"));
      if(estimate>60){
      int minutes = estimate / 60;
      float seconds = estimate-60*float(minutes);
      Serial.print(minutes);
      Serial.print(F(" minutes "));
      Serial.print(seconds,3);
      Serial.println(F(" seconds"));
      }
    } else {
    Serial.print(F("No samplesize update- value "));
    Serial.print(data);
    Serial.println(F(" out of range (1-150000 Kb)"));
    }
lcd.clear();
lcd.print(F("Agn:Sel Altr:Lft"));
lcd.setCursor(0,1);
lcd.print(F("write/test"));
}

void serialout() {
// Output raw data to a text file
byte adcapture;
unsigned long lastone;
tempfile = SD.open(bufFile, FILE_READ);
  if (!tempfile) {
  Serial.println(F("Tempfile failed to open"));
  lcd.clear();
  lcd.print(F("File Problem"));
  lcd.setCursor(0,1);
  lcd.print(F("again/alter"));
  } else {
  lcd.clear();
  lcd.print(F("Serial output.."));
  lcd.setCursor(0,1);
  lcd.print(F("Send 'q' to quit"));
  Serial.println(F("\nData display:\n"));
  tempfile.seek(44);
    for (counter=0;counter<readings;counter=counter+BUF_SIZE){      
    tempfile.read(bufa, BUF_SIZE);
        for (i=0;i<BUF_SIZE;i++){
          if ((counter+i)%50==0){
          myline.print(Serial); // print x axis
          Serial.println(counter+i);
          }
        adcapture= bufa[i];
        Serial.print(pad(adcapture,3," "));
        temp=" ";
          for(int j=0;j<adcapture;j=j+4){
          temp=temp+" ";
          }
        temp=temp+"*";
        Serial.println(temp);
        }
    lastone=counter+BUF_SIZE-1;
      if (Serial.available() > 0) {
      temp=Serial.readString();
        if (temp=="q") counter=readings;
      }
    }
  tempfile.close();
  myline.print(Serial);
  Serial.println(lastone);
  Serial.println(F("\nOutput Finished\n"));
  title();
  lcd.clear();
  lcd.print(F("Serial out ok"));
  lcd.setCursor(0,1);
  lcd.print(F("Agn:Sel Altr:Lft"));
  }
}

void waitfordate() {
// look in eeprom memory for a stored time
// Mega EEPROM memory 4k byte
// Uno EEPROM memory 1k byte
// An unchanged eeprom value = 255
  if (EEPROM.read(ememory) == 0) { // 0 indicates time data follows
  starttime = readeeprom(ememory + 1);
    if( starttime > DEFAULT_TIME) pctime=starttime;
  
  Serial.print(F("Previous time "));
  Serial.print(pctime); 
  Serial.print(F(" "));
  serialclock(pctime); // put last time on serial
  Serial.println();
  lcd.setCursor(0,0);
  lcd.print(F("LastTm "));
  lcd.print(pctime);
  lcdonelineClock(pctime); // put last time on lcd
  delay(4000);
  }

lcd.clear();
lcd.setCursor(0,0);
  if (digitalRead(inpin) == LOW) {
  lcd.print(F("Use Serial port"));
  lcd.setCursor(0,1);
  lcd.print(F("to change time"));
  Serial.println(F("Waiting for sync message:"));
  Serial.println(F("Use Send with a T + unix timestamp string"));
  setSyncProvider(requestSync);  // Output bell character for intelligent serial listening device
  starttime=millis();
    do {  
      if (Serial.available()) processSyncMessage();
    delay(10);
    } while ((timeStatus() == timeNotSet)&&(millis()-starttime<15000));
  lcd.clear();
    if (timeStatus()== timeNotSet) setTime(pctime+60); // set to default
  lcdonelineClock(pctime); // put new time on lcd
  serialclock(pctime); // put new time on serial
  delay(4000);
  } else {
  setTime(pctime+60); // set to default
  }
}

void processSyncMessage() { // Interrogate serial input
  if(Serial.find(TIME_HEADER)) {
  pctime = Serial.parseInt();
    if( pctime >= DEFAULT_TIME) { // limited check that the time is after default_time
    setTime(pctime); // Sync Arduino clock to the time received on the serial port
    timestart = millis();
    }
  }
}

time_t requestSync()
{
Serial.write(TIME_REQUEST);
Serial.println();
return 0; // the time will be sent later in response to serial mesg
}

unsigned long readeeprom(int location) { 
// read stored timestamp in eeprom
unsigned long temp = EEPROM.read(location);
temp = temp <<8;
temp = temp + EEPROM.read(location + 1);
temp = temp <<8;
temp = temp + EEPROM.read(location + 2);
temp = temp <<8;
temp = temp + EEPROM.read(location + 3);
return(temp);
}

void writeeeprom(int location, unsigned long memory) { 
// write timestamp to eeprom
cli();
int temp = int(memory  >> 24);
EEPROM.write(location, temp);
memory = memory << 8;
temp = int(memory  >> 24);
EEPROM.write(location + 1, temp);
memory = memory << 8;
temp = int(memory  >> 24);
EEPROM.write(location + 2, temp);
memory = memory << 8;
temp = int(memory  >> 24);
EEPROM.write(location + 3, temp);
EEPROM.write(location -1, 0); // flag the bytes are present
sei();
}

void lcdonelineClock(unsigned long toset){
// digital clock display of the time
lcd.setCursor(0,1);
lcdprintDigits(hour(toset));
lcd.print(F(":"));
lcdprintDigits(minute(toset));
lcd.print(F(":"));
lcdprintDigits(second(toset));
lcd.print(F(" "));
lcdprintDigits(day(toset));
lcd.print(F("/"));
lcdprintDigits(month(toset));
lcd.print(F("/"));
lcd.print(year(toset)-2010); 
}

void printDigits(int digits){
// utility function for digital clock display: prints leading 0
  if(digits < 10) Serial.print('0');
Serial.print(digits);
}

void lcdprintDigits(unsigned long digits){
// utility function for digital clock display: prints leading 0
  if(digits < 10) lcd.print('0');
lcd.print(digits);
}

void serialclock(unsigned long toset){
// digital clock display of the variable toset
printDigits(hour(toset));
Serial.print(":");
printDigits(minute(toset));
Serial.print(":");
printDigits(second(toset));
Serial.print(" ");
printDigits(day(toset));
Serial.print("/");
printDigits(month(toset));
Serial.print("/");
Serial.print(year(toset)); 
Serial.println();  
}

boolean fileopen(){
// Assemble text output filename from grab variable
grab = now();
sdfile[7]=day(grab)/10 +'0';
sdfile[8]=day(grab)%10 +'0';
sdfile[9]=hour(grab)/10 +'0';
sdfile[10]=hour(grab)%10 +'0';
sdfile[11]=minute(grab)/10 +'0';
sdfile[12]=minute(grab)%10 +'0';
sdfile[13]=second(grab)/10 +'0';
sdfile[14]=second(grab)%10 +'0';
// Assemble buffer filename from grab variable
bufFile[7]=day(grab)/10 +'0';
bufFile[8]=day(grab)%10 +'0';
bufFile[9]=hour(grab)/10 +'0';
bufFile[10]=hour(grab)%10 +'0';
bufFile[11]=minute(grab)/10 +'0';
bufFile[12]=minute(grab)%10 +'0';
bufFile[13]=second(grab)/10 +'0';
bufFile[14]=second(grab)%10 +'0';
  if (SD.exists(bufFile)) {SD.remove(bufFile);}
tempfile = SD.open(bufFile, FILE_WRITE | O_TRUNC);
//tempfile = SD.open(bufFile, O_CREAT | O_TRUNC | O_CREAT);
  if (!tempfile) {
  Serial.println(F("Tempfile failed to open"));
  return(false);
  } else{
  tempfile.write(wavheader, 44); // write wav header
  tempfile.seek(44); //set data start
  hasdata=false;
  written=false;
  counter=0;
  bufcount=0;
  float var = float(readings)/1000;
  float estimate = var / frequency;
  lcd.clear();
  lcd.print(F("Fast A/D"));
  lcd.setCursor(0,1);
  lcd.print(F("Logging "));
  lcd.print(estimate,2);
  lcd.print(F("S"));
  Serial.print(F("\nLogging ("));
  Serial.print(estimate,2);
  Serial.println(F(" S)"));
  return(true);
  }
}

String pad(unsigned long myval,int digit,String p){
// pad a number to digit characters with string p
  temp = String(myval);
int k=temp.length();
  for(int j=0;j < (digit-k);j++){
  temp = p + temp;
  }
  return(temp);
}

boolean togglepw(boolean tgl){
// toggle test square wave on pwm
  if(!tgl){
  analogWrite(testpin, 127);
  Serial.println(F("PWM test wave available on PW3"));
  return(true);
  } else {
  analogWrite(testpin, 0);
  Serial.println(F("PWM test wave not available on PW3"));
  return(false);
  }
}

void error(char* s) {
// display error and enter stasus
Serial.print(F("Error "));
Serial.println(s);
lcd.clear();
lcd.print(F("Error"));
lcd.setCursor(0,1);
lcd.print(s);
while(1); // no point in continuing
}

