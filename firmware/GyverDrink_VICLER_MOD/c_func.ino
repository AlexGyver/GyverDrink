// различные функции

void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
    disp.runningString(serviceText, sizeof(serviceText), 150);
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    delay(200);
    servoON();
    servo.attach();
    int servoPos = PARKING_POS;
    long pumpTime = 0;
    timerMinim timer100(100);
    dispNum(PARKING_POS);
    bool flag;
    while (1) {
      enc.tick();
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
            dispNum((i + 1) * 1000 + shotPos[i]);
            currShot = i;
          } else if (digitalRead(SW_pins[i]) && shotStates[i] == EMPTY)  {
            strip.setLED(i, mCOLOR(BLACK));
            shotStates[i] = NO_GLASS;
            currShot = -1;
            dispNum(servoPos);
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
          dispNum((currShot + 1) * 1000 + shotPos[currShot]);
        }
        else if (!flag) dispNum(servoPos);
      }

      if (btn.holded()) {
        servo.setTargetDeg(PARKING_POS);
        break;
      }
    }
    disp.clear();
    while (!servo.tick());
    servoOFF();
    servo.detach();
    
    // сохраняем настройки таймера налива
    if (pumpTime > 0) {
      time50ml = pumpTime;
      volumeTick = 20.0f * 50.0f / time50ml;
      EEPROM.write(1001, 47);
      EEPROM.put(10, pumpTime);
    }
    // сохраняем значения углов в память
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)  EEPROM.write(100 + i, shotPos[i]);
  }
}

// выводим объём и режим
void dispMode() {
  dispNum(thisVolume);
  if (workMode) {
    disp.point(true);
  }
  else {
    disp.point(false);
    pumpOFF();
  }
}

void dispNum(uint16_t num) {
  static int lastNum = 0;
  if (num == lastNum) return;
  lastNum = num;
  if (num < 100) {
    disp.displayByte(0, 0x00);
    if (num < 10) disp.displayByte(1, 0x00);
    else disp.display(1, num / 10);
    disp.display(2, num % 10);
    disp.displayByte(3, 0x00);
  }
  else if (num < 1000) {
    disp.display(0, num / 100);
    disp.display(1, (num % 100) / 10);
    disp.display(2, num % 10);
    disp.displayByte(3, 0x00);
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
    static uint8_t shotCount = 0;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (!digitalRead(SW_pins[i]) && shotStates[i] == NO_GLASS) {  // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        strip.setLED(i, mCOLOR(ORANGE));                               // подсветили
        LEDchanged = true;
        shotCount++;
        DEBUG("set glass");
        DEBUG(i);
      }
      if (digitalRead(SW_pins[i]) && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        strip.setLED(i, mCOLOR(BLACK));                             // нигра
        LEDchanged = true;
        timeoutReset();                                             // сброс таймаута
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                         // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                  // помпу выкл
        }
        volumeCount = 0;
        dispMode();
        shotCount--;
        DEBUG("take glass");
        DEBUG(i);
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
    }
    else {
      TIMEOUTtimer.stop();
    }

    if (workMode) {         // авто
      flowRoutnie();        // крутим отработку кнопок и поиск рюмок
    } else {                // ручной
      if (btn.clicked()) {  // клик!
        systemON = true;    // система активирована
        timeoutReset();     // таймаут сброшен
      }
      if (systemON) flowRoutnie();  // если активны - ищем рюмки и всё такое
    }
  }
}

// поиск и заливка
void flowRoutnie() {

  if (systemState == SEARCH) {                            // если поиск рюмки
    bool noGlass = true;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if ( *(shotStates + i) == EMPTY && i != curPumping) {    // поиск
        TIMEOUTtimer.stop();
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения
        if (shotPos[i] != servo.getCurrentDeg()) {        // включаем серво только если целевая позиция не совпадает с текущей
          servoON();                                      // вкл питание серво
          servo.attach();
          servo.setTargetDeg(shotPos[curPumping]);        // задаём цель
          parking = false;
        }

        DEBUG("found glass");
        DEBUG(curPumping);
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки и не припаркованны
      if (workMode) {                                       // если в авто режиме:
        servoOFF();                                           // выключили серво
        servo.detach();
        systemON = false;                                     // выключили систему
        parking = true;                                       // уже на месте!
        DEBUG("parked!");
      }
      else {                                              // если же в ручном режиме:
        servoON();                                          // включаем серво и паркуемся
        servo.attach();
        servo.setTargetDeg(PARKING_POS);

        if (servo.tick()) {                                 // едем до упора
          servoOFF();                                       // выключили серво
          servo.detach();
          systemON = false;                                 // выключили систему
          parking = true;                                   // на месте!
          DEBUG("no glass");
        }
      }
    }
    else if (!workMode && noGlass)                        // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
      systemON = false;

  } else if (systemState == MOVING) {                     // движение к рюмке
    if (servo.tick()) {                                   // если приехали
      servoOFF();                                         // отключаем сервопривод
      servo.detach();
      systemState = PUMPING;                              // режим - наливание
      delay(300);
      FLOWtimer.setInterval((long)thisVolume * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                  // сброс таймера
      pumpON();                                           // НАЛИВАЙ!
      volumeCount = 0;
      DEBUG("fill glass");
      DEBUG(curPumping);
    }

  } else if (systemState == PUMPING) {                      // если качаем
    dispNum(volumeCount += volumeTick);                     // выводим текущий объём на дисплей
    int colorCount = MIN_COLOR + COLOR_SCALE * volumeCount / thisVolume;  // thisVolume;  // расчёт цвета для текущего обьёма
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;

    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      dispMode();
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      curPumping = -1;                                    // снимаем выбор рюмки
      systemState = WAIT;                                 // режим работы - ждать
      WAITtimer.reset();
      DEBUG("wait");
    }
  } else if (systemState == WAIT) {
    if (WAITtimer.isReady()) {
      systemState = SEARCH;
      DEBUG("search");
    }
  }
}

// отрисовка светодиодов по флагу (100мс)
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
    strip.show();
  }
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState) disp.brightness(7);
  timeoutState = true;
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();
  DEBUG("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady() && systemState == SEARCH) {
    DEBUG("timeout");
    timeoutState = false;
    disp.brightness(1);
    servoOFF();
    servo.detach();
    systemON = false;
    POWEROFFtimer.reset();
    jerkServo();
  }

  // дёргаем питание серво, это приводит к скачку тока и powerbank не отключает систему
  if (!timeoutState && TIMEOUTtimer.isReady()) {
    if (!POWEROFFtimer.isReady()) {   // пока не сработал таймер полного отключения
      jerkServo();
    } else {
      disp.displayByte(0x00, 0x00, 0x00, 0x00);
      disp.point(false);
    }
  }
}

void jerkServo() {
  if (KEEP_POWER) {
    disp.brightness(7);
    servoON();
    servo.attach();
    servo.write(random(0, 4));
    delay(200);
    servo.detach();
    servoOFF();
    disp.brightness(1);
  }
}

void rainbowFlow(bool _state, uint8_t _shotNum) {
  static byte count[NUM_SHOTS] = {130};
  if (!_state) {
    count[_shotNum] = 130;
    return;
  }
  leds[_shotNum] = mHSV(count[_shotNum], 255, 255);
  count[_shotNum] += 1;
  LEDchanged = true;
}

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
}
