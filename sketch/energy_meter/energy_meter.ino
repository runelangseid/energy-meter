
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
 * @todo Potoresistor D2 - Interrupt 1 on UNO ???
 * 433,92mhz sender - AVR Pin 5 (PB0 - DDRB 0)
 * Power to sender - AVR Pin 2,3 (PB3, PB4)
 *
 * Requires
 * - TellstickSensor library (http://forum.arduino.cc/index.php?topic=211371.0)
 * - Board setup for the ARduino IDE - https://github.com/damellis/attiny
 *
 * Notes
 *
 * Bugs
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

int count; // kwh count
int subCount; // kwh sub count

volatile int light; // interrupt flag

void setup()
{
  // Setup power pins
  pinMode(power1, OUTPUT);
  pinMode(power2, OUTPUT);

  // Set sensor device id
  Sensor1.SetDeviceID(202);

  //Serial.begin(9600);
  //Serial.println("START");

  count = EEPROM.read(0);
  subCount = EEPROM.read(1);
  subCount = 0;

  // Setup interrupt routine
  //attachInterrupt(0, handleInterrupt, CHANGE);
//  attachInterrupt(0, handleInterrupt, FALLING);


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

  // Interrupt triggered
  //if (light == 1)
  {
    subCount++;
    if ( subCount >= 1000 )
    {
      subCount = 0;
      count++;

      EEPROM.write(0, count);
      EEPROM.write(1, subCount);
    }

    powerSender(HIGH);
    delay(100);
    Sensor1.SetHumidity((uint8_t)55);
    Sensor1.SetTemperature((int16_t)(subCount*10));
    Sensor1.Transmitt();
    delay(10);
    powerSender(LOW);

    //Serial.print("Light - ");
    //Serial.print("Count ");
    //Serial.print(count);
    //Serial.print(" - Subcount ");
    //Serial.println(subCount);
    light = 0;
    delay(500);
    //sei();
  }
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
