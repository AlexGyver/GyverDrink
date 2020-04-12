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
  stepper.setRPM(STEPPER_SPEED);
  stepper.setMode(ABSOLUTE);
  
#ifdef STEPPER_ENDSTOP
  if(STEPPER_ENDSTOP_INVERT == 0) pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif

  while (rainbowFadeFlow(100, 65)) {
    stepper.update();
#ifdef STEPPER_ENDSTOP
    if (!parking) {
      if(!homing()) {
        stepper.setRPM(10);
        stepper.setAngle(PARKING_POS);
        parking = true;
      }
    }
#else
    stepper.resetPos(PARKING_POS);
#endif
    showAnimation(2, 50);
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
