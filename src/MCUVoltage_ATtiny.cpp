/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

/* ATtiny3224/3226/3227 Hardware Oversampling Methods */


#include "MCUVoltage.h"


// Methods in this page will only be compiled if chip is ATtiny3224/3226/3227
#if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)


/*================================================================================*/


// Set up the ADC according to target bit depth
// Since this is a must-call function before reading,
// also precomputes and constrain the HWOS bit depth res and extra bits
void MCUVoltage::ADCSetup_HWOS(byte targetBitDepth)
{

  // Default to 16 bit if it does not lies between 13 and 17
  // This part not working!!!
  if (targetBitDepth < minBD_HWOS || targetBitDepth > maxBD_HWOS)
  {
    bitDepth_HWOS = defaultBD_HWOS;
  }
  else { bitDepth_HWOS = targetBitDepth; }

  // Calculate and update the oversampled resolution
  resolution_HWOS = intPow(2, bitDepth_HWOS);

  // Calculate extra bits oversampled
  extraBits_HWOS = bitDepth_HWOS - bitDepth;

  // Most of the setup is the same
  ADCSetup(); 
  
  switch (bitDepth_HWOS)
  {
    case 13:
      ADC0.CTRLF = 0b00000010; // Freerun and left adj disabled, accu 4^1=4 samples
      break;
    case 14:
      ADC0.CTRLF = 0b00000100; // Freerun and left adj disabled, accu 4^2=16 samples
      break;   
    case 15:
      ADC0.CTRLF = 0b00000110; // Freerun and left adj disabled, accu 4^3=64 samples
      break;   
    case 16:
      ADC0.CTRLF = 0b00001000; // Freerun and left adj disabled, accu 4^4=256 samples
      break;
    case 17:
      ADC0.CTRLF = 0b00001010; // Freerun and left adj disabled, accu 4^5=1024 samples
      break;
    default: // Should never go into this
      ADC0.CTRLF = 0b00000000; // Freerun and left adj disabled, accu 4^0=1 sample
      break;      
  }

  if (bitDepth_HWOS == defaultBD_HWOS)
  {
    // if 16 bits, set to singled ended reading, bursted scaling mode (scales to 16 bit)
    ADC0.COMMAND = 0b01010000;
  }
  else
  {
    // else we set singled ended reading, bursted mode, no scaling, read entire result
    ADC0.COMMAND = 0b01000000; 
  }

}


/*================================================================================*/


// Read the ADC value of bandgap voltage against Vcc with hardware oversampling
unsigned int MCUVoltage::readADC_HWOS()
{
  ADC0.COMMAND |= 0b00000001; // Set singled ended reading, bursted scaling mode and start conversion

  // When Bit 0 of STATUS is 1, ADC is converting, wait. Conversion done when it is 0.
  while ( ADC0.STATUS > 0 ){}

  // Read the whole 32 bits
  lastADCReading = ADC0.RESULT;

  return lastADCReading;
}


/*================================================================================*/


// Read Vcc with hardware oversampling once only.
// Recommend to use the averaging method as it throws away the first reading.
unsigned long MCUVoltage::readmV_HWOS(byte targetBitDepth)
{
  mode = HARDWARE_OVERSAMPLING;
  
  // Run setup before reading 
  ADCSetup_HWOS(targetBitDepth);
  
  // lastADCReading should be updated inside
  readADC_HWOS(); 

  // If not 16 bits, decimate
  // 16 bits result will be hardware scaled, so no need for that
  // use bitDepth_HWOS instead of bitDepth_HWOS because its constrained
  if (bitDepth_HWOS != defaultBD_HWOS ) { lastADCReading >>= extraBits_HWOS; }
  
  unsigned long result = convertToVcc(bandgap, resolution_HWOS, lastADCReading);

  return result;
}


/*================================================================================*/


// Read Vcc with hardware oversampling many times and average the readings. 
// One reading is thrown away before averaging.
unsigned long MCUVoltage::readmV_HWOS(byte targetBitDepth, byte avgTimes)
{
  mode = HARDWARE_OVERSAMPLING;

  // Min averaging times is 1
  if (avgTimes <1){ avgTimes=1; }
  
  // Setup before reading ADC
  ADCSetup_HWOS(targetBitDepth);
  
  // Throw away first reading
  readADC_HWOS();

  unsigned long sum=0;
  
  for (byte i=0; i<avgTimes; i++)
  {
    readADC_HWOS(); // lastADCReading updated inside
    
    // If not 16 bits, decimate
    // 16 bits result will be hardware scaled, so no need for that
    if (bitDepth_HWOS != defaultBD_HWOS) { lastADCReading >>= extraBits_HWOS; }
    
    sum += lastADCReading;
  }

  // Store a copy of the last average reading
  lastADCReading = sum / avgTimes;
  
  unsigned long result = convertToVcc(bandgap, resolution_HWOS, lastADCReading);

  return result;
}


/*================================================================================*/


// Read the Vcc in volts with hardware oversampling
float MCUVoltage::read_HWOS(byte targetBitDepth)
{
  return readmV_HWOS(targetBitDepth)/1000.0;
}


/*================================================================================*/


// Read the Vcc in volts with hardware oversampling and averaging
float MCUVoltage::read_HWOS(byte targetBitDepth, byte avgTimes)
{
  return readmV_HWOS(targetBitDepth, avgTimes)/1000.0;
}


/*================================================================================*/


byte MCUVoltage::getBitDepth_HWOS()
{
  return bitDepth_HWOS;
}


/*================================================================================*/


unsigned long MCUVoltage::getResolution_HWOS()
{
   return resolution_HWOS;
}


/*================================================================================*/


byte MCUVoltage::getExtraBits_HWOS()
{
  return extraBits_HWOS;
}


/*================================================================================*/

#endif
