
void loop() {
#ifdef BATTERY_PIN
  while (!battery_watchdog() && !showMenu);
#endif
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
  servoTick();

}
