
/** Arduino energy meter
 * For ATTINY8
 *
 * ATMEL ATTINY85 / ARDUINO
 *                         +-\/-+
 *                    PB5  1|    |8  VCC
 *          A 3 (D 3) PB3  2|    |7  PB2 (D 2) A 1 INT0
 *   PCINT4 A 2 (D 4) PB4  3|    |6  PB1 (D 1) PWM PCINT1
 *                    GND  4|    |5  PB0 (D 0) PWM PCINT0
 *
 *
 * Connect
 * PB2 (int0)   Potoresistor     (AVR Pin 7)
 * PB0          433,92mhz sender (AVR Pin 5)
 * PB3 and PB4  Power to sender  (AVR Pin 2&3)
 *
 * Requires
 * - TellstickSensor library (http://forum.arduino.cc/index.php?topic=211371.0)
 * - Board setup for the ARduino IDE - https://github.com/damellis/attiny
 *
 * Notes
 * - Sleep-mode reference https://gist.github.com/JChristensen/5616922
 *
 * Bugs
 * - Interrupt pin must not be low then AVR goes to sleep (!!!)
 *
 */

#include <avr/sleep.h>
#include <EEPROM.h>
#include <TellstickSensor.h>

#define BODS 7                     //BOD Sleep bit in MCUCR
#define BODSE 2                    //BOD Sleep enable bit in MCUCR

// Sensor at PB0
TellstickSensor Sensor1(0);

// Power pins PB3 & PB4
int power1 = 3;
int power2 = 4;

// Indicator led PB1
int led = 1;

int count = 0; // count

void setup()
{
  // Setup power pins
  pinMode(power1, OUTPUT);
  pinMode(power2, OUTPUT);

  // led
  pinMode(led, OUTPUT);

  // Set sensor device id
  Sensor1.SetDeviceID(202);

  delay(1000);
}

void loop()
{
  // tiny test - Transmit
  /*
  powerSender(HIGH);
  delay(10);
  Sensor1.SetHumidity((uint8_t)55);
  Sensor1.SetTemperature((int16_t)(10*10));
  Sensor1.Transmitt();
  delay(10);
  powerSender(LOW);
  delay(5000);
  */
  
  goToSleep();

  count++;
  if ( count >= 10 ) // debug
  {
    count = 0;
    sendCommand(1);
    delay(100);
  }
  else
  {
    digitalWrite( led, HIGH ); // led
    delay(400);
    digitalWrite( led, LOW ); // led
  }

}

/* Transmitt RF data
 * @param int Value to transmitt
 */
void sendCommand(int value)
{
  powerSender(HIGH);
  delay(100);
  Sensor1.SetHumidity((uint8_t)value);
  Sensor1.SetTemperature((int16_t)(40*10));
  Sensor1.Transmitt();
  delay(10);
  powerSender(LOW);
}

void goToSleep(void)
{
    byte adcsra, mcucr1, mcucr2;

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    MCUCR &= ~(_BV(ISC01) | _BV(ISC00));      //INT0 on low level
    GIMSK |= _BV(INT0);                       //enable INT0
    adcsra = ADCSRA;                          //save ADCSRA
    ADCSRA &= ~_BV(ADEN);                     //disable ADC
    cli();                                    //stop interrupts to ensure the BOD timed sequence executes as required
    mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
    mcucr2 = mcucr1 & ~_BV(BODSE);            //if the MCU does not have BOD disable capability,
    MCUCR = mcucr1;                           //  this code has no effect
    MCUCR = mcucr2;
    sei();                                    //ensure interrupts enabled so we can wake up again
    sleep_cpu();                              //go to sleep
    sleep_disable();                          //wake up here
    ADCSRA = adcsra;                          //restore ADCSRA
}

ISR(INT0_vect)
{
  GIMSK = 0; // disable external interrupts - and wake up
}

/* Enables the output pins of the AVR that power the RF sender
 */
void powerSender(int flag)
{
  digitalWrite(power1, flag);
  digitalWrite(power2, flag);
}

