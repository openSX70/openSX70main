#ifndef settings_h
  #define settings_h
  #include "Arduino.h"
  //------------DEBUG OPTIONS---------------------------------------------
  #define LIGHMETER_HELPER 1
  //LIGHMETER_HELPER 0  NO helper, 1 for VE helper, 2 future dongle helper
  #define DEBUG 0
  #define SIMPLEDEBUG 0     //Simple Debug On 1 - Off 0
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
  //----------------------CAMERA PCB OPTIONS SELECTION---------------------------
  //#define TSL237T
  //#define TSL235R
  #define TCS3200
  //S1Logic LOW = Alphas, Meroë, Edwin
  //#define S1Logic HIGH //= Sonar 
  #define S1Logic LOW
  #define ALPHA 1
  #define SONAR 0
  #define ORIGAMI 0
  #define ORIGAMIV1 0 //V1 Version of Origami with inverted Rotaryswitch
  #define UDONGLE 1
  #define GREEN 6 //DONGLE GREEN LED ADRESS
  #define RED 7   //DONGLE RED LED ADRESS
  #define DOUBLECLICK 0
  #define SHUTTERDARKSLIDE 0 // makes you press shutter button to eject darkslide. This is to prevent externally powered cameras from firing the darkslide when opening.
  extern const uint8_t YDelay;
  extern const byte PowerDownDelay; //time it takes to be fully closed
  extern const byte PowerDown; //max 255 = full power/POWERUP mode
  #define DEBOUNCECOUNT 5 
  extern int currentPicture;
  extern int prevDongle;
  //----------------ISO VALUES VALUES---------------------------------------
  #define ISO_SX70 125
  #define ISO_600 640
  #define FILL_FLASH_PERCENT 0.29
  //TODO! Test out an overall magic number modifier. That way people only need to edit this file rather than going through others 
  #define DEFAULT_ISO ISO_600
  //---------------END ISO VALUES--------------------------------------------

  //---------------METER SETTINGS--------------------------------------------
  // Sets how long each meter measurement is taken in ms
  #define METER_INTERVAL 100 
  // METER_RANGE defines the percentage left and right that is acceptable for the meter to be "accurate"
  #define METER_RANGE 0.20
  // If predicted ms is over this value, warning LED will shine in auto mode
  #define METER_AUTO_WARNING 150

  #define METER_SLOPE_HANDICAP 0
  // in ms
  #define METER_PREDICTION_OFFSET 20

  //---------------END METER SETTINGS----------------------------------------

  #define EE_ADD_CUR   4    //-> CurrentPicture --> 1-8 "Pack" picture counter
  #define EE_ADD_ADD   30    //-> eeAddress Where to write next
  #define EE_ADD_PIC   13    //-> ActualPicture --> Counter from begining
  #define EE_ADD_ISO   20   //-> camera current ISO (dongleless)
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
  //this speed and SLOWER will trigger flash
  extern int mxshots; //Multiple exposure counter
  extern byte lightmeterHelper;
  /*
  struct PictureData
  {
    uint16_t StructPicture;         //total count of pictures since init
    uint16_t StructPackPicture;          //pic count within this pack
    uint16_t StructType;           //picture type
    // PictureType = 0 ---> MANUAL
    // PictureType = 1 ---> A100
    // PictureType = 2 ---> FLASH DONGLELESS
    // PictureType = 4 ---> FLASH F8 DONGLE
    // PictureType = 6 ---> A600
    // PictureType = 7 ---> BULB
    // PictureType = 8 ---> T
    // PictureType = +10 ---> MULTIPLE EXPOSURE
    // PictureType = +100 ---> TIMER DELAY
    uint16_t StructSpeed;           //shutter speed
  };

    //EEPROM CHEATSHEET
    //  0-> S  initialization purposes
    //  1-> X  initialization purppses
    //  3-> ActualPicture --> From the begining absolute picture counter
    //  4-> CurrentPicture --> 1-8 "Pack" picture counter   EE_ADD_CUR   
    // 30-> eeAddress Where to write next                   EE_ADD_ADD
    // 13-> ActualPicture --> Counter from begining         EE_ADD_PIC
    // 20-> camera current ISO (dongleless)                 EE_ADD_ISO
*/
#endif
