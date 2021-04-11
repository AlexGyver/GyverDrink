//void CPS() // cycles per second
//{
//  static uint32_t cnt_loop = 0;
//  static uint32_t tm_m = 0;
//  cnt_loop++;
//  if ((millis() - tm_m) >= 500)
//  {
////    Serial.println(cnt_loop / 1000.0);
//    disp.setFont(MAIN_FONT);
//    printStr("    ", Left, 0);
//    printInt(cnt_loop / 500.0, Left, 0);
//    cnt_loop = 0;
//    tm_m = millis();
//  }
//}

void loop() {
  while (1) {

#ifdef BATTERY_PIN
    while (!battery_watchdog() && !showMenu);
#endif
    encTick();
    btnTick();
    flowTick();
    LEDtick();
    timeoutTick();
    motorTick();
    keepPowerTick();
    //CPS();
  }
}
