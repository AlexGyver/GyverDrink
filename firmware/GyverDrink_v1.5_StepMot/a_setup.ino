void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUG("start");
#endif
  // епром
  if (EEPROM.read(1000) != 10) {
    EEPROM.write(1000, 10);
    EEPROM.put(0, thisVolume);
  }
  EEPROM.get(0, thisVolume);

  // тыкаем ленту
  strip.setBrightness(130);
  strip.clear();
  strip.show();
  DEBUG("strip init");

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (SWITCH_LEVEL == 0) pinMode(SW_pins[i], INPUT_PULLUP);
  }

  // старт дисплея
  disp.clear();
  disp.brightness(7);
  DEBUG("disp init");

  // настройка шагового двигателя
  stepper.autoPower(STEPPER_POWERSAFE);
  stepper.invertDir(INVERT_STEPPER);
  stepper.setMode(ABSOLUTE);
#ifdef STEPPER_ENDSTOP
#if (STEPPER_ENDSTOP_INVERT == 1) 
  pinMode(STEPPER_ENDSTOP, INPUT);
#else 
  pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif
  stepper.enable();
  stepper.rotate(CCW);
  stepper.setRPM(STEPPER_SPEED / 2);
  while (ENDSTOP_STATUS && stepper.update()) {} // двигаемся пока не сработал концевик
  stepper.resetPos();
#endif
  stepper.disable();


  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
}
