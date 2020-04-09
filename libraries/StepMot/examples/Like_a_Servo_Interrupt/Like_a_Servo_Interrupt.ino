#include "StepMot.h"

/* Example of using StepMot Library
 * read about another methods in StepMot.h
 */

#define STEPS_PER_REVOLUTION 2037.88642 // 28BYJ-48 geared stepper motor
#define MICROSTEPS  2                   // 1 for full step, 2 for halfstep ...
#define STEP_PIN    6
#define DIR_PIN     7
#define EN_PIN      8

StepMot motor(STEPS_PER_REVOLUTION * MICROSTEPS, STEP_PIN, DIR_PIN, EN_PIN);

void setup() {
  Serial.begin(9600);
  motor.setMode(ABSOLUTE);      // set to ABSOLUTE or RELATIVE modes (default RELATIVE)
  motor.setRPM(10);             // speed in revolutions per minute
  motor.enable();               // power on the motor

  // TIMER2 Configuration
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 780;            //20 kHz for update() function
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop() {

  while (Serial.available() > 0){
    int angle = Serial.parseInt();
    motor.setAngle(angle);
  }
  Serial.println(motor.getAngle());
}

ISR(TIMER1_COMPA_vect)
{
    motor.update();
}
