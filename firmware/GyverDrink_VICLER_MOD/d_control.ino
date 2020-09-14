// кнопки-крутилки

void encTick() {
  enc.tick();
  if (systemState == PUMPING) return;
  if (enc.isTurn()) {
    timeoutReset();
    volumeChanged = true;
    if (enc.isLeft()) {
      if (curSelected >= 0) shotVolume[curSelected] += 1;
      else thisVolume += 1;
    }
    if (enc.isRight()) {
      if (curSelected >= 0) shotVolume[curSelected] -= 1;
      else thisVolume -= 1;
    }
    if (curSelected >= 0) shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, MAX_VOLUME);
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
  if (btn.clicked()) {                        // клик!
    timeoutReset();                           // таймаут сброшен
    DEBUGln("Button pressed");
    if (systemState == PUMPING) {
      pumpOFF();                              // помпа выкл
      shotStates[curPumping] = READY;         // налитая рюмка, статус: готов
      curPumping = -1;                        // снимаем выбор рюмки
      systemState = WAIT;                     // режим работы - ждать
      WAITtimer.reset();
      DEBUGln("ABORT");
    }
    if (!workMode) systemON = true;           // система активирована
  }

  if (btn.holded()) {
    workMode = !workMode;
    if (workMode) disp.scrollByte(64, 0, 0, 64, 50);
    else  disp.scrollByte(0, 0, 0, 0, 50);
    timeoutReset();
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
    prePump();
  }
  if (btn.holdedFor(5)) {
    byte resetText[] = {_r, _E, _S, _E, _t};
    disp.runningString(resetText, sizeof(resetText), 150);
    while (!digitalRead(BTN_PIN));
    workMode = !workMode;
    dispMode();
    dispNum(thisVolume);
    resetEEPROM();
    readEEPROM();
    servoON();
    servo.attach(SERVO_PIN, parking_pos);
    delay(500);
    servo.setCurrentDeg(parking_pos);
    servoOFF();
  }
}
