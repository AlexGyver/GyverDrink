// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    timeoutReset();
    if (enc.isLeft()) thisVolume += 1;
    if (enc.isRight()) thisVolume -= 1;
    thisVolume = constrain(thisVolume, 1, 200);
    dispMode();
    EEPROM.put(0, thisVolume);
  }
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    dispMode();
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
        strip.setLED(pumpingShot, mWHEEL( (int)(volumeCount*10 + MIN_COLOR) % 1530) );
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
