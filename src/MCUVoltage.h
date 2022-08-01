/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

/* Header */

 
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif


#ifndef MCUVOLTAGE_H
#define MCUVOLTAGE_H

class MCUVoltage
{ 
  // Definitions
  #define REGULAR_READING 0
  #define SOFTWARE_OVERSAMPLING 1
  #define HARDWARE_OVERSAMPLING 2

  #define UNKNOWN_DEVICE 0
  #define A_UNO 1
  #define A_LEO 2
  #define A_MEGA 3
  #define ATTINY322X 4

  private:

  // 12 Bit ADC, default 1.024V reference for ATtiny3224/3226/3227
  #if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)
  
    unsigned int        bandgap = 1024;
    const byte          bitDepth = 12; //12 bit ADC
    const unsigned int  resolution = 4096; 

    // Hardware oversampling 
    byte          bitDepth_HWOS = 0 ;
    unsigned long resolution_HWOS = 0; 
    byte          extraBits_HWOS = 0;
    
    const byte minBD_HWOS = 13; // Hardware over sample to at least 13 bits
    const byte maxBD_HWOS = 17; // and at most 17 bits
    const byte defaultBD_HWOS = 16; // Defaults to 16 bit hwos


  // 10 Bit ADC for the others (eg Uno)
  // 1.1V reference for ATmega16u4/32u4, 48/88/168/328 and their 'P' versions,
  // 640/1280/1281/2560/2561
  #else
  
    unsigned int        bandgap = 1100;
    const byte          bitDepth = 10; //10 bit ADC
    const unsigned int  resolution = 1024;
    
  #endif

    // Common Private Variables
    unsigned long lastADCReading = 0;
    byte          bitDepth_OS = 0;
    byte          extraBits_OS = 0;
    unsigned long resolution_OS = 0;
    unsigned int  sampleCount_OS = 0;
    byte          mode = REGULAR_READING;
    byte          device = UNKNOWN_DEVICE;
    
    // Common Private Methods
    unsigned long precompValue;
    unsigned long intPow(byte base, byte exponent);
    unsigned long convertToVcc(unsigned long ADCReading);
    unsigned long convertToVcc(unsigned int bandgap, unsigned long resolution, unsigned long ADCReading);


        
  public:

    // Constructors
    MCUVoltage();
    MCUVoltage(unsigned int myBandgap);

    // Regular Readings
    void          ADCSetup();
    unsigned int  readADC();
    unsigned long readmV();
    unsigned long readmV(byte avgTimes);
    float         read();
    float         read(byte avgTimes);
    
    // Regular Reading Getters and Setters
    unsigned long getLastADCReading();
    unsigned int  getBandgap();
    byte          getBitDepth();
    unsigned int  getResolution();
    byte          getMode();
    byte          getDevice();
    bool          setBandgap(unsigned int myBandgap);

    // Software Oversampled Readings
    void          ADCSetup_OS(byte targetBitDepth);
    unsigned long readADC_OS();
    unsigned long readmV_OS(byte targetBitDepth);
    unsigned long readmV_OS(byte targetBitDepth, byte avgTimes);
    float         read_OS(byte targetBitDepth);
    float         read_OS(byte targetBitDepth, byte avgTimes);
    
    // Software Oversampling Getters and Setters
    byte          getBitDepth_OS();
    unsigned long getResolution_OS();
    byte          getExtraBits_OS();
    unsigned int  getSampleCount_OS();
    
    // Exclusive to ATtiny3224/3226/3227
    #if defined(__AVR_ATtiny3224__) || defined(__AVR_ATtiny3226__) || defined(__AVR_ATtiny3227__)
    
      // Hardware Oversampled Readings
      void          ADCSetup_HWOS(byte targetBitDepth);
      unsigned int  readADC_HWOS();
      unsigned long readmV_HWOS(byte targetBitDepth);
      unsigned long readmV_HWOS(byte targetBitDepth, byte avgTimes);
      float         read_HWOS(byte targetBitDepth);
      float         read_HWOS(byte targetBitDepth, byte avgTimes);

      // Hardware Oversampled Getters and Setters
      byte          getBitDepth_HWOS();
      unsigned long getResolution_HWOS();
      byte          getExtraBits_HWOS();
      
    #endif

    
    /*
     * Note on ADC readout return type:
     * readADC will be 10 or 12 bits, so unsigned int is ample.
     * However for oversampling, it can be higher, so we use unsigned long.
     * For hardware oversampling, it will be max 16 bit so unsigned int is ample, but the resolution
     * is 65536 and not 65535, thus unsigned long needed
     */

};
#endif
