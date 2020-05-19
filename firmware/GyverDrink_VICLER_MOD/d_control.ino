// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    timeoutReset();
    if (enc.isLeft()) {
      if (curSelected >= 0) shotVolume[curSelected] += 1;
      else thisVolume += 1;
    }
    if (enc.isRight()) {
      if (curSelected >= 0) shotVolume[curSelected] -= 1;
      else thisVolume -= 1;
    }
    shotVolume[curSelected] = constrain(shotVolume[curSelected], 1, 200);
    thisVolume = constrain(thisVolume, 1, 200);
    if (curSelected >= 0) {
      dispNum(shotVolume[curSelected]);
      DEBUG("shot ");
      DEBUG(curSelected);
      DEBUG(" volume: ");
      DEBUGln(shotVolume[curSelected]);
    }
    else {
      dispNum(thisVolume);
      for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;
      DEBUG("main volume: ");
      DEBUGln(thisVolume);
    }
  }
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    dispMode();
    if (!workMode && curPumping >= 0) {
      DEBUG("abort fill for shot: ");
      DEBUGln(curPumping);
      curPumping = -1; // снимаем выбор рюмки
      systemState = WAIT;                                         // режим работы - ждать
      WAITtimer.reset();
      pumpOFF();                                                  // помпу выкл
      systemON = true;
    }
    if (workMode) DEBUGln("automatic mode");
    else DEBUGln("manual mode");
  }
  if (encBtn.clicked()) {
    timeoutReset();
    selectShot++;
    if (selectShot == NUM_SHOTS)  selectShot = -1;
    curSelected = selectShot;
    if (curSelected >= 0) {
      DEBUG("shot selected: ");
      DEBUGln(curSelected);
    }
    else {
      DEBUGln("no shots selected");
    }


    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
      else if (shotStates[i] == EMPTY) strip.setLED(i, mCOLOR(ORANGE));
      else strip.setLED(i, mCOLOR(BLACK));
    }
    if (curSelected >= 0) dispNum(shotVolume[curSelected]);
    else dispNum(thisVolume);
    LEDchanged = true;
  }
  if (encBtn.holding()) {
    if (workMode) return;
    int8_t pumpingShot = -1;

    for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
      if (!digitalRead(SW_pins[i])) {         // нашли рюмку
        servoON();
        servo.attach();
        servo.setTargetDeg(shotPos[i]);
        pumpingShot = i;
      }
    }
    if (pumpingShot == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
    if (!timeoutState) disp.brightness(7);
    DEBUG("pumping into shot ");
    DEBUGln(pumpingShot);
    while (!servo.tick()); // едем к рюмке
    servoOFF();
    delay(300); // небольшая задержка перед наливом

    pumpON(); // включаем помпу
    timerMinim timer20(20);
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
    {
      if (timer20.isReady()) {
        volumeCount += 20 * 50.0 / time50ml;
        dispNum(round(volumeCount));
        strip.setLED(pumpingShot, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
        strip.show();
      }
    }
    pumpOFF();
    DEBUG("pumping stopped, volume: ");
    DEBUG(round(volumeCount));
    DEBUGln("ml");
    delay(300);
    servoON();
    servo.setTargetDeg(PARKING_POS);
    while (!servo.tick()); // едем назад в нулевое положение
    DEBUGln("parked!");
    servoOFF();
    servo.detach();
    timeoutReset();
  }
}
