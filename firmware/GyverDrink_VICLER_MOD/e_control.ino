
void encTick() {
  enc.tick();
  if (systemState == PUMPING) return;
  if (enc.isTurn()) {
    if (enc.isLeft()) {
      if (showMenu) menuItem++;
      else {
        if (curSelected >= 0) shotVolume[curSelected] += 1;
        else thisVolume += 1;
        volumeChanged = true;
      }
    }
    if (enc.isRight()) {
      if (showMenu) menuItem--;
      else {
        if (curSelected >= 0) shotVolume[curSelected] -= 1;
        else thisVolume -= 1;
        volumeChanged = true;
      }
    }
#ifndef TM1637
    if (showMenu) {
      displayMenu();
      return;
    }
#endif
    if (curSelected >= 0) shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, settingsList[max_volume]);
    thisVolume = constrain(thisVolume, 1, settingsList[max_volume]);
    timeoutReset();
    if (curSelected >= 0) {
      printVolume(shotVolume[curSelected]);
      DEBUG("shot ");
      DEBUG(curSelected);
      DEBUG(" volume: ");
      DEBUGln(shotVolume[curSelected]);
    }
    else {
      printVolume(thisVolume);
      for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;
      DEBUG("main volume: ");
      DEBUGln(thisVolume);
    }
  }
}

void btnTick() {
  if (btn.clicked()) {                      // клик!
    timeoutReset();                         // таймаут сброшен
    DEBUGln("Button pressed");

    if (systemState == PUMPING) {
      pumpOFF();                            // помпа выкл
      shotStates[curPumping] = READY;       // налитая рюмка, статус: готов
      curPumping = -1;                      // снимаем выбор рюмки
#ifndef TM1637
      shots_overall++;
      volume_overall += volumeCount;
      EEPROM.put(eeAddress._shots_overall, shots_overall);
      EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
      systemState = WAIT;                   // режим работы - ждать
      WAITtimer.reset();
      DEBUGln("ABORT");
    }
    if (workMode == ManualMode) systemON = true;         // система активирована
  }

  if (btn.holded()) {
    if (systemState == PUMPING) return;

#ifdef TM1637
    if (workMode == AutoMode) disp.scrollByte(64, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 64, 50);
    else  disp.scrollByte(0, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 0, 50);
#else
    showMenu = !showMenu;
    timeoutReset();
    if (showMenu) {
      disp.clear();
      displayMenu();
    }
    else {
      disp.clear();
      displayMode(workMode);
      printVolume(thisVolume);
      menuItem = 0;
      menuPage = MENU_PAGE;
    }
#endif

    timeoutReset();

    //#ifndef TM1637
    //    if (showMenu) {
    //      showMenu = false;
    //      disp.clear();
    //      displayMode(workMode);
    //      printVolume(thisVolume);
    //      menuItem = 0;
    //      menuPage = MENU_PAGE;
    //    }
    //    displayMode(workMode);
    //#endif
  }
  if (encBtn.clicked()) {
    timeoutReset();
#ifndef TM1637
    if (showMenu) {
      selectItem = 1;
      displayMenu();
    }
    else {
#endif
      selectShot++;
      if (selectShot == NUM_SHOTS)  selectShot = -1;
      curSelected = selectShot;
      if (curSelected >= 0) {
        DEBUG("shot selected: ");
        DEBUGln(curSelected);
      }
      else {
        DEBUGln("no shots selected");
      }

      for (byte i = 0; i < NUM_SHOTS; i++) {
        if (i == curSelected) strip.setLED(curSelected, mCOLOR(WHITE));
        else if (shotStates[i] == EMPTY) strip.setLED(i, mCOLOR(ORANGE));
        else strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
      }
      if (curSelected >= 0) printVolume(shotVolume[curSelected]);
      else printVolume(thisVolume);
      LEDchanged = true;
#ifndef TM1637
    }
#endif
  }
  if (encBtn.holding()) {
    if (workMode == AutoMode) return;
    prePump();
  }
#ifdef TM1637
  if (btn.holdedFor(5)) {
    if (systemState == PUMPING) return;
    byte resetText[] = {_r, _E, _S, _E, _t};
    disp.runningString(resetText, sizeof(resetText), 150);
    while (!digitalRead(BTN_PIN));
    resetEEPROM();
    readEEPROM();
    printVolume(thisVolume);
    servoON();
    servo.attach(SERVO_PIN, parking_pos);
    delay(500);
    servo.setCurrentDeg(parking_pos);
    servoOFF();
  }
#endif
}
