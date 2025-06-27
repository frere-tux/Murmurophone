#ifndef Phone_h
#define Phone_h

//#define DEBUG_PHONE
#ifdef DEBUG_PHONE
  //#define DEBUG_PHONE_DEBOUNCE
  #define DEBUG_PHONE_PRINT(x)     Serial.print (x)
  #define DEBUG_PHONE_PRINTDEC(x)     Serial.print (x, DEC)
  #define DEBUG_PHONE_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PHONE_PRINT(x)
  #define DEBUG_PHONE_PRINTDEC(x)
  #define DEBUG_PHONE_PRINTLN(x) 
#endif


/* ----- CONFIGURATION ----- */

// Pin connexions
const unsigned int pickUpSwitch = 5;     //S63 11
const unsigned int dialPulsesSwitch = 2; //S63 I
const unsigned int dialingSwitch = 4;    //S63 III 

// Phone numbers
const unsigned short phoneNumberSize = 8;
const unsigned short specialPhoneNumberSize = 2;
const String specialPhoneNumberPrefix = "1";

// Timing
const unsigned int maxToneTime = 15 * 1000;
const unsigned int maxDialingTime = 10 * 1000;
const unsigned int routingTime = 2 * 1000;
const unsigned int ringingTime = 7 * 1000;

// Debounce
const unsigned debounceDialTime = 60;
const unsigned debouncePickUpTime = 10;

/* ------------------------- */


enum PhoneState 
{
    PhoneState_None,
    PhoneState_HungUp,
    PhoneState_PickedUp,
    PhoneState_Dialing,
    PhoneState_Routing,
    PhoneState_Ringing,
    PhoneState_Occupied,
    PhoneState_NotAssigned,
    PhoneState_Connected
};


class Phone 
{
public:
	Phone();
  void Setup();

  PhoneState GetState() const { return m_state; }
  static void GetStateName(const PhoneState _state, String& _stateName);
  void GetStateName(String& _stateName) const;

  void OnEnter(const PhoneState _previousState);
  void Update();
  void OnExit(const PhoneState _newState);

  void SwitchState(const PhoneState _newState);

  void (*OnPhonePickedUpCallback)(void);
  void (*OnPhoneHungUpCallback)(void);
  void (*OnPhoneStartDialCallback)(void);
  void (*OnPhoneNumberDialedCallback)(const String& _numberDialed);
  void (*OnPhoneOccupiedCallback)(void);
  void (*OnPhoneNotAssignedCallback)(void);
  void (*OnPhoneRoutingCallback)(void);
  void (*OnPhoneRingingStartCallback)(void);
  void (*OnPhoneRingingStopCallback)(void);
  void (*OnPhoneConnectedCallback)(void);

protected:
  PhoneState m_state = PhoneState_None;

private:
  void PickUpSwitchUpdate();
  void DialSwitchUpdate();

  static void DialPulseInterrupt();
  void ResetDial();


  // Switches states
  static bool s_isPickedUp;
  static bool s_wasPickedUp;  
  static bool s_isDialing;
  static bool s_wasDialing;

  // Phone Number dialed
  static volatile int unsigned short s_numberCount;
  String m_numberDialed = "";

  // Debounce
  static volatile unsigned long s_lastDialPulseTime;
  unsigned long m_pickUpTimerStart = 0;

  unsigned long m_timer = 0;
};

#endif