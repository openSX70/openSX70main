#ifndef OPENSX70_OPENSX70_H
  #define OPENSX70_OPENSX70_H
  // Don't change the order of the includes, as there are cross dependencies !
  // All headers are included with this file, and each .cpp should include this one !
  // There are exceptions, though : specific libraries for specific hardware on the camera or for the dongle are included in their specific files.
  // includes for Arduino environnement
  #include <Arduino.h>
  //#include <EEPROM.h>
  // includes for external libraries
  #include "sx70_pcb.h"
  #include "settings.h"
  #include "camera_functions.h"
  //#include "ClickButton.h"
  #include "udongle2.h"
  #include "ClickButton.h"
  #include "eeprom_init.h"
  #include "meter.h"
#endif
