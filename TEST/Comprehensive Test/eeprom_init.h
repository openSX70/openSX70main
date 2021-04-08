#ifndef eeprom_init_h
  #define eeprom_init_h
  #include "Arduino.h"
  void init_EEPROM();
  void WritePicture(int _currentPicture);
  int ReadPicture();
  void WriteISO(int _currentPicture);
  int ReadISO();
#endif
