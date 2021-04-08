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

#include "open_sx70.h"
#if defined (TSL235R)
volatile bool integrationFinished = 0;

uint16_t outputCompare = A100;

const uint8_t PIN_OE = 9;

void meter_init(){
  tsl235_init();
}

void meter_set_iso(const uint16_t& iso){
    if (iso == ISO_600) {
      outputCompare = A600;
    } else if (iso == ISO_SX70) {
      outputCompare = A100;
    } else if (iso == ISO_600BW){
      outputCompare = A400;
    }
}

void meter_compute(){

}

void meter_integrate(){
	tsl235_start_integration();
}

bool meter_update(){
	if(integrationFinished){
		integrationFinished = 0;
		return 1;
	}
	return 0;
}



// initialise Timer 1 for light sensor integration.
void tsl235_init(){
//	pinMode(PIN_OE, OUTPUT);
//	digitalWrite(PIN_OE, LOW);

	cli();
	// Clear all interrupts flags
	TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);
	// Set timer 1 for normal operation, clocked by rising edge on T1 (port D5 / pin 5)
	TCCR1A = 0;
	TCCR1B = (1 << CS10) | (1 << CS11) | (1 << CS12);
	TCCR1C = 0;
	// set current value to 0 (needs to be done, maybe, on each new conversion)
	TCNT1 = 0;
	sei();										// Restart interrupts
}

// Start a new measure for pose.
void tsl235_start_integration(){
	cli();
	// Clear interrupts flags we are using
//	TIFR1 = (1 << OCF1A) | (1 << TOV1);
	TIFR1 = (1 << OCF1A);
	// set compare value given sensivity
	OCR1A = outputCompare;
	// clear counter value.
	TCNT1 = 0;
	// Set interrupt vectors for compare match A.
//	TIMSK1 = (1 << OCIE1A) | (1 << TOIE1);
	TIMSK1 = (1 << OCIE1A);

	sei();
}

// ISR for complete conversion. Should set a flag read by the main loop.
ISR(TIMER1_COMPA_vect){
	TIMSK1 = 0;
	integrationFinished = 1;

	// function / flag.
}

int nearest(int x, int myArray[], int elements, bool sorted)
// int nearest(int x, int myArray[], int elements, bool sorted)

{
  int idx = 0; // by default near first element

  int distance = abs(myArray[idx] - x);
  for (int i = 1; i < elements; i++)
  {
    int d = abs(myArray[i] - x);
    if (d < distance)
    {
      idx = i;
      distance = d;
    }
    else if (sorted) return idx;
  }
  return idx;
}
void meter_led(byte _selector, byte _type)
{
  int PredictedExposure = meter_compute(200);
  if (PredictedExposure == -1)
  {
    return;
  }
  if (_type == 1)
  {
    int slot = nearest(PredictedExposure, ShutterSpeed, 11, false);

/*
    Serial.print("PredictedExposure: ");
    Serial.println(PredictedExposure);

    Serial.print("Estimated SLOT: ");
    Serial.println(slot);
    Serial.print("Actual SLOT: ");
    Serial.println(_selector);
*/
    if (_selector < slot)
    {
      //     Serial.println("_selector < slot");
      digitalWrite(PIN_LED1, HIGH);
      digitalWrite(PIN_LED2, LOW);
      //digitalWrite(PIN_LED1, LOW);
      //digitalWrite(PIN_LED2, HIGH);

      return;
    }
    else if (_selector > slot)
    {
      //     Serial.println("_selector > slot");
      digitalWrite(PIN_LED2, HIGH);
      digitalWrite(PIN_LED1, LOW);
      //digitalWrite(PIN_LED2, LOW);
      //digitalWrite(PIN_LED1, HIGH);

      return;
    }
    else if (_selector == slot)
    {

      //   Serial.println("_selector == slot");
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      //digitalWrite(PIN_LED2, HIGH);
      //digitalWrite(PIN_LED1, HIGH);

      return;
    }
  }
  if (_type == 0)
  {
    if (PredictedExposure > 100)
    {
      digitalWrite(PIN_LED2, HIGH);
      digitalWrite(PIN_LED1, HIGH);
      Serial.println("LOW LIGHT");
      return;

    } else
    {
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);

    }
  }
}

int meter_compute(unsigned int _interval) /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  int _myISO = ReadISO();
  static uint32_t previousMillis = 0;
  static bool measuring = false;
  //long interval = 200;
  //uint32_t counter; defined elsewhere

  //uint32_t PredExp;
  float PredExp;

  //  uint32_t currentMillis = millis();
  //  uint32_t timeMillis;
  meter_set_iso(_myISO);

  if (!measuring)
  {

    meter_init();
    previousMillis = millis();
    measuring = true;
  }
  else
  {
    uint32_t myMillis = millis() - previousMillis;

    if (myMillis  >= _interval )
    {
      uint32_t counter = TCNT1;
      measuring = false;
      PredExp = (((float)myMillis) / ((float) counter)) * (float)outputCompare;

      //Serial.print("pr mil: ");
      //Serial.println(previousMillis);
      //Serial.print("mil: ");
      //Serial.println(myMillis);

      //Serial.print("_interval: ");
      //Serial.println(_interval);
      //Serial.print("counter: ");
      //Serial.println(counter);
      //Serial.print("output compare: ");
      //Serial.println(outputCompare);
      /*
        Serial.print("                      PredExp: ");
        Serial.println(PredExp);

      */
      //Serial.print("                      PredExp: ");
      //Serial.println(PredExp);

           PredExp = PredExp + ShutterConstant;
/*
      if (PredExp > 1100)
      {
        //    PredExp = 333;
        return 1100;
      }
      else if (PredExp < 18)
      {
        //    PredExp = 16;
        return 16;
      }*/
     // PredExp = PredExp + ShutterConstant;
      return PredExp;
    }
  }
  return -1;
}

#endif
