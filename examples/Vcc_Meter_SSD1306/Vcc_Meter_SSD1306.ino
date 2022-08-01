/*  MCU Voltage by cygig v0.4.4
 *  MCUVoltage measures the voltage supply (Vcc) of Arduino without extra components.
 *  Supported board includes Uno, Leonardo, Mega as well as the ATtiny 3224/3226/3227. 
 *  This library also supports oversampling and averaging.
 *  Hardware oversampling for the ATtiny 3224/3226/3227 is also supported.
 *  
 *  https://github.com/cygig/MCUVoltage
*/

// Example: Vcc Meter SSD1306
// This will print the Vcc to a 0.96" I2C SSD1306 oled screen
// Requires relevant Adafruit libraries and hardware connections


#include <MCUVoltage.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Enter your own calibration bandgap here!
// const unsigned int calibrationBandgap=1001;
const unsigned int calibrationBandgap=0;

MCUVoltage Vcc(calibrationBandgap);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

// Alignment to draw graphics
const byte cW = 10, cH = 14,
           digitX = 50, digitY = 35,
           digitW = 70,
           rectPad = 3,
           rectX = digitX - rectPad, rectY = digitY - rectPad,
           rectW = digitW + 2 * rectPad, rectH = cH + 2 * rectPad;

byte count = 1; // counter to draw dots
const byte os = 12; // oversample to 12 bits, 13 in the case of ATtiny3224/3226/3227
const byte avg = 5; // average 5 times
const unsigned int period = 500; // wait 50ms before measuring again


void setup() {
  
  Serial.begin(9600);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  while (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }

  oled.clearDisplay();
  oled.setTextSize(2);             // Big font size
  oled.setTextColor(SSD1306_WHITE);// Draw white text
  oled.setCursor(0, 0);            // Start at top-left corner
  oled.println(F("MCUVoltage"));
  oled.setTextSize(1);             // Small font size
  oled.println(F("by cygig"));

  oled.setCursor(0, digitY);
  oled.setTextSize(2);
  oled.println(F("VCC"));

  oled.display();
}

void loop() {
  float voltage = Vcc.read_OS(os, avg);

  oled.fillRoundRect(rectX, rectY, rectW, rectH, 3, SSD1306_WHITE); // Draw white rectangle
  oled.setTextSize(2); // Big font size
  oled.setTextColor(SSD1306_BLACK); // Black text
  oled.setCursor(digitX, digitY);
  oled.print(voltage, 3);
  oled.println(F("V"));

  oled.setTextSize(1); // Small font size
  oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // White text, black background
  switch (count)
  {
    case 1:
      oled.print(F(".     "));
      break;
    case 2:
      oled.print(F(" .    "));
      break;
    case 3:
      oled.print(F("  .   "));
      break;
    case 4:
      oled.print(F("   .  "));
      break;
    case 5:
      oled.print(F("    . "));
      break;
    case 6:
      oled.print(F("     ."));
      break;
    case 7:
      oled.print(F("    . "));
      break;
    case 8:
      oled.print(F("   .  "));
      break;
    case 9:
      oled.print(F("  .   "));
      break;
    case 10:
      oled.print(F(" .    "));
      break;
  }
  count++;
  if (count > 10) {
    count = 1;
  }

  oled.setCursor(digitX, 55);
  oled.print(Vcc.getLastADCReading()); 
  oled.print(F("/"));
  oled.print(Vcc.getResolution_OS());

  oled.display();

  delay(period);

}
