
timerMinim timer100(100);

// обработка функций сервисного режима
void serviceRoutine(serviceStates mode) {

#ifdef TM1637
  byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
  disp.runningString(serviceText, sizeof(serviceText), 150);
  while (!digitalRead(BTN_PIN));  // ждём отпускания
#endif

  //==============================================================================
  //                       настройка позиций двигателя
  //==============================================================================

  if (mode == POSITION) {
#if (MOTOR_TYPE == 1)
    stepper.autoPower(false);
#endif
    byte workModeTemp = workMode;
    workMode = AutoMode;
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
    strip.show();
#ifdef TM1637
    disp.scrollByte(_dash, _1, _dash, _empty, 50);
    delay(1000);
#elif defined OLED
    disp.clear();
    disp.setInvertMode(1);
    clearToEOL();
#if(MENU_LANG == 0)
    printStr("Парковка", Center, 0);
#else
    printStr("Parking", Center, 0);
#endif
    disp.setInvertMode(0);
#endif
    byte servoPos = parking_pos;
    printNum(servoPos, deg);
    while (1) {
      enc.tick();
      static int currShot = -1;
      // зажигаем светодиоды от кнопок
      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (!digitalRead(SW_pins[i]) && shotStates[i] != EMPTY) { // поставили рюмку
          strip.setLED(i, mHSV(255, 0, 50));
          strip.show();
          shotStates[i] = EMPTY;
          currShot = i;
          shotCount++;
          servoPos = shotPos[currShot];
#ifdef TM1637
          printNum((i + 1) * 1000 + shotPos[i], deg);
#elif defined OLED
          disp.home();
          disp.setInvertMode(1);
          clearToEOL();
#if(MENU_LANG == 0)
          printStr("Рюмка ", Center, 0);
#else
          printStr("Shot ", Center, 0);
#endif
          printInt(currShot + 1);
          clearToEOL();
          disp.write('\n');
          disp.setInvertMode(0);
          printNum(servoPos, deg);
#endif

#if (MOTOR_TYPE == 0)
          servo.setTargetDeg(servoPos);
          servo.start();
          servoON();
          while (!servo.tick());
          servoOFF();
          break;
#elif (MOTOR_TYPE == 1)
          stepper.setTargetDeg(servoPos);
          while (stepper.tick());
          break;
#endif
        }
        if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  { // убрали рюмку
          strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
          strip.show();
          shotStates[i] = NO_GLASS;
          if (currShot == i)  currShot = -1;
          shotCount--;
          if (shotCount == 0) { // убрали последнюю рюмку
            servoPos = parking_pos;
#ifdef OLED
            disp.home();
            disp.setInvertMode(1);
            clearToEOL();
#if(MENU_LANG == 0)
            printStr("Парковка", Center, 0);
#else
            printStr("Parking", Center, 0);
#endif
            clearToEOL();
            disp.write('\n');
            disp.setInvertMode(0);
#endif
            printNum(servoPos, deg);
#if (MOTOR_TYPE == 0)
            servo.setTargetDeg(servoPos);
            servo.start();
            servoON();
            while (!servo.tick());
            servo.stop();
            servoOFF();
            break;
#elif (MOTOR_TYPE == 1)
            stepper.setTargetDeg(servoPos);
            while (stepper.tick());
            break;
#endif
          }
          else continue;  // если ещё есть поставленные рюмки -> ищем заново и попадаем в следующий блок
        }
        if (shotStates[i] == EMPTY && currShot == -1) { // если стоит рюмка
          currShot = i;
          servoPos = shotPos[currShot];
#ifdef TM1637
          printNum((i + 1) * 1000 + shotPos[i], deg);
#elif defined OLED
          disp.home();
          disp.setInvertMode(1);
          clearToEOL();
#if(MENU_LANG == 0)
          printStr("Рюмка ", Center, 0);
#else
          printStr("Shot ", Center, 0);
#endif
          printInt(currShot + 1);
          clearToEOL();
          disp.write('\n');
          disp.setInvertMode(0);
          printNum(servoPos, deg);
#endif
#if (MOTOR_TYPE == 0)
          servo.setTargetDeg(servoPos);
          servo.start();
          servoON();
          while (!servo.tick());
          servoOFF();
          break;
#elif (MOTOR_TYPE == 1)
          stepper.setTargetDeg(servoPos);
          while (stepper.tick());
          break;
#endif
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight() && servoPos > 0)  servoPos -= 1;
        //servoPos = min(servoPos, 180);
#if (MOTOR_TYPE == 0)
        servoON();
        servo.attach(SERVO_PIN, servoPos, SERVO_MIN_US, SERVO_MAX_US);
#elif (MOTOR_TYPE == 1)
        stepper.setAccelerationDeg(0);
        stepper.setMaxSpeedDeg(60);
        stepper.setTargetDeg(servoPos);
        while (stepper.tick());
        stepper.setAccelerationDeg(MOTOR_ACCEL);
        stepper.setMaxSpeedDeg(parameterList[motor_speed]);
#endif
        if (shotCount == 0) parking_pos = servoPos;
        if (shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
#ifdef TM1637
          printNum((currShot + 1) * 1000 + shotPos[currShot], deg);
#elif defined OLED
          printNum(shotPos[currShot], deg);
#endif
        }
        else printNum(servoPos, deg);
      }
      if (btn.pressed()) {
        workMode = (workModes)workModeTemp;
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
        mode = VOLUME;
#elif defined OLED
        timeoutReset();
        for (byte i = 0; i < NUM_SHOTS; i++) {
          if (shotStates[i] == EMPTY) strip.setLED(i, mHSV(parameterList[leds_color], 255, 255));
        }
        strip.show();
#endif
        if (shotCount > 0) parking = false;
        break;
      }
    }

    disp.clear();

#if (MOTOR_TYPE == 0)
    servo.stop();
    servoOFF();
#elif (MOTOR_TYPE == 1)
    stepper.autoPower(MOTOR_AUTO_POWER);
#endif

    // сохраняем значения углов в память
    for (byte i = 0; i < NUM_SHOTS; i++) EEPROM.update(eeAddress._shotPos + i, shotPos[i]);
    EEPROM.update(eeAddress._parking_pos, parking_pos);
  }

  //==============================================================================
  //                            калибровка объёма
  //==============================================================================

  if (mode == VOLUME) {
    uint16_t pumpTime = 0;
    bool flag = false;
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
    strip.show();
#ifdef TM1637
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    disp.scrollByte(_dash, _2, _dash, _empty, 50);
    delay(1000);
    disp.displayInt(pumpTime);
#elif defined OLED
    disp.clear();
    printNum(pumpTime);

#endif
    if (curPumping != -1) {   // если уже стоит рюмка
      shotStates[curPumping] = EMPTY;
      systemON = false;
      systemState = SEARCH;
      curPumping = -1;
#ifdef OLED
      disp.home();
      disp.setInvertMode(1);
      clearToEOL();
#if(MENU_LANG == 0)
      printStr("Зажмите энкодер", Center, 0);
#else
      printStr("Press encoder", Center, 0);
#endif
      disp.setInvertMode(0);
#endif
    }
    else {
#ifdef OLED
      disp.home();
      disp.setInvertMode(1);
      clearToEOL();
#if(MENU_LANG == 0)
      printStr("Поставьте рюмку", Center, 0);
#else
      printStr("Place shot", Center, 0);
#endif
      disp.setInvertMode(0);
#endif
    }
    while (1) {

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW) && curPumping != -1) {
          if (flag) pumpTime += 100;
          else {
#ifdef OLED
            disp.home();
            disp.setInvertMode(1);
            clearToEOL();
#if(MENU_LANG == 0)
            printStr("   Налейте 50мл   ", Center, 0);
#else
            printStr("   Fill 50ml   ", Center, 0);
#endif
            disp.setInvertMode(0);
#endif
          }
#ifdef TM1637
          disp.displayInt(pumpTime);
#elif defined OLED
          printNum(pumpTime);
#endif
          pumpON();
          flag = true;
        } else {
          pumpOFF();
          flag = false;
        }
      }

      for (byte i = 0; i < NUM_SHOTS; i++) { // поиск рюмки
        if (!digitalRead(SW_pins[i]) && curPumping == -1) { // нашли
          delay(100);
          strip.setLED(i, mHSV(255, 0, 50));
          strip.show();
          curPumping = i;
#if (MOTOR_TYPE == 0)
          servo.setTargetDeg(shotPos[i]);
          servo.start();
          servoON();
#elif (MOTOR_TYPE == 1)
          stepper.setTargetDeg(shotPos[i]);
#endif
#ifdef OLED
          disp.home();
          disp.setInvertMode(1);
          clearToEOL();
#if(MENU_LANG == 0)
          printStr("Зажмите энкодер", Center, 0);
#else
          printStr("Press encoder", Center, 0);
#endif
          disp.setInvertMode(0);
#endif
        }
      }
      if (digitalRead(SW_pins[curPumping]) && (curPumping > -1)) {
        delay(100);
        strip.setLED(curPumping, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
        strip.show();
        if (pumpTime > 0) EEPROM.put(eeAddress._time50ml, pumpTime);
        pumpTime = 0;
        printNum(pumpTime);
        curPumping = -1;
#ifdef OLED
        disp.home();
        disp.setInvertMode(1);
        clearToEOL();
#if(MENU_LANG == 0)
        printStr("Поставьте рюмку", Center, 0);
#else
        printStr("  Place shot  ", Center, 0);
#endif
        disp.setInvertMode(0);
#endif
      }

#if (MOTOR_TYPE == 0)
      if (servo.tick()) servoOFF();
      else servoON();
#elif (MOTOR_TYPE == 1)
      stepper.tick();
#endif

      if (btn.pressed()) {
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
#ifdef BATTERY_PIN
        mode = BATTERY;
#endif
#elif defined OLED
        timeoutReset();
#endif

        curPumping = -1;
#if (MOTOR_TYPE == 0)
        servo.setTargetDeg(parking_pos);
        servo.start();
        servoON();
#elif (MOTOR_TYPE == 1)
        stepper.setTargetDeg(parking_pos);
#endif

        for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
        strip.show();
        break;
      }

    }
#if (MOTOR_TYPE == 0)
    while (!servo.tick());
    servoOFF();
#elif (MOTOR_TYPE == 1)
    while (stepper.tick());
#endif
    disp.clear();
    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 20.0 * 50.0 / time50ml;
      EEPROM.put(eeAddress._time50ml, pumpTime);
    }
  }

  //==============================================================================
  //                     калибровка напряжения аккумулятора
  //==============================================================================
#ifdef BATTERY_PIN
  if (mode == BATTERY) {
    for (byte i = 0; i < NUM_SHOTS; i++) strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
    strip.show();
#ifdef TM1637
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    disp.scrollByte(_dash, _3, _dash, _empty, 50);
    delay(1000);
#elif defined OLED
    disp.clear();
    disp.setInvertMode(1);
    disp.setFont(MAIN_FONT);
    static byte text_offset = (DISP_WIDTH - strWidth("Фактор: 0.000")) / 2;
#if(MENU_LANG == 0)
    disp.setLetterSpacing(0);
    clearToEOL();
    //    printStr("Напряжение аккум-а", Center, 0);
    printStr("Фактор: ", text_offset, 0);
    printFloat(battery_cal, 3);
#else
    clearToEOL();
    //    printStr("Battery voltage", Center, 0);
    printStr("Factor: ", text_offset, 0);
    printFloat(battery_cal, 3);
#endif /* MENU_LANG*/
    //    disp.setFont(BIG_NUM_FONT);
    disp.setInvertMode(0);
#endif /* OLED*/
    while (1) {
      enc.tick();

      if (timer100.isReady()) {
#ifdef TM1637
        printNum(get_battery_voltage() * 1000);
#elif defined OLED
        disp.setFont(BIG_NUM_FONT);
        printFloat(get_battery_voltage(), 2, Center, 3);
#endif
      }

      if (enc.isTurn()) {
        if (enc.isLeft())  battery_cal += 0.002;
        if (enc.isRight()) battery_cal -= 0.002;
        battery_cal = constrain(battery_cal, 0, 3.0);

#ifdef OLED
        disp.setInvertMode(1);
        disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
        disp.setLetterSpacing(0);
        printStr("Фактор: ", text_offset, 0);
        printFloat(battery_cal, 3);
#else
        printStr("Factor: ", text_offset, 0);
        printFloat(battery_cal, 3);
#endif /* MENU_LANG */
        disp.setInvertMode(0);
#endif /* OLED */

      }

      if (btn.pressed()) {
#ifdef TM1637
        disp.scrollByte(0, 0, 0, 0, 50);
#elif defined OLED
        disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
        disp.setLetterSpacing(0);
#endif
        disp.clear();
        if (showMenu) timeoutReset();
#endif
        break;
      }
    }
    EEPROM.put(eeAddress._battery_cal, battery_cal);
  }
#endif
}

#ifdef OLED
// обработка изменения параметра меню
void editParameter(byte parameter, byte selectedRow) {
  bool bypass = false;
  byte lastParameterValue = parameterList[parameter];
  if ( (parameter != motor_reverse) && (parameter != auto_parking) && (parameter != rainbow_flow) && (parameter != invert_display) ) { // boolean parameters
    disp.setInvertMode(0);
    printStr(MenuPages[menuPage][menuItem], 0, selectedRow);
#if (MENU_LANG == 0)
    clearToEOL('\'');
#else
    clearToEOL('.');
#endif
    disp.setInvertMode(1);
    printInt(parameterList[parameter], Right);
  }
  while (1) {
    enc.tick();

    if (enc.isTurn()) {
      if (enc.isLeft())  parameterList[parameter] += 1;
      if (enc.isRight()) parameterList[parameter] -= 1;
      if (enc.isLeftH()) parameterList[parameter] += 5;
      if (enc.isRightH()) parameterList[parameter] -= 5;

      if (parameterList[timeout_off] > 15) parameterList[timeout_off] = 0;
      //#if (MOTOR_TYPE == 0)
      //      if (parameterList[motor_speed] > 100) parameterList[motor_speed] = 0;
      //#endif

      if (parameterList[stby_time] > 0) {
        TIMEOUTtimer.setInterval(parameterList[stby_time] * 1000L); // таймаут режима ожидания
        TIMEOUTtimer.reset();
      }

      if (parameterList[keep_power] > 0) KEEP_POWERtimer.setInterval(parameterList[keep_power] * 1000L);
      else keepPowerState = 0;

      if (parameter == oled_contrast) disp.setContrast(parameterList[oled_contrast]);

      if (parameterList[parameter] <= 99 && lastParameterValue >= 100) {
        disp.setInvertMode(0);
        disp.setCursor(strWidth(MenuPages[menuPage][menuItem]), selectedRow);
#if (MENU_LANG == 0)
        clearToEOL('\'');
#else
        clearToEOL('.');//printStr(".", DISP_WIDTH - strWidth("000"));
#endif
        disp.setInvertMode(1);
      }
      if (parameterList[parameter] <= 9 && lastParameterValue >= 10) {
        disp.setInvertMode(0);
        disp.setCursor(strWidth(MenuPages[menuPage][menuItem]), selectedRow);
#if (MENU_LANG == 0)
        clearToEOL('\'');
#else
        clearToEOL('.');//printStr(".", DISP_WIDTH - strWidth("00"));
#endif
        disp.setInvertMode(1);
      }

      printInt(parameterList[parameter], Right);
      lastParameterValue = parameterList[parameter];

      timeoutReset();
    }

    if ( (parameter == motor_reverse) || (parameter == auto_parking) || (parameter == rainbow_flow) || (parameter == invert_display) ) { // boolean parameters
      parameterList[parameter] = !parameterList[parameter];
      bypass = true;
    }

    if (encBtn.pressed() ||  btn.pressed() || bypass) {
      EEPROM.update(eeAddress._timeout_off, parameterList[timeout_off]);
      EEPROM.update(eeAddress._motor_reverse, parameterList[motor_reverse]);
      EEPROM.update(eeAddress._motor_speed, parameterList[motor_speed]);
      EEPROM.update(eeAddress._auto_parking, parameterList[auto_parking]);
      EEPROM.update(eeAddress._stby_time, parameterList[stby_time]);
      EEPROM.update(eeAddress._stby_light, parameterList[stby_light]);
      EEPROM.update(eeAddress._rainbow_flow, parameterList[rainbow_flow]);
      EEPROM.update(eeAddress._max_volume, parameterList[max_volume]);
      EEPROM.update(eeAddress._keep_power, parameterList[keep_power]);
      EEPROM.update(eeAddress._invert_display, parameterList[invert_display]);
      EEPROM.update(eeAddress._leds_color, parameterList[leds_color]);
      EEPROM.update(eeAddress._oled_contrast, parameterList[oled_contrast]);

      if (parameterList[timeout_off] > 0) POWEROFFtimer.setInterval(parameterList[timeout_off] * 60000L);

#if (MOTOR_TYPE == 0)
      servo.setSpeed(parameterList[motor_speed]);
      servo.setDirection(parameterList[motor_reverse]);
      servo.start();
      servoON();
      while (!servo.tick());
      servo.stop();
      servoOFF();
#elif (MOTOR_TYPE == 1)
      stepper.setMaxSpeedDeg(parameterList[motor_speed]);
      stepper.reverse(parameterList[motor_reverse]);
      //stepper.setTargetDeg(parking_pos - 180);
      while (stepper.getState());
      //stepper.setCurrentDeg(parking_pos);
#endif

      if (thisVolume > parameterList[max_volume]) thisVolume = parameterList[max_volume];

      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (shotStates[i] == NO_GLASS) leds[i] = mHSV(parameterList[leds_color], 255, parameterList[stby_light]);
      }
      disp.invertDisplay(parameterList[invert_display]);
      timeoutReset();
      break;
    }
    keepPowerTick();
    LEDtick();
    //if ( (menuPage != SERVO_CALIBRATION_PAGE) && (menuPage != SERVICE_PAGE) ) {
    timeoutTick();
    if (!timeoutState) break;
    //}
  }
}
#endif

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      /* антидребезг датчиков с цифровым выходом
        //        static bool wait[NUM_SHOTS], swPreState[NUM_SHOTS], swState;
        //        static long stateDebounce[NUM_SHOTS];
        //        swPreState[i] = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;
        //        if(swPreState[i] && !wait[i]) stateDebounce[i] = millis();
        //        wait[i] = swPreState[i];
        //        swState = (swPreState[i] && (millis() - stateDebounce[i] > 1000)) ? true : false;
      */
      //bool swState = (bool)(analogRead(SW_pins[i]) < 512) ^ SWITCH_LEVEL;   // для датчиков с аналоговым выходом
      bool swState = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;                 // для датчиков с цифровым выходом

      if (swState && shotStates[i] == NO_GLASS) {  // поставили пустую рюмку
        if (keepPowerState) keepPowerState = false;
        shotStates[i] = EMPTY;                                      // флаг на заправку
        if (i == curSelected) strip.setLED(curSelected, mHSV(255, 0, 50));
        else  strip.setLED(i, mHSV(parameterList[leds_color], 255, 255));                      // подсветили
        LEDchanged = true;
        shotCount++;                                                // инкрементировали счётчик поставленных рюмок
        timeoutReset();                                             // сброс таймаута
        if (systemState != PUMPING && systemState != MOVING && !showMenu) {
          printNum(shotVolume[i], ml);
#ifdef OLED
          progressBar(shotVolume[i], parameterList[max_volume]);
#endif
        }
      }
      if (!swState && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected) {
          selectShot = -1;
          curSelected = -1;
        }
        if (parameterList[stby_light] > 0)
          strip.setLED(i, mHSV(parameterList[leds_color], 255, parameterList[stby_light]));
        else strip.setLED(i, mRGB(0, 0, 0));  // чёрный
        LEDchanged = true;
        //timeoutReset();                                           // сброс таймаута
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                       // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                // помпу выкл
#ifdef OLED
          volume_overall += actualVolume;
          EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
          actualVolume = 0;
        }
        shotCount--;
        if (systemState != PUMPING && systemState != MOVING && !showMenu) {
          printNum(thisVolume, ml);
#ifdef OLED
          progressBar(thisVolume, parameterList[max_volume]);
#endif
        }
      }
      if (shotStates[i] == READY) rainbowFlow(1, i);
      else  rainbowFlow(0, i);
    }
    if (shotCount == 0) {                                          // если нет ни одной рюмки
      TIMEOUTtimer.start();
      if (!parking && !systemON) systemON = true;
    }
    else  TIMEOUTtimer.stop();

    if (workMode == AutoMode)           // авто
      flowRoutine();       // крутим отработку кнопок и поиск рюмок
    else if (systemON)    // ручной
      flowRoutine();     // если активны - ищем рюмки и всё такое
  }
}

// поиск и заливка
void flowRoutine() {
  if (showMenu) return;
  static byte prepump_volume = 0;
  if (systemState == SEARCH) {                                           // если поиск рюмки
    bool noGlass = true;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (shotStates[i] == EMPTY && i != curPumping) {    // поиск
        TIMEOUTtimer.stop();
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения

        // включаем серво только если целевая позиция не совпадает с текущей
#if (MOTOR_TYPE == 0)
        if (servo.getCurrentDeg() != shotPos[i]) {
          servo.setTargetDeg(shotPos[curPumping]);        // задаём цель
          servo.start();
          servoON();                                      // вкл питание серво
#elif (MOTOR_TYPE == 1)
        if (stepper.getCurrentDeg() != shotPos[i]) {
          stepper.setTargetDeg(shotPos[curPumping]);
#endif
          parking = false;
#ifdef STATUS_LED
          LEDblinkState = true;
          LEDchanged = true;
#endif
        }
        else if (shotPos[i] == parking_pos) {             // если положение рюмки совпадает с парковочным
#if (MOTOR_TYPE == 0)
          servoON();                                      // вкл питание серво
          servo.attach(SERVO_PIN, parking_pos, SERVO_MIN_US, SERVO_MAX_US);
          delay(500);
#elif (MOTOR_TYPE == 1)
          stepper.setTargetDeg(parking_pos);
#endif
        }
#ifdef OLED
        printNum(shotVolume[curPumping], ml);
        progressBar(0);
#elif defined ANALOG_METER
        printNum(0);
#endif
        break;
      }
    }

    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки и не припаркованны
      if ( (workMode == AutoMode) && parameterList[auto_parking] == 0) {                // если в авто режиме:
        systemON = false;                                 // выключили систему
        parking = true;                                   // уже на месте!
#ifdef STATUS_LED
        if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, STATUS_LED);
        else LED = mHSV(autoModeStatusColor, 255, STATUS_LED);
#endif
      }
      else {                                              // если же в ручном режиме:
#if (MOTOR_TYPE == 0)
        if (servo.getTargetDeg() != parking_pos) {
          servo.setTargetDeg(parking_pos);
          servo.start();
          servoON();                                        // включаем серво и паркуемся
#elif (MOTOR_TYPE == 1)
        if (stepper.getTargetDeg() != parking_pos) {
          stepper.setTargetDeg(parking_pos);
#endif
#ifdef STATUS_LED
          LEDblinkState = true;
          LEDchanged = true;
#endif
#ifdef TM1637
          printNum(thisVolume);
#endif
#ifdef OLED

          progressBar(thisVolume, parameterList[max_volume]);
          displayVolumeSession();
#endif
        }
#if (MOTOR_TYPE == 0)
        if (servo.tick()) {                               // едем до упора
          servo.stop();
          servoOFF();
#elif (MOTOR_TYPE == 1)
        if (!stepper.getState()) {
#endif
          systemON = false;                               // выключили систему
          parking = true;                                 // на месте!
#ifdef STATUS_LED
          LEDblinkState = false;
          if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, STATUS_LED);
          else LED = mHSV(autoModeStatusColor, 255, STATUS_LED);
          LEDchanged = true;
#endif
//          pinMode(13, OUTPUT);
//          digitalWrite(13, 1);
//          delay(300);
//          digitalWrite(13, 0);
        }
      }
    }
    else if ( (workMode == ManualMode) && noGlass) systemON = false;     // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
  }
  else if (systemState == MOVING) {                                          // движение к рюмке
#if (MOTOR_TYPE == 0)
    if (servo.tick()) {                                   // если приехали
#elif (MOTOR_TYPE == 1)
    if (!stepper.getState()) {
#endif
#ifdef STATUS_LED
      LEDblinkState = false;
      if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, STATUS_LED);
      else LED = mHSV(autoModeStatusColor, 255, STATUS_LED);
      strip.show();
#endif
      // обнуляем счётчик
#ifdef OLED
      disp.setFont(BIG_NUM_FONT);
      disp.setCursor(0, 2);
      clearToEOL();
      byte targetX = (DISP_WIDTH - strWidth("00")) / 2 + 17;
      byte currX = 128;
      while (currX > targetX) {
        currX -= 3;
        currX = max(currX - 3, targetX);
        printStr("0 ", currX, 2);
        disp.setCursor(currX + strWidth("0"), 5);
#if(NUM_FONT == 0)
        disp.setFont(BigPostfix30x16);
#else
        disp.setFont(BigPostfix30x16_2);
#endif
        disp.write('%');
        disp.setFont(BIG_NUM_FONT);
      }
      displayVolumeSession();
#endif

      systemState = PUMPING;                              // режим - наливание
      if (!prepumped) {
        prepump_volume = PREPUMP_VOLUME; // если самая первая рюмка - учитываем прокачку
        prepumped = true;
      }
      else prepump_volume = 0;
      delay(300);
      FLOWtimer.setInterval((long)(shotVolume[curPumping] + prepump_volume) * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                  // сброс таймера
      actualVolume = 0;
      volumeCounter = 0;
      volumeColor[curPumping] = 0;
#ifdef OLED
      progressBar(-1);
#endif
      pumpON();                                           // НАЛИВАЙ!
    }
  }
  else if (systemState == PUMPING) {                           // если качаем
    //    static long tStart, tDiff, tDiffMax = 0;
    //    tStart = millis();

    volumeCounter += volumeTick;
    if ((byte)volumeCounter > actualVolume + prepump_volume) {
      actualVolume++;
      printNum(actualVolume, ml);

      //      tDiffMax = 0;

#ifdef OLED
      volume_session++;
      displayVolumeSession();
      progressBar(actualVolume, shotVolume[curPumping]);
#endif
    }

    strip.setLED(curPumping, mHSV(volumeColor[curPumping] + parameterList[leds_color], 255, 255));
    volumeColor[curPumping]++;
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
#ifdef OLED
      shots_session++;
      volume_overall += actualVolume;
      EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
      curPumping = -1;                                    // снимаем выбор рюмки
      systemState = WAIT;                                 // режим работы - ждать
      WAITtimer.reset();
      if (volumeChanged) {
        volumeChanged = false;
        EEPROM.update(eeAddress._thisVolume, thisVolume);
      }
    }

    //    tDiff = millis() - tStart;
    //    if (tDiff > tDiffMax) {
    //      tDiffMax = tDiff;
    //      disp.setFont(MAIN_FONT);
    //      printStr("  ", Left, 0);
    //      printInt(tDiffMax, Left, 0);
    //    }
  }
  else if (systemState == WAIT) {
    actualVolume = 0;
    if (WAITtimer.isReady())
      systemState = SEARCH;
  }
}

// прокачка
void prePump() {
  if (showMenu) return;
  for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
    if (!digitalRead(SW_pins[i])) {        // нашли рюмку
      curPumping = i;
#if (MOTOR_TYPE == 0)
      if (abs(servo.getCurrentDeg() - shotPos[i]) <= 3) break;
      servo.setTargetDeg(shotPos[curPumping]);
      servo.start();
      servoON();
#elif (MOTOR_TYPE == 1)
      if (abs(stepper.getCurrentDeg() - shotPos[i]) <= 3) break;
      stepper.setTargetDeg(shotPos[curPumping]);
#endif
      volumeCounter = 0;
      parking = false;
      break;
    }
  }
  if (curPumping == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
#if (MOTOR_TYPE == 0)
  while (!servo.tick()); // едем к рюмке
  servo.stop();
  servoOFF();
#elif (MOTOR_TYPE == 1)
  while (stepper.tick());
#endif
  delay(100); // небольшая задержка перед наливом

  pumpON(); // включаем помпу
  FLOWdebounce.reset();
  while (!digitalRead(SW_pins[curPumping]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
  {
    if (FLOWdebounce.isReady()) {

      volumeCounter += volumeTick;
      if ((byte)volumeCounter > actualVolume) {
        actualVolume++;
        printNum(actualVolume, ml);
      }

      strip.setLED(curPumping, mHSV(volumeColor[curPumping] + parameterList[leds_color], 255, 255));
      volumeColor[curPumping]++;
      strip.show();
    }
  }
  pumpOFF();
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState && !showMenu && (curSelected < 0)) {
    timeoutState = true;
#ifdef TM1637
    disp.brightness(7);
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    if (thisVolume < 100) {                                // объём меньше 100
      if (thisVolume < 10)
        disp.scrollByte(workMode * _dash, 0, digToHEX(thisVolume % 10), workMode * _dash, 30);  // число меньше 10 - второй индикатор пуст
      else
        disp.scrollByte(workMode * _dash, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), workMode * _dash, 30);     // число больше 9
    }
    else disp.scrollByte(digToHEX(thisVolume / 100), digToHEX((thisVolume % 100) / 10), digToHEX(thisVolume % 10), workMode * _dash, 30); // объём больше 99
#elif defined OLED
    disp.setContrast(parameterList[oled_contrast]);
    disp.invertDisplay((bool)parameterList[invert_display]);
    if ( (parameterList[timeout_off] > 0) && !POWEROFFtimer.isOn() ) {
      dispSTBicon = false;
      disp.clear();
      displayMode(workMode);
      progressBar(-1);
      if (!volumeChanged) displayVolume();
    }
    if (volumeChanged) displayVolume(); // выход из режима ожидания прокруткой энкодера - обновляем значение объёма
    // стираем иконку режима ожидания
    disp.setFont(Mode12x26);
    printInt(0, Center, 0);
#endif
  }
  TIMEOUTtimer.reset();

  if (!keepPowerState) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mHSV(255, 0, 50)); // белый
      else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(parameterList[leds_color], 255, parameterList[stby_light]);
    }
  }
#ifdef STATUS_LED
  if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, STATUS_LED);
  else LED = mHSV(autoModeStatusColor, 255, STATUS_LED);
  LEDbreathingState = false;
#endif
  //  LEDchanged = true;
  strip.show();
}

// сам таймаут
void timeoutTick() {
  if ( timeoutState && TIMEOUTtimer.isReady() && (systemState == SEARCH) ) {
    timeoutState = false;
#ifdef TM1637
    disp.brightness(0);
    printNum(thisVolume, ml);
#elif defined OLED
    disp.setContrast(1);
    if (showMenu) {
      showMenu = 0;
      menuItem = 1;
      lastMenuPage = NO_MENU;
      menuPage = MAIN_MENU_PAGE;
      disp.setInvertMode(0);
      disp.clear();
      progressBar(-1);
      displayVolume();
    }
    displayMode(workMode);

    // выводим иконку режима ожидания
    disp.setFont(Mode12x26);
    printInt(2, Center, 0);
#endif
    if (parameterList[stby_light])
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(parameterList[leds_color], 255, parameterList[stby_light] / 2);
    LEDbreathingState = true;
    LEDchanged = true;
    selectShot = -1;
    curSelected = -1;
    systemON = false;
    if (parameterList[timeout_off] > 0) POWEROFFtimer.reset();
#if (SAVE_MODE == 1)
    EEPROM.update(eeAddress._workMode, workMode);
#endif
  }

  if (parameterList[timeout_off]) {
    if (POWEROFFtimer.isReady() && !timeoutState) {
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mRGB(0, 0, 0); // black
#ifdef STATUS_LED
      LED = mHSV(0, 0, 0);  // off
      LEDbreathingState = false;
#endif
#ifdef TM1637
      disp.scrollByte(0, 0, 0, 0, 50);
#elif defined OLED
      if (parameterList[invert_display]) disp.invertDisplay(false);
      disp.clear();
      dispSTBicon = true;
      //      printStr("1", Center, 2);
#endif
      LEDchanged = true;
      POWEROFFtimer.stop();
    }
#ifdef OLED
    if (dispSTBicon) {  // отображение большой иконки режима ожидания
      if (timer100.isReady()) {
        static int8_t xDir = 1, xPos = 0;
        xPos += xDir;
        if (xPos == 92 || xPos == 0) xDir *= -1;
        disp.setFont(BigIcon36x40);
        printStr("1", xPos, 2);
      }
    }
#endif
  }
}

// обработка поддержания питания
void keepPowerTick() {
  if (parameterList[keep_power] > 0) {
    if (KEEP_POWERtimer.isReady() && (shotCount == 0)) {
      keepPowerState = 1;
      LEDchanged = true;
    }
  }
}

// обработка движения двигателя
void motorTick() {
#if (MOTOR_TYPE == 0)
  if (servo.tick()) {
#if(MOTOR_AUTO_POWER)
    servoOFF();
    servo.stop();
#endif
  }
  else {
#if(MOTOR_AUTO_POWER)
    servoON();
    servo.start();
#endif
  }
#elif (MOTOR_TYPE == 1)
  stepper.tick();
#endif
}

#if (MOTOR_TYPE == 1) && defined STEPPER_ENDSTOP
bool homing() {
  if (parking) return 0;

  if (ENDSTOP_STATUS) {
    stepper.brake();
    stepper.setRunMode(FOLLOW_POS);
    stepper.setCurrentDeg(parking_pos);
    stepper.setMaxSpeedDeg(parameterList[motor_speed]);

    parking = true;
    return 0;
  }
  else if (!stepper.tick()) {
    stepper.enable();
    stepper.setRunMode(KEEP_SPEED);
    stepper.setSpeedDeg(-STEPPER_HOMING_SPEED);
  }

  return 1;
}
#endif

// отрисовка светодиодов по флагу (50мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#ifdef STATUS_LED
    ledBreathing(LEDbreathingState);
    ledBlink(LEDblinkState);
#endif
    if (keepPowerState) keepPower();
    strip.show();
  }
}

// динамическая подсветка светодиодов
void rainbowFlow(bool _state, uint8_t _shotNum) {
  if (parameterList[rainbow_flow]) {
    static float count[NUM_SHOTS] = {0};
    if (!_state) {
      count[_shotNum] = 0;
      return;
    }
    leds[_shotNum] = mHSV((int)count[_shotNum] + volumeColor[_shotNum] + parameterList[leds_color], 255, 255);
    count[_shotNum] += 0.5;
    LEDchanged = true;
  }
}

#ifdef STATUS_LED
// еффект дыхания светодиода
void ledBreathing(bool _state) {
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
  if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, _brightness);
  else LED = mHSV(autoModeStatusColor, 255, _brightness);

  LEDchanged = true;
}

// моргание светодиода
void ledBlink(bool _state) {
  if (!_state) return;

  static bool _blink = true;

  if (timer100.isReady()) _blink = !_blink;

  LED = mHSV(11, 255, STATUS_LED * _blink); // orange

  LEDchanged = true;
}
#endif /* STATUS_LED */

// поддержание питания от повербанка
void keepPower() {
  static bool _dir = 1;
  static float _brightness = 1;
  uint8_t stby_brightness = 0;

  if (parameterList[stby_light] > 0) {
    if (timeoutState) stby_brightness = parameterList[stby_light];
    else if (POWEROFFtimer.isOn()) stby_brightness = parameterList[stby_light] / 2;
  }

  if (_brightness >= (255 - stby_brightness) ) {
    _brightness = 255 - stby_brightness;
    _dir = 0;
  }

  for (byte i = NUM_SHOTS - 1; i > 0; i--) leds[i] = leds[i - 1];
  leds[0] = mHSV(parameterList[leds_color], 255, stby_brightness + (int)_brightness);

  if (_dir) _brightness *= 1.5;
  else      _brightness /= 1.5;

  if (_brightness <= 1) {
    _brightness = 1;
    _dir = 1;
    keepPowerState = 0;
    for (byte i = 0; i < NUM_SHOTS; i++)
      leds[i] = mHSV(parameterList[leds_color], 255, stby_brightness);
    strip.show();
  }

  LEDchanged = true;
}

// функции для работы с акб
#ifdef BATTERY_PIN
// фильтрация показаний напряжения
float filter(float value) {
  static float k = 1.0, filteredValue = 4.0;
  if (battery_voltage < (BATTERY_LOW)) k = 1.0;
  else k = 0.1;
  filteredValue = (1.0 - k) * filteredValue + k * value;
  return filteredValue;
}

// проверка статуса зарядки
#ifdef CHARGER_PIN
bool charging() {
  return ( (analogRead(CHARGER_PIN) * 4.7 / 1023) >= 4.0) ? 1 : 0; // зарядка подключена, если напряжение > 4.0 вольт
}
#endif

// получение напряжения с ацп
float get_battery_voltage() {
  battery_voltage = filter(analogRead(BATTERY_PIN) * (4.7 * battery_cal) / 1023);
  return battery_voltage;
}

// преобразование напряжение в процент заряда акб
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

// мониторинг напряжения
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
#ifdef STATUS_LED
      LED = mHSV(0, 0, 0);
#endif
      strip.show();
      timeoutState = false;
#ifdef TM1637
      disp.brightness(0);
      disp.displayByte(0x39, 0x09, 0x09, 0x0F);
      delay(500);
      disp.displayByte(0x00, 0x00, 0x00, 0x00);
#elif defined OLED
      showMenu = false;
      menuItem = 1;
      lastMenuPage = NO_MENU;
      menuPage = MAIN_MENU_PAGE;
      if (parameterList[invert_display]) disp.invertDisplay(false);
#endif
    }
    else if (!lastOkStatus) {
#ifdef OLED
      progressBar(-1);
#endif
      timeoutReset();
    }
    lastOkStatus = batOk;
  }
#ifdef OLED
  if (POWEROFFtimer.isOn() || timeoutState || !batOk) displayBattery(batOk);
#endif
  return batOk;
}

#ifdef OLED
// функция вывода иконки акб
void displayBattery(bool batOk) {
  if ( batOk && showMenu ) return;

  static uint32_t currentMillis, lastDisplay = 0, lastBlink = 0;
  static bool blinkState = true;
  currentMillis = millis();
  disp.setFont(Battery12x22);

  if ( (currentMillis - lastDisplay >= 1000) && batOk) {
    lastDisplay = currentMillis;

#ifdef CHARGER_PIN
    if (charging()) {
      static byte index = 0;
      if (index == 6) index = 0;
      printInt(index, Right, 0);
      index++;
      lastDisplay -= 250;
    }
    else
#endif
      printInt(get_battery_percent(), Right, 0);
  }
  else if ( (currentMillis - lastBlink >= 500) && !batOk) {
    lastBlink = currentMillis;
    blinkState = !blinkState;
    if (blinkState) printInt(get_battery_percent(), Right, 0);
    else disp.clear();
  }
}
#endif /* OLED */
#endif /* BATTERY_PIN */
