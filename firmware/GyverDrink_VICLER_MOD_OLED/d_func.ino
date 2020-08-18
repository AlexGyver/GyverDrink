// различные функции
void serviceRoutine(serviceModes mode) {
  if (mode == VOLUME) {                      // калибровка объёма
    long pumpTime = 0;
    timerMinim timer100(100);
    bool flag = false;
    disp.setInvertMode(1);
    printStr("  Калибр. объ¿ма  \n", 0, 0);
    disp.setInvertMode(0);
    printStr("Поставьте рюмку и\n", 0, 3);
    printStr("налейте 50мл\n");
    while (curPumping == -1) {
      for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
        if (!digitalRead(SW_pins[i])) {         // нашли рюмку
          strip.setLED(i, mCOLOR(WHITE));
          strip.show();
          if (abs(servo.getCurrentDeg() - shotPos[i]) <= 3) {
            curPumping = i;
            break;
          }
          servoON();
          servo.attach();
          servo.setTargetDeg(shotPos[i]);
          curPumping = i;
          parking = false;
        }
      }
      if (btn.holded()) return;
    }
    printStr("                          \n", 0, 3); // очистили дисплей
    printStr("                      \n");
    while (!servo.tick()); // едем к рюмке
    servoOFF();
    printVolume(pumpTime);
    while (1) {
      enc.tick();
      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW)) {
          if (flag) pumpTime += 100;
          else pumpTime = 0;
          printVolume(pumpTime);
          pumpON();
          flag = true;
        } else {
          pumpOFF();
          flag = false;
        }
      }

      if (btn.holded() || digitalRead(SW_pins[curPumping])) {
        timeoutReset();
        curPumping = -1;
        servo.setTargetDeg(settingsList[parking_pos]);
        disp.clear();
        break;
      }

    }
    while (!servo.tick());
    servoOFF();
    servo.detach();
    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 15.0f * 50.0f / time50ml;
      EEPROM.update(1001, 47);
      EEPROM.put(eeAddress._time50ml, pumpTime);
    }
  }
  else if (mode == SERVO) {         // калибровка углов серво
    int servoPos = settingsList[parking_pos];
    disp.setInvertMode(1);
    printStr("   Калибр. серво   \n", 0, 0);
    disp.setInvertMode(0);
    printStr("Поставьте рюмку и\n", 0, 2);
    printStr("выставите угол.\n");
    printStr("Уберите рюмку\n");
    delay(3000);
    printStr("                         \n", 0, 2);
    printStr("                      \n");
    printStr("                      \n");
    printVolume(servoPos);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
    }
    servo.attach();
    while (1) {
      enc.tick();
      static int currShot = -1;
      // зажигаем светодиоды от кнопок
      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) {
          strip.setLED(i, mCOLOR(WHITE));
          strip.show();
          shotStates[i] = EMPTY;
          currShot = i;
          printVolume(shotPos[i]);
        } else if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
          strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
          strip.show();
          shotStates[i] = NO_GLASS;
          currShot = -1;
          printVolume(servoPos);
        }

      }
      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight())  servoPos -= 1;
        servoPos = constrain(servoPos, 0, 180);
        servoON();
        servo.write(servoPos);
        delay(10);
        servo.setCurrentDeg(servoPos);
        if (shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
          printVolume(shotPos[currShot]);
        }
        else printVolume(servoPos);
        servoOFF();
      }
      if (btn.holded()) {
        timeoutReset();
        servo.setTargetDeg(settingsList[parking_pos]);
        disp.clear();
        break;
      }
    }
    while (!servo.tick());
    servoOFF();
    servo.detach();
    // сохраняем значения углов в память
    EEPROM.update(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.update(eeAddress._shotPos + i, shotPos[i]);
  }
  else if (mode == BATTERY) {
    disp.setInvertMode(1);
    printStr("  Калибр. аккум-а  \n", 0, 0);
    disp.setInvertMode(0);
    timerMinim timer100(100);
    disp.setFont(lcdnums14x24);
    while (1) {
      enc.tick();

      if (timer100.isReady()) {
        disp.setCursor(35, 4);
        disp.print(get_battery_voltage());
      }

      if (enc.isTurn()) {
        if (enc.isLeft()) {
          battery_cal += 0.01;
        }
        if (enc.isRight()) {
          battery_cal -= 0.01;
        }
      }

      if (btn.holded()) {
        EEPROM.update(1003, 47);
        EEPROM.put(eeAddress._battery_cal, battery_cal);
        timeoutReset();
        disp.clear();
        break;
      }
    }
  }
}

void settingsMenuHandler(uint8_t row) {
  disp.setInvertMode(1);
  disp.setFont(Callibri15);
  printStr(MenuPages[menuPage][menuItem], 0, row);
  printStr("                      ");
  printNum(settingsList[menuItem - 1], Right);
  while (1) {
    enc.tick();

    if (enc.isTurn()) {
      if (enc.isLeft()) {
        settingsList[menuItem - 1] += 1;
      }
      if (enc.isRight()) {
        settingsList[menuItem - 1] -= 1;
      }
      if (menuItem == 1) settingsList[timeout_off] =    constrain(settingsList[timeout_off], 0, 15);
      if (menuItem == 2) settingsList[inverse_servo] =  constrain(settingsList[inverse_servo], 0, 1);
      if (menuItem == 3) settingsList[parking_pos] =    constrain(settingsList[parking_pos], 0, 180);
      if (menuItem == 4) settingsList[auto_parking] =   constrain(settingsList[auto_parking], 0, 1);
      if (menuItem == 5) settingsList[stby_time] =      constrain(settingsList[stby_time], 0, 255);
      if (menuItem == 6) settingsList[stby_light] =     constrain(settingsList[stby_light], 0, 255);
      if (menuItem == 7) settingsList[rainbow_flow] =   constrain(settingsList[rainbow_flow], 0, 1);
      if (menuItem == 8) settingsList[max_volume] =     constrain(settingsList[max_volume], 0, 255);
      disp.setInvertMode(1);
      disp.setFont(Callibri15);
      printStr(MenuPages[menuPage][menuItem], 0, row);
      printStr("                      ");
      printNum(settingsList[menuItem - 1], Right);
    }
    if (encBtn.clicked()) {
      EEPROM.update(eeAddress._timeout_off, settingsList[timeout_off]);
      EEPROM.update(eeAddress._inverse_servo, settingsList[inverse_servo]);
      servo.setDirection(settingsList[inverse_servo]);
      EEPROM.update(eeAddress._parking_pos, settingsList[parking_pos]);
      EEPROM.update(eeAddress._auto_parking, settingsList[auto_parking]);
      EEPROM.update(eeAddress._stby_time, settingsList[stby_time]);
      TIMEOUTtimer.setInterval(settingsList[stby_time] * 1000L); // таймаут режима ожидания
      EEPROM.update(eeAddress._stby_light, settingsList[stby_light]);
      EEPROM.update(eeAddress._rainbow_flow, settingsList[rainbow_flow]);
      EEPROM.update(eeAddress._max_volume, settingsList[max_volume]);
      if(thisVolume > settingsList[max_volume]) thisVolume = settingsList[max_volume];
      timeoutReset();
      break;
    }
  }
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      bool swState = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;
      if (swState && shotStates[i] == NO_GLASS) {  // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
        else  strip.setLED(i, mCOLOR(ORANGE));                      // подсветили оранжевым
        LEDchanged = true;
        shotCount++;                                                // инкрементировали счётчик поставленных рюмок
        if (systemState != PUMPING && !showMenu) printVolume(shotVolume[i]);
        DEBUG("set glass: ");
        DEBUG(i);
        DEBUG(", volume: ");
        DEBUGln(shotVolume[i]);
      }
      if (!swState && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected)
          strip.setLED(curSelected, mCOLOR(WHITE));
        else if (settingsList[stby_light] > 0)
          strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
        else strip.setLED(i, mCOLOR(BLACK));
        LEDchanged = true;
        //timeoutReset();                                           // сброс таймаута
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
        if (systemState != PUMPING && !showMenu) printVolume(thisVolume);
        DEBUG("take glass: ");
        DEBUGln(i);
      }
      if (shotStates[i] == READY) rainbowFlow(1, i);
      else  rainbowFlow(0, i);
    }
    if (shotCount == 0) {                                          // если нет ни одной рюмки
      TIMEOUTtimer.start();
#if (STATUS_LED)
      if (timeoutState) {                                          // отключаем динамическую подсветку режима ожидания
        LEDbreathingState = false;
        LED = mHSV(255, 0, STATUS_LED); // white
      }
#endif
      if (!parking && !systemON) {
        systemON = true;
        DEBUGln("SystemON");
      }
    }
    else  TIMEOUTtimer.stop();

    if (workMode == AutoMode)           // авто
      flowRoutnie();       // крутим отработку кнопок и поиск рюмок
    else if (systemON)    // ручной
      flowRoutnie();     // если активны - ищем рюмки и всё такое
  }
}

// поиск и заливка
void flowRoutnie() {
  if (showMenu) return;

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
          LED = mHSV(11, 255, STATUS_LED); // orange
          strip.show();
#endif
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        else if (shotPos[i] == settingsList[parking_pos]) {             // если положение рюмки совпадает с парковочным
          servoON();                                      // вкл питание серво
          servo.attach(SERVO_PIN, settingsList[parking_pos]);
          delay(500);
          DEBUG("moving to shot: ");
          DEBUGln(i);
        }
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки и не припаркованны
      if ( (workMode == AutoMode) && settingsList[auto_parking] == 0) {                // если в авто режиме:
        servoOFF();                                       // выключили серво
        servo.detach();
        systemON = false;                                 // выключили систему
        DEBUGln("SystemOFF");
        parking = true;                                   // уже на месте!
        LEDbreathingState = true;
        LEDchanged = true;
        DEBUGln("parked!");
      }
      else {                                              // если же в ручном режиме:
        servoON();                                        // включаем серво и паркуемся
        servo.attach();
        servo.setTargetDeg(settingsList[parking_pos]);
#if(STATUS_LED)
        LED = mHSV(11, 255, STATUS_LED); // orange
        LEDchanged = true;
#endif

        if (servo.tick()) {                               // едем до упора
          servoOFF();                                     // выключили серво
          servo.detach();
          systemON = false;                               // выключили систему
          DEBUGln("SystemOFF");
          parking = true;                                 // на месте!
          LEDbreathingState = true;
          LEDchanged = true;
          DEBUGln("parked!");
        }
      }
      if (!showMenu) printVolume(thisVolume);
    }
    else if ( (workMode == ManualMode) && noGlass)                        // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
    {
      systemON = false;
      DEBUGln("SystemOFF");
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

  } else if (systemState == PUMPING) {                    // если качаем
    printVolume(volumeCount += volumeTick);               // выводим текущий объём на дисплей
    int colorCount = MIN_COLOR + COLOR_SCALE * volumeCount / shotVolume[curPumping];  // расчёт цвета для текущего обьёма
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      EEPROM.put(eeAddress._shots_overall, shots_overall += 1);
      delay(5);
      EEPROM.put(eeAddress._volume_overall, volume_overall += volumeCount);
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

// отрисовка светодиодов по флагу (50мс)
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
  if (!timeoutState && !showMenu) displayMode(workMode);
  timeoutState = true;
  disp.setContrast(255);
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
    else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, settingsList[stby_light]);
  }
#if(STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  LEDbreathingState = false;
#endif
  LEDchanged = true;
  //DEBUGln("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady() && systemState == SEARCH) {
    DEBUGln("timeout");
    timeoutState = false;
    disp.setContrast(0);
    servoOFF();
    servo.detach();
    if (settingsList[stby_light]) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, settingsList[stby_light] / 2);
    }
    LEDbreathingState = true;
    LEDchanged = true;
    selectShot = -1;
    curSelected = -1;
    systemON = false;
    if (settingsList[timeout_off] > 0) {
      POWEROFFtimer.reset();
      POWEROFFtimer.start();
    }
    EEPROM.put(0, thisVolume);
  }

  if (settingsList[timeout_off] > 0) {
    if (POWEROFFtimer.isReady()) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
#if(STATUS_LED)
      LED = mHSV(255, 0, 0);  // off
      LEDbreathingState = false;
#endif
      LEDchanged = true;
    }
  }
}

void rainbowFlow(bool _state, uint8_t _shotNum) {
  if (settingsList[rainbow_flow]) {
    static float count[NUM_SHOTS] = {130};
    if (!_state) {
      count[_shotNum] = 130;
      return;
    }
    leds[_shotNum] = mHSV((int)count[_shotNum], 255, 255);
    count[_shotNum] += 0.5;
    LEDchanged = true;
  }
}

void prePump() {
  for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
    if (!digitalRead(SW_pins[i])) {         // нашли рюмку
      if (abs(servo.getCurrentDeg() - shotPos[i]) <= 3) {
        curPumping = i;
        break;
      }
      servoON();
      servo.attach();
      servo.setTargetDeg(shotPos[i]);
      curPumping = i;
      parking = false;
    }
  }
  if (curPumping == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
  //if (!timeoutState) disp.brightness(7);
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
      printVolume(round(volumeCount));
      strip.setLED(curPumping, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
      strip.show();
    }
  }
  pumpOFF();
  DEBUG("pumping stopped, volume: ");
  DEBUG(round(volumeCount));
  DEBUGln("ml");
  delay(300);
  timeoutReset();
}

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

#ifdef BATTERY_PIN
float k = 0.1, filteredValue = 4.2;
float filter(float value) {
  filteredValue = (1.0 - k) * filteredValue + k * value;
  return filteredValue;
}
float get_battery_voltage() {
  battery_voltage = filter(analogRead(BATTERY_PIN) * (4.5 * battery_cal) / 1023.f);
  DEBUGln(battery_voltage);
  return battery_voltage;
}

uint8_t get_battery_percent() {
  static uint8_t percent = 0;
  if (battery_voltage >= 4.00) percent = 5;
  else if (battery_voltage >= 3.90) percent = 4;
  else if (battery_voltage >= 3.80) percent = 3;
  else if (battery_voltage >= 3.70) percent = 2;
  else if (battery_voltage >= 3.30) percent = 1;
  else percent = 0;
  return percent;
}

bool battery_watchdog() {
  static uint16_t lastMillis = 0;
  static bool batOk, lastOkStatus = 1;
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    batOk = (get_battery_voltage() < (float)BATTERY_LOW) ? 0 : 1;
    if (!batOk) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, 0);
#if(STATUS_LED)
      LED = mHSV(255, 0, 0);
#endif
      strip.show();
      showMenu = false;
      timeoutState = false;
    }
    else if (!lastOkStatus) timeoutReset();
    lastOkStatus = batOk;
  }
  displayBattery(batOk);
  return batOk;
}

void displayBattery(bool batOk) {
  if (showMenu && batOk) return;

  static uint32_t currentMillis, lastDisplay = 0, lastBlink = 0;
  static bool blinkState = true;
  currentMillis = millis();

  if ( (currentMillis - lastDisplay >= 1000) && batOk) {
    lastDisplay = currentMillis;
    disp.setFont(Battery19x9);
    printNum(get_battery_percent(), Right, 0);
  }
  else if ( (currentMillis - lastBlink >= 500) && !batOk) {
    lastBlink = currentMillis;
    blinkState = !blinkState;
    if (blinkState) {
      disp.setFont(Battery19x9);
      printNum(get_battery_percent(), Right, 0);
    }
    else disp.clear();
  }
}
#endif

void readEEPROM() {
  // чтение последнего налитого объёма
  if (EEPROM.read(1000) != 47) {
    EEPROM.write(1000, 47);
    EEPROM.put(eeAddress._thisVolume, thisVolume);
  }
  else EEPROM.get(eeAddress._thisVolume, thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  if (EEPROM.read(1001) != 47) {
    EEPROM.write(1001, 47);
    EEPROM.put(eeAddress._time50ml, TIME_50ML);
  }
  else EEPROM.get(eeAddress._time50ml, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  if (EEPROM.read(1002) != 47) {
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
      shotPos[i] = initShotPos[i];
    }
  }
  else {
    for (byte i = 0; i < NUM_SHOTS; i++)
      EEPROM.get(eeAddress._shotPos + i, shotPos[i]);
  }

  // чтение калибровки аккумулятора
  if (EEPROM.read(1003) != 47) {
    EEPROM.write(1003, 47);
    EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);
  }
  else EEPROM.get(eeAddress._battery_cal, battery_cal);

  //==========================================================
  //                    чтение настроек
  //==========================================================
  // чтение значения таймаута
  if (EEPROM.read(1004) != 47) {
    EEPROM.write(1004, 47);
    EEPROM.put(eeAddress._timeout_off, TIMEOUT_OFF);
  }
  else EEPROM.get(eeAddress._timeout_off, settingsList[timeout_off]);

  // чтение установки инверсии серво
  if (EEPROM.read(1005) != 47) {
    EEPROM.write(1005, 47);
    EEPROM.put(eeAddress._inverse_servo, INVERSE_SERVO);
  }
  else EEPROM.get(eeAddress._inverse_servo, settingsList[inverse_servo]);

  // чтение парковочной позиции
  if (EEPROM.read(1006) != 47) {
    EEPROM.write(1006, 47);
    EEPROM.put(eeAddress._parking_pos, PARKING_POS);
  }
  else EEPROM.get(eeAddress._parking_pos, settingsList[parking_pos]);

  // чтение установки автопарковки в авторежиме
  if (EEPROM.read(1007) != 47) {
    EEPROM.write(1007, 47);
    EEPROM.put(eeAddress._auto_parking, AUTO_PARKING);
  }
  else EEPROM.get(eeAddress._auto_parking, settingsList[auto_parking]);

  // чтение таймаута режима ожидания
  if (EEPROM.read(1008) != 47) {
    EEPROM.write(1008, 47);
    EEPROM.put(eeAddress._stby_time, STBY_TIME);
  }
  else EEPROM.get(eeAddress._stby_time, settingsList[stby_time]);

  // чтение яркости подсветки в режиме ожидания
  if (EEPROM.read(1009) != 47) {
    EEPROM.write(1009, 47);
    EEPROM.put(eeAddress._stby_light, STBY_LIGHT);
  }
  else EEPROM.get(eeAddress._stby_light, settingsList[stby_light]);

  // чтение установки динамической подсветки
  if (EEPROM.read(1010) != 47) {
    EEPROM.write(1010, 47);
    EEPROM.put(eeAddress._rainbow_flow, RAINBOW_FLOW);
  }
  else EEPROM.get(eeAddress._rainbow_flow, settingsList[rainbow_flow]);

  // чтение максимального объёма
  if (EEPROM.read(1011) != 47) {
    EEPROM.write(1011, 47);
    EEPROM.put(eeAddress._max_volume, MAX_VOLUME);
  }
  else EEPROM.get(eeAddress._max_volume, settingsList[max_volume]);

  // чтение статистики
  if (EEPROM.read(1012) != 47) {
    EEPROM.write(1012, 47);
    EEPROM.put(eeAddress._shots_overall, 0);
  }
  else EEPROM.get(eeAddress._shots_overall, shots_overall);

  if (EEPROM.read(1013) != 47) {
    EEPROM.write(1013, 47);
    EEPROM.put(eeAddress._volume_overall, 0);
  }
  else EEPROM.get(eeAddress._volume_overall, volume_overall);
}

void resetEEPROM() {
  EEPROM.update(1000, 47);
  EEPROM.put(eeAddress._thisVolume, INIT_VOLUME);

  // сброс калибровки времени на 50мл
  EEPROM.update(1001, 47);
  EEPROM.put(eeAddress._time50ml, TIME_50ML);

  // сброс позиций серво над рюмками
  EEPROM.update(1002, 47);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  //сброс калибровки аккумулятора
  EEPROM.update(1003, 47);
  EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);

  // сброс значения таймаута
  EEPROM.update(1004, 47);
  EEPROM.put(eeAddress._timeout_off, TIMEOUT_OFF);

  // сброс инверсии серво
  EEPROM.update(1005, 47);
  EEPROM.put(eeAddress._inverse_servo, INVERSE_SERVO);

  // сброс парковочной позиции
  EEPROM.update(1006, 47);
  EEPROM.put(eeAddress._parking_pos, PARKING_POS);

  // сброс установки автопарковки в авторежиме
  EEPROM.update(1007, 47);
  EEPROM.put(eeAddress._auto_parking, AUTO_PARKING);

  // сброс таймаута режима ожидания
  EEPROM.update(1008, 47);
  EEPROM.put(eeAddress._stby_time, STBY_TIME);

  // сброс яркости подсветки в режиме ожидания
  EEPROM.update(1009, 47);
  EEPROM.put(eeAddress._stby_light, STBY_LIGHT);

  // сброс установки динамической подсветки
  EEPROM.update(1010, 47);
  EEPROM.put(eeAddress._rainbow_flow, RAINBOW_FLOW);

  // сброс максимального объёма
  EEPROM.update(1011, 47);
  EEPROM.put(eeAddress._max_volume, MAX_VOLUME);

  readEEPROM();
}
