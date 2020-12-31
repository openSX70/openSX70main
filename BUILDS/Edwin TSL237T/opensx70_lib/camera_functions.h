#ifndef Camera_h
  #define Camera_h
  #include "Arduino.h"
  #include "uDongle2.h"
  class Camera
  {
    public:
      Camera(uDongle *dongle);
      void S1F_Unfocus();
      void S1F_Focus();
      int S1F_Focus1();
      void SelfTimerMUP();
      void shutterCLOSE();
      void shutterOPEN();
      void mirrorDOWN();
      void mirrorUP();
      void darkslideEJECT();
      void motorON();
      void motorOFF();
      void Ydelay ();
      void BlinkTimerDelay(byte led1, byte led2, byte time = 10) ;
      //  uDongle2 *myDongle _Write_DS2408_PIO(byte _port, bool _ON);
      //  uDongle2 *myDongle (pin_S2);
      //  uDongle(pin_S2).myDongle;
      //  int Blink (int interval, int timer, int Pin, byte type);
      void Blink (unsigned int interval, int timer, int Pin, byte type = 1);
      void Blink (unsigned int interval, int timer, int PinDongle, int PinPCB, byte type);
      void HighSpeedPWM ();
      void AutoExposure(int _myISO);
      void AutoExposureFF(int _myISO);
      void ExposureStart();
      void ExposureFinish();
      void ManualExposure();
      void VariableManualExposure(int _myISO);
      void FlashBAR();
      void ShutterB();
      void ShutterT();
      void multipleExposureFinish();
      void multipleExposure(int exposureMode);
      bool setLIGHTMETER_HELPER(bool active);
      bool getLIGHTMETER_HELPER();
      void multipleExposureLastClick();
      int getGTD();
    private:
      void FastFlash();
      //  void ShutterB();
      //  void ShutterT();
      void DongleFlashF8();
      void DongleFlashNormal();
      
      //  Basics* _Basics;
      bool DebouncedRead(uint8_t pin);
      uDongle * _dongle;
  };
#endif
