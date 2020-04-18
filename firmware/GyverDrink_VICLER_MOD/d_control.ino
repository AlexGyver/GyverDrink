// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
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
    if(curSelected >= 0) dispNum(shotVolume[curSelected]);
    else {
      dispMode();
      for(byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;
    }
    EEPROM.put(0, thisVolume);
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
    if(curSelected >= 0) dispNum(shotVolume[curSelected]);
    else dispMode();
    LEDchanged = true;
  }
  if (encBtn.holding()) {
    if (workMode) return;
    int8_t pumpingShot = -1;
    if (!timeoutState) disp.brightness(7);

    for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
      if (!digitalRead(SW_pins[i])) {         // нашли рюмку
        servoON();
        servo.attach();
        servo.setTargetDeg(shotPos[i]);
        pumpingShot = i;
      }
    }
    if (pumpingShot == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
    while (!servo.tick()); // едем к рюмке
    servoOFF();
    delay(300); // небольшая задержка перед наливом

    pumpON(); // включаем помпу
    timerMinim timer50(50);
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
    {
      if (timer50.isReady()) {
        volumeCount += 50 * 50.0 / time50ml;
        dispNum(round(volumeCount));
        strip.setLED(pumpingShot, mWHEEL( (int)(volumeCount * 10 + MIN_COLOR) % 1530) );
        strip.show();
      }
    }
    pumpOFF();
    delay(300);
    servoON();
    servo.setTargetDeg(PARKING_POS);
    while (!servo.tick()); // едем назад в нулевое положение
    servoOFF();
    servo.detach();
    timeoutReset();
  }
}
