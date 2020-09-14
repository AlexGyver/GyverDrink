#include "StepMot.h"

/* Example of using StepMot Library
 * read about another methods in StepMot.h
 */

#define STEPS_PER_REVOLUTION  2037.88642  // 28BYJ-48 geared stepper motor
#define MICROSTEPS            1          // 1 for full step, 2 for halfstep ...
#define STEP_PIN              16
#define DIR_PIN               10
#define EN_PIN                14

StepMot motor(STEPS_PER_REVOLUTION * MICROSTEPS, STEP_PIN, DIR_PIN, EN_PIN);

void setup() {
  Serial.begin(9600);
  motor.setMode(ABSOLUTE);  // set to ABSOLUTE or RELATIVE modes (default RELATIVE)
  motor.setRPM(10);        // speed in revolutions per minute
  motor.enable();         // power on the motor
}

void loop() {

  while (Serial.available() > 0){
    int angle = Serial.parseInt();
    motor.setAngle(angle);
  }

  if(motor.update()) Serial.println(motor.getAngle());

}
