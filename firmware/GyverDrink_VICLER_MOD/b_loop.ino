// луп

void loop() {
#ifdef BATTERY_PIN
  while (!battery_watchdog());
#endif
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
  servoTick();
}
