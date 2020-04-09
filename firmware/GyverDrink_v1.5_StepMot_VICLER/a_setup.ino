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
  stepper.enable();
  stepper.setRPM(5);
  stepper.rotate(CCW);
  HeadLED = ORANGE;
  while (ENDSTOP_STATUS && stepper.update()); // двигаемся пока не сработал концевик
  stepper.resetPos();
  stepper.setRPM(STEPPER_SPEED);
  stepper.setAngle(PARKING_POS);
  while(stepper.update());
  stepper.disable();
  parking = true;
#else 
  stepper.setRPM(STEPPER_SPEED);
  stepper.resetPos(PARKING_POS);
#endif

  // animation
  timerMinim durationTimer(5110); //5110
  timerMinim timer20(20);
  timerMinim timer60(60);
  byte anim = random(0, 6);
  while (!durationTimer.isReady()) {
    if (timer20.isReady()) {
      static byte counter = 0;
      strip.setBrightness(counter);
      for (byte i = 0; i < NUM_SHOTS + 1; i++) {
        leds[i] = mHSV(counter + i * (255 / NUM_SHOTS + 1), 255, 255);
      }
      strip.setBrightness(255 - counter);
      strip.show();
      counter++;
    }
    if (timer60.isReady()) showAnimation(anim);
  }
  strip.clear();
  strip.setBrightness(255);
  HeadLED = WHITE;

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  DEBUG("ready");
}
