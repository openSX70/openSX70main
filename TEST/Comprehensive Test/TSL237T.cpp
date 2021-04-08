#include "open_sx70.h"
#if defined (TSL237T)
  volatile bool integrationFinished = 0;
  int _myISO = 0;

  bool measuring = false;

  uint32_t startMillis;
  uint32_t endMillis;
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
      Serial.println(F("Stop Timer"));
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
    } 
    else if (iso == ISO_SX70) {
      outputCompare = A100;
    }
    else{
      outputCompare = iso; //FF Delay Magicnumber as well as      
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
      Serial.print(F("Integration finished CTC "));
      Serial.print(F("Counter1 Time: "));
      Serial.println(TCNT1);
      Serial.print(F("Exposure Time: "));
      Serial.println(stopIntTime - startIntTime);
    #endif
  }

  ISR(TIMER1_OVF_vect){//Timer overflow
    //#if LMDEBUG
      //Serial.print(F("Timer overflow, Timver value before reset: "));
      //Serial.print(TCNT1);
    //#endif
      //TCNT1 = 0;             //set Counter to 0
    //#if LMDEBUG
      //Serial.print(F(" Timer overflow, Timver value after reset: "));
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

int meter_compute(byte _selector,int _activeISO){
    int _myISO = _activeISO;

    if(measuring == false){
      meter_set_iso(_activeISO);
      measuring = true;
      meter_init();
      startMillis = millis();

      #if LMHELPERDEBUG
        Serial.print("Metering started at: ");
        Serial.print(startMillis);
        Serial.println(" ms");
      #endif
    }
    else{
      endMillis = millis();
      uint32_t timeElapsed =  endMillis - startMillis;
      if((timeElapsed) >= METER_INTERVAL){
        uint32_t counter = TCNT1;
        measuring = false;

        float slope = (float(counter)/float(timeElapsed)) + METER_SLOPE_HANDICAP;
        int pred_milli = round(float(outputCompare)/float(slope)); 

        #if LMHELPERDEBUG
          Serial.print("Metering ended at ");
          Serial.print(endMillis);
          Serial.println(" ms");

          Serial.print("Metering time elapsed: ");
          Serial.print(timeElapsed);
          Serial.println(" ms");

          Serial.print("Magic number used: ");
          Serial.println(outputCompare);

          Serial.print("Meter counter ended at ");
          Serial.println(counter);

          Serial.print("Magic number hit at ");
          Serial.print(pred_milli);
          Serial.println(" ms");

          Serial.print("slope: ");
          Serial.println(slope);
        #endif

        /*
        #if LMDEBUG
          Serial.print("PREDMILLI: ");
          Serial.println(pred_milli);
        #endif
        */

        // pred_milli is how many ms the meter will take to reach the set 
        // magic number. Every scene should generally have a linear increase
        // of the counter, therefore we can use basic algebra to extrapolate
        // when the counter will hit the magic number.
        return pred_milli; 
      }
    }
    return -1;
  }
  
  
    void meter_led(byte _selector, byte _type){
    if(_type == 0){ //OFF
      digitalWrite(PIN_LED1, LOW);
      digitalWrite(PIN_LED2, LOW);
      return;
    }

    int predictedMillis;
    int activeISO;

    if((ShutterSpeed[_selector]) == AUTO600){
      activeISO = ISO_600;
    }
    else if((ShutterSpeed[_selector] == AUTO100)){
      activeISO = ISO_SX70;
    }
    else{
      activeISO = ReadISO();
    }

    predictedMillis = meter_compute(_selector, activeISO);

    if(predictedMillis == -1){ // Still measuring!
      return;
    }

    //int meterRange = round(ShutterSpeed[_selector] * MeterRange[_selector]);
    int meterDifference = abs(predictedMillis - ShutterSpeed[_selector]);


    #if LMHELPERDEBUG
      Serial.print(F("meter range at Selector: "));
      Serial.print(_selector);
      Serial.print(F(" "));
      Serial.print(ShutterSpeed[_selector]);
      Serial.print(F(" is "));
      Serial.println(meterRange);
      Serial.print(F("Predictedmillis: "));
      Serial.println(predictedMillis);
    #endif

    if(_type ==2){ // Manual mode
      predictedMillis = predictedMillis + METER_PREDICTION_OFFSET;
      // Within range
      if((predictedMillis <= MaxRange[_selector])  && (predictedMillis >= MinRange[_selector])){
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, HIGH);
        #if LMHELPERDEBUG
          Serial.println(F("Selector within meter range"));
        #endif
        return;
      }
      // Lower speed required
      else if(predictedMillis < MinRange[_selector]){
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, HIGH);
        #if LMHELPERDEBUG
          Serial.println(F("Selector under meter range"));
        #endif
        return;
      }
      // Higher speed needed
      else{
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, LOW);
        #if LMHELPERDEBUG
          Serial.println(F("Selector over meter range"));
        #endif
        return;
      }
    }
    else if(_type == 1){ // Automode
      if(predictedMillis >= METER_AUTO_WARNING){ //Low light warning
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, LOW);
        #if LMHELPERDEBUG
          Serial.println(F("Auto mode low light warning"));
        #endif
      }
      else{ //Low light warning
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, LOW);
        #if LMHELPERDEBUG
          Serial.println(F("Enough light detected"));
        #endif
      }
      /*
      else{
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, HIGH);
        #if LMHELPERDEBUG
          Serial.println("Auto mode Enough light");
        #endif
      }
      */
    }

  }
#endif
