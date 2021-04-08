#ifndef settings_h
  #define settings_h
  #include "Arduino.h"
  //------------DEBUG OPTIONS---------------------------------------------
  #define DEBUG 0
  #define SIMPLEDEBUG 0     //Debug of opensx70 functions
  #define ADVANCEDEBUG 0    //Simple Debug On 1 - Off 0
  #define BASICDEBUG 0      // Debug of Basic Camera Functions
  #define MXDEBUG 0         // Multiple Exposure Debug
  #define LMDEBUG 0         //Lightmeter Debug
  #define LMHELPERDEBUG 0   //Lightmeter Debug
  #define ROTARYDEBUG 0     //Rotaryswitch on Dongle Debug
  #define STATEDEBUG 0      //Debug state transitions
  #define FOCUSDEBUG 0      //Focus Debug on Sonar
  #define FFDEBUG 0         //AutoexposureFillFlash Debug
  //----------------------END DEBUG OPTIONS------------------------------
  //----------------------CAMERA PCB OPTIONS SELECTION-------------------
  #define TCS3200         //TSL237T TSL235R TCS3200
  #define S1Logic LOW   //HIGH for sonar, LOW for Alpha
  #define ALPHA 1        //1 if ALPHA camera/PCB
  #define SONAR 0         //   
  #define ORIGAMI 0
  #define ORIGAMIV1 0
  #define UDONGLE 1
  #define GREEN 6 //DONGLE GREEN LED ADRESS
  #define RED 7   //DONGLE RED LED ADRESS
  #define DOUBLECLICK 0
  #define SHUTTERDARKSLIDE 0 // makes you press shutter button to eject darkslide. This is to prevent externally powered cameras from firing the darkslide when opening.
  #define APERTURE_PRIORITY 0
  #define COUNTER_BLINK 0
  #define TIMER_MIRROR_UP 1
  #define EIGHT_SHOT_PACK 1
  #define LIGHMETER_HELPER 1
  #define EJECT_AFTER_DEPRESSING 0 // Ejects the film only after depressing the shutter button. This gives time to the photographer to shield the camera from light.
  //----------------ISO VALUES VALUES---------------------------------------
  #define ISO_SX70 125
  #define ISO_600 640
  #define ISO_600BW 400
  #define DEFAULT_ISO ISO_600
  //---------------END ISO VALUES--------------------------------------------

  //---------------Shutter Settings------------------------------------------
  //#define SHUTTER_SPEED_VARIANCE 0.70 // This is a percentage. range (0.0 - 1.0) Sets minimum bounds for shutter to fire at high EVs. EX at .80 at EV17 (25ms) it will fire within the range of 20ms-25ms (5ms or 20% variance)
  #define SELECTOR_LIMIT 5 // Sets what selector it will stop varying the speed at (starting from 0 to the number you set)
  //---------------End Shutter Settings--------------------------------------

  //---------------METER SETTINGS--------------------------------------------
  #define METER_INTERVAL 100 // Sets how long each meter measurement sample is taken in ms
  //#define METER_RANGE 0.20 // METER_RANGE defines the percentage left and right that is acceptable for the meter to be "accurate"
  #define METER_AUTO_WARNING 150 // If predicted ms is over this value, warning LED will shine in auto mode
  #define METER_PREDICTION_OFFSET 20 // in ms. This gets added to the prediction. At f8 I noticed all meter predictions were around 20ms off
  #define METER_SLOPE_HANDICAP 0 // Not used currently. Used to increase/decrease the slope of the prediction.
  //---------------END METER SETTINGS----------------------------------------

  #define EE_ADD_CUR   4    //-> CurrentPicture --> 1-8 "Pack" picture counter
  #define EE_ADD_ADD   30    //-> eeAddress Where to write next
  #define EE_ADD_PIC   13    //-> ActualPicture --> Counter from begining
  #define EE_ADD_ISO   20   //-> camera current ISO (dongleless)

  //---------------MULTIPLE EXPOSURES MODE SETTINGS--------------------------
  #define MULTIPLE_EXPOSURES_TIMEOUT_ENABLED 1 // 1 -> MX mode will finish after timeout. 0 -> No time limit for MX mode.
  #define MULTIPLE_EXPOSURES_TIMEOUT 120000 // Max time that MX mode can run for after the first exposure (in milliseconds)
  //---------------END MULTIPLE EXPOSURES MODE SETTINGS----------------------

  //---------------METER SETTINGS--------------------------------------------
  //---------------OPTION REGARDING SELECTOR WHEEL---------------------------
  //enum positions_t {POST = -100, POSB, AUTO600, AUTO100 };//ANALOGUEWORKS
  enum positions_t {POST = -100, POSB, AUTO600, AUTO100};//ANALOGUEWORKS AUTO 600BW
  //enum positions_t {AUTO600 = -100, AUTO100, POST, POSB}; //uDONGLE original
  //enum positions_t {POST = -100, AUTO600, AUTO600BW, AUTO100}; //uDONGLE Optoflow
  //enum positions_t {POST = -100, POSB, AUTO600, AUTO600BW, AUTO100}; //uDONGLE Optoflow POST = -100, POSB=-99, AUTO600=-98, AUTO600BW=-97, AUTO100=-96
  /*
    TO CLARIFY:
  
      uDongle               AnalogueDongle    AnalogueDongle 600BW
      AUTO600 = - 100       POST = -100       POST =-100
      AUTO100 = -99         POSB = -99        POSB =-99
      POST = -98            AUTO600 = - 98    AUTO600 = -98
      POSB = -97            AUTO100 = - 97    AUTO600BW = -97
                                              AUTO100 = -96
  */
  extern int ShutterConstant;
  //OPTION
  extern int ShutterSpeed[]; //reduced speeds from 25 (slot5) to compensate flash firing
  //flashDelay is the time it takes to fire the flash (even if none is connected) when slow shutterspeeds
  extern int flashDelay; //new flash "system"

  extern int MeterRange[];

  extern int MaxRange[];

  extern int MinRange[];

  extern int ShutterVariance[];
  //this speed and SLOWER will trigger flash
  extern int mxshots; //Multiple exposure counter
  extern byte lightmeterHelper;
  extern const uint8_t YDelay;
  extern const byte PowerDownDelay; //time it takes to be fully closed
  extern const byte PowerDown; //max 255 = full power/POWERUP mode
  #define DEBOUNCECOUNT 5 
  extern int currentPicture;
  extern int prevDongle;
#endif
