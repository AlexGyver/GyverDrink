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
      constrain(thisVolume, 5, 1000);
    }
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
