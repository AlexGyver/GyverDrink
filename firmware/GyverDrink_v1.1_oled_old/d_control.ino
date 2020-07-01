// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    timeoutReset();
    if (enc.isLeft()) {
      thisVolume += 5;
    }
    if (enc.isRight()) {
      thisVolume -= 5;
    }
    thisVolume = constrain(thisVolume, 5, 200);
    EEPROM.put(0, thisVolume);
    dispMode();
    dispNum(thisVolume, ml);
  }
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    EEPROM.put(sizeof(thisVolume), workMode);
    dispMode();
    dispNum(thisVolume, ml);
  }
  if (encBtn.holded()) {
    if(workMode) workMode = !workMode;
    dispMode();
    dispNum(thisVolume, ml);
    if(digitalRead(SW_pins[0])) return;

    servo.start();
    servo.write(shotPos[0]-5);
    delay(500);
    pumpON();
    
    while (!digitalRead(ENC_SW) && !digitalRead(SW_pins[0]));
    timeoutReset();
    pumpOFF();
    delay(500);
    servo.write(0);
    //servo.setTargetDeg(0);
  }  
}
