/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

/* Core Methods */


#include "MCUVoltage.h"


/*================================================================================*/


// Constructor
// Pass zero so the default bandgap will be used
MCUVoltage::MCUVoltage() : MCUVoltage(0)
{
  // Empty
}


/*================================================================================*/


// Constructor with user input bandgap voltage
MCUVoltage::MCUVoltage(unsigned int myBandgap)
{
  setBandgap(myBandgap);
  
  // MUST cast to unsigned long.
  precompValue = (unsigned long)bandgap*(unsigned long)resolution;

  #if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)
  
    device = ATTINY322X;
    
  #elif defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__)

    device = A_UNO;

  #elif defined(__AVR_ATmega32U4__)
  
    device = A_LEO;
    
  #elif  defined(__AVR_ATmega2560__)
    
    device = A_MEGA;

  #else

    device = UNKNOWN_DEVICE;

  #endif
  
}


/*================================================================================*/


//******************** 2021 MCU ********************//
//Compile only for ATtiny 3224/3226/3227
#if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)

  // Setup to read a single conversion in 12 bits
  void MCUVoltage::ADCSetup()
  {
    // Set reference voltage to 1.024V
    VREF.CTRLA = 0b00000000; 
    
    // Set to 256, so DAC reference voltage (also used by ADC) will be 256/256*VREF=1.024V
    AC0.DACREF = 0b11111111; 

    // Enable ADC, ignore RUNSTDBY and LOWLAT
    ADC0.CTRLA |= 0b00000001;
    
    // Set refence voltage as incoming voltage, disable PGA
    ADC0.MUXPOS = 0b00110011; 
    
    // Freerun and left adj disabled, single sample
    ADC0.CTRLF = 0b00000000; 

    // Compare against Vcc, clear last three bits, ignore TIMEBASE
    ADC0.CTRLC &= ~(0b00000111); 

    // Set 12 bit mode
    ADC0.COMMAND |= 0b00010000; 
  }
  
  // Read the ADC value of bandgap against VCC
  unsigned int MCUVoltage::readADC()
  {
    ADC0.COMMAND |= 0b00000001; // Start conversion

    // When Bit 0 of STATUS is 1, ADC is converting, wait. Conversion done when it is 0.
    while ( ADC0.STATUS > 0 ){}

    // lastADCReading enough to hold all of RESULT
    lastADCReading = ADC0.RESULT;
  
    return lastADCReading;
  }


//******************** TRADITIONAL MCU ********************//
// Compile only for ATmega16u4/32u4,
// 328/328P, 48/48P, 88/88P, 168/168P
#else

  // Setup to read a single conversion in 12 bits
  void MCUVoltage::ADCSetup()
  {
     // For Leonardo, Micro, Pro Micro
    #if defined (__AVR_ATmega16U4__) || defined(__AVR_ATmega32U4__)
    
      // Set bandgap to measure against VCC, disable left adj
      ADMUX = 0b01011110;
      // turn off MUX5 from ADCSRB ~00100000=11011111
      ADCSRB &= 0b11011111; 

    #elif defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || \
          defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

      // Set bandgap to measure against VCC, disable left adj
      ADMUX = 0b01011110;
      // turn off MUX5 from ADCSRB ~00001000=11110111
      ADCSRB &= 0b11110111; 
      
    
    #else // Else assume to be ATmega48/88/168/328 and their 'P' versions
    
      // Set bandgap to measure against VCC, disable left adj
      ADMUX = 0b01001110; 
      
    #endif

    // Enable ADC, we leave start conversion alone first (activate later on) 
    ADCSRA |= 0b10000000;   

    // Disable auto trigger ~(0b00100000) is 0b11011111
    // We leave interrupt flag, interrupt enable and prescalar alone
    ADCSRA &= 0b11011111; 
        
  }
  
  // Read the ADC value of bandgap against Vcc
  unsigned int MCUVoltage::readADC()
  {
    
    ADCSRA |= 0b01000000; // Start the conversion

    // When Bit 6 (ADSC) becomes 0, the conversion is completed
    while((ADCSRA & 0b01000000) > 0){} 
  
    lastADCReading = ADCL; // Must read ADCL first
    lastADCReading |= ADCH<<8; // Shift 8 bits to the left and add on to value
  
    return lastADCReading;
  }
#endif


/*================================================================================*/


// Read Vcc with once only.
// Recommend to use the averaging method as it throws away the first reading.
unsigned long MCUVoltage::readmV()
{
  mode = REGULAR_READING;
  
  ADCSetup(); // Always call setup before reading
  readADC(); // A copy of lastADCReading should be stored during this function
  return convertToVcc(lastADCReading);
}


/*================================================================================*/


// Read Vcc many times and average the readings. 
// One reading is thrown away before averaging.
unsigned long MCUVoltage::readmV(byte avgTimes)
{
  mode = REGULAR_READING; 

  // Min averaging times is 1
  if (avgTimes <1){ avgTimes=1; }

  // Setup before reading ADC
  ADCSetup();

  // Throw away first reading
  readADC();
  
  unsigned long ADCReadings = 0;

  for (byte i=0; i<avgTimes; i++) 
  {
    ADCReadings += readADC();
  }

  // Keep a copy of the averaged results
  lastADCReading = ADCReadings / avgTimes;

  return convertToVcc(lastADCReading); 

}


/*================================================================================*/


// Read the Vcc in volts
float MCUVoltage::read()
{
  return (float)readmV()/1000.0;
}


/*================================================================================*/


// Read Vcc in volts after averaging
float MCUVoltage::read(byte avgTimes)
{
  return (float)readmV(avgTimes)/1000.0;
}


/*================================================================================*/


unsigned long MCUVoltage::getLastADCReading()
{
  return lastADCReading;
}


/*================================================================================*/


unsigned int MCUVoltage::getBandgap()
{
  return bandgap;
}


/*================================================================================*/


byte MCUVoltage::getBitDepth()
{
  return bitDepth;
}


/*================================================================================*/


unsigned int MCUVoltage::getResolution()
{
  return resolution;
}

/*================================================================================*/


unsigned int MCUVoltage::getSampleCount_OS()
{
  return sampleCount_OS;
}


/*================================================================================*/


bool MCUVoltage::setBandgap(unsigned int myBandgap)
{
  // Bandgap cannot be zero.
  // Purposely set to 0 in the case of non-parameter constructor
  // to use defauly bandgap values
  if (myBandgap>0)
  {
    bandgap=myBandgap;
    return true;
  }

  else { return false; }
}


/*================================================================================*/


byte MCUVoltage::getMode()
{
  return mode;
}


/*================================================================================*/


byte MCUVoltage::getDevice()
{
  return device;
}


/*================================================================================*/


// Math power function but for integers only
unsigned long MCUVoltage::intPow(byte base, byte exponent)
{
  unsigned long result=1;
  for (byte i=0; i<exponent; i++)
  {
    result*=(unsigned long)base;
  }

  return result;
}


/*================================================================================*/


// This will use the precomputed value to calculated VCC in millivoltes
unsigned long MCUVoltage::convertToVcc(unsigned long ADCReading)
{
  // Math time!
  // Vbg/Vcc = ADCReading/1024
  // Vcc = (Vbg*1024)/ADCReading
  // (Vbg*1024) already precomputed    
  return precompValue/ADCReading;

}


/*================================================================================*/


// This will do the equation using all the variables
unsigned long MCUVoltage::convertToVcc(unsigned int bandgap, unsigned long resolution, unsigned long ADCReading)
{
  // Math time!
  // Vbg/Vcc = ADCReading/resolution
  // Vcc = (Vbg*resolution)/ADCReading
  return ((unsigned long)bandgap*resolution)/ADCReading;
}


/*================================================================================*/
