//#include "microWire.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

void setup() {
  Wire.begin();
  Wire.setClock(800000L);
  
  oled.begin(&Adafruit128x64, 0x3C); // SSD1306 / SSD1309
  //oled.begin(&SH1106_128x64, 0x3C);   // SH1106
  
  oled.setFont(System5x7);
}
void loop() {
  
  oled.clear();
  oled.setContrast(255);
  oled.print("Contrast MAX");
  delay(1000);
  
  oled.clear();
  oled.setContrast(0);
  oled.print("Contrast MIN");
  delay(1000);
}
