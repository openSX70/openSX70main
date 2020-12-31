#ifndef OPENSX70_METER_H
  #define OPENSX70_METER_H
  #include "settings.h"
  // Meter deals with integrated lightmeter.
  // These functions are to be defined for each specific meter chip that is used on the board.
  // They will implement whatever is needed for this chip to provide the needed data.
  // note : as they are hardware-specific implemented, there is no meter.cpp file.
  // As we are in the developing process, their argument may evolve, as well as their return type.
  // Pin enable, one-wire / I2C / SPI interface initialisation
  extern void meter_init();
  // give the sensitivity to the meter
  extern void meter_set_iso(uint16_t const& iso);
  // compute the time needed for current ligth condition so it can be display as an indication in the viewfinder
  extern int meter_compute(unsigned int _interval);
  // start an integration cycle for auto exposure
  extern void meter_integrate();
  // get the update from the sensor. When enough light has reached the film, returns true.
  extern bool meter_update();
  extern void meter_set_iso(uint16_t const& iso);
  extern void meter_set_iso(uint16_t const& iso, float percent);
  //void meter_led(byte _selector, bool _type);
  // preprocessor conditionnal inlusion for specific chip header file.
  #if defined (TSL237T)
    #	include "TSL237T.h"
  #elif defined (TSL235R)
    #	include "TSL235R.h"
  #elif defined (TCS3200)
    #	include "TCS3200.h"
  #elif defined (TSL2591)
    #	include "TSL2591.h"
  #else
    #	warning	"no meter included"
  #endif
#endif
