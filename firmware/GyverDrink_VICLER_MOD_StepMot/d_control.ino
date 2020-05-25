// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    volumeChanged = true;
    timeoutReset();
    if (enc.isLeft()) {
      if (curSelected >= 0)  shotVolume[curSelected] += 1;
      else  thisVolume += 1;
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
      else if (STANDBY_LIGHT == 1)  strip.setLED(i, mHSV(20, 255, 10));
      else strip.setLED(i, mCOLOR(BLACK));
    }
    if (curSelected >= 0) dispNum(shotVolume[curSelected]);
    else dispNum(thisVolume);
    LEDchanged = true;
  }
  if (encBtn.holded()) {
    int8_t pumpingShot = -1;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (!digitalRead(SW_pins[i])) {
        stepper.enable();
        pumpingShot = i; 
      }
    }
    if (pumpingShot == -1) return;
    if (!timeoutState) disp.brightness(7);
    DEBUG("pumping into shot ");
    DEBUGln(pumpingShot);
    stepper.setRPM(STEPPER_SPEED);
    stepper.setAngle(shotPos[pumpingShot]);
    while (stepper.update());
    parking = false;
    atHome = false;
    delay(300);
    pumpON();
    timerMinim timer(20);
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) {
      if (timer.isReady()) {
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
    timeoutReset();
  }
}
