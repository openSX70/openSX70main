#ifndef OPENSX70_METER_H
  #	error "inlude meter.h instead of this file"
#endif

#ifndef OPENSX70_METER_CHIP_H
  #	define OPENSX70_METER_CHIP_H
  #else
  #	error "several meter chip set. Only one should be set. See settings.h"
#endif

#ifndef OPENSX70_TSL237T_H
  #define OPENSX70_TSL237T_H
  #define A100 950 //Magicnumber SX70
  #define A600 420 //Magicnumber 600

  #define A100 950
  #define A600 420

  //Numbers for auto FF
  //FD 33% of MN, FE 43% of MN
  #define FD100 317
  #define FD600 140
  #define FE100 409
  #define FE600 181
  
  /* These functions are called by the main loop.
   * They provide uniform handling for whatever light sensor is used on a specific board.
   * Note : over/under-exposure is handled by the wheel on the camera at left hand.
   * So it is transparent for the program.
   */
  
  //for memory, generic functions :
  /*
  void meter_init();
  void meter_set_iso();
  void meter_compute();
  void meter_integrate();
  bool meter_update();
  */
  void meter_led(byte _selector, byte _type);
  
  // Init timer 1 for reading sensor, enable sensor
  void lmTimer_init();
  void lmTimer_stop();
  
  // Start to integrate measure.
  void tsl237_start_integration();
  //extern void meter_set_iso(uint16_t const& iso);
  //void meter_led(byte _selector, bool _type);
#endif
