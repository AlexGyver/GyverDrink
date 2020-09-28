
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
      timeoutReset();
      return;
    }
#endif

    if (curSelected >= 0) {
      shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, settingsList[max_volume]);
      printNum(shotVolume[curSelected], ml);
#ifndef TM1637
      progressBar(shotVolume[curSelected], settingsList[max_volume]);
#endif
    }
    else {
      thisVolume = constrain(thisVolume, 1, settingsList[max_volume]);
      printNum(thisVolume, ml);
#ifndef TM1637
      progressBar(thisVolume, settingsList[max_volume]);
#endif
      for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;
    }

    timeoutReset();
  }
}

// активация/остановка налива
void btnTick() {
  if (btn.pressed()) { // нажатие на основную кнопку
    timeoutReset(); // таймаут сброшен

    if (systemState == PUMPING) {
      pumpOFF();                      // помпа выкл
      shotStates[curPumping] = READY; // налитая рюмка, статус: готов
      curPumping = -1;                // снимаем выбор рюмки
#ifndef TM1637
      shots_overall++;
      volume_overall += volumeCount;
      //      EEPROM.put(eeAddress._shots_overall, shots_overall);
      //      EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
      systemState = WAIT; // режим работы - ждать
      WAITtimer.reset();
    }
    if (workMode == ManualMode && !showMenu) systemON = true; // система активирована
#ifndef TM1637
    if (showMenu) {
      disp.clear();
      if (menuPage != MAIN_MENU_PAGE) {
        menuItem = menuPage + 1;
        menuPage = MAIN_MENU_PAGE;
        displayMenu();
      }
      else {
        showMenu = 0;
        menuItem = 0;
        lastMenuPage = NO_MENU;
        menuPage = MAIN_MENU_PAGE;
        displayMode(workMode);
      }
    }
#endif
  }

  // смена режима/вход в меню
  if (btn.holded()) {
    if (systemState == PUMPING) return;
#ifdef TM1637
    workMode = (workModes)!workMode;
    disp.scrollByte(64 * workMode, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), 64 * workMode, 50);
#else
    showMenu = !showMenu;
    disp.clear();
    if (showMenu) displayMenu();
    else {
      menuItem = 0;
      lastMenuPage = NO_MENU;
      menuPage = MAIN_MENU_PAGE;
      displayMode(workMode);
    }
#endif
    timeoutReset();
  }

  // промывка
  if (encBtn.holding()) {
    if (workMode == AutoMode) return;
#ifndef TM1637
    printNum(0, ml);
    progressBar(0);
#endif
    prePump();
  }

  // выбор рюмки
  if (encBtn.pressed()) {
#ifndef TM1637
    if (showMenu) {
      itemSelected = 1;
      displayMenu();
      return;
    }
#endif

    if (shotCount < 2) return;

    for (int8_t i = selectShot + 1; i <= NUM_SHOTS; i++) {
      if (i == NUM_SHOTS) {
        selectShot = -1;
        break;
      }
      else if (shotStates[(byte)i] == EMPTY) {
        selectShot = i;
        break;
      }
    }

    curSelected = selectShot;

    for (byte i = 0; i < NUM_SHOTS; i++) {
      if (i == curSelected) strip.setLED(curSelected, mRGB(255, 255, 255)); // white
      else if (shotStates[i] == EMPTY)  strip.setLED(i, mRGB(255, 48, 0));  // orange
      else  strip.setLED(i, mHSV(20, 255, settingsList[stby_light]));
    }
    LEDchanged = true;

    if (curSelected >= 0) {
      printNum(shotVolume[curSelected], ml);
#ifndef TM1637
      progressBar(shotVolume[curSelected], settingsList[max_volume]);
#endif
    }
    else  {
      printNum(thisVolume, ml);
#ifndef TM1637
      progressBar(thisVolume, settingsList[max_volume]);
#endif
    }

    timeoutReset();
  }

#ifdef TM1637
  // сброс настроек
  if (btn.holdedFor(5)) {
    if (systemState == PUMPING) return;
    byte resetText[] = {_r, _E, _S, _E, _t};
    disp.runningString(resetText, sizeof(resetText), 150);
    while (!digitalRead(BTN_PIN));
    resetEEPROM();
    readEEPROM();
    printNum(thisVolume, ml);
    servoON();
    servo.attach(SERVO_PIN, parking_pos);
    delay(500);
    servoOFF();
  }
#endif
}
