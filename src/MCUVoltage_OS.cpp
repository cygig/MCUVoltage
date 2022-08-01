/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

/* Software Oversampling Methods */


#include "MCUVoltage.h"


/*================================================================================*/


void MCUVoltage::ADCSetup_OS(byte targetBitDepth)
{
  
  // Oversampling at least one bit above the ADC bitdepth
  if (targetBitDepth <= bitDepth){ bitDepth_OS = bitDepth+1; }
  else { bitDepth_OS = targetBitDepth; }

  // Update the resolution for oversampling
  resolution_OS=intPow(2, bitDepth_OS);

  // Update the extra bits from oversampling
  extraBits_OS = bitDepth_OS - bitDepth;
  
  // We need this many samples for oversampling
  sampleCount_OS=intPow(4, extraBits_OS );

  // Regular reading ADC setup
  ADCSetup();
}


/*================================================================================*/


// Read the ADC value of bandgap voltage against Vcc with software oversampling
unsigned long MCUVoltage::readADC_OS()
{
  
  // To store the sum of samples and averages
  // We use two unsigned long to reduce chances of overflowing
  unsigned long  sumOfSamples=0;

  // Sum the oversampled values
  for (unsigned int i=0; i<sampleCount_OS; i++)
  {
    sumOfSamples += readADC();
  }

  lastADCReading = sumOfSamples >> extraBits_OS ; // Decimate
  return lastADCReading;
}


/*================================================================================*/


// Read Vcc with software oversampling once only.
// Recommend to use the averaging method as it throws away the first reading.
unsigned long MCUVoltage::readmV_OS(byte targetBitDepth)
{
  mode = SOFTWARE_OVERSAMPLING;

  // Setup first
  ADCSetup_OS(targetBitDepth);

  // lastADCReading should be updated here
  readADC_OS(); 
  return convertToVcc(bandgap, resolution_OS, lastADCReading);
}


/*================================================================================*/


// Read Vcc with software oversampling many times and average the readings. 
// One reading is thrown away before averaging.
unsigned long MCUVoltage::readmV_OS(byte targetBitDepth, byte avgTimes)
{
  mode = SOFTWARE_OVERSAMPLING;

  // Min averaging times is 1
  if (avgTimes <1){ avgTimes=1; }
  
  // Setup before reading ADC
  ADCSetup_OS(targetBitDepth);

  // Throw away first reading
  readADC();

  unsigned long sumOfAvg=0;

  for (byte i=0; i<avgTimes; i++)
  {
    sumOfAvg += readADC_OS();
  }

  // Update lastADCReading
  lastADCReading = sumOfAvg / avgTimes;
  
  return convertToVcc(bandgap, resolution_OS, lastADCReading);

}


/*================================================================================*/


// Read the Vcc in volts with software oversampling
float MCUVoltage::read_OS(byte targetBitDepth)
{
  return readmV_OS(targetBitDepth)/1000.0;
}


/*================================================================================*/


// Read the Vcc in volts with software oversampling
float MCUVoltage::read_OS(byte targetBitDepth, byte avgTimes)
{
  return readmV_OS(targetBitDepth,avgTimes)/1000.0;
}


/*================================================================================*/


byte MCUVoltage::getBitDepth_OS()
{
  return bitDepth_OS;
}


/*================================================================================*/


unsigned long MCUVoltage::getResolution_OS()
{
  return resolution_OS;
}


/*================================================================================*/


byte MCUVoltage::getExtraBits_OS()
{
  return extraBits_OS;
}


/*================================================================================*/
