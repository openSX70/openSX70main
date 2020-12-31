#include "open_sx70.h"
#if defined (TSL237T)
  volatile bool integrationFinished = 0;
  int _myISO = 0;
  //uint16_t outputCompare = A100;
  uint16_t outputCompare;
  const uint8_t PIN_OE = 9;
  long counter = 0;
  uint32_t startIntTime = 0;
  uint32_t stopIntTime = 0;
  
  void meter_init() {
    lmTimer_init();
  }

  // initialise Timer 1 for light sensor integration.
  void lmTimer_init() {
    pinMode(PIN_OE, OUTPUT); //Output Enable (OE) pin to enable/disable the Lightsensor
    digitalWrite(PIN_OE, LOW); //
    cli(); //stops interrupts
    // Clear all interrupts flags (Timer/Counter Interrupt Flag Register)
    TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);
    // Set timer 1 for normal operation, clocked by rising edge on T1 (port D5 / pin 5)
    TCCR1A = 0; //(Counter1 Control Register A)
    TCCR1B = (1 << CS10) | (1 << CS11) | (1 << CS12); //This Bits set the Prescaler for Timer1, 111 Extrenal Source on T1 Pin, rising edge
    TCCR1C = 0;
    TCNT1 = 0;     // set current value to 0 (needs to be done, maybe, on each new conversion)
        TIMSK1 |= (1 << TOIE1); //Timer Overflow Interrupt activate
    sei();                    // Restart interrupts
  }

  void lmTimer_stop(){ //Stop the Timer1
    #if ADVANCEDEBUG
      Serial.println("Stop Timer");
    #endif
    pinMode(PIN_OE, OUTPUT); //Output Enable (OE) pin to enable/disable the Lightsensor
    digitalWrite(PIN_OE, HIGH); //Turn off LM
    cli();  // One way to disable the timer, and all interrupts

    TCCR1B &= ~(1<< CS12);  // turn off the clock altogether
    TCCR1B &= ~(1<< CS11);
    TCCR1B &= ~(1<< CS10);

    TIMSK1 &= ~(1 << OCIE1A); // turn off the timer interrupt
  }
  
  void meter_set_iso(const uint16_t& iso) { //set the output Compare Value for Timer1 -- Magicnumber for ISO
    if (iso == ISO_600) {
      outputCompare = A600;
    } else if (iso == ISO_SX70) {
      outputCompare = A100;
    }
  }

  void meter_integrate() {
    tsl237_start_integration();
  }
  

  // Start a new measure for pose. Only Autoexposure
  void tsl237_start_integration() {
    cli(); //Stop all Interrupts
    //  TIFR1 = (1 << OCF1A) | (1 << TOV1);
    TIFR1 = (1 << OCF1A);   // Clear interrupts flags we are using
    OCR1A = outputCompare;  // set compare value given sensivity (Magicnumber)
    TCNT1 = 0;  // clear counter value.
    //  TIMSK1 = (1 << OCIE1A) | (1 << TOIE1);  // Set interrupt vectors for compare match A with Overflow
    TIMSK1 = (1 << OCIE1A);   // Set interrupt vectors for compare match A.
    TIMSK1 |= (1 << TOIE1); //Timer Overflow Interrupt activate
    startIntTime = millis();
    sei(); //restart Interrupts
  }
  
  // ISR for complete conversion. Should set a flag read by the main loop. CTC
  ISR(TIMER1_COMPA_vect) {
    TIMSK1 = 0;
    integrationFinished = 1;
    stopIntTime = millis();
    #if LMDEBUG
      Serial.print("Integration finished CTC ");
      Serial.print("Counter1 Time: ");
      Serial.println(TCNT1);
      Serial.print("Exposure Time: ");
      Serial.println(stopIntTime - startIntTime);
    #endif
  }

  ISR(TIMER1_OVF_vect){//Timer overflow
    //#if LMDEBUG
      //Serial.print("Timer overflow, Timver value before reset: ");
      //Serial.print(TCNT1);
    //#endif
      //TCNT1 = 0;             //set Counter to 0
    //#if LMDEBUG
      //Serial.print(" Timer overflow, Timver value after reset: ");
      //Serial.println(TCNT1);
    //#endif
    counter = -1;
    return;
  }

  bool meter_update() {
    if (integrationFinished) {
      integrationFinished = 0;
      return 1;
    }
    return 0;
  }

  int meter_compute(unsigned int _interval,int _activeISO) //Light Meter Helper Compute uses assigned ISO parameter
  {
    //int _myISO = ReadISO(); //Read ISO from EEPROM
    _myISO = _activeISO;
    #if ADVANCEDEBUG
      //Serial.print("Meter Compute: Uses this ISO for metering: ");
      //Serial.println(_myISO);
    #endif
    static uint32_t previousMillis = 0;
    static bool measuring = false;
    uint32_t PredExp;
    meter_set_iso(_activeISO); //Set the correct compare Table for the set ISO
  
    if (!measuring) //Initialize Measuring
    {
      meter_init();
      previousMillis = millis();
      measuring = true;
    }
    else
    {
      uint32_t myMillis = millis() - previousMillis;
      if (myMillis  >= _interval)
      {
        if(counter==-1){
            counter = 65536;
            #if LMDEBUG
              Serial.println("Using Max Value(16bit) on Lightsensor overflow");
            #endif
        }else{
          counter = TCNT1;
        }
        PredExp = round((((float)myMillis) / ((float) counter)) * (float)outputCompare);
        measuring = false; //set measuring to false because the current measure is finished
        #if LMDEBUG
        Serial.print ("pr mil: ");
        Serial.print (previousMillis);
        Serial.print (", mil: ");
        Serial.print (myMillis);
        Serial.print (", _interval: ");
        Serial.print (_interval);
        Serial.print (", counter: ");
        Serial.print (counter);
        Serial.print (", output compare: ");
        Serial.print (outputCompare);
        Serial.print(", PredExp: ");
        Serial.print(PredExp);
        Serial.print(" PredExp+ShutterConstant: ");
        Serial.println(PredExp+ShutterConstant);
        #endif
        PredExp = PredExp + ShutterConstant;
        if(PredExp>44250){ //bigger then a reliable Value | doesnt know if its needed
          #if LMDEBUG
          Serial.println("Exception: PredExp > 44250");
          #endif
          return -2;
        }
        return PredExp; //return -1 for firstrun(nothing computed only initialized)
      }
    }
    return -1; //return -1 on the initial run because the meter has not calculated yet
  }
  
  int nearest(int predExpVal, int shutterSpeeds[], int slots, bool sorted) //int slot = nearest(PredictedExposure, ShutterSpeed, 11, false); //Calculate the slot (
  {
    int idx = 0; // by default near first element
    int distance = abs(shutterSpeeds[idx] - predExpVal);
    for (int i = 1; i < slots; i++)
    {
      int d = abs(shutterSpeeds[i] - predExpVal);
      if (d < distance)
      {
        idx = i;
        distance = d;
      }
      else if (sorted) return idx;
    }
    return idx;
  }

  int predictSlot(int predExpVal, int shutterSpeeds[], int slots, bool sorted) //int slot = nearest(PredictedExposure, ShutterSpeed, 11, false); //Calculate the slot (
  {      //Shutterspeeds: 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, 
    int idx = 0; // by default near first element
    int distance = abs(shutterSpeeds[idx] - predExpVal);
    for (int i = 1; i <= slots; i++)
    {
      int d = abs(shutterSpeeds[i] - predExpVal);
      if (d < distance)
      {
        idx = i;
        distance = d;
      }
      else if (sorted) return idx;
    }
    if(predExpVal<(shutterSpeeds[0]-6)){ //Let the LM Led light Blue and blink Red or light red and blink blue as a warning for Exposure Values out of possible Shutter Speeds
      /*Serial.print("predictedValue is smaller than smallest Shutterspeed");
      Serial.print(" sugested Shutterspeed slot: ");
      Serial.print(shutterSpeeds[idx]);
      Serial.print(" smallest Shutterspeed slot: ");
      Serial.println(shutterSpeeds[0]);
      */
      return -2; //If the predivtedValue is faster than the fastest Shutterspeed with a Margin of one ~Aperture Value
    }
   else if(predExpVal>(shutterSpeeds[slots]+128)){
      /*Serial.print("predictedValue is bigger than biggest Shutterspeed");
      Serial.print(" sugested Shutterspeed slot: ");
      Serial.print(shutterSpeeds[idx]);
      Serial.print(" bigest Shutterspeed slot: ");
      Serial.println(shutterSpeeds[slots]);
      */
      return (slots+1); //If the predivted Exposure Value is slower than the slowest Shutterspeed with a Margin of one ~Aperture Value
    }
    return idx;
  }
  void meter_led(byte _selector, byte _type)
  {
    if (_type == 0) //OFF
    {
      #if LMDEBUG
      Serial.println("LM Helper OFF ");
      #endif
      digitalWrite(PIN_LED1, LOW);
      digitalWrite(PIN_LED2, LOW);
      return;
    }
    int PredictedExposure;
    int activeISO = ReadISO(); //read ISO from EEPROM
    //original int PredictedExposure = meter_compute(200);
    if((ShutterSpeed[_selector]) == AUTO600)
    {
      activeISO = ISO_600;
    }/*
    else if((ShutterSpeed[_selector]) == AUTO600BW)
    {
      activeISO = ISO_600BW;
    }*/else if((ShutterSpeed[_selector] == AUTO100))
    {
      activeISO = ISO_SX70;
    }
      
    PredictedExposure = meter_compute(200,activeISO); //Calculates the estimated Exposure Value

    if (PredictedExposure == -1) //-1 occurs on First run of LM
    {
      return;
    }
    if (_type == 2) //Manual Mode
    {
      //PredictedExposure = meter_compute(200,activeISO);
      //int slot = nearest(PredictedExposure, ShutterSpeed, 11, false); //Calculate the slot (
      int slot = predictSlot(PredictedExposure, ShutterSpeed, 10, false);
      //Shutterspeeds: 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, 
      #if LMHELPERDEBUG
        Serial.print ("PredictedExposure: ");
        Serial.print (PredictedExposure);
        Serial.print (" Estimated SLOT: ");
        Serial.print (slot);
        Serial.print (" Actual SLOT: ");
        Serial.println (_selector);
      #endif
      if(slot == -2){//PredExpValue slower than slowest Shutterspeed
        digitalWrite(PIN_LED2, digitalRead(PIN_LED2) ^ 1); //Blink RED LED
        //digitalWrite(PIN_LED1, HIGH); //maybe blink
        digitalWrite(PIN_LED1, LOW); 
      }
      else if(slot == 11){ //PredExpValue faster then fastest Shutterspeed
        digitalWrite(PIN_LED2, LOW);
        digitalWrite(PIN_LED1, digitalRead(PIN_LED1) ^ 1); //Blink BLUE LED
        //digitalWrite(PIN_LED2, HIGH);
      }
      else
      {
        if (_selector < slot)
        {
          //Serial.println ("_selector < slot");
          digitalWrite(PIN_LED1, HIGH);
          digitalWrite(PIN_LED2, LOW);
          return;
        }
        else if (_selector > slot)
        {
          //Serial.println ("_selector > slot");
          digitalWrite(PIN_LED1, LOW);
          digitalWrite(PIN_LED2, HIGH);
          return;
        }
        else if (_selector == slot)
        {
          //Serial.println ("_selector == slot");
          digitalWrite(PIN_LED1, LOW);
          digitalWrite(PIN_LED2, LOW);
          return;
        }
      }
    }
    else if (_type == 1) //Automode
    {
      #if LMHELPERDEBUG
      Serial.print ("LM Helper PredictedExposure on Auto Mode , PredictedExposure: ");
      Serial.println (PredictedExposure);
      #endif
      if (PredictedExposure > 100)
      {
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, HIGH);
        #if LMDEBUG
          Serial.println ("LOW LIGHT");
        #endif
        return;
      } else
      {
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, LOW);
        return;
      }
    }
  }
#endif
