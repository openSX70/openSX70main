#include "ClickButton.h"
ClickButton::ClickButton(uint8_t buttonPin)
{
  _pin           = buttonPin;
  _activeHigh    = LOW;           // Assume active-low button
  _btnState      = !_activeHigh;  // initial button state in active-high logic
  _lastState     = _btnState;
  _clickCount    = 0;
  clicks         = 0;
  depressed      = false;
  _lastBounceTime= 0;
  debounceTime   = 20;            // Debounce timer in ms
  multiclickTime = 250;           // Time limit for multi clicks
  longClickTime  = 1000;          // time until long clicks register
  changed        = false;
  pinMode(_pin, INPUT);
}

ClickButton::ClickButton(uint8_t buttonPin, boolean activeType)
{
  _pin           = buttonPin;
  _activeHigh    = activeType;
  _btnState      = !_activeHigh;  // initial button state in active-high logic
  _lastState     = _btnState;
  _clickCount    = 0;
  clicks         = 0;
  depressed      = 0;
  _lastBounceTime= 0;
  debounceTime   = 50;            // Debounce timer in ms
  multiclickTime = 250;           // Time limit for multi clicks
  longClickTime  = 1000;          // time until long clicks register
  changed        = false;
  pinMode(_pin, INPUT);
}

ClickButton::ClickButton(uint8_t buttonPin, boolean activeType, boolean internalPullup)
{
  _pin           = buttonPin;
  _activeHigh    = activeType;
  _btnState      = !_activeHigh;  // initial button state in active-high logic
  _lastState     = _btnState;
  _clickCount    = 0;
  clicks         = 0;
  depressed      = 0;
  _lastBounceTime= 0;
  debounceTime   = 20;            // Debounce timer in ms
  multiclickTime = 250;           // Time limit for multi clicks
  longClickTime  = 1000;          // time until "long" click register
  changed        = false;
  pinMode(_pin, INPUT);
  // Turn on internal pullup resistor if applicable
  if (_activeHigh == LOW && internalPullup == CLICKBTN_PULLUP) {
    digitalWrite(_pin,HIGH);
  }
}

void ClickButton::Reset()
{
  _btnState      = !_activeHigh;  // initial button state in active-high logic
  _lastState     = _btnState;
  _clickCount    = 0;
  clicks         = 0;
  depressed      = 0;
  _lastBounceTime= 0;
  changed        = false;

}

void ClickButton::Update()
{
  long now = (long)millis();      // get current time
  _btnState = digitalRead(_pin);  // current appearant button state
  // Make the button logic active-high in code
  if (!_activeHigh) _btnState = !_btnState;
  // If the switch changed, due to noise or a button press, reset the debounce timer
  if (_btnState != _lastState) 
  {
    _lastBounceTime = now;
  }

  // debounce the button (Check if a stable, changed state has occured)
  if (now - _lastBounceTime > debounceTime && _btnState != depressed)
  {
    depressed = _btnState;
    if (depressed) _clickCount++;
  }

  if(_lastState == _btnState) {
    changed = false;
  }
  _lastState = _btnState;
  
  #if (ALPHA == 1) //Not sure if this is working!! 26.10.
    // If the button released state is stable, report nr of clicks and start new cycle
    if (!depressed && (now - _lastBounceTime) > multiclickTime)
    {
      // positive count for released buttons
      clicks = _clickCount;
      //Serial.print("Clickcount = ");
      //Serial.println(clicks); 
      _clickCount = 0;
      if(clicks != 0){
        changed = true;
      //#if SIMPLEDEBUG
        Serial.println(F("Single Click"));
      //#endif
      }
    }
  #endif
  
  // Check for "long click"
  if (depressed && (now - _lastBounceTime > longClickTime))
  {
    // negative count for long clicks
    clicks = 0 - _clickCount;
    //Serial.print("Clickcount = ");
    //Serial.println(clicks); 
    _clickCount = 0;
    if(clicks != 0){
      changed = true;
      //#if SIMPLEDEBUG
        Serial.println(F("Long Click"));
      //#endif
    }
  }
}
