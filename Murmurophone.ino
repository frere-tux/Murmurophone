#include "Arduino.h"
#include "Phone.h"
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

#define DEBUG
#ifdef DEBUG
  //#define DEBUG_DEBOUNCE
  #define DEBUG_PRINT(x)     Serial.print (x)
  #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x) 
#endif

#define SOUND_TONE 0001
#define SOUND_OCCUPIED 0002
#define SOUND_ROUTING 0003
#define SOUND_NOT_ASSIGNED 0004
#define SOUND_RINGING 0005

// Phone
Phone phone;

// Phone numbers
const bool usePhoneNumberPrefix = true;
const String phoneNumberPrefix = "3590";

// DFPlayer
SoftwareSerial softSerial(/*rx =*/11, /*tx =*/10);
#define FPSerial softSerial

DFRobotDFPlayerMini dfPlayer;
void printDetail(uint8_t type, int value);

enum dfPlayerSDState 
{
    Stopped = 0x0200,
    Playing = 0x0201
};

const uint8_t dfVolume = 30;

int indexFileToPlay = 0;

void setup()
{
  FPSerial.begin(9600);
  Serial.begin(115200);

  Serial.println();
  Serial.println(F("Scotel S63 Test"));

  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!dfPlayer.begin(FPSerial, true, true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  dfPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  dfPlayer.volume(dfVolume);  //Volume value [0-31].
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  //----Read DFPlayer information----
  DEBUG_PRINTLN("DFPlayer (" + String(dfPlayer.readState()) + ")"); //read mp3 state
  DEBUG_PRINTLN("Volume: " + String(dfPlayer.readVolume())); //read current volume
  DEBUG_PRINTLN("EQ: " + String(dfPlayer.readEQ())); //read EQ setting
  DEBUG_PRINTLN("File count:" + String(dfPlayer.readFileCounts())); //read all file counts in SD card

  //----Initilize S63----
  
  pinMode(LED_BUILTIN, OUTPUT);

  phone.Setup();
  phone.OnPhonePickedUpCallback = &onPhonePickedUp;
  phone.OnPhoneHungUpCallback = &onPhoneHungUp;
  phone.OnPhoneStartDialCallback = &onPhoneStartDial;
  phone.OnPhoneNumberDialedCallback = &onPhoneNumberDialed;
  phone.OnPhoneRoutingCallback = &onPhoneRouting;
  phone.OnPhoneRingingStartCallback = &onPhoneRingingStart;
  phone.OnPhoneRingingStopCallback = &onPhoneRingingStop;
  phone.OnPhoneOccupiedCallback = &onPhoneOccupied;
  phone.OnPhoneNotAssignedCallback = &onPhoneNotAssigned;
  phone.OnPhoneConnectedCallback = &onPhoneConnected;
}

void onPhonePickedUp()
{
  dfPlayer.playMp3Folder(SOUND_TONE);
}

void onPhoneHungUp()
{
  dfPlayer.pause();
}

void onPhoneStartDial()
{
  dfPlayer.pause();
}

void onPhoneOccupied()
{
  dfPlayer.playMp3Folder(SOUND_OCCUPIED);
}

void onPhoneNotAssigned()
{
  dfPlayer.playMp3Folder(SOUND_NOT_ASSIGNED);
}

void onPhoneRouting()
{
  dfPlayer.playMp3Folder(SOUND_ROUTING);
}

void onPhoneRingingStart()
{
  dfPlayer.playMp3Folder(SOUND_RINGING);
}

void onPhoneRingingStop()
{
  dfPlayer.pause();
}

void onPhoneNumberDialed(const String& _numberDialed)
{
  Serial.print(F("Calling: "));
  Serial.println(_numberDialed);

  const unsigned int numberSize = _numberDialed.length();

  bool validNumber = true;

  if(numberSize != specialPhoneNumberSize && numberSize != phoneNumberSize)
  {
    validNumber = false;
  }
  else if (usePhoneNumberPrefix && numberSize == phoneNumberSize)
  {
    validNumber = _numberDialed.startsWith(phoneNumberPrefix);
  }

  if (validNumber)
  {
    String fileName = _numberDialed;
    if (numberSize == phoneNumberSize)
    {
      fileName = fileName.substring(phoneNumberSize - 4, phoneNumberSize);
    }

    Serial.print("File name: ");
    Serial.println(fileName);

    indexFileToPlay = fileName.toInt();

    dfPlayer.volume(0);
    dfPlayer.playMp3Folder(indexFileToPlay);

    const unsigned long waitEndTime = millis() + 500;

    while (waitEndTime > millis())
    {
        if (dfPlayer.available() && dfPlayer.readType() == DFPlayerError && dfPlayer.read() == FileMismatch)
        {
          validNumber = false;
          break;
        }
    }

    dfPlayer.pause();
    dfPlayer.volume(dfVolume);
  }

  if (validNumber)
  {
    delay(routingDelay);
    phone.SwitchState(PhoneState_Routing);
  }
  else
  {
    delay(routingDelay);
    phone.SwitchState(PhoneState_NotAssigned);
    indexFileToPlay = 0;
  }
}

void onPhoneConnected()
{
  if (indexFileToPlay == 0)
  {
    Serial.println("Phone connected but invalid index file");
    return;
  }

  dfPlayer.playMp3Folder(indexFileToPlay);

  indexFileToPlay = 0;
}

void loop()
{
  phone.Update();

  if (phone.GetState() == PhoneState_Connected && dfPlayer.readType() == DFPlayerPlayFinished)
  {
    delay(callHangingUpDelay);
    phone.SwitchState(PhoneState_Occupied);
  }
  
  if (dfPlayer.available()) 
  {
    printDetail(dfPlayer.readType(), dfPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
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