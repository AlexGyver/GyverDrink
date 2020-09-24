
void serviceRoutine(serviceStates mode) {
#ifdef TM1637
  byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
  disp.runningString(serviceText, sizeof(serviceText), 150);
  while (!digitalRead(BTN_PIN));  // ждём отпускания
#endif

  //==============================================================================
  //                       настройка серводвигателя
  //==============================================================================
  timerMinim timer100(100);

  if (mode == SERVO) {         // калибровка углов серво
    byte workModeTemp = workMode;
    workMode = AutoMode;
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
    strip.show();
#ifdef TM1637
    disp.scrollByte(_dash, _1, _dash, _empty, 50);
    delay(1000);
#else
    disp.clear();
    //disp.setInvertMode(1);
#if(MENU_LANG == 1)
    printStr("Парковка", 1, 0);
#else
    printStr("Parking position", 1, 0);
#endif
    //disp.setInvertMode(0);
#endif
    byte servoPos = parking_pos;
    printNum(servoPos, deg);
    while (1) {
      enc.tick();
      static int currShot = -1;
      // зажигаем светодиоды от кнопок
      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) {
          strip.setLED(i, mHSV(255, 0, 50));
          strip.show();
          shotStates[i] = EMPTY;
          currShot = i;
          shotCount++;
          servoPos = shotPos[currShot];
#ifdef TM1637
          printNum((i + 1) * 1000 + shotPos[i], deg);
#else
#if(MENU_LANG == 1)
          printStr("Рюмка ", 1, 0);
#else
          printStr("Shot ", 1, 0);
#endif
          printInt(currShot + 1);
          clearToEOL();
          disp.write('\n');
          printNum(servoPos, deg);
#endif
          servo.setTargetDeg(servoPos);
          servo.start();
          servoON();
          while (!servo.tick());
          servoOFF();
          break;
        }
        if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
          strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
          strip.show();
          shotStates[i] = NO_GLASS;
          if (currShot == i)  currShot = -1;
          shotCount--;
          if (shotCount == 0) { // убрали последнюю рюмку
            servoPos = parking_pos;
#ifndef TM1637
#if(MENU_LANG == 1)
            printStr("Парковка", 1, 0);
#else
            printStr("Parking position", 1, 0);
#endif
            clearToEOL();
            disp.write('\n');
#endif
            printNum(servoPos, deg);
            servo.setTargetDeg(servoPos);
            servo.start();
            servoON();
            while (!servo.tick());
            servoOFF();
            break;
          }
          else continue;  // если ещё есть поставленные рюмки -> ищем заново и попадаем в следующий блок
        }
        if (shotStates[i] == EMPTY && currShot == -1) { // если стоит рюмка
          currShot = i;
          servoPos = shotPos[currShot];
#ifdef TM1637
          printNum((i + 1) * 1000 + shotPos[i], deg);
#else
#if(MENU_LANG == 1)
          printStr("Рюмка ", 1, 0);
#else
          printStr("Shot ", 1, 0);
#endif
          printInt(currShot + 1);
          clearToEOL();
          disp.write('\n');
          printNum(servoPos, deg);
#endif
          servo.setTargetDeg(servoPos);
          servo.start();
          servoON();
          while (!servo.tick());
          servoOFF();
          break;
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight() && servoPos > 0)  servoPos -= 1;
        servoPos = min(servoPos, 180);
        servoON();
        servo.attach(SERVO_PIN, servoPos);
        if (shotCount == 0) parking_pos = servoPos;
        if (shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
#ifdef TM1637
          printNum((currShot + 1) * 1000 + shotPos[currShot], deg);
#else
          printNum(shotPos[currShot], deg);
#endif
        }
        else printNum(servoPos, deg);
      }
      if (btn.pressed()) {
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
        mode = VOLUME;
#else
        timeoutReset();
        //        servo.setTargetDeg(parking_pos);
        //        servo.start();
        //        servoON();
        for (byte i = 0; i < NUM_SHOTS; i++) {
          if (shotStates[i] == EMPTY) strip.setLED(i, mCOLOR(ORANGE));
          else strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
        }
        strip.show();
#endif
        workMode = (workModes)workModeTemp;
        break;
      }
    }
    //    while (!servo.tick());
    servo.stop();
    servoOFF();
    disp.clear();
    // сохраняем значения углов в память
    for (byte i = 0; i < NUM_SHOTS; i++) EEPROM.update(eeAddress._shotPos + i, shotPos[i]);
    EEPROM.update(eeAddress._parking_pos, parking_pos);
  }

  //==============================================================================
  //                            калибровка объёма
  //==============================================================================

  if (mode == VOLUME) {                      // калибровка объёма
    uint16_t pumpTime = 0;
    bool flag = false;
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
    strip.show();
#ifdef TM1637
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    disp.scrollByte(_dash, _2, _dash, _empty, 50);
    delay(1000);
    disp.displayInt(pumpTime);
#else
    disp.clear();
    //disp.setInvertMode(1);
    printStr(calibration_menu[mode - 1], Center, 0);
    //disp.setInvertMode(0);
    printNum(pumpTime);
#endif
    if (curPumping != -1) {   // если уже стоит рюмка
      shotStates[curPumping] = EMPTY;
      systemON = false;
      systemState = SEARCH;
      curPumping = -1;
    }
    while (1) {

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW) && curPumping != -1) {
          if (flag) pumpTime += 100;
#ifdef TM1637
          disp.displayInt(pumpTime);
#else
          printNum(pumpTime);
#endif
          pumpON();
          flag = true;
        } else {
          pumpOFF();
          flag = false;
        }
      }

      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (!digitalRead(SW_pins[i]) && curPumping == -1) {
          delay(100);
          strip.setLED(i, mHSV(255, 0, 50));
          strip.show();
          curPumping = i;
          servo.setTargetDeg(shotPos[i]);
          servo.start();
          servoON();
        }
      }
      if (digitalRead(SW_pins[curPumping]) && (curPumping > -1)) {
        delay(100);
        strip.setLED(curPumping, mHSV(20, 255, settingsList[stby_light]));
        strip.show();
        if (pumpTime > 0) EEPROM.put(eeAddress._time50ml, pumpTime);
        pumpTime = 0;
        printNum(pumpTime);
        curPumping = -1;
      }

      if (servo.tick()) servoOFF();
      else servoON();

      if (btn.pressed()) {
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
#ifdef BATTERY_PIN
        mode = BATTERY;
#endif
#else
        timeoutReset();
#endif

        curPumping = -1;
        servo.setTargetDeg(parking_pos);
        servo.start();
        servoON();

        for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
        strip.show();
        break;
      }

    }
    while (!servo.tick());
    servoOFF();
    disp.clear();
    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 15.0f * 50.0f / time50ml;
      EEPROM.put(eeAddress._time50ml, pumpTime);
    }
  }

  //==============================================================================
  //                     калибровка напряжения аккумулятора
  //==============================================================================
#ifdef BATTERY_PIN
  if (mode == BATTERY) {
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
    strip.show();
#ifdef TM1637
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    disp.scrollByte(_dash, _3, _dash, _empty, 50);
    delay(1000);
#else
    disp.clear();
    //disp.setInvertMode(1);
    printStr(calibration_menu[mode - 1], Center, 0);
    //disp.setInvertMode(0);
#endif
    while (1) {
      enc.tick();

      if (timer100.isReady())
#ifdef TM1637
        printNum(get_battery_voltage() * 1000);
#else
        printFloat(get_battery_voltage(), 2, Center, 3);
#endif

      if (enc.isTurn()) {
        if (enc.isLeft())  battery_cal += 0.01;
        if (enc.isRight()) battery_cal -= 0.01;
        battery_cal = constrain(battery_cal, 0, 3.0);
      }

      if (btn.pressed()) {
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
#else
        disp.clear();
        timeoutReset();
#endif
        break;
      }
    }
    EEPROM.put(eeAddress._battery_cal, battery_cal);
  }
#endif
}

#ifndef TM1637
void settingsMenuHandler(uint8_t selectedItem) {
  bool bypass = false;
  uint8_t parameter = menuItem - 1;
  if (menuItem == menuItemsNum[menuPage]) { // сброс настроек
    resetEEPROM();
    bypass = true;
  }
  else {
    disp.setInvertMode(1);
    printStr(MenuPages[menuPage][menuItem], 0, selectedItem);
    clearToEOL();
    disp.setInvertMode(0);
    printInt(settingsList[parameter], Right);
  }
  while (1) {
    enc.tick();

    if (enc.isTurn()) {
      if (enc.isLeft()) {
        settingsList[parameter] += 1;
      }
      if (enc.isRight()) {
        settingsList[parameter] -= 1;
      }

      if (settingsList[timeout_off] > 15) settingsList[timeout_off] = 0;
      if (settingsList[servo_speed] > 100) settingsList[servo_speed] = 0;

      if (settingsList[stby_time]) {
        TIMEOUTtimer.setInterval(settingsList[stby_time] * 1000L); // таймаут режима ожидания
        TIMEOUTtimer.reset();
      }
      if (settingsList[keep_power]) KEEP_POWERtimer.setInterval(settingsList[keep_power] * 1000L);
      else keepPowerState = 0;


      disp.setInvertMode(1);
      printStr(MenuPages[menuPage][menuItem], 0, selectedItem);
      clearToEOL();
      disp.setInvertMode(0);
      printInt(settingsList[parameter], Right);

      timeoutReset();
    }

    if ( (parameter == inverse_servo) || (parameter == auto_parking) || (parameter == rainbow_flow) || (parameter == invert_display) ) {
      settingsList[parameter] = !settingsList[parameter];
      bypass = true;
    }

    if (encBtn.pressed() || bypass) {
      EEPROM.update(eeAddress._timeout_off, settingsList[timeout_off]);
      EEPROM.update(eeAddress._inverse_servo, settingsList[inverse_servo]);
      EEPROM.update(eeAddress._servo_speed, settingsList[servo_speed]);
      EEPROM.update(eeAddress._auto_parking, settingsList[auto_parking]);
      EEPROM.update(eeAddress._stby_time, settingsList[stby_time]);
      EEPROM.update(eeAddress._stby_light, settingsList[stby_light]);
      EEPROM.update(eeAddress._rainbow_flow, settingsList[rainbow_flow]);
      EEPROM.update(eeAddress._max_volume, settingsList[max_volume]);
      EEPROM.update(eeAddress._keep_power, settingsList[keep_power]);
      EEPROM.update(eeAddress._invert_display, settingsList[invert_display]);

      servo.setSpeed(settingsList[servo_speed]);
      servo.setDirection(settingsList[inverse_servo]);
      servo.start();
      servoON();
      while (!servo.tick());
      servo.stop();
      servoOFF();
      if (thisVolume > settingsList[max_volume]) thisVolume = settingsList[max_volume];
      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, settingsList[stby_light]);
      }
      disp.invertDisplay(settingsList[invert_display]);
      timeoutReset();
      break;
    }

    timeoutTick();
    if (!timeoutState) return;
    LEDtick();
  }
}
#endif

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
        if (systemState != PUMPING && !showMenu) {
          printNum(shotVolume[i], ml);
#ifndef TM1637
          progressBar(shotVolume[i], settingsList[max_volume]);
#endif
        }
      }
      if (!swState && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected) {
          selectShot = -1;
          curSelected = -1;
        }
        if (settingsList[stby_light] > 0)
          strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
        else strip.setLED(i, mCOLOR(BLACK));
        LEDchanged = true;
        //timeoutReset();                                           // сброс таймаута
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                       // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                // помпу выкл
          volumeCount = 0;
        }
        shotCount--;
        if (systemState != PUMPING && !showMenu) {
          printNum(thisVolume, ml);
#ifndef TM1637
          progressBar(thisVolume, settingsList[max_volume]);
#endif
        }
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
      if (!parking && !systemON) systemON = true;
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
        if ( abs(shotPos[i] - servo.getCurrentDeg()) > 3) {        // включаем серво только если целевая позиция не совпадает с текущей
          servo.setTargetDeg(shotPos[curPumping]);        // задаём цель
          servo.start();
          servoON();                                      // вкл питание серво
          parking = false;
#if(STATUS_LED)
          LED = mHSV(11, 255, STATUS_LED); // orange
          strip.show();
#endif
        }
        else if (shotPos[i] == parking_pos) {             // если положение рюмки совпадает с парковочным
          servoON();                                      // вкл питание серво
          servo.attach(SERVO_PIN, parking_pos);
          delay(500);
        }
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки и не припаркованны
      if ( (workMode == AutoMode) && settingsList[auto_parking] == 0) {                // если в авто режиме:
        systemON = false;                                 // выключили систему
        parking = true;                                   // уже на месте!
        LEDbreathingState = true;
        LEDchanged = true;
      }
      else {                                              // если же в ручном режиме:

        if (abs(servo.getTargetDeg() - parking_pos) > 3) {
          servo.setTargetDeg(parking_pos);
          servo.start();
          servoON();                                        // включаем серво и паркуемся
#if(STATUS_LED)
          LED = mHSV(11, 255, STATUS_LED); // orange
          LEDchanged = true;
#endif
        }
        if (servo.tick()) {                               // едем до упора
          systemON = false;                               // выключили систему
          parking = true;                                 // на месте!
          LEDbreathingState = true;
          LEDchanged = true;
        }
      }
      if (!showMenu) printNum(thisVolume, ml);
    }
    else if ( (workMode == ManualMode) && noGlass)                        // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
    {
      systemON = false;
    }


  } else if (systemState == MOVING) {                     // движение к рюмке

    if (servoReady) {                                   // если приехали
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
    }

  } else if (systemState == PUMPING) {                    // если качаем
    static byte lastVolumeCount = 0;
    volumeCount += volumeTick;
    if (round(volumeCount) != lastVolumeCount) {
      printNum(round(volumeCount), ml);               // выводим текущий объём на дисплей
      lastVolumeCount = round(volumeCount);
#ifndef TM1637
      progressBar(round(volumeCount), shotVolume[curPumping]);
#endif
    }

    int colorCount = MIN_COLOR + COLOR_SCALE * volumeCount / shotVolume[curPumping];  // расчёт цвета для текущего обьёма
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
#ifndef TM1637
      shots_overall++;
      volume_overall += volumeCount;
      EEPROM.put(eeAddress._shots_overall, shots_overall);
      EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
      curPumping = -1;                                    // снимаем выбор рюмки
      systemState = WAIT;                                 // режим работы - ждать
      WAITtimer.reset();
    }
  } else if (systemState == WAIT) {
    volumeCount = 0;
    if (WAITtimer.isReady()) {
      systemState = SEARCH;
#ifndef TM1637
      displayMode(workMode);
#endif
    }
  }
}

// прокачка
void prePump() {
  if (showMenu) return;
  for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
    if (!digitalRead(SW_pins[i])) {        // нашли рюмку
      curPumping = i;
      if (abs(servo.getCurrentDeg() - shotPos[i]) <= 3) break;
      servo.setTargetDeg(shotPos[curPumping]);
      servo.start();
      servoON();
      parking = false;
      break;
    }
  }
  if (curPumping == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
  while (!servo.tick()); // едем к рюмке
  servoOFF();
  delay(300); // небольшая задержка перед наливом

  pumpON(); // включаем помпу
  FLOWdebounce.reset();
  while (!digitalRead(SW_pins[curPumping]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
  {
    if (FLOWdebounce.isReady()) {
      static byte lastVolumeCount = 0;
      volumeCount += volumeTick;
      if (round(volumeCount) != lastVolumeCount) {
        printNum(round(volumeCount), ml);
        lastVolumeCount = round(volumeCount);
      }

      strip.setLED(curPumping, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
      strip.show();
    }
  }
  pumpOFF();
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState && !showMenu && (curSelected < 0)) {
#ifdef TM1637
    disp.brightness(7);
    if (!volumeChanged) disp.displayByte(0x00, 0x00, 0x00, 0x00);
    if (workMode) disp.scrollByte(64, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 64, 50);
    else  disp.scrollByte(0, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 0, 50);
#else
    disp.invertDisplay((bool)settingsList[invert_display]);
    displayMode(workMode);
#endif
  }
  timeoutState = true;
  TIMEOUTtimer.reset();
  if (!keepPowerState) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
      else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, settingsList[stby_light]);
    }
  }
#if(STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  LEDbreathingState = false;
#endif
  LEDchanged = true;
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady() && systemState == SEARCH) {
    timeoutState = false;
#ifdef TM1637
    disp.brightness(0);
    printNum(thisVolume, ml);
#else
    //disp.setContrast(0);
    if (showMenu) {
      showMenu = 0;
      menuItem = 0;
      menuPage = MAIN_MENU_PAGE;
      disp.clear();
      displayMode(workMode);
    }
#endif
    if (settingsList[stby_light]) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, settingsList[stby_light] / 2);
    }
    LEDbreathingState = true;
    LEDchanged = true;
    selectShot = -1;
    curSelected = -1;
    systemON = false;
    if (settingsList[timeout_off]) POWEROFFtimer.reset();
    if (volumeChanged) {
      volumeChanged = false;
      EEPROM.update(eeAddress._thisVolume, thisVolume);
    }
#if (SAVE_MODE == 1)
    EEPROM.update(eeAddress._workMode, workMode);
#endif
  }

  if (settingsList[keep_power]) {
    //if (KEEP_POWERtimer.isReady() && (shotCount == 0) && ( (settingsList[keep_power] < settingsList[stby_time]) || showMenu) && (curSelected == -1)) {
    if (KEEP_POWERtimer.isReady() && (shotCount == 0) && (curSelected == -1)) {
      keepPowerState = 1;
      LEDchanged = true;
    }
  }

  if (settingsList[timeout_off]) {
    if (POWEROFFtimer.isReady() && !timeoutState) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
#if(STATUS_LED)
      LED = mHSV(255, 0, 0);  // off
      LEDbreathingState = false;
#endif
#ifdef TM1637
      disp.scrollByte(0, 0, 0, 0, 50);
#else
      if (settingsList[invert_display]) disp.invertDisplay(false);
      disp.clear();
      if (showMenu) {
        menuPage = MAIN_MENU_PAGE;
        menuItem = 1;
        showMenu = false;
      }
#endif
      LEDchanged = true;
      POWEROFFtimer.stop();
    }
  }
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

// отрисовка светодиодов по флагу (50мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#if(STATUS_LED)
    ledBreathing(LEDbreathingState, timeoutState);
#endif
    if (settingsList[keep_power]) keepPower();
    strip.show();
  }
}

// динамическая подсветка светодиодов
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
  if (mode) LED = mHSV(130, 255, _brightness); // бирюзовый
  else {
    LED = mHSV(255, 0, _brightness);  // белый
  }

  LEDchanged = true;
}
#endif

void keepPower() {
  static bool _dir = 1;
  static float _brightness = 1;
  uint8_t stby_brightness = settingsList[stby_light];
  if (settingsList[timeout_off]) {
    stby_brightness = settingsList[stby_light] * (POWEROFFtimer.isOn() || timeoutState);
  }
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
  static uint32_t lastMillis = 0;
  static bool batOk, lastOkStatus = 1;
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    batOk = (get_battery_voltage() < (float)BATTERY_LOW) ? 0 : 1;
    if (!batOk) {
      if (systemState == PUMPING) {
        pumpOFF();                      // помпа выкл
        shotStates[curPumping] = READY; // налитая рюмка, статус: готов
        curPumping = -1;                // снимаем выбор рюмки
        systemState = WAIT; // режим работы - ждать
        WAITtimer.reset();
        systemON = false;
      }
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, 0);
#if(STATUS_LED)
      LED = mHSV(255, 0, 0);
#endif
      strip.show();
      timeoutState = false;
#ifdef TM1637
      disp.brightness(0);
      disp.displayByte(0x39, 0x09, 0x09, 0x0F);
      delay(500);
      disp.displayByte(0x00, 0x00, 0x00, 0x00);
#endif
      showMenu = false;
    }
    else if (!lastOkStatus) timeoutReset();
    lastOkStatus = batOk;
  }
#ifndef TM1637
  if (POWEROFFtimer.isOn() || timeoutState) displayBattery(batOk);
#endif
  return batOk;
}

#ifndef TM1637
void displayBattery(bool batOk) {
  if ( batOk && showMenu ) return;

  static uint32_t currentMillis, lastDisplay = 0, lastBlink = 0;
  static bool blinkState = true;
  currentMillis = millis();
  disp.setFont(Battery11x21);

  if ( (currentMillis - lastDisplay >= 1000) && batOk) {
    lastDisplay = currentMillis;
    printInt(get_battery_percent(), Right, 0);
  }
  else if ( (currentMillis - lastBlink >= 500) && !batOk) {
    lastBlink = currentMillis;
    blinkState = !blinkState;
    if (blinkState) printInt(get_battery_percent(), Right, 0);
    else disp.clear();
  }

#if(MENU_LANG == 1)
  disp.setFont(CenturyGothic10x16);
#else
  disp.setFont(ZevvPeep8x16);
#endif
}
#endif
#endif
