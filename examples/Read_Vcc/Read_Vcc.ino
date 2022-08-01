/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

// Example: Read_Vcc
// Upload this code to your Arduino and open the Serial monitor to 
// observe the various Vcc readings.

#include <MCUVoltage.h>

MCUVoltage Vcc;

const byte os = 13; // Oversample to 13 bits
const byte avg = 5; // Average results of 5 readings

void setup() {
  Serial.begin(9600);
}

void loop() {

  // Display device used
  Serial.print(F("Device: "));
  switch (Vcc.getDevice())
  {
    case A_UNO:
      Serial.println(F("Arduino Uno"));
      break;
    case A_LEO:
      Serial.println(F("Arduino Leonardo"));
      break;
    case A_MEGA:
      Serial.println(F("Arduino Mega"));
      break;
    case ATTINY322X:
      Serial.println(F("ATtiny3224/3226/3227"));
      break;
    default:
      Serial.println(F("Unknown"));
      break;
  }

  // Display regular readings
  Serial.print(F("Vcc: "));
  Serial.print(Vcc.read(avg),3); // Use read() to get Vcc, display to 3 decimal places
  Serial.print(F("V ("));
  Serial.print(Vcc.getLastADCReading()); // This displays the ADC reading of the bandgap voltage against Vcc as reference
  Serial.print(F("/"));
  Serial.print(Vcc.getResolution()); // Max value of the ADC reading
  Serial.print(F(", native "));
  Serial.print(Vcc.getBitDepth()); // The number of bits ADC can measure
  Serial.println(F("bits)"));
  
  // Display software oversampled readings
  Serial.print(F("Vcc: "));
  Serial.print(Vcc.read_OS(os, avg),3); // Use read_OS() for oversampled readings
  Serial.print(F("V ("));
  Serial.print(Vcc.getLastADCReading()); // lastADCReading() can be used oversampled or not
  Serial.print(F("/"));
  Serial.print(Vcc.getResolution_OS()); // Get software oversampled resolution
  Serial.print(F(", software oversampled "));
  Serial.print(Vcc.getBitDepth_OS()); // Get software oversampled bitdepth
  Serial.println(F("bits)"));


  // This part only compile for ATtiny 3324/3326/3327
  #if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)

  // Display hardware oversampled readings
  Serial.print(F("Vcc: "));
  Serial.print(Vcc.read_HWOS(16, avg),3); // Hardware oversample to 16 bits
  Serial.print(F("V ("));
  Serial.print(Vcc.getLastADCReading()); // lastADCReading() can be used oversampled or not
  Serial.print(F("/"));
  Serial.print(Vcc.getResolution_HWOS()); // Get hardware oversampled resolution
  Serial.print(F(", hardware oversampled "));
  Serial.print(Vcc.getBitDepth_HWOS()); // Get hardware oversampled bitdepth
  Serial.println(F("bits)"));
  
  #endif

  Serial.println(F("----------"));


  delay(3000);
}
