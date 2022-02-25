// кнопки-крутилки

void encTick() {
  // enc.tick(); 
  if (enc.isTurn()) {
    volumeChanged = true;
    timeoutReset();
    if (enc.isLeft()) {
      DEBUG("   turn LEFT");
      thisVolume += 5;
      thisVolume = constrain(thisVolume, 5, 1000);
    }
    if (enc.isRight()) {
      DEBUG("   turn RIGHT");
      thisVolume -= 5;
      thisVolume = constrain(thisVolume, 5, 1000);
    }
    dispMode();
  }
  else
    ;//DEBUG("no turn");
}

void btnTick() {
  if (btn.holded()) {
    timeoutReset();
    workMode = !workMode;
    dispMode();
  }
  if (encBtn.holded()) {
    pumpON();
    while (/*!digitalRead(ENC_SW)*/ !analogReadBool (ENC_BUTTON));
    timeoutReset();
    pumpOFF();
  }  
}
