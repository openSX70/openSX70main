#ifndef OPENSX70_METER_H
#	error "inlude meter.h instead of this file"
#endif

#ifndef OPENSX70_METER_CHIP_H
  #	define OPENSX70_METER_CHIP_H
#else
  #	error "several meter chip set. Only one should be set. See settings.h"
#endif

#ifndef OPENSX70_TCS3200_H
  #define OPENSX70_TCS3200_H
  
  #define A100 400
  #define A600 150

  // Fill Flash delays (old way)
  //#define FD100 116
  //#define FD600 44

  
  /* These functions are called by the main loop.
   * They provide uniform handling for whatever light sensor is used on a specific board.
   * Note : over/under-exposure is handled by the wheel on the camera at left hand.
   * So it is transparent for the program.
   */
  
  // for memory, generic functions :
  /*
  void meter_init();
  void meter_set_iso();
  void meter_compute();
  void meter_integrate();
  bool meter_update();
  */
  
  // Init timer 1 for reading sensor, enable sensor
  void tcs3200_init();
  
  void lmTimer_stop(); //Stops the timer
  
  // Start to integrate measure.
  void tcs3200_start_integration();
  int nearest(int x, int myArray[], int elements, bool sorted);
  void meter_led(byte _selector, byte _type);
#endif
