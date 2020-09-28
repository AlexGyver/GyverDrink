/*!
 * @file Adafruit_TiCoServo.h
 *
 * Adafruit_TiCoServo library: uses Timer/Counter 1 (or 3,4,5) to allow use
 * of NeoPixels and servos in the same project (with lots of caveats -- see
 * the examples for further explanation). This is similar in ways to the
 * early (pre-0016) Arduino Servo library, but updated for new boards.
 * For AVR/ATtiny only.
 *
 * Written by Phil Burgess for Adafruit Industries, incorporating work
 * by Paul Stoffregen, Jesse Tane, Jérôme Despatis, Michael Polli and
 * Dan Clemens. The file 'known_16bit_timers.h' is taken verbatim from
 * Paul Stoffregen's fork of the TimerOne library:
 * https://github.com/PaulStoffregen/TimerOne
 * and this code is provided under the same "CC BY 3.0 US" license terms.
 *
 * Open Source License
 *
 * Adafruit_TiCoServo is free software. You can redistribute it and/or
 * modify it under the terms of Creative Commons Attribution 3.0 United
 * States License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by/3.0/us/
 */

#ifndef _ADAFRUIT_TICOSERVO_H_
#define _ADAFRUIT_TICOSERVO_H_

#include "known_16bit_timers.h" // Timer pins for various Arduino-alikes.
#include <Arduino.h>
// This header is used (instead of the stock Arduino digitalPinToTimer()
// in pins_arduino.h) because the latter doesn't always give precedence
// to 16-bit timers on certain pins; e.g. on Leonardo, pin 11 refers to
// TIMER0A instead of TIMER1C, which we'd prefer. This means that the more
// esoteric Arduino-compatibles may not work off the bat until this header
// file is suitably amended.

#ifdef TIMER1_A_PIN          ///< Defined in known_16bit_timers.h if present
typedef uint16_t servoPos_t; ///< Servo positions are 16-bit values
// Min/max pulse range is the same as stock Arduino Servo library:
#define MIN_PULSE_WIDTH 544  ///< Shortest pulse sent to a servo (microsec)
#define MAX_PULSE_WIDTH 2400 ///< Longest pulse sent to a servo (microsec)
#else                        ///< No 16-bit timers, presumably ATtiny device
typedef uint8_t servoPos_t; ///< Servo positions are 8-bit values
#define TIMER1_A_PIN 1       ///< First of two pins supported on ATtiny
#define TIMER1_B_PIN 4       ///< Second of two pins supported on ATtiny
#define __TINY_SERVO__       ///< Functionality is limited on ATtiny
#endif

/*!
    @brief  Class encapsulating Timer/Counter Servo functionality.
*/
class Adafruit_TiCoServo {
public:
#ifdef __TINY_SERVO__

  // Behavior is slightly different for ATtiny vs other devices.
  // Due to the limited timer resolution, servo read/write only work in
  // 'raw' timer/counter ticks; degrees or microseconds are never used.

  /*!
    @brief  Constructor for ATtiny devices.
  */
  Adafruit_TiCoServo(void) : pin(-1), on(false){};

  /*!
    @brief  Enable timer/counter-driven servo output on pin. Must call this
            before any write() operation. Pin selection is VERY limited,
            see examples (or known_16_bit_timers.h) for a list.
    @param  pin  Arduino pin number.
  */
  void attach(const int8_t pin);

  /*!
    @brief  Set servo position.
    @param  pos  Position in 'raw timer units,' which are a function of
                 the timer clock. On ATtiny devices, only a few distinct
                 positions are available.
  */
  void write(const servoPos_t pos);

  /*!
    @brief   Get last-set servo position.
    @return  Position in 'raw timer units,' which are a function of
             the timer clock. On ATtiny devices, only a few distinct
             positions are available.
  */
  inline servoPos_t read(void) __attribute__((always_inline)) { return *ocr; }

#else

  // On other boards, read/write are available both in degrees and
  // microseconds, consistent with original Arduino Servo library.

  /*!
    @brief  Constructor for AVR devices.
  */
  Adafruit_TiCoServo(void)
      : pin(-1), on(false), minPulse(MIN_PULSE_WIDTH),
        maxPulse(MAX_PULSE_WIDTH){};

  /*!
    @brief  Enable timer/counter-driven servo output on pin. Must call this
            before any write() operation. Pin selection is VERY limited,
            see examples (or known_16_bit_timers.h) for a list.
    @param  pin  Arduino pin number.
    @param  min  Shortest pulse sent to servo (microseconds).
    @param  max  Longest pulse sent to servo (microseconds).
  */
  void attach(const int8_t pin, const uint16_t min = MIN_PULSE_WIDTH,
              const uint16_t max = MAX_PULSE_WIDTH);

  /*!
    @brief  Set servo position.
    @param  pos  Position in either degrees or microseconds (library can
                 spot the difference, degrees are always much smaller than
                 the shortest allowable servo pulse in microseconds).
  */
  void write(const uint16_t pos);

  /*!
    @brief   Get last-set servo position (degrees).
    @return  Servo position, in degrees.
  */
  uint8_t read(void);

  /*!
    @brief   Get last-set servo position (microseconds).
    @return  Servo position, in microseconds.
  */
  uint16_t readMicroseconds(void);

#endif

  /*!
    @brief  Stop servo output on pin.
  */
  void detach(void);

  /*!
    @brief   Is TiCoServo object currently attached to a pin?
    @return  true if attached (active), false otherwise.
  */
  inline bool attached(void) __attribute__((always_inline)) { return on; }

private:
  int8_t pin;                   ///< Arduino pin #, or -1 if invalid
  volatile servoPos_t *ocr;     ///< Output compare register for pin
#ifdef TIMER3_A_PIN             ///< For chips with >1 timer/counter...
  volatile servoPos_t *counter; ///< Timer/Counter count reg. for pin
#endif                          ///< (else always TCNT1)
  boolean on;                   ///< True after first write() call
  void toggle(void);            ///< PWM on/off
#ifndef __TINY_SERVO__
  uint16_t minPulse; ///< Min pulse duration, microseconds
  uint16_t maxPulse; ///< Max pulse duration, microseconds
#endif
};

#ifndef __TINY_SERVO__
#define writeMicroseconds(x) write(x) ///< Same thing on AVR
#endif

#endif // _ADAFRUIT_TICOSERVO_H_
