
/** Arduino energy meter
 * For ATTINY8
 *
 * ATMEL ATTINY85 / ARDUINO
 *                  +-\/-+
 *            PB5  1|    |8  VCC
 *   A3 (D 3) PB3  2|    |7  PB2 (D 2) A 1
 *   A2 (D 4) PB4  3|    |6  PB1 (D 1) PWM
 *            GND  4|    |5  PB0 (D 0) PWM
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

#include <EEPROM.h>
#include <TellstickSensor.h>

// Sensor at PB0 (Pin 5)
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
  powerSender(HIGH);
  delay(10);
  Sensor1.SetHumidity((uint8_t)55);
  Sensor1.SetTemperature((int16_t)(10*10));
  Sensor1.Transmitt();
  delay(10);
  powerSender(LOW);
  delay(5000);


  // Interrupt triggered
  if (light == 1)
  {
    subCount++;
    if ( subCount >= 1000 )
    {
      subCount = 0;
      count++;

      EEPROM.write(0, count);
      EEPROM.write(1, subCount);
    }

    Sensor1.SetHumidity((uint8_t)55);
    Sensor1.SetTemperature((int16_t)(subCount*10));
    Sensor1.Transmitt();

    //Serial.print("Light - ");
    //Serial.print("Count ");
    //Serial.print(count);
    //Serial.print(" - Subcount ");
    //Serial.println(subCount);
    light = 0;
    delay(500);
    sei();
  }
}

void handleInterrupt()
{
  light = 1;
  cli();
}

/* Enables the output pins of the AVR that power the RF sender
 */
void powerSender(int flag)
{
  digitalWrite(power1, flag);
  digitalWrite(power2, flag);
}
