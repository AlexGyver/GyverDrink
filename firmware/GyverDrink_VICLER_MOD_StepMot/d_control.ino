// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    volumeChanged = true;
    timeoutReset();
    if (enc.isLeft()) thisVolume += 1;
    if (enc.isRight())  thisVolume -= 1;
    thisVolume = constrain(thisVolume, 1, 200);
    dispMode();
  }
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    dispMode();
  }
  if (encBtn.holded()) {
    if (!timeoutState) disp.brightness(7);
    int8_t pumpingShot = -1;
    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (!digitalRead(SW_pins[i])) {
        stepper.enable();
        stepper.setAngle(shotPos[i]);
        pumpingShot = i;
      }
    }
    if (pumpingShot == -1) return;
    while (stepper.update());
    delay(300);
    timerMinim timer(100);
    pumpON();
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) {
      if (timer.isReady()) {
        volumeCount += round(100 * 50.0 / time50ml);
        dispNum(volumeCount);
        strip.setLED(pumpingShot, mWHEEL( (int)(volumeCount*10 + MIN_COLOR) % 1530) );
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
