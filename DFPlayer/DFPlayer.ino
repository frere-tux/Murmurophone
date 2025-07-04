#include "Arduino.h"
#include <DFRobotDFPlayerMini.h>

#if (defined(ARDUINO_AVR_UNO))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/11, /*tx =*/10);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

char serialData[4];

const unsigned int volume = 30;

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
  FPSerial.begin(9600);

  Serial.begin(115200);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Playground"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).
//  myDFPlayer.volumeUp(); //Volume Up
//  myDFPlayer.volumeDown(); //Volume Down
  
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
//  myDFPlayer.EQ(DFPLAYER_EQ_POP);
//  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
//  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
//  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
//  myDFPlayer.EQ(DFPLAYER_EQ_BASS);
  
  //----Set device we use SD as default----
//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
  
  //----Mp3 control----
//  myDFPlayer.sleep();     //sleep
//  myDFPlayer.reset();     //Reset the module
//  myDFPlayer.enableDAC();  //Enable On-chip DAC
//  myDFPlayer.disableDAC();  //Disable On-chip DAC
//  myDFPlayer.outputSetting(true, 15); //output setting, enable the output and set the gain to 15
  
  //----Mp3 play----
  // myDFPlayer.next();  //Play next mp3
  // delay(1000);
  // myDFPlayer.previous();  //Play previous mp3
  // delay(1000);
  // myDFPlayer.play(1);  //Play the first mp3
  // delay(1000);
  // myDFPlayer.loop(1);  //Loop the first mp3
  // delay(1000);
  // myDFPlayer.pause();  //pause the mp3
  // delay(1000);
  // myDFPlayer.start();  //start the mp3 from the pause
  // delay(1000);
  // myDFPlayer.playFolder(15, 4);  //play specific mp3 in SD:/15/004.mp3; Folder Name(1~99); File Name(1~255)
  // delay(1000);
  // myDFPlayer.enableLoopAll(); //loop all mp3 files.
  // delay(1000);
  // myDFPlayer.disableLoopAll(); //stop loop all mp3 files.
  // delay(1000);
  // myDFPlayer.playMp3Folder(4); //play specific mp3 in SD:/MP3/0004.mp3; File Name(0~65535)
  // delay(1000);
  // myDFPlayer.advertise(3); //advertise specific mp3 in SD:/ADVERT/0003.mp3; File Name(0~65535)
  // delay(1000);
  // myDFPlayer.stopAdvertise(); //stop advertise
  // delay(1000);
  // myDFPlayer.playLargeFolder(2, 999); //play specific mp3 in SD:/02/004.mp3; Folder Name(1~10); File Name(1~1000)
  // delay(1000);
  // myDFPlayer.loopFolder(5); //loop all mp3 files in folder SD:/05.
  // delay(1000);
  // myDFPlayer.randomAll(); //Random play all the mp3.
  // delay(1000);
  // myDFPlayer.enableLoop(); //enable loop.
  // delay(1000);
  // myDFPlayer.disableLoop(); //disable loop.
  // delay(1000);

  //----Read imformation----
  Serial.println(myDFPlayer.readState()); //read mp3 state
  Serial.println(myDFPlayer.readVolume()); //read current volume
  Serial.println(myDFPlayer.readEQ()); //read EQ setting
  Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
  Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
  //Serial.println(myDFPlayer.readFileCountsInFolder(3)); //read file counts in folder SD:/03
}

void loop()
{
  static unsigned long timer = millis();

  String incomingString = "";
  if (Serial.available() >= 4) 
  {
    int inChar = Serial.readBytes(serialData, 4);
    serialData[inChar];

    incomingString = serialData;
  }

  if (incomingString.length() > 0)
  {
    if (incomingString.toInt() == 8888)
    {
      Serial.println("Enabling loop");
      myDFPlayer.enableLoop();
    }
    else if (incomingString.toInt() == 9999)
    {
      Serial.println("Disabling loop");
      myDFPlayer.disableLoop();
    }
    else
    {

      Serial.print("Playing: ");
      Serial.println(incomingString.toInt());
      myDFPlayer.volume(0);
      myDFPlayer.playMp3Folder(incomingString.toInt());

      const unsigned long endTime = millis() + 500;

      bool validFile = true;

      while (endTime > millis())
      {
          if (myDFPlayer.available() && myDFPlayer.readType() == DFPlayerError && myDFPlayer.read() == FileMismatch)
          {
            validFile = false;
            break;
          }
      }

      myDFPlayer.pause();
      myDFPlayer.volume(volume);

      if (validFile)
      {
        Serial.println("Valid file");
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.println(".");
        Serial.println("Playing");
        myDFPlayer.playMp3Folder(incomingString.toInt());
      }
      else
      {
        Serial.println("Invalid file");
      }
    }
  }
  
  if (myDFPlayer.available()) 
  {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}