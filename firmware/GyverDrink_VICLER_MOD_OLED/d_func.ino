// различные функции

void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    DEBUGln("service Mode");
    delay(200);
    servoON();
    servo.attach();
    int servoPos = PARKING_POS;
    long pumpTime = 0;
    timerMinim timer100(100);
    //dispNum(PARKING_POS);
    bool flag = false;
    while (1) {
      enc.tick();
      static int currShot = -1;

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW)) {
          if (flag) pumpTime += 100;
          else pumpTime = 0;
          //disp.displayInt(pumpTime);
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
            //dispNum((i + 1) * 1000 + shotPos[i]);
          } else if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
            if (STBY_LIGHT > 0) strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
            else  strip.setLED(i, mCOLOR(BLACK));
            shotStates[i] = NO_GLASS;
            currShot = -1;
            //dispNum(servoPos);
          }
          strip.show();
        }
      }

      if (enc.isTurn()) {   // крутим серво от энкодера
        pumpTime = 0;
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight())  servoPos -= 1;
        servoPos = constrain(servoPos, 0, 180);
        servo.write(servoPos);
        servo.setCurrentDeg(servoPos);
        if (!flag && shotStates[currShot] == EMPTY) {
          shotPos[currShot] = servoPos;
          //dispNum((currShot + 1) * 1000 + shotPos[currShot]);
        }
        //else if (!flag) dispNum(servoPos);
      }

      if (btn.holded()) {
        servo.setTargetDeg(PARKING_POS);
        break;
      }
    }
    //disp.clear();
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
        if (systemState != PUMPING) {
          disp.setFont(CenturyNum22x34);
          printNum(shotVolume[i], Center, 3);
        }
        DEBUG("set glass: ");
        DEBUG(i);
        DEBUG(", volume: ");
        DEBUGln(shotVolume[i]);
      }
      if (!swState && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        if (i == curSelected)
          strip.setLED(curSelected, mCOLOR(WHITE));
        else if (STBY_LIGHT > 0)
          strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
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
        if (systemState != PUMPING) {
          disp.setFont(CenturyNum22x34);
          printNum(thisVolume, Center, 3);
        }
        DEBUG("take glass: ");
        DEBUGln(i);
      }
      if (shotStates[i] == READY) {
        rainbowFlow(1, i);
      }
      else {
        rainbowFlow(0, i);
      }
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
    else {
      TIMEOUTtimer.stop();
    }

    if (workMode) {         // авто
      flowRoutnie();        // крутим отработку кнопок и поиск рюмок
    }
    else {                // ручной
      if (btn.clicked()) {  // клик!
        DEBUGln("Button pressed");
        if (systemState == PUMPING) {
          pumpOFF();                                          // помпа выкл
          shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
          curPumping = -1;                                    // снимаем выбор рюмки
          systemState = WAIT;                                 // режим работы - ждать
          WAITtimer.reset();
          DEBUGln("ABORT");
        }
        systemON = true;    // система активирована
        DEBUGln("SystemON");
        timeoutReset();     // таймаут сброшен
      }
      if (systemON) {
        DEBUGln("flowRoutine");
        flowRoutnie();  // если активны - ищем рюмки и всё такое
      }
    }
  }
}

// поиск и заливка
void flowRoutnie() {

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
        DEBUGln("SystemOFF");
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
      disp.setFont(CenturyNum22x34);
      printNum(thisVolume, Center, 3);
    }
    else if (!workMode && noGlass)                        // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
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

  } else if (systemState == PUMPING) {                      // если качаем
    disp.setFont(CenturyNum22x34);
    printNum(volumeCount += volumeTick, Center, 3);   // выводим текущий объём на дисплей
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

// отрисовка светодиодов по флагу (50мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
#if(STATUS_LED)
    ledBreathing(LEDbreathingState, NUM_SHOTS, timeoutState);
#endif
    strip.show();
  }
}

// сброс таймаута
void timeoutReset() {
  //if (!timeoutState) disp.brightness(7);
  timeoutState = true;
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();
  if (STBY_LIGHT > 0) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
      else if (shotStates[i] == NO_GLASS) leds[i] = mHSV(20, 255, STBY_LIGHT);
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
    //disp.brightness(0);
    //dispNum(thisVolume);
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
    POWEROFFtimer.reset();
    EEPROM.put(0, thisVolume);
    jerkServo();
  }

  // дёргаем питание серво, это приводит к скачку тока и powerbank не отключает систему
  if (!timeoutState && TIMEOUTtimer.isReady()) {
    if (!POWEROFFtimer.isReady()) {   // пока не сработал таймер полного отключения
      jerkServo();
    } else {
      //disp.displayByte(0x00, 0x00, 0x00, 0x00);
      //disp.point(false);
      for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mCOLOR(BLACK);
    }
  }
}

void jerkServo() {
  if (KEEP_POWER) {
    //disp.brightness(7);
    servoON();
    servo.attach();
    servo.write(servo.getCurrentDeg() + 2);
    delay(100);
    servo.write(servo.getCurrentDeg());
    delay(100);
    servo.detach();
    servoOFF();
    //disp.brightness(1);
  }
}

void rainbowFlow(bool _state, uint8_t _shotNum) {
  static float count[NUM_SHOTS] = {130};
  if (!_state) {
    count[_shotNum] = 130;
    return;
  }
  leds[_shotNum] = mHSV((int)count[_shotNum], 255, 255);
  count[_shotNum] += 0.5;
  LEDchanged = true;
}

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


#ifdef BATTERY_PIN
float get_battery_voltage() {
  battery_voltage = analogRead(BATTERY_PIN) * 4.75 / 1023.f;
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
  static uint16_t lastCheck = 0;
  static bool ok = 0;
  if (millis() - lastCheck >= 1000) {
    lastCheck = millis();
    ok = (get_battery_voltage() < BATTERY_LOW) ? 1 : 0;
    if (ok) {
      disp.clear();
      if (STBY_LIGHT > 0)
        for (byte i = 0; i < NUM_SHOTS; i++) leds[i] = mHSV(20, 255, 0);
#if(STATUS_LED)
      LED = mHSV(255, 0, _brightness);
      strip.show();
#endif
      delay(500);
    }
    disp.setFont(Battery19x9);
    printNum(get_battery_percent(), Right, 0);
  }
  return ok;
}
#endif
