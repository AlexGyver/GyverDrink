// кнопки-крутилки

void encTick() {
  enc.tick();
  if (systemState == PUMPING) return;
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
    if (curSelected) shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, MAX_VOLUME);
    thisVolume = constrain(thisVolume, 1, MAX_VOLUME);

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
    if (workMode) {
      DEBUGln("automatic mode");
    }
    else {
      DEBUGln("manual mode");
    }
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
      else if (STBY_LIGHT > 0)  strip.setLED(i, mHSV(20, 255, STBY_LIGHT));
      else strip.setLED(i, mCOLOR(BLACK));
    }
    if (curSelected >= 0) dispNum(shotVolume[curSelected]);
    else dispNum(thisVolume);
    LEDchanged = true;
  }
  if (encBtn.holding()) {
    if (workMode) return;

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
    if (!timeoutState) disp.brightness(7);
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
        dispNum(round(volumeCount));
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
}
