void setup() {
#if (DEBUG_UART == ON)
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
  strip.clear();
  strip.show();
  strip.setBrightness(255);
  DEBUG("strip init");

  // настройка пинов
  pinMode(PUMP_POWER, OUTPUT);
#ifdef VALVE_PIN
  pinMode(VALVE_PIN, OUTPUT);
#endif
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
  stepper.setBacklash(STEPER_BACKLASH);
  stepper.setMode(ABSOLUTE);
#ifdef STEPPER_ENDSTOP
#if (STEPPER_ENDSTOP_INVERT == 1)
  pinMode(STEPPER_ENDSTOP, INPUT);
#else
  pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif
  while (homing());   // двигаемся пока не сработал концевик
  if (PARKING_POS != 0) {
    stepper.setAngle(PARKING_POS);
    while (stepper.update());
  }
#else
  stepper.setRPM(STEPPER_SPEED);
  stepper.resetPos(PARKING_POS);
#endif
  stepper.disable();

  // animation
  timerMinim LedColorTimer(20);
  timerMinim DispFrameTimer(100);
  uint8_t _brightness = 255;
  while (_brightness) {
    if (LedColorTimer.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + 1; i++) {
        leds[i] = mHSV(_brightness + i * (255 / NUM_SHOTS + 1), 255, _brightness);
      }
      _brightness--;
      strip.show();
      if (DispFrameTimer.isReady()) showAnimation(2);
    }
  }
  strip.clear();
  HeadLED = WHITE;

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  DEBUG("ready");
}
