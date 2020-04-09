// луп

void loop() {
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
  if(stepper.update()) DEBUG(stepper.getAngle());
}
