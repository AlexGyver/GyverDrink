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

  if (EEPROM.read(500) != 47) {
    EEPROM.write(500, 47);
    EEPROM.put(10, TIME_50ML);
  }
  EEPROM.get(10, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;


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
  stepper.setRPM(STEPPER_SPEED);
  stepper.setMode(ABSOLUTE);

#ifdef STEPPER_ENDSTOP
  if (STEPPER_ENDSTOP_INVERT == 0) pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif

  while (rainbowFadeFlow(100, 65)) {
#ifdef STEPPER_ENDSTOP
    if (!homing() && !parking) {
      stepper.setRPM(10);
      stepper.setAngle(PARKING_POS);
      if (!stepper.update()) {
        parking = true;
        DEBUG("parked!");
      }
    }
#else
    stepper.resetPos(PARKING_POS);
#endif
    showAnimation(2, 60);
  }
  stepper.setRPM(STEPPER_SPEED);
  stepper.disable();

  strip.clear();
  HeadLED = WHITE;
  strip.show();

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  DEBUG("ready");
}
