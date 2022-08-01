/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

// Example: Calibrate_Bandgap
// Upload code to Arduino, open the Serial Monitor.
// Measure the actual Vcc using a digital multimeter.
// Follow the instructions to enter the actual Vcc.
// Take note of the actual bandgap voltage, you can use it
// in your code to calibrate the Vcc readings by passing it in the 
// constructor or use the setBandgap(unsigned int myBandgap) function.

#include <MCUVoltage.h>

MCUVoltage Vcc;
float lastVcc, aVcc;
unsigned int newBg;
char sep[] = "--------------------";

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  // If nothing in serial buffer
  if (Serial.available()==0)
  {
    // Report Vcc
    lastVcc = Vcc.read_OS(16,10);
    Serial.print("Vcc: ");
    Serial.print(lastVcc,3);
    Serial.println("V");

    Serial.print("Bandgap: ");
    Serial.print(Vcc.getBandgap());
    Serial.println("mV");

    Serial.println("Measure the Vcc with a digital multimeter and");
    Serial.println("enter the actual value in volts into the Serial Monitor" );
    Serial.println("(e.g. 4.95)");
    Serial.println(sep);
   
  }
  
  // If there is something in the serial buffer
  else
  {
    // Read it as a float
    aVcc = Serial.parseFloat();

    // Reject the value if it is less than 0
    if (aVcc<=0)
    {
      Serial.println("Invalid value");
      Serial.println(sep);
    }

    
    else
    {
      Serial.print("Actual Vcc: ");
      Serial.print(aVcc);
      Serial.println("V");

      // Calculate the actual bandgap voltage
      newBg = aVcc/lastVcc*Vcc.getBandgap();

      Serial.print("Bandgap should be: ");
      Serial.print(newBg);
      Serial.println("mV");

      // Check the new bandgap voltage, this will fail if it is 0
      if (Vcc.setBandgap(newBg)) { Serial.println("Bandgap value updated."); }
      else { Serial.println("Bandgap updated failed. Default value used."); };
      
      // Clear the buffer
      while(Serial.available()>0){Serial.read();}
      Serial.println(sep);
    }
  }

  delay(3000);
}
