# Arduino library for Stepper Motors
For all StepStick motor drivers

### Features
this library:
* allows you to drive your stepper motor very precisely and easy!
* will not block your code so you can do your stuff and drive the motor simultaneously!
* can compensate a backlash for geared motors!
* saves your battery and protects the motor from overheating by using power saving mode!
* supports A4899, DRV8825, LV8729, TMC2100/2208/2209

#### Methods
```C++
StepMot(float steps_per_revolution,
        uint8_t step_pin, 
        uint8_t dir_pin, 
        uint8_t en_pin);
void setMode(bool mode);            // ABSOLUTE or RELATIVE(default)
void autoPower(bool status);        // ON: auto power on by moving and power off if stopped. OFF: power is always on (default)
void setBacklash(float angle);      // set backlash angle and activate compensation
void setRPM(float rpm);             // set the speed in revolutions per minute
void enable();                      // power on 
void disable();                     // power off
void setDir(bool dir);              // clockwise CW or counterclockwise CCW
void invertDir(bool invertState);   // software direction inverting
void setSteps(uint32_t steps);      // moving by steps
void setAngle(float newAngle);      // moving by angle
void rotate(bool dir);              // rotating clockwise CW or counterclockwise CCW
void rotate();                      // set speed (RPM) and rotate. Positive speed -> clockwise, negative -> counterclockwise
float getAngle();                   // get the absolute actual position
void resetPos();                    // reset the actual position to 0
void step();                        // make one step
bool ready();                       // get status if target is reached
bool update();                      // this method drives the motor, so it must be called as often as possible. Returns true if the motor is moving and false otherwise
```
