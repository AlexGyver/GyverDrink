// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    timeoutReset();
    if (enc.isLeft()) {
      thisVolume += 5;
      thisVolume = constrain(thisVolume, 10, 200);
    }
    if (enc.isRight()) {
      thisVolume -= 5;
      thisVolume = constrain(thisVolume, 10, 200);
    }
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
  if (encBtn.holded()) {
    if (digitalRead(SW_pins[PUMPING_SHOT])) return;
    driverSTBY(0);
    servoON();
    servo.write(shotPos[PUMPING_SHOT]);
    servo.setTargetDeg(shotPos[PUMPING_SHOT]);
    delay(500);
    pumpON();
    
    while (!digitalRead(ENC_SW) && !digitalRead(SW_pins[PUMPING_SHOT]));
    pumpOFF();
    delay(300);
    servoON();
    servo.write(HOME_POS);
    delay(100);
    servoOFF();
    timeoutReset();
  }
}
