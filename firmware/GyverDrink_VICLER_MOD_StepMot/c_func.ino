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
    long pumpTime = 0;
    timerMinim timer100(100);
    dispNum(PARKING_POS);
    bool flag;
    while (1) {
      enc.tick();
      stepper.update();

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
        if (enc.isLeft()) stepperPos += 1;
        if (enc.isRight())  stepperPos -= 1;
        stepperPos = constrain(stepperPos, 0, 360);
        dispNum(stepperPos);
        stepper.setAngle(stepperPos);
      }

      if (btn.holded()) break;
    }
    disp.clear();
    HeadLED = ORANGE;
    strip.show();
#ifdef STEPPER_ENDSTOP
    while (homing()); // двигаемся пока не сработал концевик
    stepper.setAngle(PARKING_POS);
    while (stepper.update());
#else
    stepper.setAngle(PARKING_POS);
    while (stepper.update());
#endif
    stepper.disable();
    stepper.autoPower(STEPPER_POWERSAFE);
    HeadLED = WHITE;
  }
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
    pumpOFF();
  }
}

void dispNum(int num) {
  static int lastNum = 0;
  if(num == lastNum) return;
  lastNum = num;
  if (num < 100) {
    disp.displayByte(0, 0x00);
    disp.display(1, num / 10);
    disp.display(2, num % 10);
    disp.displayByte(3, 0x00);
  }
  else {
    disp.display(0, num / 100);
    disp.display(1, (num % 100) / 10);
    disp.display(2, num % 10);
    disp.displayByte(3, 0x00);
  }
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    static uint8_t shotCount = 0;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      bool swState = !digitalRead(SW_pins[i]) ^ SWITCH_LEVEL;
      if (swState && shotStates[i] == NO_GLASS) {                   // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        strip.setLED(i, ORANGE);                                    // подсветили
        LEDchanged = true;
        shotCount += 1;
        DEBUG("set glass");
        DEBUG(i);
      }
      else if (!swState && shotStates[i] != NO_GLASS) {             // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        strip.setLED(i, mCOLOR(BLACK));                             // нигра
        LEDchanged = true;
        //timeoutReset();                                           // сброс таймаута
        if (i == curPumping) {
          curPumping = -1; // снимаем выбор рюмки
          systemState = WAIT;                                       // режим работы - ждать
          WAITtimer.reset();
          pumpOFF();                                                // помпу выкл
          //dispMode();
        }
        dispMode();
        shotCount -= 1;
        DEBUG("take glass");
        DEBUG(i);
        volumeCount = 0;
      }
    }
    if (shotCount == 0) {                                          // если нет ни одной рюмки 
      TIMEOUTtimer.start();
      rainbow = false;
      towerLight = false;
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
      if (shotStates[i] == EMPTY && i != curPumping) {    // поиск
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения

        if (shotPos[curPumping] != (int)stepper.getAngle()) {  // если цель отличается от актуальной позиции
          stepper.enable();
          stepper.setAngle(shotPos[curPumping]);          // задаём цель
          parking = false;
          HeadLED = ORANGE;
          strip.show();
          DEBUG("GO!");
        }
        DEBUG("found glass");
        DEBUG(curPumping);
        break;
      }
    }
    if (noGlass && !parking) {                            // если не нашли ни одной пустой рюмки
      stepper.setAngle(PARKING_POS);                      // цель -> домашнее положение
      HeadLED = ORANGE;
      LEDchanged = true;
      if (stepper.ready()) {                              // приехали
        stepper.disable();                                // выключили шаговик
        systemON = false;                                 // выключили систему
        HeadLED = WHITE;
        strip.show();
        parking = 1;
        rainbow = true;                                   // радужная подсветка наполненных рюмок
        DEBUG("parked!");
      }
    }
    else if (!workMode && noGlass)                       // если в ручном режиме, припаркованны и нет рюмок - отключаемся нахрен
      systemON = false;

  } else if (systemState == MOVING) {                    // движение к рюмке
    if (stepper.ready()) {                               // если приехали
      systemState = PUMPING;                             // режим - наливание
      delay(300);
      FLOWtimer.setInterval((long)thisVolume * time50ml / 50);  // перенастроили таймер
      FLOWtimer.reset();                                 // сброс таймера
      pumpON();                                          // НАЛИВАЙ!
      volumeCount = 0;
      strip.setLED(curPumping, mCOLOR(YELLOW));
      HeadLED = WHITE;
      strip.show();
      DEBUG("fill glass");
      DEBUG(curPumping);
    }

  } else if (systemState == PUMPING) {                    // если качаем
    volumeCount += volumeTick;
    dispNum(round(volumeCount));
    int colorCount = MIN_COLOR + volumeCount * COLOR_SCALE / thisVolume;
    strip.setLED(curPumping, mWHEEL(colorCount));
    LEDchanged = true;
    
    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      strip.setLED(curPumping, mCOLOR(AQUA));
      strip.show();
      curPumping = -1;                                    // снимаем выбор рюмки
      systemState = WAIT;                                 // режим работы - ждать
      WAITtimer.reset();
      DEBUG("wait");
    }
  } else if (systemState == WAIT) {
    if (WAITtimer.isReady()) {                            // подождали после наливания
      systemState = SEARCH;
      timeoutReset();
      DEBUG("search");
    }
  }
}

// отрисовка светодиодов
void LEDtick() {
  if (LEDchanged && LEDtimer.isReady()) {
    LEDchanged = false;
    strip.show();
  }
  if (rainbow)  rainbowFlow(130);
  if (towerLight) towerBreathing(10);
}

// сброс таймаута
void timeoutReset() {
  if (!timeoutState) {
    disp.brightness(7);
    dispMode();
  }
  towerLight = false;
  HeadLED = WHITE;
  LEDchanged = true;
  
  timeoutState = true;
  TIMEOUTtimer.reset();
  TIMEOUTtimer.start();
  DEBUG("timeout reset");
}

// сам таймаут
void timeoutTick() {
  if (systemState == SEARCH && timeoutState && TIMEOUTtimer.isReady()) {
    DEBUG("timeout");
    timeoutState = false;
    disp.brightness(0);
    LEDchanged = true;
    POWEROFFtimer.reset();
    jerkServo();
    if (volumeChanged) {
      volumeChanged = false;
      EEPROM.put(0, thisVolume);
    }
    towerLight = true;
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
  if (KEEP_POWER == ON) {
    stepper.enable();
    delay(200);
    stepper.disable();
  }
}

#ifdef STEPPER_ENDSTOP
bool homing() {
  if (ENDSTOP_STATUS) {
    stepper.setRPM(STEPPER_SPEED);
    stepper.resetPos();
    return 0;
  }
  stepper.enable();
  stepper.setRPM(5);
  stepper.rotate(CCW);
  stepper.update();
  return 1;
}
#endif


void towerBreathing(uint8_t period) {
  static timerMinim timer(period);
  static uint8_t i = 127;
  static int8_t dir = -1;

  if (timer.isReady()) {
    i += dir;
    if (i == 0 || dir * i == 127) dir *= -1;
    leds[NUM_SHOTS] = mHSV(255, 0, i);
    strip.show();
  }
}

bool rainbowFadeFlow(uint8_t startBrightness, uint32_t period) {
  static timerMinim timer(period);
  static uint8_t count = startBrightness;
  if (timer.isReady()) {
    for (byte i = 0; i < NUM_SHOTS + 1; i++) {
      leds[i] = mHSV(count + i * (255 / NUM_SHOTS + 1), 255, count);
    }
    count--;
    strip.show();
  }
  return (count) ? 1 : 0;
}

void rainbowFlow(uint8_t Brightness) {
  static timerMinim timer(50);
  static uint8_t count = 0;
  if (timer.isReady()) {
    for (byte i = 0; i < NUM_SHOTS + 1; i++) {
      if (shotStates[i] == READY)
        leds[i] = mHSV(count + i * (255 / NUM_SHOTS + 1), 255, Brightness);
    }
    count++;
    strip.show();
  }
}

void showAnimation(byte mode, uint8_t period) {
  static timerMinim timer(period);
  if (!timer.isReady()) return;

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
