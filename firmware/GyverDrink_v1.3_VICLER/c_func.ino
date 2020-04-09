// различные функции

void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    service = true;
    byte serviceText[] = {_S, _E, _r, _U, _i, _C, _E};
    disp.runningString(serviceText, sizeof(serviceText), 150);
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    delay(200);
    driverSTBY(0);
    servoON();
    int servoPos = HOME_POS;
    long pumpTime = 0;
    timerMinim timer100(100);
    disp.displayInt(HOME_POS);
    bool flag;
    while (1) {
      servo.tick();
      enc.tick();

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
          if (!digitalRead(SW_pins[i])) {
            strip.setLED(i, mCOLOR(GREEN));
          } else {
            strip.setLED(i, mCOLOR(BLACK));
          }
          strip.show();
        }
      }

      if (enc.isTurn()) {
        // крутим серво от энкодера
        pumpTime = 0;
        if (enc.isLeft()) servoPos += 1;
        if (enc.isRight())  servoPos -= 1;
        servoPos = constrain(servoPos, 0, 180);
        disp.displayInt(servoPos);
        servo.setTargetDeg(servoPos);
      }

      if (btn.holded()) {
        servo.setTargetDeg(HOME_POS);
        break;
      }
    }
  }
  disp.displayByte(0x00, 0x00, 0x00, 0x00);
  while (!servo.tick());
  servoOFF();
  service = false;
}

// выводим объём и режим
void dispMode() {
  if (thisVolume < 100) {
    disp.displayByte(0, 0x00);
    disp.display(1, thisVolume / 10);
    disp.display(2, thisVolume % 10);
    disp.displayByte(3, 0x00);
  }
  else {
    disp.display(0, thisVolume / 100);
    disp.display(1, (thisVolume % 100) / 10);
    disp.display(2, thisVolume % 10);
    disp.displayByte(3, 0x00);
  }

  if (workMode) {
    disp.point(true);
  }
  else {
    disp.point(false);
    DEBUG("MANUAL MODE");
    pumpOFF();
  }
}

void dispMode(uint8_t num) {
  if (num < 100) {
    disp.displayByte(0, 0x00);
    disp.display(1, num / 10);
    disp.display(2, num % 10);
  }
  else {
    disp.display(0, num / 100);
    disp.display(1, (num % 100) / 10);
    disp.display(2, num % 10);
  }
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (!digitalRead(SW_pins[i]) && shotStates[i] == NO_GLASS) {  // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        strip.setLED(i, mCOLOR(ORANGE));                               // подсветили
        LEDchanged = true;
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
          dispMode();
          volumeCount = 0;
        }
        DEBUG("take glass");
        DEBUG(i);
      }
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
        servoON();                                        // вкл питание серво
        servo.setTargetDeg(shotPos[curPumping]);          // задаём цель
        DEBUG("found glass");
        DEBUG(curPumping);
        break;
      }
    }
    if (noGlass) {                                        // если не нашли ни одной рюмки
      if (workMode) {
        servoOFF();                                       // выключили серво
        systemON = false;                                 // выключили систему
        DEBUG("no glass");
      }
      else {
        servoON();
        servo.setTargetDeg(HOME_POS);                              // цель серво - 180

        if (servo.tick()) {                                 // едем до упора
          DEBUG("Servo Tick");
          servoOFF();                                       // выключили серво
          systemON = false;                                 // выключили систему
          DEBUG("no glass");
        }
      }
    }
  } else if (systemState == MOVING) {                     // движение к рюмке
    if (servo.tick()) {                                   // если приехали
      systemState = PUMPING;                              // режим - наливание
      delay(300);
      FLOWtimer.setInterval((long)thisVolume * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                  // сброс таймера
      pumpON();                                           // НАЛИВАЙ!
      servoOFF();
      strip.setLED(curPumping, mCOLOR(OLIVE));           // зажгли цвет
      strip.show();
      DEBUG("fill glass");
      DEBUG(curPumping);
    }

  } else if (systemState == PUMPING) {                    // если качаем
    dispMode(volumeCount += volumeTick);
    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      volumeCount = 0;
      dispMode();
      delay(300);
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      strip.setLED(curPumping, mCOLOR(AQUA));             // подсветили
      strip.show();
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
  driverSTBY(0);
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
    driverSTBY(1);
    servoOFF();
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
