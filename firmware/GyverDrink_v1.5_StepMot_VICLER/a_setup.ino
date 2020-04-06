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
  strip.setBrightness(255);
  strip.clear();
  //strip.setLED(5, ORANGE);
  headLight(RED);
  strip.show();
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
  stepper.setMode(ABSOLUTE);
#ifdef STEPPER_ENDSTOP
#if (STEPPER_ENDSTOP_INVERT == 1)
  pinMode(STEPPER_ENDSTOP, INPUT);
#else
  pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif
  stepper.enable();
  stepper.setRPM(STEPPER_SPEED / 4);
  stepper.rotate(CCW);
  while (ENDSTOP_STATUS && stepper.update()) {} // двигаемся пока не сработал концевик
  stepper.resetPos();
  stepper.disable();
#endif

  headLight(WHITE);
  //strip.show();

  // animation
  timerMinim durationTimer(5110); //5110
  timerMinim timer20(20);
  timerMinim timer60(60);
  strip.setBrightness(255);
  while (1) {
    if (durationTimer.isReady()) break;
    if (timer20.isReady()) {
      static byte counter = 0;
      strip.setBrightness(counter);
      for (byte i = 0; i < NUM_SHOTS; i++) {
        leds[i] = mHSV(counter + i * (255 / NUM_SHOTS), 255, 255);
      }
      strip.setBrightness(255 - counter);
      strip.show();
      counter++;
    }
    if (timer60.isReady()) showAnimation(2);
  }
  strip.clear();
  strip.setBrightness(255);
  

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
}
