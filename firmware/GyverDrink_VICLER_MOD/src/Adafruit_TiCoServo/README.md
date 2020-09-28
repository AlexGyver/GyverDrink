#Adafruit_TiCoServo [![Build Status](https://github.com/adafruit/Adafruit_TiCoServo/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_TiCoServo/actions)
==================

An alternate servo library when NeoPixels and servos are both needed in the same sketch (the stock Arduino Servo library stutters with the addition of NeoPixels). Has some limitations of its own, see examples for pros and cons. See Adafruit_TiCoServo.cpp for attribution.

This library only handles the servo control, you'll still need to install and use the NeoPixel library for the LEDs. It's not dependent on that library though, and could be used separately for other (non-NeoPixel) situations that might benefit from hardware-PWM-driven servos.

Compatible pins are determined by the specific hardware. 
* Arduino Uno, Diecimila and Adafruit Pro Trinket, pins 9 and 10 are supported.
* Arduino Leonardo: pins 5, 9, 10 and 11. 
* Adafruit Flora: pins D9, D10. 
* PJRC Teensy 2.0 (not Teensy++ or 3.X): pins 4, 9, 14, 15.
* Arduino Mega: pins 2, 3, 5, 6, 7, 8, 11, 12, 13, 44, 45, 46.
