# Arduino library for Stepper Motors
For all stepper motor drivers with STEP/DIR interface

### Features
* this library allows you to drive your stepper motor very precisely in ABSOLUTE mode
* the timing is based on micros() so you can do another stuff parallel to driving the motor (but don't use delays while the motor is moving)

#### Methods
```C++
StepMot(float steps_per_revolution,
        uint8_t step_pin, 
        uint8_t dir_pin, 
        uint8_t en_pin);
void setMode(bool mode);            // ABSOLUTE or RELATIVE(default)
void autoPower(bool status);        // ON: auto power on by moving and power off if stopped. OFF: power is always on (default)
void setRPM(float rpm);             // set the speed in revolutions per minute
void enable();                      // power on 
void disable();                     // power off
void setDir(bool dir);              // clockwise CW or counterclockwise CCW
void invertDir(bool invertState);   // software direction inverting
void setSteps(uint32_t steps);      // moving by steps
void setAngle(float newAngle);      // moving by angle
void rotate(bool dir);              // rotating clockwise CW or counterclockwise CCW
float getAngle();                   // get the actual position 
void resetPos();                    // reset the actual position to 0
void step();                        // make one step
bool ready();                       // get status if target is reached
bool update();                      // this method drives the motor, so it must be in the loop() function. Returns true if the motor is moving and false otherwise
```
