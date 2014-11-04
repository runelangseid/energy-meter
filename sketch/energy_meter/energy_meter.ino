
/** Arduino energy meter
 *
 * Connect
 * Potoresistor D2 - Interrupt 1 on UNO
 * 433,92mhz sender - D8 (DDRB 0)
 *
 * Requires
 * - TellstickSensor library (http://forum.arduino.cc/index.php?topic=211371.0)
 *
 * Notes
 *
 * Bugs
 *
 */

#include <EEPROM.h>
#include <TellstickSensor.h>

TellstickSensor Sensor1(0);

// IR
int IRledPin =  13;    // LED connected to digital pin 13

volatile int light; // interrupt flag

int count; // kwh count
int subCount; // kwh sub count

//TellstickSensor Sensor1(0);

void setup()
{
  Serial.begin(9600);
  Serial.println("START");

  count = EEPROM.read(0);
  subCount = EEPROM.read(1);
  subCount = 0;

  // Setup interrupt routine
  //attachInterrupt(0, handleInterrupt, CHANGE);
  attachInterrupt(0, handleInterrupt, FALLING);

  Sensor1.SetDeviceID(202);

}

void loop()
{
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

    Serial.print("Light - ");
    Serial.print("Count ");
    Serial.print(count);
    Serial.print(" - Subcount ");
    Serial.println(subCount);
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

