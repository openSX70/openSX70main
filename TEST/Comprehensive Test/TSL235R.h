/*
 * openSX70 : opensource project to take the SX70 instant camera beyond its original functionnalities
 * This file is part of openSX70 project.
 *
 *
 * This project is licensed under the terms of CreactiveCommons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)
 *
 * You are free to :
 * Share — copy and redistribute the material in any medium or format
 *
 * Adapt — remix, transform, and build upon the material 
 * The licensor cannot revoke these freedoms as long as you follow the license terms.
 *
 *
 * Under the following terms :
 * Attribution — You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made.
 * You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use. 
 *
 * NonCommercial — You may not use the material for commercial purposes.
 *
 * No additional restrictions — You may not apply legal terms or technological measures that legally restrict
 * others from doing anything the license permits.
 * 
 *
 * Notices:
 * You do not have to comply with the license for elements of the material in the public domain or
 * where your use is permitted by an applicable exception or limitation.
 *
 * No warranties are given. The license may not give you all of the permissions necessary for your intended use.
 * For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.
 *
 * Complete license terms are available at : https://creativecommons.org/licenses/by-nc/4.0/legalcode
 */

#ifndef OPENSX70_METER_H
#	error "inlude meter.h instead of this file"
#endif

#ifndef OPENSX70_METER_CHIP_H
#	define OPENSX70_METER_CHIP_H
#else
#	error "several meter chip set. Only one should be set. See settings.h"
#endif

#ifndef OPENSX70_TSL235R_H
#define OPENSX70_TSL235R_H

#define A100 485
#define A600 225
#define A400 300 //need to estimate right Value for BW 600


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

void meter_led(byte _selector, byte _type);

// Init timer 1 for reading sensor, enable sensor
void tsl235_init();

// Start to integrate measure.
void tsl235_start_integration();

//extern void meter_set_iso(uint16_t const& iso);
//void meter_led(byte _selector, byte _type);

#endif
