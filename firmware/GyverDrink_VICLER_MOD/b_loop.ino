// луп

void loop() {
#ifdef BATTERY_PIN
  if (!battery_watchdog()) return;
#endif
  encTick();
  btnTick();
  flowTick();
  LEDtick();
  timeoutTick();
}
