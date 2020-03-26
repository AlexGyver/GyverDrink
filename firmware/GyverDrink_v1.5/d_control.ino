// кнопки-крутилки

void encTick() {
  enc.tick();
  if (enc.isTurn()) {
    volumeChanged = true;
    timeoutReset();
    if (enc.isLeft()) thisVolume += 5;
    if (enc.isRight())  thisVolume -= 5;
    thisVolume = constrain(thisVolume, 5, 1000);
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
    pumpON();
    while (!digitalRead(ENC_SW));
    timeoutReset();
    pumpOFF();
  }  
}
