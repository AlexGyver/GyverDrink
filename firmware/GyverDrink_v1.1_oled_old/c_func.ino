// различные функции

void serviceMode() {
  if (!digitalRead(BTN_PIN)) {
    serviceState = true;
    dispMode();
    u8g2.sendBuffer();
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    delay(200);
    //servoON();
    servo.start();
    int servoPos = 0;
    int pumpTime = 0;
    timerMinim timer100(100);
    bool flag;
    for (;;) {
      servo.tick();
      enc.tick();

      if (timer100.isReady()) {   // период 100 мс
        // работа помпы со счётчиком
        if (!digitalRead(ENC_SW)) {
          if (flag) pumpTime += 100;
          else pumpTime = 0;
          //disp.displayInt(pumpTime);
          //dispMode();
          //dispNum(pumpTime, none);
          DEBUG(pumpTime);
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
        if (enc.isLeft()) {
          servoPos += 1;
        }
        if (enc.isRight()) {
          servoPos -= 1;
        }
        servoPos = constrain(servoPos, 0, 180);
        //disp.displayInt(servoPos);
        //dispMode();
        //dispNum(servoPos, none);
        DEBUG(servoPos);
        servo.start();
        //servo.setTargetDeg(servoPos);
        servo.write(servoPos);
      }

      if (btn.holded()) {
        //servo.start();
        //servo.setTargetDeg(0);
        servo.write(0);
        serviceState = false;
        break;
      }
    }
  }
  //disp.clear();
  //while (!servo.tick());
  //servoOFF();
}

// выводим объём и режим
void dispMode() {
  u8g2.clearBuffer();
  char *modeStr = "";

  if (serviceState) {
    modeStr = "Service";
  }
  else {
    if (workMode)  modeStr = "Auto";
    else {
      modeStr = "Manual";
      pumpOFF();
    }
  }
  // mode display
  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_10x20_t_cyrillic);
  u8g2.setCursor(0, 13);
  u8g2.print(modeStr);

  // battery display
  char batStr[3];
  uint8_t bat = getVoltagePercent() / 20;
  sprintf_P(batStr, (PGM_P)F("%d"), bat);
  u8g2.setFont(u8g2_font_battery19_tn);
  u8g2.setFontDirection(1);
  u8g2.setCursor(108, 0);
  u8g2.print(batStr);

//  u8g2.setCursor(70, 13);
//  u8g2.print(battVoltage);
}

// наливайка, опрос кнопок
void flowTick() {
  if (FLOWdebounce.isReady()) {
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (!digitalRead(SW_pins[i]) && shotStates[i] == NO_GLASS) {  // поставили пустую рюмку
        timeoutReset();                                             // сброс таймаута
        shotStates[i] = EMPTY;                                      // флаг на заправку
        strip.setLED(i, mCOLOR(RED));                               // подсветили
        LEDchanged = true;
        DEBUG("set glass");
        DEBUG(i);
      }
      if (digitalRead(SW_pins[i]) && shotStates[i] != NO_GLASS) {   // убрали пустую/полную рюмку
        shotStates[i] = NO_GLASS;                                   // статус - нет рюмки
        strip.setLED(i, mCOLOR(BLACK));                             // нигра
        LEDchanged = true;
        timeoutReset();                                             // сброс таймаута
        if (systemState == PUMPING || systemState == MOVING) {                               // убрали во время заправки!
          if (i == curPumping) {
            curPumping = -1; // снимаем выбор рюмки
            systemState = WAIT;                                         // режим работы - ждать
            WAITtimer.reset();
            pumpOFF();                                                  // помпу выкл
          }
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
      if (shotStates[i] == EMPTY && i != curPumping) {    // поиск рюмки
        noGlass = false;                                  // флаг что нашли хоть одну рюмку
        curPumping = i;                                   // запоминаем выбор
        systemState = MOVING;                             // режим - движение
        shotStates[curPumping] = IN_PROCESS;              // стакан в режиме заполнения
        //servoON();                                        // вкл питание серво
        servo.attach(SERVO_PIN);
        servo.setTargetDeg(shotPos[curPumping]);          // задаём цель
        DEBUG("find glass");
        DEBUG(curPumping);
        break;
      }
    }
    if (noGlass) {                                        // если не нашли ни одной рюмки
      //servoON();
      servo.setTargetDeg(0);                              // цель серво - 0
      if (servo.tick()) {                                 // едем до упора
        //servoOFF();                                       // выключили серво
        systemON = false;                                 // выключили систему
        DEBUG("no glass");
        TIMEOUTtimer.start();
      }
    }
  } else if (systemState == MOVING) {                     // движение к рюмке
    if (servo.tick()) {                                   // если приехали
      systemState = PUMPING;                              // режим - наливание
      delay(500);
      FLOWtimer.setInterval(thisVolume * time1ml);  // перенастроили таймер
      FLOWtimer.reset();                                  // сброс таймера
      pumpON();                                           // НАЛИВАЙ!
      strip.setLED(curPumping, mCOLOR(YELLOW));           // зажгли цвет
      strip.show();
      DEBUG("fill glass");
      DEBUG(curPumping);
    }

  } else if (systemState == PUMPING) {                    // если качаем
    if (FLOWtimer.isReady()) {                            // если налили (таймер)
      pumpOFF();                                          // помпа выкл
      shotStates[curPumping] = READY;                     // налитая рюмка, статус: готов
      strip.setLED(curPumping, mCOLOR(LIME));             // подсветили
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
  //if (!timeoutState) disp.brightness(7);
  timeoutState = true;
  TIMEOUTtimer.stop();
}

// сам таймаут
void timeoutTick() {
  if (timeoutState && TIMEOUTtimer.isReady()) {
    timeoutState = false;
    //disp.brightness(1);
    POWEROFFtimer.reset();
  }
}

float getVoltage(){
  return  analogRead(BATTERY_PIN) * 5.0 / 1023;
}

uint8_t getVoltagePercent(){
  uint8_t percent = 0;
  if(battVoltage >= 4.02) percent = 100;
  else if(battVoltage >= 3.87) percent = 80;
  else if(battVoltage >= 3.8) percent = 60;
  else if(battVoltage >= 3.73) percent = 40;
  else if(battVoltage >= 3.27) percent = 20;
  return percent;
}
// Battery management
void batteryCheck(){
  if(BatteryTimer.isReady()){
    battVoltage = BatteryFilter.smooth(getVoltage());
    //battVoltage = getVoltage();
    dispMode();
    dispNum(thisVolume, ml);
    //DEBUG(battVoltage);
  }
}
