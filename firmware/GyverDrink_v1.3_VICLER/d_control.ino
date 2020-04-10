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
    int8_t pumpingShot = -1;
    if (!workMode) {
      for (byte i = 0; i < NUM_SHOTS; i++) {    // поиск наличия рюмки
        if (!digitalRead(SW_pins[i])) {         // нашли рюмку
          servoON();
          servo.attach(SERVO_PIN);
          servo.setTargetDeg(shotPos[i]);
          pumpingShot = i;
        }
      }
      if (pumpingShot == -1) return; // нет рюмок -> нет прокачки, ищем заново ^
      while (!servo.tick()); // едем к рюмке
      delay(300); // небольшая задержка перед наливом
    }
    else if(systemState != PUMPING) return;
    else pumpingShot = curPumping;

    pumpON(); // включаем помпу
    timerMinim timer100(100);
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW)) // пока стоит рюмка и зажат энкодер, продолжаем наливать
    {
      if (timer100.isReady()) {
        volumeCount += round(100 * 50.0 / time50ml);
        disp.displayInt(volumeCount);
      }
    }
    pumpOFF();
    delay(300);
    servo.setTargetDeg(0);
    while (!servo.tick()); // едем назад в нулевое положение
    servoOFF();
    servo.detach();
    timeoutReset();
    if (workMode) systemState = WAIT;
  }
}
