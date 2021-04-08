#ifndef SX70_PCB_H 
  #define SX70_PCB_H 
  #include "settings.h"

  // Input pin definition
  const uint8_t PIN_S1 = 12; //Sonar S1T S1 Trigger Redbutton
  const uint8_t PIN_S2 = A4;
  const uint8_t PIN_S3 = 8;
  // const uint8_t PIN_S4 = ;
  const uint8_t PIN_S5 = 7;
  const uint8_t PIN_S8 = A1;
  const uint8_t PIN_S9 = A0;
  
  // Output pin definition
  const uint8_t PIN_SOL1 = 3; //orig 3 PD3(INT1)
  const uint8_t PIN_SOL2 = 11;
  
  const uint8_t PIN_MOTOR = A2;
  const uint8_t PIN_LED1 = 13; //LED RED
  const uint8_t PIN_LED2 = A3; //LED BLUE
  const uint8_t PIN_FF = 4;
  
  #if SONAR
  const uint8_t PIN_GTD = A5; // Sonar gone that distance
  const uint8_t PIN_S1F = 10; //Sonar S1 Focus //Analog imput only
  const uint8_t PIN_FT = A7; //Sonar Flash Triggerd INPUT -- Cave Analoge Input only!
  const uint8_t PIN_S1F_FBW = 2; //engage SONAR prefocus
  #endif

  void io_init();
#endif
