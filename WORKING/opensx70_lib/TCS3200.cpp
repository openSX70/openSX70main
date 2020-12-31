#include "open_sx70.h"
#if defined (TCS3200)
  volatile bool integrationFinished = 0;
  
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
    if (iso == ISO_600){
      outputCompare = A600;
    } 
    else if (iso == ISO_SX70){
      outputCompare = A100;
    }
  }

  // Doing this as an alternate to setting individual magic numbers for whatever we need to do.
  // Hopefully won't cost too much as far as time cost while making it easier to configure down the line.
  // EX 29% for fill flash and 50% for auto multiple exposure. 
  void meter_set_iso(const uint16_t& iso, float percent){
      if (iso == ISO_600){
        outputCompare = round(A600 * percent);
      } 
      else if (iso == ISO_SX70){
        outputCompare = round(A100 * percent);
      }
  }
  
  int meter_compute(unsigned int _interval) //Light Meter Helper Compute
  {
    int _myISO = ReadISO();
    static uint32_t previousMillis = 0;
    static bool measuring = false;
    //long interval = 200;
    //uint32_t counter; defined elsewhere
  
    //uint32_t PredExp;
    uint32_t PredExp;
  
    //  uint32_t currentMillis = millis();
    //  uint32_t timeMillis;
    meter_set_iso(_myISO); //set outputcompare Value for the selected ISO -- where the Timer is counting Pulses from Lightsensor to
  
    if (!measuring)
    {
      meter_init();
      previousMillis = millis();
      measuring = true;
    }
    else
    {
      uint32_t myMillis = millis() - previousMillis;
      if (myMillis  >= _interval )
      {
        uint32_t counter = TCNT1; //Timer count Value
        measuring = false;
        PredExp = round((((float)myMillis) / ((float) counter)) * (float)outputCompare);
        PredExp = PredExp + ShutterConstant;
        if(PredExp>40000){ //bigger then a reliable Value | doesnt know if its needed
            #if LMDEBUG
            Serial.println("Exception: PredExp > 44250 ");
            #endif
            return -1;
          }
        return PredExp;
      }
    }
    return -1;
  }
  
  int meter_compute(unsigned int _interval,int _activeISO) //Light Meter Helper Compute uses assigned ISO parameter
    {
      //int _myISO = ReadISO(); //Read ISO from EEPROM
      int _myISO = _activeISO;
      #if LMDEBUG
        Serial.print("Lightmeter Helper compute: Uses this ISO for metering: ");
        Serial.println(_myISO);
      #endif
      static uint32_t previousMillis = 0;
      static bool measuring = false;
      uint32_t PredExp;
      meter_set_iso(_myISO); //set outputcompare Value for the selected ISO -- the Timer is counting Pulses from Lightsensor to this outputcompare Value
      if (!measuring)
      {
        meter_init();
        previousMillis = millis();
        measuring = true;
      }
      else
      {
        uint32_t myMillis = millis() - previousMillis;
        if (myMillis  >= _interval )
        {
          uint32_t counter = TCNT1; //Timer count Value
          measuring = false;
          PredExp = round((((float)myMillis) / ((float) counter)) * (float)outputCompare);
          #if LMHELPERDEBUG
            Serial.print("pr mil: ");
            Serial.print(previousMillis);
            Serial.print(" mil: ");
            Serial.print(myMillis);
            Serial.print(" _interval: ");
            Serial.print(_interval);
            Serial.print(" counter: ");
            Serial.print(counter);
            Serial.print(" output compare: ");
            Serial.print(outputCompare);
            Serial.print(" PredExp: ");
            Serial.println(PredExp);
          #endif
          PredExp = PredExp + ShutterConstant;
          if(PredExp>44250){ //bigger then a reliable Value | doesnt know if its needed
            #if LMDEBUG
            Serial.println("Exception: PredExp > 44250 ");
            #endif
            return -2;
          }
          return PredExp;
        }
      }
      return -1; //return -1 for firstrun(nothing computed only initialized)
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
    //#if ALMDEBUG
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
    #if LMDEBUG
      Serial.print("Timer overflow, Timver value before reset: ");
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

  int predictSlot(int predExpVal, int shutterSpeeds[], int slots, bool sorted) //int slot = nearest(PredictedExposure, ShutterSpeed, 11, false); //Calculate the slot
  {      //Shutterspeeds: 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, 302, 600, 1100,
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
      else if (sorted){
        return idx;
      }
    }
    if(predExpVal<(shutterSpeeds[0]-6)){ //Let the LM Led light Blue and blink Red or light red and blink blue as a warning for Exposure Values out of possible Shutter Speeds 17-3=14
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
      #if LMDHELPEREBUG
      Serial.println("LM Helper OFF ");
      #endif
      digitalWrite(PIN_LED1, LOW);
      digitalWrite(PIN_LED2, LOW);
      return;
    }
    int PredictedExposure;
    int activeISO;
    //original int PredictedExposure = meter_compute(200);
    if((ShutterSpeed[_selector]) == AUTO600)
    {
      activeISO = ISO_600;
    }
    else if((ShutterSpeed[_selector] == AUTO100))
    {
      activeISO = ISO_SX70;
    }
    else{
      activeISO = ReadISO(); //read ISO from EEPROM
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
      int slot = predictSlot(PredictedExposure, ShutterSpeed, 11, false);
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
      else if(slot == 12){ //PredExpValue faster then fastest Shutterspeed
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
