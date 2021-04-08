#include "open_sx70.h"
#if defined (TCS3200)
  volatile bool integrationFinished = 0;

  bool measuring = false;
  uint32_t startMillis;
  uint32_t endMillis;
  
  uint16_t outputCompare = A100;
  
  //LM is pin 5
  #if SONAR
  const uint8_t TCS3200_S1_Pin = 9;
  const uint8_t TCS3200_S3_Pin = 6;
  #endif

  #if ALPHA
  const uint8_t PIN_OE = 9;         //Pin 13 on Meroe2  (PB1-OC1A)
  const uint8_t TCS3200_S1_Pin = 2; //Pin 32 on Meroe2  (PD2-INT0)
  const uint8_t TCS3200_S3_Pin = 6; //Pin 10 on Meroe2 (PD6-AIN0)
  #endif
  
  void meter_init(){
    tcs3200_init();
  }
  
  // initialise Timer 1 for light sensor integration.
  void tcs3200_init(){
    #if ALPHA
    pinMode(PIN_OE, OUTPUT); 
    digitalWrite(PIN_OE, LOW); //LOW = enabled
    #endif
    integrationFinished = 0; //not sure if needed
    //TCS3200_S0_Pin = HIGH(3.3V) Jumper on PCB
    //TCS3200_S1_Pin = On Pin 9 ATMEGA
    //TCS3200_S2_Pin = HIGH(3.3V) Jumper on PCB
    //TCS3200_S3_Pin = On Pin 6 ATMEGA
    //TCS3200_OE_Pin = LOW(GND) on PCB
    
    //pinMode(PIN_OE, OUTPUT); //Output Enable (OE) pin to enable/disable the Lightsensor
    //digitalWrite(PIN_OE, LOW);
    pinMode(TCS3200_S1_Pin, OUTPUT); //Output frequency scaling selection input
    pinMode(TCS3200_S3_Pin, OUTPUT); //Photodiode type selection input
    //S2 (Photodiode type selection pin) & S0 (Output frequency scaling selection pin) should be high,
    // both can be modified via jumper in PCB 
    digitalWrite(TCS3200_S1_Pin, HIGH); //scaling LOW = 20% | HIGH = 100%
    digitalWrite(TCS3200_S3_Pin, LOW); //filter LOW = clear | HIGH = green
    //S2 & S0 should be high can be modified via jumper in PCB 
    //digitalWrite(S1_Pin, HIGH); //scaling LOW = 20% | HIGH = 100%
    //digitalWrite(S3_Pin, LOW); //filter LOW = clear | HIGH = green
  
    cli(); //Stop all Interupts
    TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);   // Clear all interrupts flags
    // Set timer 1(16 Bit) for normal operation, clocked by rising edge on T1 (port D5 / pin 5)
    TCCR1A = 0; //(Counter1 Control Register A)
    TCCR1B = (1 << CS10) | (1 << CS11) | (1 << CS12); //This Bits set the Prescaler for Timer1, 111 Extrenak Source on T1 Pin(=PIN_OE=9), rising edge
    TCCR1C = 0; //(Counter1 Control Register C)
    TCNT1 = 0;  // set current Timer value to 0 (needs to be done, maybe, on each new conversion)
    sei();  // Restart interrupts
  }
  
  void lmTimer_stop(){ //Stop the Timer1
    #if ADVANCEDEBUG
      Serial.println("Stop Timer");
    #endif
    //cli();  // One way to disable the timer, and all interrupts
    TCCR1B &= ~(1<< CS12);  // turn off the clock altogether
    TCCR1B &= ~(1<< CS11);
    TCCR1B &= ~(1<< CS10);
    //TIMSK1 &= ~(1 << OCIE1A); // turn off the timer interrupt
  }
  
  void meter_set_iso(const uint16_t& iso){ //set the output Compare Value for Timer1 -- Magicnumber for ISO
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
        int pred_milli; 
        if(slope == 0){
          pred_milli = 9999;
        }
        else{
          pred_milli = round(float(outputCompare)/float(slope)); 
        }
        
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

  void meter_integrate(){
    tcs3200_start_integration();
  }
  
  bool meter_update(){
    if(integrationFinished){
      integrationFinished = 0;
      return 1;
    }
    return 0;
  }
  
  // Start a new measure for picture Taking.
  void tcs3200_start_integration(){
    cli(); //Stop all Interrupts
    //  TIFR1 = (1 << OCF1A) | (1 << TOV1);
    TIFR1 = (1 << OCF1A);   // Clear interrupts flags we are using
    OCR1A = outputCompare;  // set compare value given sensivity (Magicnumber)
    TCNT1 = 0;  // clear counter value.
    //  TIMSK1 = (1 << OCIE1A) | (1 << TOIE1);  // Set interrupt vectors for compare match A with Overflow
    TIMSK1 = (1 << OCIE1A);   // Set interrupt vectors for compare match A.
    TIMSK1 |= (1 << TOIE1); //Timer Overflow Interrupt activate
    sei(); //restart Interrupts
  }
  
  ISR(TIMER1_COMPA_vect){ // ISR for complete conversion. Should set a flag read by the main loop.
    TIMSK1 = 0;
    integrationFinished = 1;
    //#if ALMHELPERDEBUG
      //Serial.print("Integration finished CTC ");
      //Serial.print("Counter1 Time: ");
      //Serial.println(TCNT1);
    //#endif
    // function / flag.
  }

  volatile unsigned int timer1CounterValue;

  ISR (TIMER1_CAPT_vect)
  {
    timer1CounterValue = ICR1;  
    //Serial.println(ICR1);
    // possibly other stuff
  }

  ISR(TIMER1_OVF_vect){//Timer overflow
    #if LMHELPERDEBUG
      Serial.print(F("Timer overflow, Timver value before reset: "));
      Serial.print(TCNT1);
      TCNT1 = 0;             //set Counter to 0
    #endif
  }

  int nearest(int x, int myArray[], int elements, bool sorted) //estimate the correct Slot for the Estimated Exposure Value
  {
    int idx = 0; // by default near first element
    int distance = abs(myArray[idx] - x);
    for (int i = 1; i < elements; i++)
    {
      int d = abs(myArray[i] - x);
      if (d < distance)
      {
        idx = i;
        distance = d;
      }
      else if (sorted){ 
        return idx;
      }
    }
    return idx;
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
      Serial.print(F(" min: "));
      Serial.println(MinRange[_selector]);
      Serial.print(F(" max "));
      Serial.println(MaxRange[_selector]);
      Serial.print(F("Predictedmillis: "));
      
      Serial.println(predictedMillis);
    #endif
    /*
    if(_type ==2){ // Manual mode
      predictedMillis = predictedMillis + METER_PREDICTION_OFFSET;
      // Within range
      if((predictedMillis <= (ShutterSpeed[_selector] + meterRange)) && (predictedMillis >= (ShutterSpeed[_selector] - meterRange))){
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, HIGH);
        #if LMHELPERDEBUG
          Serial.println(F("Selector within meter range"));
        #endif
        return;
      }
      // Lower speed required
      else if((predictedMillis < (ShutterSpeed[_selector] - meterRange))){
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
    */
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
      else{ //Low light warning off
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, LOW);
        #if LMHELPERDEBUG
          Serial.println(F("Enough Light Detected"));
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
