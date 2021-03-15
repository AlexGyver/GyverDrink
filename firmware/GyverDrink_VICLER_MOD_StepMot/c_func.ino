// различные функции

void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
    disp.runningString(serviceText, sizeof(serviceText), 150);
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    delay(200);
    stepper.autoPower(OFF);
    stepper.enable();
    int stepperPos = PARKING_POS;
    atHome = false;
    uint16_t pumpTime = 0;
    timerMinim timer100(100);
    dispNum(PARKING_POS);
    bool flag = false;
    while (1) {
      enc.tick();
      stepper.update();
      static int currShot = -1;

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW)) {
          if (flag) pumpTime += 100;
          else pumpTime = 0;
          disp.displayInt(pumpTime);
          pumpON();
          flag = true;
        } else {
          pumpOFF();
          flag = false;
        }

        // зажигаем светодиоды от кнопок
        for (byte i = 0; i < NUM_SHOTS; i++) {
          if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) {
            strip.setLED(i, mCOLOR(WHITE));
            shotStates[i] = EMPTY;
            currShot = i;
            dispNum((i + 1) * 1000 + shotPos[i]);
          } else if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
            if (STBY_LIGHT > 0) strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
            else  strip.setLED(i, mCOLOR(BLACK));
            shotStates[i] = NO_GLASS;
            currShot = -1;
            dispNum(stepperPos);
          }
          strip.show();
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) stepperPos += 1;
        if (enc.isRight())  stepperPos -= 1;
        stepperPos = constrain(stepperPos, 0, 240);
        stepper.setAngle(stepperPos);
        if (!flag && shotStates[currShot] == EMPTY) {
          shotPos[currShot] = stepperPos;
          dispNum((currShot + 1) * 1000 + shotPos[currShot]);
        }
        else if (!flag) dispNum(stepperPos);
      }

      if (btn.holded()) break;
    }
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
#if(STATUS_LED)
    LED = mHSV(11, 255, STATUS_LED); // orange
    strip.show();
#endif
#ifdef STEPPER_ENDSTOP
    while (homing()) stepper.update();   // двигаемся пока не сработал концевик
    stepper.setAngle(PARKING_POS);
    while (stepper.update());
#else
    stepper.setAngle(PARKING_POS);
    while (stepper.update());
#endif
    stepper.disable();
    stepper.autoPower(STEPPER_POWERSAFE);
#if(STATUS_LED)
    LED = mHSV(255, 0, STATUS_LED); // white
    strip.show();
#endif

    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 15.0f * 50.0f / time50ml; // volume per one FLOWdebouce timer tick
      EEPROM.write(500, 47);
      EEPROM.put(10, pumpTime);
    }
    // сохраняем значения углов в память
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.write(100 + i, shotPos[i]);


  }
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      bool swState = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;
      if (swState && shotStates[i] == NO_GLASS) {                   // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
        else  strip.setLED(i, mCOLOR(ORANGE));                            // подсветили
        LEDchanged = true;
        shotCount++;                                                // обновили счётчик поставленных рюмок
        if (systemState != PUMPING)
          dispNum(shotVolume[i]);                                     // отобразили объём поставленной рюмки
        DEBUG("set glass: ");
        DEBUG(i);
        DEBUG(", volume: ");
        DEBUGln(shotVolume[i]);
      }
      else if (!swState && shotStates[i] != NO_GLASS) {             // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected)
          strip.setLED(curSelected, mCOLOR(WHITE));
        else if (STBY_LIGHT > 0)
          strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
        else
          strip.setLED(i, mCOLOR(BLACK));                             // нигра
        LEDchanged = true;
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                       // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                // помпу выкл
          DEBUG("abort fill for shot: ");
          DEBUGln(i);
          volumeCount = 0;                                            // обнулили счётчик объёма жидкости реального времени
        }
        shotCount--;                                                // обновили счётчик поставленных рюмок
        if (systemState != PUMPING)
          dispNum(thisVolume);                                        // отобразили общий объём
        DEBUG("take glass: ");
        DEBUGln(i);
      }
      if (shotStates[i] == READY) rainbowFlow(1, i);
      else  rainbowFlow(0, i);
    }
    if (shotCount == 0) {                                          // если нет ни одной рюмки
      TIMEOUTtimer.start();                                        // запускаем таймер для режима ожидания
#if (STATUS_LED)
      if (timeoutState) {                                          // отключаем динамическую подсветку режима ожидания
        LEDbreathingState = false;
        LED = mHSV(255, 0, STATUS_LED); // white
      }
#endif
      if (!parking) systemON = true;
    }
    else {
      TIMEOUTtimer.stop();                                         // если стоит хоть одна рюмка - останавливаем таймер режима ожидания
    }

    if (workMode)            // авто
      flowRoutine();        // крутим отработку кнопок и поиск рюмок
    else if (systemON)    // ручной
      flowRoutine();      // если активны - ищем рюмки и всё такое
  }
}

// поиск и заливка
void flowRoutine() {
  if (systemState == SEARCH) {                            // если поиск рюмки
    bool noGlass = true;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (shotStates[i] == EMPTY && i != curPumping) {    // поиск
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения
        DEBUG("found glass: ");
        DEBUGln(curPumping);

        if (shotPos[curPumping] != (int)stepper.getAngle()) {  // если цель отличается от актуальной позиции
          stepper.enable();
          stepper.setRPM(STEPPER_SPEED);
          stepper.setAngle(shotPos[curPumping]);          // задаём цель
          parking = false;
          atHome = false;
#if(STATUS_LED)
          LED = mHSV(11, 255, STATUS_LED); // orange
          strip.show();
#endif
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки
      if (!stepper.enabled()) {
#if(STATUS_LED)
        LED = mHSV(11, 255, STATUS_LED); // orange
        LEDchanged = true;
#endif
        stepper.setAngle(PARKING_POS);
      }

#ifdef STEPPER_ENDSTOP
      if (PARKING_POS == 0) homing();
#endif

      if (stepper.ready()) {                              // приехали
        stepper.disable();                                // выключили шаговик
        systemON = false;                                 // выключили систему
        parking = 1;
        LEDbreathingState = true;
        LEDchanged = true;
        DEBUGln("parked!");
        dispNum(thisVolume);
      }
    }
    else if (!workMode && noGlass)                       // если в ручном режиме, припаркованны и нет пустых рюмок - отключаемся нахрен
      systemON = false;

  } else if (systemState == MOVING) {                    // движение к рюмке
    if (stepper.ready()) {                               // если приехали
      DEBUG("reached position: ");
      DEBUGln(stepper.getAngle());
#if(STATUS_LED)
      LED = mHSV(255, 0, STATUS_LED); // white
      strip.show();
#endif
      delay(300);
      systemState = PUMPING;                             // режим - наливание
      FLOWtimer.setInterval((long)shotVolume[curPumping] * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                 // сброс таймера
      volumeCount = 0;
      pumpON();                                          // НАЛИВАЙ!
      DEBUG("fill glass: ");
      DEBUG(curPumping);
      DEBUG(" for ");
      DEBUG((long)shotVolume[curPumping] * time50ml / 50);
      DEBUGln("ms");
    }

  } else if (systemState == PUMPING) {                    // если качаем
    dispNum(volumeCount += volumeTick);
    int colorCount = MIN_COLOR + COLOR_SCALE * volumeCount / shotVolume[curPumping];  // расчёт цвета для текущего обьёма
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      curPumping = -1;                                    // снимаем выбор рюмки
      systemState = WAIT;                                 // режим работы - ждать
      WAITtimer.reset();
      DEBUGln("WAIT");
    }
  } else if (systemState == WAIT) {
    if (WAITtimer.isReady()) {                            // подождали после наливания
      systemState = SEARCH;
      timeoutReset();
      DEBUGln("SEARCH");
    }
  }
}

// отрисовка светодиодов
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#if(STATUS_LED)
    ledBreathing(LEDbreathingState, timeoutState);
#endif
    strip.show();
  }
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState)  {
    disp.brightness(7);
    dispMode();
    dispNum(thisVolume);
  }

#if (STBY_LIGHT > 0)
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
    else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, STBY_LIGHT);
  }
#endif
#if(STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  LEDbreathingState = false;
#endif
  LEDchanged = true;
  timeoutState = true;
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();

  //  DEBUGln("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (systemState == SEARCH && timeoutState && TIMEOUTtimer.isReady()) {
    //    DEBUGln("timeout");
    timeoutState = false;
    disp.brightness(0);
    dispNum(thisVolume);
    //for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
#if (STBY_LIGHT > 0)
    for (byte i = 0; i < NUM_SHOTS; i++)  leds[i] = mHSV(20, 255, STBY_LIGHT / 2);
#endif
    selectShot = -1;
    curSelected = -1;
    LEDbreathingState = true;
    LEDchanged = true;
#if (TIMEOUT_OFF > 0)
    POWEROFFtimer.reset();
    POWEROFFtimer.start();
#endif
    if (volumeChanged) {
      volumeChanged = false;
      EEPROM.put(0, thisVolume);
    }
  }

#if(TIMEOUT_OFF > 0)
  if (POWEROFFtimer.isReady() && !timeoutState) {
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
#if(STATUS_LED)
    LED = mHSV(255, 0, 0); // off
    LEDbreathingState = false;
#endif
    LEDchanged = true;
  }
#endif
}

#ifdef STEPPER_ENDSTOP
bool homing() {
  if (atHome) return 0;

  if (ENDSTOP_STATUS) {
    //stepper.setRPM(STEPPER_SPEED);
    stepper.resetPos();
    atHome = true;
    DEBUG("at Home: ");
    DEBUGln((int)stepper.getAngle());
    return 0;
  }
  stepper.enable();
  stepper.setRPM(-STEPPER_HOMING_SPEED);
  stepper.rotate();

  DEBUGln("Homing..");
  return 1;
}
#endif

#if(STATUS_LED)
void ledBreathing(bool _state, bool mode) {
  static float _brightness = STATUS_LED;
  static int8_t _dir = -1;
  if (!_state) {
    _brightness = STATUS_LED;
    _dir = -1;
    return;
  }
  _brightness += _dir * (STATUS_LED / 50.0);
  if (_brightness < 0) {
    _brightness = 0;
    _dir = 1;
  }
  else if (_brightness > STATUS_LED) {
    _brightness = STATUS_LED;
    _dir = -1;
  }
  if (mode) LED = mHSV(130, 255, _brightness);
  else {
    LED = mHSV(255, 0, _brightness);
  }

  LEDchanged = true;
}
#endif

bool rainbowFadeFlow(uint8_t startBrightness, uint32_t period) {
  static timerMinim timer(period);
  static uint8_t count = startBrightness;
  if (!count) return 0;
  if (timer.isReady()) {
    for (byte i = 0; i < NUM_SHOTS + statusLed; i++) {
      leds[i] = mHSV(count + i * (255 / (NUM_SHOTS + statusLed) ), 255, count);
    }
    count--;
    strip.show();
  }
  return (count) ? 1 : 0;
}

void rainbowFlow(bool _state, uint8_t _shotNum) {
#if (RAINBOW_FLOW)
  static byte count[NUM_SHOTS] = {130};
  if (!_state) {
    count[_shotNum] = 130;
    return;
  }
  leds[_shotNum] = mHSV(count[_shotNum], 255, 255);
  count[_shotNum] += 1;
  LEDchanged = true;
#endif
}
