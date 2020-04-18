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
    thisVolume = constrain(thisVolume, 1, 200);
    shotVolume[curSelected] = constrain(shotVolume[curSelected], 1, 200);
    if (curSelected >= 0) dispNum(shotVolume[curSelected]);
    else {
      dispMode();
      for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;
    }
  }
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    dispMode();
  }
  if (encBtn.clicked()) {
    timeoutReset();
    selectShot++;
    if (selectShot == NUM_SHOTS)  selectShot = -1;
    curSelected = selectShot;

    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
      else if (shotStates[i] == EMPTY) strip.setLED(i, mCOLOR(ORANGE));
      else strip.setLED(i, mCOLOR(BLACK));
    }
    if (curSelected >= 0) dispNum(shotVolume[curSelected]);
    else dispMode();
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
    stepper.setAngle(shotPos[pumpingShot]);
    while (stepper.update());
    delay(300);
    timerMinim timer(100);
    pumpON();
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) {
      if (timer.isReady()) {
        volumeCount += round(100 * 50.0 / time50ml);
        dispNum(volumeCount);
        strip.setLED(pumpingShot, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
        strip.show();
      }
    }
    pumpOFF();
    delay(300);
    stepper.setAngle(PARKING_POS);
    timeoutReset();
    if (workMode) systemState = WAIT;
  }
}
