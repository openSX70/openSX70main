#ifndef uDongle_h
  #define uDongle_h
  
  #if (ARDUINO >=100)
    #include "Arduino.h"
  #else
    #include "WProgram.h" //??
  #endif
  #include "DS2408.h"
  
  class uDongle {
    public:
      // Constructor 
      uDongle(uint8_t pin);
      
      // Methods
      void initDS2408();
      byte selector();
      byte switch1();
      byte switch2();
      //byte Read_DS2408_PIO(int slot);
      void Write_DS2408_PIO(byte port, bool ON);
      byte checkDongle();
      void dongleLed (byte led,bool on);
      void simpleBlink (int _times, int _led);
      void bothBlink (int _times);
      void doubleBlink (int _times);
 
    private:
      byte Read_DS2408_PIO(int slot);
      //void Write_DS2408_PIO(byte port, bool ON);
      uint8_t _Pin;
      //uint8_t _dongleDevice;
      uint8_t _device_count;
      //byte _Selector;
      //byte _Switch1;
      //byte _Switch2;
      //byte _Switch;
      DS2408* _ds;
      Device  _dongleDevice;
  };
#endif
