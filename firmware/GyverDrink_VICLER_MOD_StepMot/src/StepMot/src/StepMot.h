/*
  Arduino library for stepper motor using STEP/DIR
  https://github.com/VICLER/StepMot

  v 1.1 - bug fixes
  v 1.2 - speed improvements
  v 1.3 - ready() function for separating update() from main loop
  v 1.4 - invert direction fix, small bugfixes and stability improvements
  v 1.5 - add backlash compensation. Set direction with rpm (positive -> CW, negative -> CCW)
  v 1.6 - bugfixes
  v 1.7 - fix moving wrong direction after rotate() with backlash
  by VICLER
*/

#pragma once

#include <Arduino.h>

enum MODE {
  RELATIVE,
  ABSOLUTE
};

enum DIR {
  CCW,
  CW
};

enum STATE {
  OFF,
  ON
};

class StepMot
{
  public:
    StepMot(float steps_per_revolution, uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin);
    void setMode(bool mode);          // ABSOLUTE or RELATIVE(default)
    void autoPower(bool status);      // ON: auto power on by moving and power off if stopped. OFF: power is always on (default)
    void setBacklash(float angle);    // set backlash angle and activate compensation
    void setRPM(float rpm);           // set the speed in revolutions per minute
    void enable();                    // power on
    void disable();                   // power off
    void setDir(bool dir);            // clockwise CW or counterclockwise CCW
    void invertDir(bool invertState); // software direction inverting
    void setSteps(uint32_t steps);    // moving by steps
    void setAngle(float newAngle);    // moving by angle
    void rotate(bool dir);            // rotating clockwise CW or counterclockwise CCW
    void rotate();                    // set speed (RPM) and rotate. Positive speed -> clockwise, negative -> counterclockwise
    float getAngle();                 // get the absolute actual position
    void resetPos(float pos = 0.0);   // reset the actual position to pos. Default is 0
    void step();                      // make one step
    bool ready();                     // get status if target is reached
    bool update();                    // this method drives the motor, so it must be in the loop() function. Returns true if the motor is moving and false otherwise
    bool enabled();                   // get status if motor is enabled

  private:
    const float _stepsPerRevolution;
    const uint8_t _stepPin;
    const uint8_t _dirPin;
    const uint8_t _enPin;
    const float _anglePerStep;
    const float _stepsPerAngle;
    uint16_t _stepPeriod = 1000;
    uint32_t _prevStepTime = 0;
    uint32_t _targetSteps = 0;
    int32_t _currentSteps = 0;
    int8_t _stepCounter = 1;
    float _targetAngle = 0.0f;
    float _currentAngle = 0.0f;
    float _lastAngle = 0.0f;
    float _backlash = 0.0f;
    bool _mode = RELATIVE;
    bool _autoPower = false;
    bool _inverted = false;
    bool _enabled = false;
    bool _ready = true;
    bool _dir = CW;
    bool _stop = true;
};
