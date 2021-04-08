#include "Arduino.h"
#include "settings.h"
#include "EEPROM.h"

void WritePicture(int _currentPicture){
  EEPROM.update(EE_ADD_PIC, _currentPicture);
}

int ReadPicture(){
  int _currentPicture;
  EEPROM.get(EE_ADD_PIC, _currentPicture);
  return (_currentPicture+256);
}

void WriteISO(int _currentISO){
  EEPROM.put(EE_ADD_ISO, _currentISO);
}

int ReadISO()
{
  int _currentISO;
  EEPROM.get(EE_ADD_ISO, _currentISO);
  return _currentISO;
}

void init_EEPROM(){
  int initJP[2];
  initJP[0] = EEPROM.read(0);
  initJP[1] = EEPROM.read(1);
  //int currentPicture = 1;
  //const uint8_t EE_ADD_PIC = 13;
  if (initJP[0] != 255 || initJP[1] != 255)
  {
    #if SIMPLEDEBUG
        Serial.println(F("Initializing EEPROM...."));
        Serial.print(F("initJP[0] = "));
        Serial.print(initJP[0]);
        Serial.print(F("initJP[1] = "));
        Serial.println(initJP[0]);
    #endif
      EEPROM.update(0, 255);
      EEPROM.update(1,255);
      //    int eeAddress = 101;
      //    DEFAULT_ISO = 640;
      //WritePicture(0); //Write picture 0 moved to Darkslide Fucntion 
      WriteISO(DEFAULT_ISO);
      //    EEPROM.put(EE_ADD_PIC, currentPicture);
      //    EEPROM.put(EE_ADD_ISO, DEFAULT_ISO);
      #if SIMPLEDEBUG
        Serial.print(F("ISO in EEPROM: "));
        Serial.println(ReadISO());
      #endif
      return;
  } 
}
