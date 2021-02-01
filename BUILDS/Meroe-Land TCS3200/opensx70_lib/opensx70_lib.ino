 #include "Arduino.h"
#include "open_SX70.h"

/* 
  Version for Edwin, Meroe, Land, and Sonar-FBW PCBs. Works with TSL237T and TCS3200 sensors.
  Changed code to be balanced between readablility and efficiency. 
  Main change from previous versions of the code is the implementation of a state machine. This makes each loop more efficient by far.
  For example, rather than checking every possible dongle-based variable when you do not have a dongle in, the no-dongle state will only check
  for shutter button presses and if it needs to transition to the Dongle state or the Flashbar state. This makes every state of the camera faster
  and ensures that there is no lag between shutter press and the exposure modes beginning.
  The move to a state machine also cuts down on if statement checks.

  The sonar code was entirely done by Hannes (Thank you!).
  Merged last Soanr Version with Zanes Version (Greetings Hannes)
*/

ClickButton sw_S1(PIN_S1, S1Logic);

int selector ;
bool switch1 ;
bool switch2 ;

uDongle myDongle (PIN_S2);
Camera openSX70(&myDongle);
byte prev_selector = 0;
int prevDongle ;
int nowDongle ;
int savedISO;
int activeISO;
//static int checkedcount;
static int inizialized = 0;
static int metercount;

extern bool mEXPFirstRun = false;
extern bool multipleExposureMode = false;
static int multipleExposureCounter = 0;
/*
#if SONAR == 0 
  bool GTD = 1; //For non Sonar Models
#endif
*/
#if SONAR
  bool FT = 0;
  bool S1F = 0;
  bool isFocused = 0; //neccessary? should be done by GTD???
  int currentPicOnFocus; //dont know what this is for
#endif


/*------------BEGIN STATE MACHINE SET_UP------------*/
typedef enum{
  STATE_DARKSLIDE,
  STATE_NODONGLE,
  STATE_DONGLE,
  STATE_FLASHBAR,
  STATE_MULTI_EXP,
  STATE_N
} camera_state;

typedef camera_state (*camera_state_funct)(void);

camera_state do_state_darkslide (void);
camera_state do_state_noDongle (void);
camera_state do_state_dongle (void);
camera_state do_state_flashBar (void);
camera_state do_state_multi_exp (void);

static const camera_state_funct STATE_MACHINE [STATE_N] = {
  &do_state_darkslide,
  &do_state_noDongle,
  &do_state_dongle,
  &do_state_flashBar,
  &do_state_multi_exp
};

//Default state
camera_state state = STATE_DARKSLIDE;

void setup() {//setup - Inizialize
  currentPicture = ReadPicture();
  #if DEBUG
    Serial.begin(9600);
    Serial.println("Welcome to openSX70 Version: 28_10_2020_SONAR_FBW-2_TCS3200 GTD and UDONGLE - SM Version + AutoFF");
    Serial.print("Magic Number: A100=");
    Serial.print(A100);
    Serial.print("| A600 =");
    Serial.print(A600);
    Serial.println(" scaling = 100% | filter = clear");
    Serial.println("State machine core by Zane, Sonar code by Hannes");
    Serial.println("PCB design and original code by Joaquin");
    Serial.print("currentPicture stored in EEPROM: ");
    Serial.println(currentPicture);
  #endif
  
  myDongle.initDS2408();
  init_EEPROM(); //#writes Default ISO to EEPROM

  // (These are default if not set, but changeable for convenience)
  sw_S1.debounceTime   = 15;   // Debounce timer in ms 15
  sw_S1.multiclickTime = 250;  // Time limit for multi clicks
  #if SONAR
  sw_S1.longClickTime = 300;
  #else
  sw_S1.longClickTime  = 0; // time until "held-down clicks" register
  #endif

  selector = myDongle.selector();
  prev_selector = selector;

  io_init();
  metercount = 0; //For the Lightmeter Helper Skipping Function
  checkFilmCount();
  inizialized++;

  if (digitalRead(PIN_S5) != LOW)
  {
    openSX70.mirrorDOWN();
    #if SIMPLEDEBUG
      Serial.println ("Initialize: mirrorDOWN");
    #endif
  }

  #if SIMPLEDEBUG
    Serial.print("Inizialized: ");
    Serial.println(inizialized);
    Serial.print("currentPicture: ");
    Serial.println(currentPicture);
  #endif
}

/*LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP*/
void loop() {
  savedISO = ReadISO();
  selector = myDongle.selector();
  #if SONAR
    preFocus();
  #endif
  normalOperation();
  state = STATE_MACHINE[state]();
  #if SONAR
    unfocusing();
  #endif
}

camera_state do_state_darkslide (void) {
  camera_state result = STATE_DARKSLIDE;
  #if SHUTTERDARKSLIDE
  sw_S1.Update();
  if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1)){
  #endif
    if (digitalRead(PIN_S8) == HIGH && digitalRead(PIN_S9) == LOW){
      currentPicture = 0; 
      WritePicture(currentPicture);
      checkFilmCount();
      if(myDongle.checkDongle() == 0){
        openSX70.darkslideEJECT(); 
      }
      else{
        myDongle.dongleLed(GREEN, HIGH); //green uDongle LED on while ejecting Darkslide
        openSX70.darkslideEJECT();
        myDongle.dongleLed(GREEN, LOW); //switching off green uDongle LED
      }
      #if SIMPLEDEBUG
        Serial.println("STATE1: EJECT DARK SLIDE");
        Serial.print("currentPicture on Darkslide eject: ");
        Serial.println(currentPicture);
      #endif
    }
    
    if ((selector <= 15) && (myDongle.checkDongle() > 0)){ //((selector <= 15) && (myDongle.checkDongle() > 0))
      result = STATE_DONGLE;
      delay(100);
      BlinkISO();
      #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_DONGLE FROM STATE_DARKSLIDE");
      #endif
    }
    else if ((selector == 100) && (myDongle.checkDongle() == 0)){
      result = STATE_FLASHBAR;

      #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_FLASHBAR FROM STATE_DARKSLIDE");
      #endif
    }
    else{
      result = STATE_NODONGLE;

      #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_NODONGLE FROM STATE_DARKSLIDE");
      #endif
    }
  #if SHUTTERDARKSLIDE
  sw_S1.Reset();
  }
  #endif
  
  return result;
}

camera_state do_state_noDongle (void){
  camera_state result = STATE_NODONGLE;
  //savedISO = ReadISO();
  #if SONAR
  if ((digitalRead(PIN_S1F) == HIGH)){ //Do only if S1F is pressed
  #endif
  LightMeterHelper(1);
  if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1)){
    LightMeterHelper(0); 
    beginExposure();
    openSX70.AutoExposure(savedISO);
    sw_S1.Reset();
  }
  #if DOUBLECLICK
  if (sw_S1.clicks == 2){ //Doubleclick the Red Button with no Dongle inserted
    LightMeterHelper(0); 
    beginExposure();
    delay (10000);
    openSX70.AutoExposure(savedISO);
    sw_S1.Reset();
  }
  #endif
  #if SONAR
  }
  #endif
  //Checks for dongle or flashbar insertion
  if (myDongle.checkDongle() > 0){
    #if STATEDEBUG
      Serial.println("TRANSITION TO STATE_DONGLE FROM STATE_NODONGLE");
    #endif
    result = STATE_DONGLE;
    //myDongle.initDS2408();
    if(((myDongle.switch1() == 1) && (myDongle.switch2() == 1))){
      saveISOChange();
    }
    else if(myDongle.selector()<=13){ //Dont blink on AUTOMODE
      //Serial.println("Transition from no dongle to dongle");
      BlinkISO();
    }
  }
  else if ((selector == 100) && (myDongle.checkDongle() == 0)){
    result = STATE_FLASHBAR;
    #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_FLASHBAR FROM STATE_NODONGLE");
    #endif
  }
  return result;
}

camera_state do_state_dongle (void){
  camera_state result = STATE_DONGLE;
  DongleInserted();
  
  #if SONAR
  if ((digitalRead(PIN_S1F) == HIGH)){
  #endif
    if(selector<11){
      LightMeterHelper(2); //LMHelper Manual Mode
    }
    else if(selector==14 || selector==15){
      LightMeterHelper(1); //LMHelper Auto Mode
    }
  #if SONAR
  }
  #endif
  
  if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0)){
    LightMeterHelper(0); //Turns off LMHelper on picutre Taking
    beginExposure(); //may just move this directly before each other call. may feel clunky
    if(switch2 == 1){
      switch2Function(0); //switch2Function Manual Mode
    }

    if((selector>=0) && (selector<=3)){ //fast manual speeds
      openSX70.VariableManualExposure(savedISO);
    }
    else if((selector>3) && (selector<12)){ //MANUAL SPEEDS  
      openSX70.ManualExposure();
    }
    else if(selector == 12){ //POST
      lmTimer_stop();
      turnLedsOff();
      openSX70.ShutterT();
    }
    else if(selector == 13){ //POSB
      lmTimer_stop();
      turnLedsOff(); //why?
      openSX70.ShutterB();
    }
    else{ //Auto catch-all. Passes the value stored in the ShutterSpeed list at the selector value
      switch(ShutterSpeed[selector]){
        case AUTO100:
          openSX70.AutoExposure(ISO_SX70);
          break;
        case AUTO600:
          openSX70.AutoExposure(ISO_600);
          break;
      }
    }
    sw_S1.Reset();
    checkFilmCount();
  } 

  // Dongle Removed
  if (myDongle.checkDongle() == 0){
    result = STATE_NODONGLE;
    #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_NODONGLE FROM STATE_DONGLE");
    #endif
  } 
  // Multiple Exposure switch flipped
  else if ((switch1 == 1) && (switch2 == 0)){
    result = STATE_MULTI_EXP;
    multipleExposureMode = true;
    mEXPFirstRun = true;
    #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_MULTI_EXP FROM STATE_DONGLE");
    #endif
  }

  return result;
}

camera_state do_state_flashBar (void){
  camera_state result = STATE_FLASHBAR;
  if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1))
  {
    beginExposure();
    openSX70.AutoExposureFF(savedISO);
    sw_S1.Reset();
    checkFilmCount();
  }
  #if DOUBLECLICK
  if (sw_S1.clicks == 2)
  {
    beginExposure();
    delay(10000); //Switch Two Function in Flash Mode
    openSX70.AutoExposureFF(savedISO);
    sw_S1.Reset();
    checkFilmCount(); 
  } 
  #endif
  
  if ((selector == 200) && (myDongle.checkDongle() == 0)){
    result = STATE_NODONGLE;
    #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_NODONGLE FROM STATE_FLASHBAR");
    #endif
  } 
  return result;
}

camera_state do_state_multi_exp (void){
  camera_state result = STATE_MULTI_EXP;
  DongleInserted();

  /*

  Don't know how we would implement the meter while in mEXP mode. Would not make sense.

  #if SONAR
    if ((digitalRead(PIN_S1F) == HIGH)){
    #endif
      if(selector<11){
        LightMeterHelper(2); //LMHelper Manual Mode
      }
      else if(selector==14 || selector==15){
        LightMeterHelper(1); //LMHelper Auto Mode
      }
    #if SONAR
    }
  #endif
  */
  
  if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0)){
    LightMeterHelper(0); //Turns off LMHelper on picutre Taking
    if(switch1 == 1){ //Why Switch1 == true?!    This is for checking if we are still taking mexp pics
      if(mEXPFirstRun){
        beginExposure();
        mEXPFirstRun = false;
      }
      if(switch2 == 1){
        switch2Function(0);
      }

      if((selector>=0) && (selector<=3)){ //fast manual speeds
        openSX70.VariableManualExposure(savedISO);
        multipleExposureCounter++;
      }
      else if((selector>3) && (selector<12)){ //MANUAL SPEEDS  
        openSX70.ManualExposure();
        multipleExposureCounter++;
      }
      else if(selector == 12){ //POST
        lmTimer_stop();
        turnLedsOff();
        openSX70.ShutterT();
        multipleExposureCounter++;
      }
      else if(selector == 13){ //POSB
        lmTimer_stop();
        turnLedsOff(); //why?
        openSX70.ShutterB();
        multipleExposureCounter++;
      }
      else{ //Auto catch-all. Passes the value stored in the ShutterSpeed list at the selector value
        switch(ShutterSpeed[selector]){
        case AUTO100:
          openSX70.AutoExposure(ISO_SX70);
          break;
        case AUTO600:
          openSX70.AutoExposure(ISO_600);
          break;
      }
        multipleExposureCounter++;
      }
    }
    else if(switch1 == 0 && multipleExposureCounter > 0){
      openSX70.multipleExposureLastClick();
      checkFilmCount();
      multipleExposureCounter = 0;
      result = STATE_DONGLE;

      #if STATEDEBUG
        Serial.println("TRANSITION TO STATE_DONGLE FROM STATE_MULTI_EXP");
      #endif
    }
    sw_S1.Reset();
  }

  if(switch1 == 0 && multipleExposureCounter == 0){
    result = STATE_DONGLE;
    multipleExposureMode = false;
    #if STATEDEBUG
      Serial.println("TRANSITION TO STATE_DONGLE FROM STATE_MULTI_EXP");
    #endif
  }
  return result;
}

#if SONAR
void printReadings() {
  //Serial.print("GTD: ");
  //Serial.print(openSX70.getGTD());
  Serial.print("Is Focused: ");
  Serial.print(isFocused);
  Serial.print(" | GTD: ");
  Serial.print(analogRead(PIN_GTD));
  Serial.print(" | S1F: ");
  Serial.print(digitalRead(PIN_S1F));
  Serial.print(" | FT: ");
  Serial.print(analogRead(PIN_FT));
  Serial.print(" | FF: ");
  Serial.println(digitalRead(PIN_FF));
  return;
}

void preFocus() {
  if ((digitalRead(PIN_S1F) == HIGH) && (isFocused == 0)) { // S1F pressed
    openSX70.S1F_Focus();
    currentPicOnFocus = currentPicture;
    isFocused = 1;
    return;
  }
}

void unfocusing(){
  //delay(100);
  if ((digitalRead(PIN_S1F) == LOW) && (isFocused == 1)) { // S1F pressed  -- selftimer (doubleclick the red button) is not working this way
    //delay(100);
    openSX70.S1F_Unfocus();
    //currentPicOnFocus = currentPicture;
    isFocused = 0;
    turnLedsOff();
    //return;
  }
}
#endif

//Added to remove check for multiple exposure mode from standard manual exposure state. 
//Offload the check to multiple exposure state
void beginExposure(){
  currentPicture++;
  WritePicture(currentPicture);

  openSX70.shutterCLOSE();
  delay(100);
  openSX70.mirrorUP();
}

void turnLedsOff(){ //todo:move to camerafunction
   digitalWrite(PIN_LED1, LOW);
   digitalWrite(PIN_LED2, LOW);
   //delay(400); 
}

void DongleInserted() { //Dongle is pressend LOOP
  if (digitalRead(PIN_S1) != S1Logic) { //Dont run DongleInserted Function on S1T pressed
  #if SONAR
    if (digitalRead(PIN_S1F) != S1Logic) { //Dont run DongleInserted Function on S1F pressed
  #endif
      { //Serial.println("S1F HIGH");
        //selector = myDongle.selector();
        switch1 = myDongle.switch1();
        switch2 = myDongle.switch2();
        //saveISOChange();//added 26.10.
        lmEnable(); //added 26.10.
        //BlinkISO(); //check if dongle inserted, read the default ISO and blink once for SX70 and twice for 600.
        if ((selector != prev_selector)) //Update Dongle changes
        {
          #if ADVANCEDEBUG
            Serial.print ("DONGLE Mode:  ");
            Serial.print ("Selector: ");
            Serial.print (selector);
            Serial.print (" Switch1: ");
            Serial.print (switch1);
            Serial.print (" Switch2: ");
            Serial.print (switch2);
            Serial.print (" speed: ");
            Serial.println (ShutterSpeed[selector]);
          #endif
          blinkAutomode();
          //blinkSpecialmode(); //B and T Mode Selector LED Blink
          prev_selector = selector;
          //return;
        }
      }
  #if SONAR
    }
  #endif
  }
}

void lmEnable(){
  if(selector == 12){ //POST
      if ((switch2 == 1) && (switch1 == 1)) {
        if (openSX70.getLIGHTMETER_HELPER() == false) {
          openSX70.setLIGHTMETER_HELPER(true);
          turnLedsOff();
          digitalWrite(PIN_LED2, HIGH); //Blink Blue -- LMH On
          myDongle.simpleBlink(1, GREEN);
          delay(100);
          digitalWrite(PIN_LED2, LOW);
          #if SIMPLEDEBUG
            Serial.println("Lightmeter is on");
          #endif
        }
      }
    }
    else if(selector == 13){ //POSB
       if ((switch2 == 1) && (switch1 == 1)) {
        if (openSX70.getLIGHTMETER_HELPER() == true) {
          openSX70.setLIGHTMETER_HELPER(false);
          turnLedsOff();
          digitalWrite(PIN_LED1, HIGH); //Blink RED -- LMH Off
          myDongle.simpleBlink(1, RED);
          delay(100);
          digitalWrite(PIN_LED1, LOW);
        #if SIMPLEDEBUG
          Serial.println("Lightmeter is off");
        #endif
        }
      }
    }
}

void BlinkISO() { //read the default ISO and blink once for SX70 and twice for 600
  switch1 = myDongle.switch1();
  switch2 = myDongle.switch2();
  if((switch2 != 1) || (switch1 != 1)){ //Not Save ISO //Changed to OR 01.06.2020
      #if SIMPLEDEBUG
        Serial.println("Blink for the saved ISO setting on Dongle insertion.");
      #endif
      //blinkAutomode();
      savedISO = ReadISO();
      turnLedsOff();
      if (savedISO == ISO_600){
        myDongle.simpleBlink(2, GREEN);
        delay(500);
      }
      else if (savedISO == ISO_SX70){
        myDongle.simpleBlink(1, GREEN);
        delay(500);
      }
      else{
      #if SIMPLEDEBUG
        Serial.println("No ISO Selected");
        myDongle.simpleBlink(5, RED);
      #endif
      }
      #if SIMPLEDEBUG
          Serial.print ("EEPROM READ ISO: ");
          Serial.println (savedISO);
      #endif
      prevDongle = nowDongle;
      checkFilmCount();
      //return;
    }
}

void blinkAutomode(){
  if ((switch2 != 1) || (switch1 != 1)) { //Save ISO Mode
    turnLedsOff();
    if(ShutterSpeed[selector]== AUTO600){
      myDongle.simpleBlink(2, GREEN);
      #if SIMPLEDEBUG
        Serial.print("blinkAutomode() - Blink 2 times Green on Auto600 selected: ");
        Serial.println(ShutterSpeed[selector]);
      #endif
      //return;
    }else if(ShutterSpeed[selector]== AUTO100){
      myDongle.simpleBlink(1, GREEN);
      #if SIMPLEDEBUG
        Serial.print("blinkAutomode() - Blink 1 times Green on Auto100 selected: ");
        Serial.println(ShutterSpeed[selector]);
      #endif
      //return;
    }
    checkFilmCount(); //Needed to Check if Counter is on 8 and 9 or 10 to activate LED again
  }
}

void blinkSpecialmode(){
  turnLedsOff();
  if((switch2 != 1) || (switch1 != 1)){ //Not Save ISO Mode
    if(ShutterSpeed[selector]== POST){
      myDongle.simpleBlink(1, GREEN);
      myDongle.simpleBlink(1, RED);
    }
    if(ShutterSpeed[selector]== POSB){
      myDongle.simpleBlink(1, RED);
      myDongle.simpleBlink(1, GREEN);
    }
    checkFilmCount();  
  }
}

void BlinkISORed() { //read the active ISO and blink once for SX70 and twice for 600 - on ISO change
  #if SIMPLEDEBUG
      Serial.print("Blink RED on ISO change: ");
  #endif
  turnLedsOff();
  if (activeISO == ISO_SX70){
    myDongle.simpleBlink(1, RED);
  }
  else if (activeISO == ISO_600){
    myDongle.simpleBlink(2, RED);
  }
  #if SIMPLEDEBUG
      Serial.print ("active ISO: ");
      Serial.println (activeISO);
  #endif
  checkFilmCount();
  //return;
}

void switch2Function(int mode) {
  //0 Manual, 1 Auto600, 2 AutoSX70, FlashBar
  if (mode == 0) {
    openSX70.shutterCLOSE();
    #if SONAR
      openSX70.S1F_Unfocus();
    #endif
    openSX70.SelfTimerMUP();
    digitalWrite(PIN_LED2, LOW);
    digitalWrite(PIN_LED1, LOW);
    #if SONAR
      openSX70.S1F_Focus();
    #endif
    openSX70.BlinkTimerDelay (GREEN, RED, 10);
  }
  else if (mode == 1) {
    openSX70.SelfTimerMUP();
    #if SONAR
      openSX70.S1F_Unfocus();
    #endif
    digitalWrite(PIN_LED2, LOW);
    digitalWrite(PIN_LED1, LOW);
    #if SONAR
      openSX70.S1F_Focus();
    #endif
    openSX70.BlinkTimerDelay (GREEN, RED, 10);
  } else if (mode == 2) {
    openSX70.SelfTimerMUP();
    #if SONAR
      openSX70.S1F_Unfocus();
    #endif
    digitalWrite(PIN_LED2, LOW);
    digitalWrite(PIN_LED1, LOW);
    #if SONAR
      openSX70.S1F_Focus();
    #endif
    openSX70.BlinkTimerDelay (GREEN, RED, 10);
  } else if (mode == 3) {
    #if SONAR
      openSX70.S1F_Unfocus();
    #endif
    openSX70.SelfTimerMUP();
    delay (10000); //NoDongleMode
    //preFocus();
    #if SONAR
      openSX70.S1F_Focus();
    #endif
    delay(1000);
  }
}

void checkFilmCount(){
  if ((currentPicture == 8) || (currentPicture == 9)){
      #if SIMPLEDEBUG
        Serial.print("Two Frames left!");
        Serial.print(", currentPicture on Two Frames left: ");
        Serial.println(currentPicture);
      #endif
      //myDongle.simpleBlink(2, RED);
      myDongle.dongleLed(RED, LOW);
      myDongle.dongleLed(GREEN, HIGH);
      //return;
  }
  else if(currentPicture == 10){ 
    #if SIMPLEDEBUG
      Serial.print("Ten Frames shot!");
      Serial.print(", currentPicture: ");
      Serial.println(currentPicture);
    #endif
    myDongle.dongleLed(GREEN, LOW);
    myDongle.dongleLed(RED, HIGH);
    //return;
  }
}

void ispackEmpty(){ //This is doing nothing right now
  static int firstRun = 0;
  //STATE 2: PACK IS EMPTY--> NO WASTE OF FLASH
  //Camera Counter is Zero and Switch S9 is CLOSED
  // changed this to allow shooting until counter is actually 0, in case "something" happens and I loose count!
  if ((digitalRead(PIN_S8) == LOW && digitalRead(PIN_S9) == HIGH) && (currentPicture >= 8))
  {
    if (firstRun==0){ //Run only one time when Switch S9 change to HIGH
      firstRun++;
      if  (nowDongle != 0) {
        //Serial.println("STATE2: Set LED RED to High");
        //myDongle.dongleLed(RED, HIGH);
      }
      #if SIMPLEDEBUG
          Serial.print("STATE2: PACK IS EMPTY - S9 Closed");
          Serial.print(", Current Picture on Empty Pack: ");
          Serial.println(currentPicture);
      #endif
    }
  }
}

void normalOperation(){
  //STATE 3: NORMAL OPERATION *************************************************************************************************************************************************
  if (digitalRead(PIN_S8) == LOW && digitalRead(PIN_S9) == LOW){
      //WHAT TO DO WHEN POWER-UP:
      //  S8     S9
      // closed  open  --> EJECT DARKSLIDE (DEFAULT)
      // open  closed --> FILM REACH 0 (NO FLASH)
      // open   open  --> NORMAL OPERATION 10 TO 1
      // ///////////////////////////////////PICTURE TAKING OPERATION//////////////////////////////////////////////////
      //    FOUR CASES:
      //   *  CASE 1 NORMAL OPERATION: FULL CYCLE
      //   *  SELECTOR = NORMAL (LOW)
      //   *  NXSHOTS = 0
      //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
      //   *
      //   *  CASE 2 DOUBLE EXPOSURE FIRST SHOT: MIRROR DOWN AND FIRST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
      //   *  SELECTOR = DOUBLE (HIGH)
      //   *  MXSHOTS = 0
      //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
      //   *
      //   *  CASE 3 DOUBLE EXPOSURE ULTERIOR MXSHOTS: NO MOTOR OPERATION JUST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
      //   *  SELECTOR = DOUBLE (HIGH)
      //   *  NXSHOTS >= 1
      //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
      //   *
      //   *  CASE 4 PICTURE EXPULSION AFTER DOUBLE EXPOSURE: MIRROR DOWN AND SHUTTER OPENING (NO PICTURE TAKEN)
      //   *
      //   *  SELECTOR = NORMAL (LOW)
      //   *  MXSHOTS >= 1
    sw_S1.Update();
  }
}

void saveISOChange() {
  int _selectedISO;
  selector = myDongle.selector();
  savedISO = ReadISO(); //read the savedISO from the EEPROM
  if (((ShutterSpeed[selector]) == AUTO600)) {
    _selectedISO = ISO_600;
  }
  else if (((ShutterSpeed[selector]) == AUTO100)) {
    _selectedISO = ISO_SX70;
  }
  else {
    _selectedISO = DEFAULT_ISO;
  }
  if (savedISO != _selectedISO) { //Check if new ISO is diffrent to the ISO saved in EEPROM
    #if SIMPLEDEBUG
      Serial.print("SaveISOChange() Function: ");
      Serial.print("ISO has changed, previos saved ISO (from EEPROM): ");
      Serial.println(savedISO);
      Serial.print("Saving new selected ISO ");
      Serial.print(_selectedISO);
      Serial.println(" to the EEPROM");
    #endif
    activeISO = _selectedISO; //Save selectedISO to volatile Variable activeISO
    WriteISO(_selectedISO); //Write ISO to EEPROM
    savedISO = ReadISO();
    BlinkISORed();
  }
  else{
    #if SIMPLEDEBUG
      Serial.print("SaveISOChange() Function: ");
      Serial.println("savedISO is equal to selected ISO, dont save!");
    #endif
    activeISO = _selectedISO;
    BlinkISORed(); //Blink ISO Red
  }
  prev_selector = selector; //prevents green blink after ISO change
}

void LightMeterHelper(byte ExposureType){
  if(openSX70.getLIGHTMETER_HELPER()){
    meter_led(selector, ExposureType);
    metercount=0;
  }
}
