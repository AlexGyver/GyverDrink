// луп

void loop() {
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
  stepper.update();
}
