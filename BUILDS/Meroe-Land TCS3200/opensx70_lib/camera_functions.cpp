#include "Arduino.h"
#include "camera_functions.h"
#include "meter.h"
#include "open_sx70.h"
#include "sx70_pcb.h"
//#include "Clickbutton.h"
#include "settings.h"
#include "uDongle2.h"

extern bool mEXPFirstRun;
extern bool multipleExposureMode;
extern int selector;

int GTD = 0;

Camera::Camera(uDongle *dongle){
  _dongle = dongle;
  //   io_init();
  //  init_EEPROM();
}

#if SONAR
int Camera::getGTD(){
   GTD = digitalRead(PIN_GTD);
   return GTD;  
}

/*For Sonar-FBW where DigitalRead is not working
#if SONAR
int Camera::getGTD() {
  //GTD = 1;
  int val =  10;
  int aGTD[val];
  int dvdGTD = 0;
  //int aGTD = 0;
  for (int i=0; i <= val; i++){
    aGTD[i] = analogRead(PIN_GTD);
  }
  for (int i=0; i <= val; i++){
    if(aGTD[i] >= 310){
      if(aGTD[i]==aGTD[i-1]){
        dvdGTD++;
      }
    }
  }
  #if SIMPLEDEBUG
  Serial.print("dvdGTD: ");
  Serial.println(dvdGTD);
  Serial.print("aGTD[0]: ");
  Serial.println(aGTD[0]);
  #endif
  if(dvdGTD>=(val-1)){
    if(aGTD[0] >= 310){
      GTD = 1;
      //delay(5000);
      Serial.println("GTD True");
      return GTD;
    }else if(aGTD[0] <= 309)
    {
      GTD = 0;
      return GTD;
    }
  }
  //GTD = digitalRead(PIN_GTD);
  return 0;
}
*/

void Camera::S1F_Focus(){
    //int i=0;
    #if FOCUSDEBUG
      Serial.println ("Focus on");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite(PIN_S1F_FBW, HIGH);
    
    /*test(!getGTD());
    {
      i++;
      Serial.println("Wait for GTD to go 1");
      Serial.print("getGTD iteration: ");
      Serial.println(i);
      
      digitalWrite(PIN_S1F_FBW, HIGH);
    }*/
    return;
}

int Camera::S1F_Focus1(){
    #if FOCUSDEBUG
      Serial.println ("Focus on");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite(PIN_S1F_FBW, HIGH);
    int i = 0;
    while(getGTD()!=1){
      i++;
      #if FOCUSDEBUG
        Serial.println("Wait for GTD to go 1");
      #endif
      if(i==40){
        break;
        return 0;
      } 
    }
    return 1;
}

void Camera::S1F_Unfocus(){
    #if FOCUSDEBUG
      Serial.println ("Focus off");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite (PIN_S1F_FBW, LOW);
    return;
}

void Camera::ExposureStart(){
  int i = 0;
  while(S1F_Focus1()!=1){
    i++;
    Serial.println("Wait for GTD");
    if(i>=20){
      break;
    }
  }
  
  //while(getGTD()!=1){ //Not sure if this is nececcary!!!
  //if(GTD==1)
  //  break;
  //S1F_Focus();

  //Serial.println("getGTD");
  //delay(1000);
  //}
  
  return;
  //delay(200);
}
#endif

void Camera::SelfTimerMUP(){
    #if BASICDEBUG
      Serial.println("Selftimer preMirror Up");
    #endif
    Camera::mirrorUP();
}

void Camera::shutterCLOSE(){
  #if BASICDEBUG
    Serial.println ("shutterCLOSE");
  #endif
  Camera::HighSpeedPWM();
  analogWrite(PIN_SOL1, 255);
  delay (PowerDownDelay);
  analogWrite (PIN_SOL1, PowerDown);
  return;
}

void Camera::shutterOPEN(){
  #if BASICDEBUG
    Serial.println ("shutterOPEN");
  #endif
  analogWrite (PIN_SOL1, 0);
  return; //Added 26.10.
}

void Camera::motorON(){
  #if BASICDEBUG
    Serial.println("motorON");
  #endif
  digitalWrite(PIN_MOTOR, HIGH);
}

void Camera::motorOFF(){
  #if BASICDEBUG
    Serial.println ("motorOFF");
  #endif
  digitalWrite(PIN_MOTOR, LOW);
}

void Camera::mirrorDOWN(){
  #if BASICDEBUG
    Serial.println ("mirrorDOWN");
  #endif
  Camera::motorON();
  while (Camera::DebouncedRead(PIN_S5) != LOW){
    #if BASICDEBUG
      Serial.println("Wait for PIN_S5 to go LOW");
    #endif
  }
  motorOFF();
  
}

void Camera::mirrorUP(){
  #if BASICDEBUG
    Serial.println ("mirrorUP");
  #endif
  motorON ();

  while (DebouncedRead(PIN_S5) != HIGH){
    #if BASICDEBUG
      Serial.println("Wait for S5 to go low");
    #endif
  }

  motorOFF ();
}

void Camera::darkslideEJECT(){
  #if SIMPLEDEBUG
    Serial.println ("darkslideEJECT");
  #endif
  Camera::shutterCLOSE();
  Camera::mirrorUP();
  Camera::mirrorDOWN();
  Camera::shutterOPEN();
}

void Camera::DongleFlashNormal(){
  pinMode(PIN_S2, OUTPUT);
  digitalWrite(PIN_SOL2, LOW);      //So FFA recognizes the flash as such
  digitalWrite(PIN_FF, HIGH);       //FLASH TRIGGERING
  delay (1);                        //FLASH TRIGGERING
  digitalWrite(PIN_FF, LOW);        //FLASH TRIGGERING
  pinMode(PIN_SOL2, INPUT_PULLUP);  //S2 back to dongle mode
}

/*
void DongleFlashF8()
{ 
    #if SIMPLEDEBUG
    Serial.println ("DONGLE FLASH F8");
    #endif
    //                 byte PictureType = 4;
    //                 CurrentPicture = EEPROM.read(4) ;
    //
    //                 eepromUpdate ();
    //  if (takePicture == true)
    {
     byte PictureType = 6;
    //    eepromUpdate ();
     //         HighSpeedPWM ();
     //         analogWrite(Solenoid2, 255);
     Camera::shutterCLOSE ();
     mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
     ///////while (digitalRead(S3) != HIGH)            //waiting for S3 to OPEN
     while (DebouncedRead(S3) != HIGH)            //waiting for S3 to OPEN
       ;
     //         analogWrite (Solenoid2, 130);
    delay     (YDelay);                               //S3 is now open start Y-delay (40ms)
     shutterOPEN ();
     //                  delay (66);
     delay (80);
     Write_DS2408_PIO (7, 1); // this is for dongle (jack flash)
     //                  digitalWrite(FFA, HIGH); //this is for in-camera flash
     delay (1);
     //                  analogWrite (Solenoid2,0);
     //                  digitalWrite(FFA, LOW);
     Write_DS2408_PIO (7, 0);
     delay (10u);
     shutterCLOSE();
     delay (500);
     delay (200);                             //AGAIN is this delay necessary?
     mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
     delay (200);                             //AGAIN is this delay necessary?
     shutterOPEN();
     mxshots = 0;
     return;
    }
}*/

void Camera::Ydelay (){
  delay (120);
}

bool Camera::DebouncedRead(uint8_t pin){
  pinMode(pin, INPUT_PULLUP); // GND
  bool lastState = digitalRead(pin);
  uint8_t stablecount = 0;
  while (stablecount < DEBOUNCECOUNT){
    delay(1);
    bool thisState = digitalRead(pin);
    if (thisState == lastState)
      stablecount++;
    else
      stablecount = 0;
    lastState = thisState;
  }
  return lastState;
}

void Camera::HighSpeedPWM(){
  const byte n =224;      // for example, 71.111 kHz
  //PWM high speed
  //one N_Mosfet powerdown
  //taken from: https://www.gammon.com.au/forum/?id=11504
  /*
    Timer 0
    input     T0     pin  6  (D4)
    output    OC0A   pin 12  (D6)
    output    OC0B   pin 11  (D5)

    Timer 1
    input     T1     pin 11  (D5)
    output    OC1A   pin 15  (D9)
    output    OC1B   pin 16  (D10)

    Timer 2
    output    OC2A   pin 17  (D11)
    output    OC2B   pin  5  (D3)
  */
  TCCR2A = bit (WGM20) | bit (WGM21) | bit (COM2B1); // fast PWM, clear OC2A on compare
  TCCR2B = bit (WGM22) | bit (CS20);                 // fast PWM, no prescaler
  OCR2A =  n;                                        // Value to count to - from table
  OCR2B = ((n + 1) / 2) - 1;                         // 50% duty cycle
  //THIS AFFECTS OUTPUT 3 (Solenoid1) AND OUTPUT 11 (Solenoid2)
}

void Camera::BlinkTimerDelay(byte led1, byte led2, byte time) {
  // DONGLE-LED BLINKS ON COUNTDOWN (10secs)
  // this is a test function to do the progressing blinking of the LED using my blink function
  // it last exactly 10 seconds (2500x4) and I could not accomplish this with the delay()
  // everytime the led (in pin 5) blinks faster 1000, 700, 400, and 100.
  //uint32_t startTimer = millis();
  //*******************************************************
  uint32_t steps = (time * 1000) / 4;
  // DS2408 and DONGLE LED BLINK
  /*
  #if SONAR
    S1F_Unfocus(); //Camera Unfocus and start Focus again an almost end of the SelfTimer
  #endif
  Camera::Blink (1000, steps, led1, PIN_LED2, 2);
  Camera::Blink (600, steps, led1, PIN_LED2, 2);
  Camera::Blink (200, steps, led1, PIN_LED2, 2);
  steps = steps / 2;
  #if SONAR
    S1F_Focus();
  #endif
  Camera::Blink (80, steps, led1, PIN_LED2, 2);
  Camera::Blink (80, steps, led2, PIN_LED1, 2);
  }
  */

  // DS2408 LED BLINK
  #if SONAR
    S1F_Unfocus(); //Camera Unfocus and start Focus again an almost end of the SelfTimer
  #endif
  Camera::Blink (1000, steps, led1, 2);
  Camera::Blink (600, steps, led1, 2);
  Camera::Blink (200, steps, led1, 2);
  steps = steps / 2;
  #if SONAR
    S1F_Focus();
  #endif
  Camera::Blink (80, steps, led1, 2);
  Camera::Blink (80, steps, led2, 2);
}


// blink (blink interval=blinking speed, timer=duration blinking, Pin=pin of LED
//type 1 = ONBOARD LED
//type 2 = DONGLE LED

void Camera::Blink(unsigned int interval, int timer, int Pin, byte type){
  int ledState = LOW;             // ledState used to set the LED
  pinMode(Pin, OUTPUT);
  uint32_t previousMillis = 0;        // will store last time LED was updated
  uint32_t currentMillisTimer = millis();
  while (millis() < (currentMillisTimer + timer)){
    uint32_t currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // if the LED is off turn it on and vice-versa:
      if (ledState == 0){
        ledState = 1;
      } 
      else{
        ledState = 0;
      }
      // set the LED with the ledState of the variable:
      if (type == 1){
        digitalWrite (Pin, ledState);
      } 
      else if (type == 2){
        _dongle->Write_DS2408_PIO (Pin, ledState);
      }
    }
  }
}

void Camera::Blink (unsigned int interval, int timer, int PinDongle, int PinPCB, byte type){
  int ledState = LOW;             // ledState used to set the LED
  pinMode(PinDongle, OUTPUT);
  pinMode(PinPCB, OUTPUT);
  uint32_t previousMillis = 0;        // will store last time LED was updated
  uint32_t currentMillisTimer = millis();
  while (millis() < (currentMillisTimer + timer)){
    uint32_t currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // if the LED is off turn it on and vice-versa:
      if (ledState == 0) {
        ledState = 1;
      } 
      else {
        ledState = 0;
      }
      // set the LED with the ledState of the variable:
      if (type == 1) {
        //Serial.println ("TYPE 1 - PCB Only");
        digitalWrite (PinPCB, ledState);
      }  
      else if (type == 2) {
        //Serial.println ("TYPE 2 - PCB and DONGLE");
        digitalWrite (PinPCB, ledState);
        _dongle->Write_DS2408_PIO (PinDongle, ledState);
      }
    }
  }
}


void Camera::ManualExposure(){
  //changed sonar compile check
  #if SONAR
  Camera::ExposureStart();
  #endif

  #if SIMPLEDEBUG
    Serial.print("take single Picture on  Manual Mode");
    Serial.print(", current Picture: ");
    Serial.println(currentPicture);
  #endif

  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){            //waiting for S3 to OPEN˚
     #if BASICDEBUG
     Serial.println("waiting for S3 to OPEN");
     #endif
  }
  delay (YDelay);

  int ShutterSpeedDelay = ((ShutterSpeed[selector]) + ShutterConstant);
  if (selector >= 6){
    ShutterSpeedDelay = (ShutterSpeedDelay - flashDelay);
  }
  #if ADVANCEDEBUG
    extern int selector;
    Serial.print("Manual Exposure Debug: ");
    Serial.print("ShutterSpeed[");
    Serial.print(selector);
    Serial.print("] :");
    Serial.println(ShutterSpeed[selector]);
    Serial.print("ShutterConstant:");
    Serial.println(ShutterConstant);
    Serial.print("ShutterSpeedDelay:");
    Serial.println(ShutterSpeedDelay);
  #endif
  
  Camera::shutterOPEN();
  uint32_t initialMillis = millis();
  while (millis() <= (initialMillis + ShutterSpeedDelay)){
    //Take the Picture
  }
  if (selector >= 3){
    #if SIMPLEDEBUG
        Serial.println("FF - Fill Flash");
    #endif
    Camera::FastFlash ();
  }
  #if LMDEBUG
    uint32_t shutterCloseTime = millis(); //Shutter Debug
  #endif
  Camera::ExposureFinish();
  #if LMDEBUG
      uint32_t exposureTime = shutterCloseTime - initialMillis; //Shutter Debug
      Serial.print("ExposureTime on Manualmode: ");
      Serial.println(exposureTime);
  #endif
  return; //Added 26.10.
}

/*
  Might add another argument to this function. May want to change how varied the shutter is 
  based on the selector. Smaller vs larger ranges.
*/
void Camera::VariableManualExposure(int _myISO){

  #if SONAR
  Camera::ExposureStart();
  #endif

  #if SIMPLEDEBUG
    Serial.print("take single Picture on  Manual Mode");
    Serial.print(", current Picture: ");
    Serial.println(currentPicture);
  #endif

  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){            //waiting for S3 to OPEN˚
     #if BASICDEBUG
     Serial.println("waiting for S3 to OPEN");
     #endif
  }
  delay (YDelay);

  int ShutterSpeedDelay = ShutterSpeed[selector];
  int MinShutterSpeedDelay = (ShutterSpeedDelay - 15);
  
  #if ADVANCEDEBUG
    extern int selector;
    Serial.print("Manual Exposure Debug: ");
    Serial.print("ShutterSpeed[");
    Serial.print(selector);
    Serial.print("] :");
    Serial.println(ShutterSpeed[selector]);
    Serial.print("ShutterConstant:");
    Serial.println(ShutterConstant);
    Serial.print("ShutterSpeedDelay:");
    Serial.println(ShutterSpeedDelay);
  #endif

  meter_set_iso(_myISO);
  meter_init();
  meter_integrate();

  uint32_t initialMillis = millis();
  Camera::shutterOPEN();
  delay(MinShutterSpeedDelay);
  while(meter_update() == false){
    if(millis() >= (initialMillis + ShutterSpeedDelay)){
      break;
    }
  }

  #if LMDEBUG
    uint32_t shutterCloseTime = millis(); //Shutter Debug
  #endif
  Camera::ExposureFinish();
  #if LMDEBUG
      uint32_t exposureTime = shutterCloseTime - initialMillis; //Shutter Debug
      Serial.print("ExposureTime on Manualmode: ");
      Serial.println(exposureTime);
  #endif
  return; //Added 26.10.
}

void Camera::AutoExposure(int _myISO){
  #if SONAR
  Camera::ExposureStart();
  #endif

  #if SIMPLEDEBUG
    Serial.print("take a picture on Auto Mode with ISO: ");
    Serial.print(_myISO);
    Serial.print(", current Picture: ");
    Serial.println(currentPicture);
  #endif

  meter_set_iso(_myISO); 

  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){            //waiting for S3 to OPEN˚
     #if BASICDEBUG
     Serial.println("waiting for S3 to OPEN");
     #endif
  }
  delay(YDelay);

  meter_init();
  meter_integrate();
  Camera::shutterOPEN();
  #if LMDEBUG
    uint32_t shutterOpenTime = millis(); //Shutter Debug
  #endif
  while (meter_update() == false){
  }
  #if LMDEBUG
    uint32_t shutterCloseTime = millis(); //Shutter Debug
  #endif

  Camera::ExposureFinish();

  #if LMDEBUG
    uint32_t exposureTime = shutterCloseTime - shutterOpenTime; //Shutter Debug
    Serial.print("ExposureTime on Automode: ");
    Serial.println(exposureTime);
  #endif
  return; //Added 26.10.
}

void Camera::AutoExposureFF(int _myISO){
  #if SONAR
  Camera::ExposureStart();
  #endif
  #if SIMPLEDEBUG
      Serial.print("take a picture on Auto Mode + Fill Flash with ISO: ");
      Serial.print(_myISO);
      Serial.print(", current Picture: ");
      Serial.println(currentPicture);
  #endif
  Camera::shutterCLOSE();
  Camera::mirrorUP();   
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){            //waiting for S3 to OPEN˚
     #if BASICDEBUG
     Serial.println("waiting for S3 to OPEN");
     #endif
  }
  pinMode(PIN_SOL2, OUTPUT);  //Define SOL2 as OUTPUT
  pinMode(PIN_FF, OUTPUT);    //Define FF as OUTPUT
  #if FFDEBUG
    Serial.println("SOL2 255");
  #endif
  Camera::HighSpeedPWM();
  analogWrite(PIN_SOL2, 255); //SOL2 POWER UP (S2 Closed)
  delay(YDelay);           //AT Yd and POWERS OFF AT FF
  #if FFDEBUG
    Serial.print("_myISO: ");
    Serial.println(_myISO);
  #endif
  int FD_MN = 0;  //FlashDelay Magicnumber
  if(_myISO == ISO_SX70){
     FD_MN = FD100;  
  }
  else if(_myISO == ISO_600){
    FD_MN = FD600;
  }
  meter_set_iso(FD_MN);
  #if FFDEBUG
    Serial.print("FlashDelay Magicnumber: ");
    Serial.println(FD_MN);
  #endif
  #if LMDEBUG
    uint32_t shutterOpenTime = millis(); //Shutter Debug
  #endif
  uint32_t integrationStartTime = millis();
  analogWrite (PIN_SOL2, 130);    //SOL2 Powersaving
  #if FFDEBUG
    Serial.println("SOL2: 130 - Powersave");
  #endif   
  meter_init();
  meter_integrate();
  Camera::shutterOPEN(); //Power released from SOL1 - 25ms to get Shutter full open
  //Start FlashDelay  
  while (meter_update() == false){ //Start FlashDelay: Integrate with the 1/3 of the Magicnumber in Automode of selected ISO
    if((millis() - integrationStartTime) >= 56){ //Flash can occure anytime of the Flash Delay 56+-7ms depending on scene brightness
      break;
    }  
  }
  #if FFDEBUG
    Serial.print(millis()-integrationStartTime);
    Serial.println("ms Flash Delay Time, Flash fired!");
  #endif
  digitalWrite(PIN_FF, HIGH);  //FireFlash
  delay(2);   //Capture Flash 
  #if FFDEBUG
    Serial.print((millis() - flashExposureStartTime));
    Serial.println("ms FlashExposure Integrationtime");
  #endif
  digitalWrite(PIN_FF, LOW);  //Turn FF off
  analogWrite (PIN_SOL2, 0); //SOL2 POWER OFF
  delay(15);
  #if FFDEBUG
    Serial.print((millis()-flashExposureStartTime));
    Serial.println("ms EndFlashExposure: FF and SOL off");
  #endif
  
  #if LMDEBUG
    uint32_t shutterCloseTime = millis(); //Shutter Debug
  #endif

  #if FFDEBUG
  Serial.print("FF Status: ");
  Serial.println(FFState);
  #endif
  Camera::ExposureFinish();

  #if LMDEBUG
    uint32_t exposureTime = shutterCloseTime - shutterOpenTime; //Shutter Debug
    Serial.print("ExposureTime on Automode + FF: ");
    Serial.println(exposureTime);
  #endif
  currentPicture++; 
  WritePicture(currentPicture);
  return; //Added 26.10.
}

void Camera::ShutterB()
{
  #if SONAR
  Camera::ExposureStart();
  #endif

  #if SIMPLEDEBUG
     Serial.print("take B Mode Picture");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif

  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){
     #if BASICDEBUG
     Serial.println("waiting for S3 to OPEN");
     #endif
  }
  delay (40);
  if (_dongle->switch2() ==  1){
    analogWrite(PIN_SOL2, 255);
  }

  Camera::shutterOPEN ();
  if (_dongle->switch2() ==  1){
    analogWrite(PIN_SOL2, 130);
  }
  while (digitalRead(PIN_S1) == S1Logic){
    
  }
  Camera::FastFlash();
  Camera::shutterCLOSE ();
  if (_dongle->switch2() ==  1){
    analogWrite(PIN_SOL2, 0);
  }
  #if SONAR
      delay(100);
      S1F_Unfocus(); //neccesary???
      #if FOCUSDEBUG
        Serial.println("Unfocus");
      #endif
  #endif

  ExposureFinish();
  return; //Added 26.10.
}

void Camera::ShutterT(){
  #if SONAR
  Camera::ExposureStart();
  #endif

  #if SIMPLEDEBUG
     Serial.print("take T Mode picture: ");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif

  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (DebouncedRead(PIN_S3) != HIGH){
    //waiting for S3 to OPEN˚
  }

  delay (40);

  #if SONAR
  while (digitalRead(PIN_S1F) == HIGH){
  }
  #endif
  Camera::shutterOPEN ();
  while (digitalRead(PIN_S1) == !S1Logic){
    #if BASICDEBUG
      Serial.println("Shutter stays open");
    #endif
    //do nothing
  }
  if (digitalRead(PIN_S1) == S1Logic){
    Camera::FastFlash();
    Camera::shutterCLOSE ();
  }

  #if SONAR
    delay(100);
    S1F_Unfocus(); //neccesary???
    #if FOCUSDEBUG
      Serial.println("Unfocus");
    #endif
  #endif

  //multiple exposure test (Should not work in T Mode?!)
  ExposureFinish();
  return; //Addes 26.10.
}

void Camera::ExposureFinish()
{
  Camera::shutterCLOSE();
  lmTimer_stop(); //Lightmeter Timer stop
  #if SONAR
    delay (100);
    S1F_Unfocus(); //neccesary???
    #if FOCUSDEBUG
      Serial.println("Unfocus");
    #endif
  #endif
  delay (200); //Was 20

  if(multipleExposureMode == true){
    while(digitalRead(PIN_S1) == S1Logic){
      //wait for s1 to stop being pressed...
      #if BASICDEBUG
        Serial.println("wait for s1 to stop being pressed...");
      #endif
    }
    #if MXDEBUG
      Serial.println("mEXP");
    #endif
  }
  else if (_dongle->checkDongle() > 0){ //Dongle present
    delay (100);
    Camera::mirrorDOWN ();
    delay (300); //WAS 100
    while(digitalRead(PIN_S1) == S1Logic){ //should be on the end
      //wait for s1 to stop being pressed...
      #if BASICDEBUG
        Serial.println("wait for s1 to stop being pressed...");
      #endif
    }
    Camera::shutterOPEN();
    #if SIMPLEDEBUG
      Serial.println("Exposure Finish - Dongle Mode, ");
      //Serial.print("mxshots count: ");
      //Serial.println(mxshots);
    #endif
    return; //added 26.10.
  }
  else if (_dongle->checkDongle() == 0){ //No Dongle
    delay (100);
    Camera::mirrorDOWN ();
    delay (300); //WAS 60
    while(digitalRead(PIN_S1) == S1Logic){
      #if BASICDEBUG
        Serial.println("wait for s1 to stop being pressed...");
      #endif
    }
    Camera::shutterOPEN();
    #if SIMPLEDEBUG
      Serial.print("Exposure Finish - No Dongle Mode, ");
    #endif
    return; //added 26.10.
  }
}

void Camera::multipleExposureLastClick(){
  #if MXDEBUG
    Serial.print("Multiexposure last Red Button Click, mxshots: ");
    Serial.print(mxshots);
    Serial.print(", CurrentPicture: ");
    Serial.println(currentPicture);
  #endif
  Camera::mirrorDOWN(); 
  delay(50);                             //AGAIN is this delay necessary? 100-->50
  #if SONAR
    delay (100);                             //AGAIN is this delay necessary?
    S1F_Unfocus(); //neccesary???
    #if FOCUSDEBUG
      Serial.println("Unfocus");
    #endif
  #endif
  Camera::shutterOPEN();
  multipleExposureMode = false;
}

void Camera::FastFlash(){
  #if BASICDEBUG
    Serial.println("FastFlash");
  #endif
  pinMode(PIN_S2, OUTPUT);
  //     delay (2);
  digitalWrite (PIN_S2, LOW);     //So FFA recognizes the flash as such
  //     delay (2);
  digitalWrite(PIN_FF, HIGH);    //FLASH TRIGGERING
  delay (1);                      //FLASH TRIGGERING
  digitalWrite(PIN_FF, LOW);     //FLASH TRIGGERING
  pinMode(PIN_S2, INPUT_PULLUP);  //S2 back to normal
}

bool Camera::setLIGHTMETER_HELPER(bool state){
  #if LMHELPERDEBUG
    Serial.print("Set Lightmeterhelper status: ");
    Serial.println(state);
  #endif
  lightmeterHelper = state;
  return state;
}

bool Camera::getLIGHTMETER_HELPER(){
  #if LMHELPERDEBUG
    //Serial.println("Lightmeterhelper status: ");
    //Serial.println(lightmeterHelper));
  #endif
  return lightmeterHelper;
}
