// луп

void loop() {
#ifdef BATTERY_PIN
  while(!battery_watchdog() && menuPage != CALIBRATION_PAGE);
#endif
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
}
