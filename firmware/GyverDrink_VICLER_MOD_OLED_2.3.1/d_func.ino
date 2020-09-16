
void serviceRoutine(serviceModes mode) {
  //==============================================================================
  //                            калибровка объёма
  //==============================================================================
  timerMinim timer100(100);

  if (mode == VOLUME) {                      // калибровка объёма
    long pumpTime = 0;
    bool flag = false;
    disp.setInvertMode(1);
    printStr("  Калибр. объ¿ма  \n", 0, 0);
    disp.setInvertMode(0);
    printStr(" Поставьте рюмку\n", 0, 3);
    printStr(" Налейте 50мл\n");
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
    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 15.0f * 50.0f / time50ml;
      EEPROM.put(eeAddress._time50ml, pumpTime);
    }
  }
  //==============================================================================
  //                       настройка серводвигателя
  //==============================================================================
  else if (mode == SERVO) {         // калибровка углов серво
    int servoPos = settingsList[parking_pos];
    printStr("Поставьте рюмку\n", 0, 0);
    printStr("Выставите угол\n");
    printStr("Уберите рюмку\n");
    printStr("Повторите для всех");
    delay(5000);
    disp.clear();
    disp.setInvertMode(1);
    printStr("   Калибр. серво   \n", 0, 0);
    disp.setInvertMode(0);
    printVolume(servoPos);
    for (byte i = 0; i < NUM_SHOTS; i++)
      strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
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
          printVolume(servoPos);
          servo.setTargetDeg(servoPos);
          servoON();
          servo.start();
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
            servoPos = settingsList[parking_pos];
            printVolume(servoPos);
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
          printVolume(servoPos);
          servo.setTargetDeg(servoPos);
          servoON();
          servo.start();
          while (!servo.tick());
          servoOFF();
          break;
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight())  servoPos -= 1;
        servoPos = constrain(servoPos, 0, 180);
        servoON();
        servo.attach(SERVO_PIN, servoPos);
        if (shotCount == 0) settingsList[parking_pos] = servoPos;
        if (shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
          printVolume(shotPos[currShot]);
        }
        else printVolume(servoPos);
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
    // сохраняем значения углов в память
    for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.update(eeAddress._shotPos + i, shotPos[i]);
    EEPROM.update(eeAddress._parking_pos, settingsList[parking_pos]);
  }
  //==============================================================================
  //                     калибровка напряжения аккумулятора
  //==============================================================================
#ifdef BATTERY_PIN
  else if (mode == BATTERY) {
    disp.setInvertMode(1);
    disp.setFont(CenturyGothic10x16);
    printStr("  Калибр. аккум-а  \n", 0, 0);
    disp.setInvertMode(0);
    //timerMinim timer100(100);
    disp.setFont(lcdnums14x24);
    while (1) {
      enc.tick();

      if (timer100.isReady()) printFloat(get_battery_voltage(), 2, Center, 4);

      if (enc.isTurn()) {
        if (enc.isLeft())  battery_cal += 0.01;
        if (enc.isRight()) battery_cal -= 0.01;
        battery_cal = constrain(battery_cal, 0, 3.0);
      }

      if (btn.holded()) {
        EEPROM.put(eeAddress._battery_cal, battery_cal);
        timeoutReset();
        disp.clear();
        break;
      }
    }
  }
#endif
}

void settingsMenuHandler(uint8_t row) {
  bool bypass = false;
  uint8_t parameter = menuItem - 1;
  if (menuItem == menuItemsNum[menuPage] - 1) {
    resetEEPROM();
    readEEPROM();
    bypass = true;
  }
  else {
    disp.setInvertMode(1);
    disp.setFont(Callibri15);
    printStr(MenuPages[menuPage][menuItem], 0, row);
    printStr("                       ");
    printNum(settingsList[parameter], Right);
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

      //      if (parameter == parking_pos) {
      //        servoON();
      //        servo.attach(SERVO_PIN, settingsList[parking_pos]);
      //        delay(15);
      //      }

      if (settingsList[stby_time]) {
        TIMEOUTtimer.setInterval(settingsList[stby_time] * 1000L); // таймаут режима ожидания
        TIMEOUTtimer.reset();
      }
      if (settingsList[keep_power]) KEEP_POWERtimer.setInterval(settingsList[keep_power] * 1000L);
      else keepPowerState = 0;


      disp.setInvertMode(1);
      disp.setFont(Callibri15);
      printStr(MenuPages[menuPage][menuItem], 0, row);
      printStr("                       ");
      printNum(settingsList[parameter], Right);

      timeoutReset();
    }

    if ( (parameter == inverse_servo) || (parameter == auto_parking) || (parameter == rainbow_flow) || (parameter == invert_display) ) {
      settingsList[parameter] = !settingsList[parameter];
      bypass = true;
    }

    if (encBtn.clicked() || bypass) {
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

      servo.setSpeed(settingsList[servo_speed] * 2);
      servo.setDirection(settingsList[inverse_servo]);
      servoON();
      servo.attach(SERVO_PIN, settingsList[parking_pos]);
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
    LEDtick();
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
        systemON = false;                                 // выключили систему
        DEBUGln("SystemOFF");
        parking = true;                                   // уже на месте!
        LEDbreathingState = true;
        LEDchanged = true;
        DEBUGln("parked!");
      }
      else {                                              // если же в ручном режиме:

        servoON();                                        // включаем серво и паркуемся
        servo.setTargetDeg(settingsList[parking_pos]);
#if(STATUS_LED)
        LED = mHSV(11, 255, STATUS_LED); // orange
        LEDchanged = true;
#endif

        if (servoReady) {                               // едем до упора
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

  } else if (systemState == PUMPING) {                    // если качаем
    printVolume(volumeCount += volumeTick);               // выводим текущий объём на дисплей
    int colorCount = MIN_COLOR + COLOR_SCALE * volumeCount / shotVolume[curPumping];  // расчёт цвета для текущего обьёма
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      shots_overall++;
      volume_overall += volumeCount;
      EEPROM.put(eeAddress._shots_overall, shots_overall);
      EEPROM.put(eeAddress._volume_overall, volume_overall);
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
  if (showMenu) return;
  for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
    if (!digitalRead(SW_pins[i])) {        // нашли рюмку
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
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState && !showMenu && (curSelected < 0)) displayMode(workMode);
  timeoutState = true;
  disp.setContrast(255);
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
  //DEBUGln("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady() && systemState == SEARCH) {
    DEBUGln("timeout");
    timeoutState = false;
    disp.setContrast(0);
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
      EEPROM.update(0, thisVolume);
    }
  }

  if (settingsList[keep_power]) {
    if (KEEP_POWERtimer.isReady() && (shotCount == 0) && ( (settingsList[keep_power] < settingsList[stby_time]) || showMenu) && (curSelected == -1)) {
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
      LEDchanged = true;
      disp.clear();
      POWEROFFtimer.stop();
      if (showMenu) {
        menuPage = MENU_PAGE;
        menuItem = 1;
        showMenu = false;
      }
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
  if (mode) LED = mHSV(130, 255, _brightness);
  else {
    LED = mHSV(255, 0, _brightness);
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
  static uint32_t lastMillis = 0;
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
  if (POWEROFFtimer.isOn() || timeoutState) displayBattery(batOk);
  return batOk;
}
void displayBattery(bool batOk) {
  if ( batOk && showMenu ) return;

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
