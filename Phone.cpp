#include "Arduino.h"
#include "Phone.h"


bool Phone::s_isPickedUp = false;
bool Phone::s_wasPickedUp = false;  
bool Phone::s_isDialing = false;
bool Phone::s_wasDialing = false;

volatile int unsigned short Phone::s_numberCount = 0;
volatile unsigned long Phone::s_lastDialPulseTime = 0;

Phone::Phone() 
{
}

void Phone::Setup()
{
  pinMode(dialPulsesSwitch, INPUT_PULLUP);
  pinMode(dialingSwitch, INPUT_PULLUP);
  pinMode(pickUpSwitch, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(dialPulsesSwitch), DialPulseInterrupt, RISING);
}

void Phone::GetStateName(const PhoneState _state, String& _stateName)
{
  switch (_state)
  {
    case PhoneState_None:
      _stateName = "None";
      break;
    case PhoneState_HungUp:
      _stateName = "HungUp";
      break;
    case PhoneState_PickedUp:
      _stateName = "PickedUp";
      break;
    case PhoneState_Dialing:
      _stateName = "Dialing";
      break;
    case PhoneState_Routing:
      _stateName = "Routing";
      break;
    case PhoneState_Ringing:
      _stateName = "Ringing";
      break;
    case PhoneState_Occupied:
      _stateName = "Occupied";
      break;
    case PhoneState_NotAssigned:
      _stateName = "NotAssigned";
      break;
    case PhoneState_Connected:
      _stateName = "Connected";
      break;
    default:
      _stateName = "Unknown";
  }
}

void Phone::GetStateName(String& _stateName) const
{
  GetStateName(m_state, _stateName);
}

void Phone::OnEnter(const PhoneState _previousState)
{
#ifdef DEBUG_PHONE
  String stateName;
  GetStateName(stateName);
  DEBUG_PHONE_PRINT("Phone entering state: ");
  DEBUG_PHONE_PRINTLN(stateName);
#endif

  switch (m_state)
  {
    case PhoneState_None:
      break;

    case PhoneState_HungUp:
      Serial.println("Phone hung up");
      OnPhoneHungUpCallback();
      break;

    case PhoneState_PickedUp:
      Serial.println("Phone picked up");
      m_timer = millis() + maxToneTime;
      OnPhonePickedUpCallback();
      break;

    case PhoneState_Dialing:
      ResetDial();
      m_timer = millis() + maxDialingTime;
      OnPhoneStartDialCallback();
      break;

    case PhoneState_Routing:
      m_timer = millis() + routingTime;
      OnPhoneRoutingCallback();
      break;

    case PhoneState_Ringing:
      m_timer = millis() + ringingTime;
      OnPhoneRingingStartCallback();
      break;

    case PhoneState_Occupied:
      OnPhoneOccupiedCallback();
      break;

    case PhoneState_NotAssigned:
      OnPhoneNotAssignedCallback();
      break;

    case PhoneState_Connected:
      OnPhoneConnectedCallback();
      break;

    default:
      break;
  }
}

void Phone::Update()
{
  PickUpSwitchUpdate();
  DialSwitchUpdate();

  switch (m_state)
  {
    case PhoneState_None:
      if (s_isPickedUp)
      {
        SwitchState(PhoneState_PickedUp);
      }
      else
      {
        SwitchState(PhoneState_HungUp);
      }
      break;

    case PhoneState_HungUp:
      if (s_isPickedUp)
      {
        SwitchState(PhoneState_PickedUp);
      }
      break;

    case PhoneState_PickedUp:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      else if (s_isDialing)
      {
        SwitchState(PhoneState_Dialing);
      }
      else if (millis() > m_timer)
      {
        SwitchState(PhoneState_Occupied);
      }
      break;
      
    case PhoneState_Dialing:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      else
      {
        if (!s_isDialing && s_wasDialing && s_numberCount > 0)
        {
          m_numberDialed += s_numberCount % 10;
          s_numberCount = 0;
          s_lastDialPulseTime = 0;

          DEBUG_PHONE_PRINT("Current number dialed: ");
          DEBUG_PHONE_PRINTLN(m_numberDialed);

          m_timer = millis() + maxDialingTime;
        }

        bool isSpecialNumber = m_numberDialed.length() == specialPhoneNumberSize && m_numberDialed.startsWith(specialPhoneNumberPrefix);
        if (isSpecialNumber || m_numberDialed.length() >= phoneNumberSize)
        {
          Serial.print("Number dialed: ");
          Serial.println(m_numberDialed);

          OnPhoneNumberDialedCallback(m_numberDialed);
        }
        else if (millis() >= m_timer)
        {
          SwitchState(PhoneState_Occupied);
        }
      }
      break;
    
    case PhoneState_Routing:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      else if (millis() >= m_timer)
      {
        SwitchState(PhoneState_Ringing);
      }
      break;

    case PhoneState_Ringing:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      else if (millis() >= m_timer)
      {
        SwitchState(PhoneState_Connected);
      }
      break;

    case PhoneState_Occupied:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      break;

    case PhoneState_NotAssigned:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      break;

    case PhoneState_Connected:
      if (!s_isPickedUp)
      {
        SwitchState(PhoneState_HungUp);
      }
      break;

    default:
      break;
  }
}

void Phone::OnExit(const PhoneState _newState)
{
  switch (m_state)
  {
    case PhoneState_None:

      break;

    case PhoneState_HungUp:

      break;

    case PhoneState_PickedUp:
      break;

    case PhoneState_Dialing:

      break;

    case PhoneState_Routing:

      break;

    case PhoneState_Ringing:
      OnPhoneRingingStopCallback();
      break;

    case PhoneState_Occupied:

      break;

    case PhoneState_NotAssigned:

      break;

    case PhoneState_Connected:

      break;

    default:
      break;
  }
}

void Phone::SwitchState(const PhoneState _newState)
{
  OnExit(_newState);

  const PhoneState previousState = m_state;
  m_state = _newState;

  OnEnter(previousState);
}

void Phone::PickUpSwitchUpdate()
{
  s_wasPickedUp = s_isPickedUp;

  // Debounce
  bool pickUpRead = digitalRead(pickUpSwitch) == LOW;
  if (s_wasPickedUp != pickUpRead)
  {
    if (m_pickUpTimerStart == 0)
    {
      m_pickUpTimerStart = millis();
    }
    else
    {
      if (millis() >= m_pickUpTimerStart + debouncePickUpTime)
      {
        m_pickUpTimerStart = 0;
        s_isPickedUp = pickUpRead;
      }
    }
  }
  else if (m_pickUpTimerStart > 0)
  {
#ifdef DEBUG_REBOUNCE
    DEBUG_PRINT("Debounce PickUp Time ");
    DEBUG_PRINTLN(millis() - m_pickUpTimerStart);
#endif
    m_pickUpTimerStart = 0;
  }
}

void Phone::DialSwitchUpdate()
{
  s_wasDialing = s_isDialing;
  s_isDialing = digitalRead(dialingSwitch) == LOW;
}

void Phone::DialPulseInterrupt()
{
  if (s_isDialing)
  {
    const unsigned long dialPulseTime = millis();

    if (dialPulseTime - s_lastDialPulseTime >= debounceDialTime)
    {
      ++s_numberCount;
      s_lastDialPulseTime = dialPulseTime;
    }
#ifdef DEBUG_DEBOUNCE
    else
    {
      Serial.print("Bounce Dial Pulses time: ");
      Serial.println(dialPulseTime - s_lastDialPulseTime);
    }
#endif
  }
}

void Phone::ResetDial()
{
    s_isDialing = false;
    s_wasDialing = false;  
    s_numberCount = 0;
    s_lastDialPulseTime = 0;
    m_numberDialed = "";
}

