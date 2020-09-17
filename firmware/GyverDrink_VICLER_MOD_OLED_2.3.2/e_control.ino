
void encTick() {
  enc.tick();
  if (systemState == PUMPING) return;
  if (enc.isTurn()) {
    timeoutReset();
    if (enc.isLeft()) {
      if (showMenu) {
        menuItem++;
      }
      else {
        if (curSelected >= 0) shotVolume[curSelected] += 1;
        else thisVolume += 1;
        volumeChanged = true;
      }
    }
    if (enc.isRight()) {
      if (showMenu) {
        menuItem--;
      }
      else {
        if (curSelected >= 0) shotVolume[curSelected] -= 1;
        else thisVolume -= 1;
        volumeChanged = true;
      }
    }
    if (showMenu) {
      displayMenu();
      return;
    }
    if (curSelected >= 0) shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, settingsList[max_volume]);
    thisVolume = constrain(thisVolume, 1, settingsList[max_volume]);
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
      shots_overall++;
      volume_overall += volumeCount;
      EEPROM.put(eeAddress._shots_overall, shots_overall);
      EEPROM.put(eeAddress._volume_overall, volume_overall);
      systemState = WAIT;                   // режим работы - ждать
      WAITtimer.reset();
      DEBUGln("ABORT");
    }
    if (workMode == ManualMode) systemON = true;         // система активирована
  }

  if (btn.holded()) {
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

    if ( (workMode == ManualMode) && curPumping >= 0) {
      DEBUG("abort fill for shot: ");
      DEBUGln(curPumping);
      curPumping = -1; // снимаем выбор рюмки
      systemState = WAIT;                                         // режим работы - ждать
      WAITtimer.reset();
      pumpOFF();                                                  // помпу выкл
      systemON = true;
      DEBUGln("SystemON");
    }
    if (workMode == AutoMode) {
      DEBUGln("automatic mode");
    }
    else {
      DEBUGln("manual mode");
    }
  }
  if (encBtn.clicked()) {
    timeoutReset();
    if (showMenu) {
      selectItem = 1;
      displayMenu();
    }
    else {
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
    }
  }
  if (encBtn.holding()) {
    if (workMode == AutoMode) return;
    prePump();
  }
}
