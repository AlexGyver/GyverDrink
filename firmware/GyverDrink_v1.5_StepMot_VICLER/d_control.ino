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
    int8_t pumpingShot = -1;
    for(byte i = 0; i < NUM_SHOTS; i++){
      if(shotStates[i] == EMPTY){
        stepper.enable();
        stepper.setAngle(shotPos[i]);
        pumpingShot = i;
        Serial.println(i);
      }
    }
    if(pumpingShot == -1) return;
    while(stepper.update());
    delay(500);
    pumpON();
    while (!digitalRead(SW_pins[pumpingShot]) && !digitalRead(ENC_SW));
    pumpOFF();
    delay(300);
    stepper.setAngle(PARKING_POS);
    stepper.disable();
    timeoutReset();
  }  
}
