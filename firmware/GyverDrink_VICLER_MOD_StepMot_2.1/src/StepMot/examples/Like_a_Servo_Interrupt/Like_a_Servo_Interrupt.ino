#include "StepMot.h"

/* Example of using StepMot Library
 * 
 * Open Serial port( Baud 115200) and type an angle in degrees
 * 
 */

#define STEPS_PER_REVOLUTION 2037.88642 // 28BYJ-48 geared stepper motor
#define MICROSTEPS  2                   // 1 for full step, 2 for halfstep ...
#define STEP_PIN    6
#define DIR_PIN     7
#define EN_PIN      8

StepMot motor(STEPS_PER_REVOLUTION * MICROSTEPS, STEP_PIN, DIR_PIN, EN_PIN);

void setup() {
  Serial.begin(115200);
  motor.setMode(ABSOLUTE);      // set to ABSOLUTE or RELATIVE modes (default RELATIVE)
  motor.setRPM(10);             // speed in revolutions per minute
  motor.enable();               // power on the motor

  // TIMER2 Configuration for 20kHz refresh rate
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A = 25;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS20) | (1 <<CS21);   // 32 prescaler
  TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt
  sei();
}

void loop() {

  while (Serial.available() > 0){
    int angle = Serial.parseInt();
    motor.setAngle(angle);
  }
  Serial.println(motor.getAngle());
  
}

ISR(TIMER2_COMPA_vect)
{
    motor.update();
}
