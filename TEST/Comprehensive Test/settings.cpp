#include "settings.h"
#include "Arduino.h"
int currentPicture = 0; //why is this set to 1?
int mxshots = 0; //Multiple exposure counter
const uint8_t YDelay = 120;
const byte PowerDownDelay = 15; //time it takes to be fully closed
const byte PowerDown = 195; //max 255 = full power/POWERUP mode
int ShutterConstant = 0; // was 9; Setting this to 0 to remove need for doing math. putting this in the raw speed. - Zane
byte lightmeterHelper = true;
//enum positions_t {AUTO600 = -100, AUTO100, POST, POSB}; //uDONGLE
//enum positions_t {POST = -100, POSB, AUTO600, AUTO100 };//ANALOGUEWORKS
//  enum positions_t {POST = -100, AUTO600, AUTO600BW, AUTO100}; //uDONGLE Optoflow
//uDONGLE WHEEL
//int ShutterSpeed[] = { 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, 302, AUTO600, AUTO100, POST, POSB }; //reduced speeds from 25 (slot5) to compensate flash firing Original U-Dongle3.3
//int ShutterSpeed[] = { 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, 302, AUTO600, AUTO600BW, AUTO100, POST }; //reduced speeds from 25 (slot5) to compensate flash firing
//int ShutterSpeed[] = { 17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, AUTO600, AUTO600BW, AUTO100, POST, POSB }; //with 600BW | reduced speeds from 25 (slot5) to compensate flash firing
//int ShutterSpeed[] = { 16, 20, 23, 25, 30, 35, 45, 55, 166, 302, 600, POST, POSB, AUTO600, AUTO600BW, AUTO100 };  //SANTI DONGLE WHEEL 600BW
//NEW WHEEL ANALOGUEDONGLE
//int ShutterSpeed[] = { 3, 5, 9, 11, 20, 30, 35, 55, 166, 302, 600, 1100, POST, POSB, AUTO600, AUTO100 };  //SANTI DONGLE WHEEL

// Added to remove the need to check for selector values prior to picture taking.
int ShutterSpeed[] = {     25,   29,   32,    34,  39,   44,  64,   64,    175,  311, 609, 1109, POST, POSB, AUTO600, AUTO100 };
//int ShutterSpeed[] = { 2000, 1000,  500,   250, 125,   60,  30,   15,      8,    4,   2,    1, T, B, AUTO600, AUTO100 };
//                          0     1     2      3    4     5    6     7       8     9    10   11
//OPTION line above are the wheel "raw" speeds (have to keep in mind smaller time = smaller aperture -> inefficient shutter)
// this are the "SLOTS":+
//int ShutterSpeed[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F };
//int ShutterSpeed[] = { EV17, EV16, EV15, EV14, EV13, EV12, EV11.5, EV11, EV10.5, EV10, EV9, EV8, AUTO600, AUTO100, T, B };
//int ShutterSpeed[] = { 2000, 1000,  500,   260, 130,   60,     45,   30,     25,   20,   8,   4, AUTO600, AUTO100, T, B };
//int ShutterSpeed[] = {   17,   20,   23,    25,  30,   35,     45,   55,     68,  102, 166, 302, AUTO600, AUTO100, POST, POSB };    //uDONGLE SETTINGS
//int ShutterSpeed[] = { 2000, 1000,  500,   250, 125,   60,     30,   15,      8,    4,   2,   1, T, B, AUTO600, AUTO100 };
//int ShutterSpeed[] = {   16,   20,   23,    25,  30,   35,     55,   55,    166,  302, 600, 1100, POST, POSB, AUTO600, AUTO100 };  //SANTI DONGLE WHEEL
// to change the speed in the slot position just change the number corresponding.
int flashDelay = 1; //new flash "system"
  // (These are default if not set, but changeable for convenience)

int MeterRange[] = {     0.20, 0.11, 0.09,  0.07, 0.07, 0.07,0.20, 0.20,   0.20, 0.20,0.20,0.20,1,1,1,1};
//int ShutterSpeed[] = { 2000, 1000,  500,   250,  125,   60,  30,   15,      8,    4,   2,   1
//                          0     1     2      3    4     5    6     7       8     9    10   11

int ShutterVariance[] = {  15,   10,   8,     6,    6,    6,   6,    6,      10,    20,   40,   40};
//int ShutterSpeed[] = { 2000, 1000,  500,   250,  125,   60,  30,   15,      8,    4,   2,   1
//                          0     1     2      3    4     5    6     7       8     9    10   11


int MinRange[] =       {   10,   20,   24,    28,   33,   38,  44,   50,    121,   200, 551,  1001};
//int ShutterSpeed[] = { 2000, 1000,  500,   250,  125,   60,  30,   15,      8,    4,   2,   1
//                          0     1     2      3    4     5    6     7       8     9    10   11

int MaxRange[] =       {   25,   29,   32,    34,   39,   49,  89,   120,    199,  550, 1000, 1250};
//int ShutterSpeed[] = { 2000, 1000,  500,   250,  125,   60,  30,   15,      8,    4,   2,   1
//                          0     1     2      3    4     5    6     7       8     9    10   11
