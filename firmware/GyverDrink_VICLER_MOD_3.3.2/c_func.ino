
// сервисный режим
void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
    disp.runningString(serviceText, sizeof(serviceText), 150);
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    DEBUGln("service Mode");
    timerMinim timer100(100);
    bool workModeTemp = workMode;
    workMode = 1;

    //==============================================================================
    //             настройка углов под стопки и парковочной позиции
    //==============================================================================
    if (serviceState == SERVO) {
      disp.scrollByte(_dash, _1, _dash, _empty, 50);
      delay(1000);
      int servoPos = parking_pos;
      dispNum(servoPos);
      while (1) {
        enc.tick();
        static int currShot = -1;

        if (timer100.isReady()) {   // период 100 мс

          // зажигаем светодиоды от кнопок
          for (byte i = 0; i < NUM_SHOTS; i++) {
            if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) { // поставили рюмку
              strip.setLED(i, mHSV(255, 0, 50));
              strip.show();
              shotStates[i] = EMPTY;
              currShot = i;
              shotCount++;
              servoPos = shotPos[currShot];
              dispNum((i + 1) * 1000 + shotPos[i]);
              servo.setTargetDeg(servoPos);
              servoON();
              servo.start();
              while (!servo.tick());
              servoOFF();
              break;
            }
            if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {  // убрали рюмку
              if (STBY_LIGHT > 0) strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
              else  strip.setLED(i, mCOLOR(BLACK));
              strip.show();
              shotStates[i] = NO_GLASS;
              if (currShot == i)  currShot = -1;
              shotCount--;
              if (shotCount == 0) { // убрали последнюю рюмку
                servoPos = parking_pos;
                dispNum(servoPos);
                servo.setTargetDeg(servoPos);
                servoON();
                servo.start();
                while (!servo.tick());
                servoOFF();
                break;
              }
              else continue;  // если ещё есть поставленные рюмки -> ищем заново и попадаем в следующий блок
            }
            if (shotStates[i] == EMPTY && currShot == -1) { // если стоит рюмка
              currShot = i;
              servoPos = shotPos[currShot];
              dispNum((i + 1) * 1000 + shotPos[i]);
              servo.setTargetDeg(servoPos);
              servoON();
              servo.start();
              while (!servo.tick());
              servoOFF();
              break;
            }
          }
        }

        if (enc.isTurn()) {   // крутим серво от энкодера
          if (enc.isLeft()) servoPos += 1;
          if (enc.isRight())  servoPos -= 1;
          servoPos = constrain(servoPos, 0, 180);
          servoON();
          servo.attach(SERVO_PIN, servoPos);
          if (shotCount == 0) parking_pos = servoPos;
          if (shotStates[currShot] == EMPTY) {
            shotPos[currShot] = servoPos;
            dispNum((currShot + 1) * 1000 + shotPos[currShot]);
          }
          else dispNum(servoPos);
        }

        if (btn.holded()) {
          disp.displayByte(0x00, 0x00, 0x00, 0x00);
          serviceState = VOLUME;
          servoOFF();
          break;
        }
      }
      // сохраняем значения углов в память
      for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.update(6 + i, shotPos[i]);
      // сохраняем значение домашней позиции
      EEPROM.update(13, parking_pos);
      workMode = workModeTemp;
    }
    //==============================================================================
    //                            калибровка объёма
    //==============================================================================
    if (serviceState == VOLUME) {
      while (!digitalRead(BTN_PIN));  // ждём отпускания
      disp.scrollByte(_dash, _2, _dash, _empty, 50);
      delay(1000);
      long pumpTime = 0;
      bool flag = false;
      disp.displayInt(pumpTime);
      while (1) {

        if (timer100.isReady()) {
          // работа помпы со счётчиком
          if (!digitalRead(ENC_SW) && shotCount) {
            if (flag) pumpTime += 100;
            else  pumpTime = 0;
            disp.displayInt(pumpTime);
            flag = true;
            pumpON();
          } else {
            pumpOFF();
            flag = false;
          }

          for (byte i = 0; i < NUM_SHOTS; i++) {
            if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) {
              strip.setLED(i, mHSV(20, 255, 255));
              strip.show();
              shotStates[i] = EMPTY;
              shotCount++;
              servoON();
              servo.setTargetDeg(shotPos[i]);
            }
            else if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
              if (STBY_LIGHT > 0) strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
              else  strip.setLED(i, mCOLOR(BLACK));
              strip.show();
              shotStates[i] = NO_GLASS;
              shotCount--;
              if (shotCount == 0) {
                servoON();
                servo.setTargetDeg(parking_pos);
              }
              else if (shotPos[i] == servo.getCurrentDeg()) {
                for (byte i = 0; i < NUM_SHOTS; i++) {
                  if (shotStates[i] == EMPTY) {
                    servoON();
                    servo.setTargetDeg(shotPos[i]);
                    continue;
                  }
                }
              }
            }
          }
        }

        if (servo.tick()) servoOFF();
        else servoON();

        if (btn.holded()) {
          servo.setTargetDeg(parking_pos);
          disp.displayByte(0x00, 0x00, 0x00, 0x00);
          for (byte i = 0; i < NUM_SHOTS; i++) {
            if (STBY_LIGHT > 0) strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
            else  strip.setLED(i, mCOLOR(BLACK));
          }
          while (!servo.tick());
          servoOFF();
#ifdef BATTERY_PIN
          serviceState = BATTERY;
#else
          serviceState = SERVO;
#endif
          break;
        }
      }
      // сохраняем настройки таймера налива
      if (pumpTime > 0) {
        time50ml = pumpTime;
        volumeTick = 15.0f * 50.0f / time50ml;
        EEPROM.put(2, pumpTime);
      }
    }
    //==============================================================================
    //                     калибровка напряжения аккумулятора
    //==============================================================================
#ifdef BATTERY_PIN
    if (serviceState == BATTERY) {
      while (!digitalRead(BTN_PIN));  // ждём отпускания
      disp.scrollByte(_dash, _1, _dash, _empty, 50);
      delay(1000);
      dispNum(get_battery_voltage() * 1000, 1);
      while (1) {
        enc.tick();
        if (timer100.isReady()) dispNum(get_battery_voltage() * 1000, 1);

        if (enc.isTurn()) {
          if (enc.isLeft())   battery_cal += 0.01;
          if (enc.isRight())  battery_cal -= 0.01;
          battery_cal = constrain(battery_cal, 0, 3.0);
        }
        if (btn.holded()) {
          disp.scrollByte(0x00, 0x00, 0x00, 0x00);
          serviceState = SERVO;
          break;
        }
      }
      // сохраняем калибровку аккумулятора
      EEPROM.put(14, battery_cal);
    }
#endif

    DEBUG("time for 1ml: ");
    DEBUGln(time50ml / 50);
    DEBUG("volume per tick: ");
    DEBUGln(volumeTick);
    DEBUGln("shot positions:");
    DEBUG("parking position: ");
    DEBUGln(parking_pos);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      DEBUG(i);
      DEBUG(" -> ");
      DEBUG(shotPos[i]);
      DEBUGln("°");
    }
    DEBUGln("exit service Mode");
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
          servo.setTargetDeg(shotPos[curPumping]);        // задаём цель
          parking = false;
#if(STATUS_LED)
          LED = mHSV(20, 255, STATUS_LED); // orange
          strip.show();
#endif
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        else if (shotPos[i] == parking_pos) {             // если положение рюмки совпадает с парковочным
          servoON();                                      // вкл питание серво
          servo.attach(SERVO_PIN, parking_pos);
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
        systemON = false;                                 // выключили систему
        parking = true;                                   // уже на месте!
        LEDbreathingState = true;
        LEDchanged = true;
        DEBUGln("parked!");
      }
      else {                                              // если же в ручном режиме:
        servoON();                                        // включаем серво и паркуемся
        servo.setTargetDeg(parking_pos);
#if(STATUS_LED)
        LED = mHSV(20, 255, STATUS_LED); // orange
        LEDchanged = true;
#endif

        if (servoReady) {                               // едем до упора
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
    if (servoReady) {                                   // если приехали
      DEBUG("actual position: ");
      DEBUG(servo.getCurrentDeg());
      DEBUGln("°");
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

// прокачка
void prePump() {
  for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
    if (!digitalRead(SW_pins[i])) {         // нашли рюмку
      curPumping = i;
      if (abs(servo.getCurrentDeg() - shotPos[i]) <= 3) break;
      servoON();
      servo.start();
      servo.setTargetDeg(shotPos[curPumping]);
      parking = false;
      break;
    }
  }
  if (curPumping == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
  if (!timeoutState) disp.brightness(7);
  DEBUG("pumping into shot ");
  DEBUGln(curPumping);
  while (!servo.tick()); // едем к рюмке
  servoOFF();
  delay(300); // небольшая задержка перед наливом

  pumpON(); // включаем помпу
  timerMinim timer20(20);
  while (!digitalRead(SW_pins[curPumping]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
  {
    if (timer20.isReady()) {
      volumeCount += 20 * 50.0 / time50ml;
      dispNum(round(volumeCount));
      strip.setLED(curPumping, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
      strip.show();
    }
  }
  pumpOFF();
  DEBUG("pumping stopped, volume: ");
  DEBUG(round(volumeCount));
  DEBUGln("ml");
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState)  {
    disp.brightness(7);
    if (!volumeChanged) {
      disp.displayByte(0x00, 0x00, 0x00, 0x00);
      if (workMode) disp.scrollByte(64, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 64, 50);
      else  disp.scrollByte(0, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 0, 50);
    }
  }
  if ( (systemState != PUMPING)  && (curSelected < 0) ) dispNum(thisVolume);
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
      EEPROM.update(0, thisVolume);
    }
    EEPROM.update(19, workMode);
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

// обработка движения серво
void servoTick() {
  if (servo.tick()) {
    servoOFF();
    servoReady = 1;
  }
  else {
    servoON();
    servoReady = 0;
  }
}

// отрисовка светодиодов по флагу (30мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#if(STATUS_LED)
    ledBreathing(LEDbreathingState, timeoutState);
#endif
#if(KEEP_POWER)
    keepPower();
#endif
    strip.show();
  }
}

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

// battery management
#ifdef BATTERY_PIN
float filter(float value) {
  static float k = 1.0, filteredValue = 4.0;
  if (battery_voltage < (BATTERY_LOW)) k = 1.0;
  else k = 0.1;
  filteredValue = (1.0 - k) * filteredValue + k * value;
  return filteredValue;
}
float get_battery_voltage() {
  battery_voltage = filter(analogRead(BATTERY_PIN) * (4.7 * battery_cal) / 1023.f);
  DEBUG("battery: ");
  DEBUG(battery_voltage);
  DEBUGln("V");
  return battery_voltage;
}
bool battery_watchdog() {
  static uint32_t lastMillis = 0;
  static bool batOk, lastOkStatus = 1;
  if ( (millis() - lastMillis) >= 1000) {
    lastMillis = millis();
    batOk = (get_battery_voltage() < (float)BATTERY_LOW) ? 0 : 1;
    if (!batOk) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, 0);
#if(STATUS_LED)
      LED = mHSV(255, 0, 0);
#endif
      strip.show();
      timeoutState = false;
      disp.brightness(0);
      disp.displayByte(0x39, 0x09, 0x09, 0x0F);
      delay(500);
      disp.displayByte(0x00, 0x00, 0x00, 0x00);
    }
    else if (!lastOkStatus) timeoutReset();
    lastOkStatus = batOk;
  }
  return batOk;
}
#endif
