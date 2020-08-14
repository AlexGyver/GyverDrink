// различные функции

// сервисный режим
void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
    disp.runningString(serviceText, sizeof(serviceText), 150);
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    DEBUGln("service Mode");
    delay(200);
    servoON();
    servo.attach();
    int servoPos = PARKING_POS;
    long pumpTime = 0;
    timerMinim timer100(100);
    dispNum(PARKING_POS);
    bool flag = false;
    while (1) {
      enc.tick();
      static int currShot = -1;

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW)) {
          if (flag) pumpTime += 100;
          else  pumpTime = 0;
          disp.displayInt(pumpTime);
          flag = true;
          pumpON();
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
            dispNum(servoPos);
          }
          strip.show();
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight())  servoPos -= 1;
        servoPos = constrain(servoPos, 0, 180);
        servo.write(servoPos);
        servo.setCurrentDeg(servoPos);
        if (!flag && shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
          dispNum((currShot + 1) * 1000 + shotPos[currShot]);
        }
        else if (!flag) dispNum(servoPos);
      }

      if (btn.holded()) {
        servo.setTargetDeg(PARKING_POS);
        break;
      }
    }
    //disp.clear();
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    while (!servo.tick());
    servoOFF();
    servo.detach();

    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 15.0f * 50.0f / time50ml;
      EEPROM.write(1001, 47);
      EEPROM.put(10, pumpTime);
    }
    // сохраняем значения углов в память
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.write(100 + i, shotPos[i]);

    DEBUG("time for 1ml: ");
    DEBUGln(time50ml / 50);
    DEBUG("volume per tick: ");
    DEBUGln(volumeTick);
    DEBUGln("shot positions:");
    for (byte i = 0; i < NUM_SHOTS; i++) {
      DEBUG(i);
      DEBUG(" -> ");
      DEBUG(shotPos[i]);
      DEBUGln("°");
    }
    DEBUGln("exit service Mode");
  }
}

// выводим режим
void dispMode() {
  if (workMode) {
    if (thisVolume < 100) disp.displayByte(0, 64);
    disp.displayByte(3, 64);
  }
  else {
    if (thisVolume < 100) disp.displayByte(0, 0x00);
    disp.displayByte(3, 0x00);
  }
}

void dispNum(uint16_t num) {
  //  static uint16_t lastNum = 65535;
  //  if (num == lastNum) return;
  //  lastNum = num;

  if (num < 100) {
    if (!workMode) disp.displayByte(0, 0x00);
    else disp.displayByte(0, 0x40);
    if (num < 10) disp.displayByte(1, 0x00);
    else disp.display(1, num / 10);
    disp.display(2, num % 10);
    if (!workMode) disp.displayByte(3, 0x00);
    else disp.displayByte(3, 0x40);
  }
  else if (num < 1000) {
    disp.display(0, num / 100);
    disp.display(1, (num % 100) / 10);
    disp.display(2, num % 10);
    if (!workMode) disp.displayByte(3, 0x00);
    else disp.displayByte(3, 0x40);
  }
  else {
    disp.display(0, num / 1000);                                            // тысячные
    if ( (num % 1000) / 100 > 0 )  disp.display(1, (num % 1000) / 100);     // сотые
    else disp.displayByte(1, 0x00);
    if ( ((num % 100) / 10 > 0) || ((num % 1000) / 100 > 0) )  disp.display(2, (num % 100) / 10);         // десятые
    else disp.displayByte(2, 0x00);
    disp.display(3, num % 10);
  }
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      bool swState = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;
      if (swState && shotStates[i] == NO_GLASS) {                       // поставили пустую рюмку
        timeoutReset();                                                 // сброс таймаута
        shotStates[i] = EMPTY;                                          // флаг на заправку
        if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
        else  strip.setLED(i, mCOLOR(ORANGE));                          // подсветили оранжевым
        LEDchanged = true;
        shotCount++;                                                    // инкрементировали счётчик поставленных рюмок
        if (systemState != PUMPING)
          dispNum(shotVolume[i]);
        DEBUG("set glass: ");
        DEBUG(i);
        DEBUG(", volume: ");
        DEBUGln(shotVolume[i]);
      }
      if (!swState && shotStates[i] != NO_GLASS) {                  // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected)
          strip.setLED(curSelected, mCOLOR(WHITE));
        else if (STBY_LIGHT > 0)
          strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
        else strip.setLED(i, mCOLOR(BLACK));
        LEDchanged = true;
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                       // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                // помпу выкл
          DEBUG("abort fill for shot: ");
          DEBUGln(i);
          volumeCount = 0;
        }
        shotCount--;
        if (systemState != PUMPING)
          dispNum(thisVolume);
        DEBUG("take glass: ");
        DEBUGln(i);
      }
      if (shotStates[i] == READY) rainbowFlow(1, i);
      else  rainbowFlow(0, i);
    }
    if (shotCount == 0) {               // если нет ни одной рюмки
      TIMEOUTtimer.start();
#if (STATUS_LED)
      if (timeoutState) {               // отключаем динамическую подсветку режима ожидания
        LEDbreathingState = false;
        LED = mHSV(255, 0, STATUS_LED); // white
      }
#endif
      if (!parking) systemON = true;
    }
    else  TIMEOUTtimer.stop();

    if (workMode)           // авто
      flowRoutine();       // крутим отработку кнопок и поиск рюмок
    else if (systemON)    // ручной
      flowRoutine();     // если активны - ищем рюмки и всё такое
  }
}

// поиск и заливка
void flowRoutine() {

  if (systemState == SEARCH) {                            // если поиск рюмки
    bool noGlass = true;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (shotStates[i] == EMPTY && i != curPumping) {    // поиск
        TIMEOUTtimer.stop();
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения
        DEBUG("found glass: ");
        DEBUGln(curPumping);
        if ( abs(shotPos[i] - servo.getCurrentDeg()) > 3) {        // включаем серво только если целевая позиция не совпадает с текущей
          servoON();                                      // вкл питание серво
          servo.attach();
          servo.setTargetDeg(shotPos[curPumping]);        // задаём цель
          parking = false;
#if(STATUS_LED)
          LED = mHSV(20, 255, STATUS_LED); // orange
          strip.show();
#endif
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        else if (shotPos[i] == PARKING_POS) {             // если положение рюмки совпадает с парковочным
          servoON();                                      // вкл питание серво
          servo.attach(SERVO_PIN, PARKING_POS);
          delay(500);
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки и не припаркованны
      if (workMode && AUTO_PARKING == 0) {                // если в авто режиме:
        servoOFF();                                       // выключили серво
        servo.detach();
        systemON = false;                                 // выключили систему
        parking = true;                                   // уже на месте!
        LEDbreathingState = true;
        LEDchanged = true;
        DEBUGln("parked!");
      }
      else {                                              // если же в ручном режиме:
        servoON();                                        // включаем серво и паркуемся
        servo.attach();
        servo.setTargetDeg(PARKING_POS);
#if(STATUS_LED)
        LED = mHSV(20, 255, STATUS_LED); // orange
        LEDchanged = true;
#endif

        if (servo.tick()) {                               // едем до упора
          servoOFF();                                     // выключили серво
          servo.detach();
          systemON = false;                               // выключили систему
          parking = true;                                 // на месте!
          LEDbreathingState = true;
          LEDchanged = true;
          DEBUGln("parked!");
        }
      }
      dispNum(thisVolume);
    }
    else if (!workMode && noGlass) {                       // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
      systemON = false;
    }

  } else if (systemState == MOVING) {                     // движение к рюмке
    if (servo.tick()) {                                   // если приехали
      DEBUG("actual position: ");
      DEBUG(servo.getCurrentDeg());
      DEBUGln("°");
      servoOFF();                                         // отключаем сервопривод
      servo.detach();
#if(STATUS_LED)
      LED = mHSV(255, 0, STATUS_LED); // white
      strip.show();
#endif
      systemState = PUMPING;                              // режим - наливание
      delay(300);
      FLOWtimer.setInterval((long)shotVolume[curPumping] * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                  // сброс таймера
      volumeCount = 0;
      pumpON();                                           // НАЛИВАЙ!
      DEBUG("fill glass: ");
      DEBUG(curPumping);
      DEBUG(" for ");
      DEBUG(FLOWtimer.getInterval());
      DEBUGln("ms");
    }

  } else if (systemState == PUMPING) {                      // если качаем
    dispNum(volumeCount += volumeTick);                     // выводим текущий объём на дисплей
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
    if (WAITtimer.isReady()) {
      systemState = SEARCH;
      DEBUGln("SEARCH");
    }
  }
}

// отрисовка светодиодов по флагу (30мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#if(STATUS_LED)
    ledBreathing(LEDbreathingState, NUM_SHOTS, timeoutState);
#endif
#if(KEEP_POWER)
    keepPower();
#endif
    strip.show();
  }
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState)  disp.brightness(7);
  if (systemState != PUMPING) {
    dispMode();
    dispNum(thisVolume);
  }
  timeoutState = true;
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();
  keepPowerState = false;
#if (STBY_LIGHT)
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
    else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, STBY_LIGHT);
  }
#endif
#if(STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  LEDbreathingState = false;
#endif
#if (KEEP_POWER)
  KEEP_POWERtimer.reset();
#endif
  LEDchanged = true;
  DEBUGln("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady() && systemState == SEARCH) {
    DEBUGln("timeout");
    timeoutState = false;
    disp.brightness(0);
    dispNum(thisVolume);
    servoOFF();
    servo.detach();
#if (STBY_LIGHT)
    for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, STBY_LIGHT / 2);
#endif
    LEDbreathingState = true;
    LEDchanged = true;
    selectShot = -1;
    curSelected = -1;
    systemON = false;
#if (TIMEOUT_OFF)
    POWEROFFtimer.reset();
#endif
    if (volumeChanged) {
      volumeChanged = false;
      EEPROM.put(0, thisVolume);
    }
  }

#if(KEEP_POWER)
  if (KEEP_POWERtimer.isReady() && (shotCount == 0) && ( (KEEP_POWER < STBY_TIME) || !timeoutState) && (curSelected == -1)) {
    keepPowerState = 1;
    LEDchanged = true;
  }
#endif

#if(TIMEOUT_OFF)
  if (POWEROFFtimer.isReady() && !timeoutState) {
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
#if(STATUS_LED)
    LED = mHSV(255, 0, 0); // off
    LEDbreathingState = false;
#endif
    LEDchanged = true;
    POWEROFFtimer.stop();
  }
#endif
}

// поддержание питания повербанка коротким повышением потребления
#if(KEEP_POWER)
void keepPower() {
  static bool _dir = 1;
  static float _brightness = 1;
  uint8_t stby_brightness = STBY_LIGHT;
#if (TIMEOUT_OFF)
  stby_brightness = STBY_LIGHT * POWEROFFtimer.isOn();
#endif
  if (!timeoutState) stby_brightness /= 2;

  if (!keepPowerState) return;

  if (_brightness >= (255 - stby_brightness) ) {
    _brightness = 255 - stby_brightness;
    _dir = 0;
  }

  for (byte i = NUM_SHOTS - 1; i > 0; i--) leds[i] = leds[i - 1];
  leds[0] = mHSV(20, 255, stby_brightness + (int)_brightness);

  if (_dir) _brightness *= 1.5;
  else      _brightness /= 1.5;

  if (_brightness <= 1) {
    _brightness = 1;
    _dir = 1;
    keepPowerState = 0;
    for (byte i = 0; i < NUM_SHOTS; i++)
      leds[i] = mHSV(20, 255, stby_brightness);
  }

  LEDchanged = true;
}
#endif

// динамическая подсветка светодиодов
void rainbowFlow(bool _state, uint8_t _shotNum) {
#if (RAINBOW_FLOW)
  static float count[NUM_SHOTS] = {130};
  if (!_state) {
    count[_shotNum] = 130;
    return;
  }
  leds[_shotNum] = mHSV((int)count[_shotNum], 255, 255);
  count[_shotNum] += 0.5;
  LEDchanged = true;
#endif
}

// дыхание статус-светодиода
#if(STATUS_LED)
void ledBreathing(bool _state, uint8_t _shotNum, bool mode) {
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

// анимация TM1637
void showAnimation(byte mode) {
  static byte i = 0;
  if (mode == 0) {
    if (i >= 20) i = 0;
    disp.displayByte(AnimationData_0[i++]);
  }
  else if (mode == 1) {
    if (i >= 12) i = 0;
    disp.displayByte(AnimationData_1[i++]);
  }
  else if (mode == 2) {
    if (i >= 12) i = 0;
    disp.displayByte(AnimationData_2[i++]);
  }
  else if (mode == 3) {
    if (i >= 60) i = 0;
    disp.displayByte(AnimationData_3[i++]);
  }
  else if (mode == 4) {
    if (i >= 2) i = 0;
    disp.displayByte(AnimationData_4[i++]);
  }
  else if (mode == 5) {
    if (i >= 6) i = 0;
    disp.displayByte(AnimationData_5[i++]);
  }
  else if (mode == 6) {
    if (i >= 8) i = 0;
    disp.displayByte(AnimationData_6[i++]);
  }
  else if (mode == 7) {
    if (i >= 12) i = 0;
    disp.displayByte(AnimationData_7[i++]);
  }
}

void readEEPROM() {
  // чтение последнего налитого объёма
  if (EEPROM.read(1000) != 47) {
    EEPROM.write(1000, 47);
    EEPROM.put(0, thisVolume);
  }
  else EEPROM.get(0, thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  if (EEPROM.read(1001) != 47) {
    EEPROM.write(1001, 47);
    EEPROM.put(10, TIME_50ML);
  }
  else EEPROM.get(10, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  if (EEPROM.read(1002) != 47) {
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      EEPROM.write(100 + i, initShotPos[i]);
      shotPos[i] = initShotPos[i];
    }
  }
  else {
    for (byte i = 0; i < NUM_SHOTS; i++)
      EEPROM.get(100 + i, shotPos[i]);
  }
}

void resetEEPROM() {
  // сброс калибровки времени на 50мл
  EEPROM.write(1001, 47);
  EEPROM.put(10, TIME_50ML);

  // сброс позиций серво над рюмками
  EEPROM.write(1002, 47);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.write(100 + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }
}
