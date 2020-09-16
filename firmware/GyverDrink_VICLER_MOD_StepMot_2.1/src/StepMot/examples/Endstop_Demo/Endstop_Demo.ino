#include "StepMot.h"

/* Example of using StepMot Library
 * read about another methods in StepMot.h
 */

#define STEPS_PER_REVOLUTION 2037.88642 // 28BYJ-48 geared stepper motor
#define MICROSTEPS  1                   // 1 is full step, 2 for halfstep ...
#define STEP_PIN  16
#define DIR_PIN   10
#define EN_PIN    14

#define ENDSTOP_PIN 7 // active low switch

StepMot motor(STEPS_PER_REVOLUTION * MICROSTEPS, STEP_PIN, DIR_PIN, EN_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);
  
  motor.setMode(ABSOLUTE);                                // set to ABSOLUTE or RELATIVE modes (default RELATIVE)
  motor.setRPM(5);                                        // speed in revolutions per minute
  motor.enable();                                         // power on the motor
  motor.rotate(CCW);                                      // go in home direction
  Serial.println("go home..");
  while(digitalRead(ENDSTOP_PIN) && motor.update()) {}    // rotating until endstop switches low
  motor.resetPos();                                       // reset position
  motor.setRPM(10);
}

void loop() {

  if(!motor.update()) {
    static uint16_t angle = 0;
    
    Serial.print("Reached angle: ");
    Serial.print(motor.getAngle());
    Serial.print("  Error: ");
    Serial.println(angle - motor.getAngle());
    Serial.println();
    
    angle = random(0,360);
    motor.setAngle(angle);
    Serial.print("Target -> ");
    Serial.println(angle);
    delay(1000);
  }
}
