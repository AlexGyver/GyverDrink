
void encTick() {
  enc.tick();
  if (systemState == PUMPING) return;
  if (enc.isTurn()) {
    if (enc.isLeftH()) {
      if (curSelected >= 0 && shotVolume[curSelected] < 246) shotVolume[curSelected] += 10;
      else if (thisVolume < 246) thisVolume += 10;
      volumeChanged = true;
    }
    if (enc.isRightH()) {
      if (curSelected >= 0 && shotVolume[curSelected] > 10) shotVolume[curSelected] -= 10;
      else if (thisVolume > 10) thisVolume -= 10;
      volumeChanged = true;
    }
    if (enc.isLeft()) {
      if (showMenu) menuItem++;
      else {
        if (curSelected >= 0) shotVolume[curSelected] += 1;
        else if (thisVolume < 255) thisVolume += 1;
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

#ifdef OLED
    if (showMenu) {
      displayMenu();
      timeoutReset();
      return;
    }
#endif

    if (curSelected >= 0) {
      shotVolume[(byte)curSelected] = constrain(shotVolume[(byte)curSelected], 1, parameterList[max_volume]);
      printNum(shotVolume[curSelected], ml);
#ifdef OLED
      progressBar(shotVolume[curSelected], parameterList[max_volume]);
#endif
    }
    else {
      thisVolume = constrain(thisVolume, 1, parameterList[max_volume]);
      if (timeoutState) printNum(thisVolume, ml);
#ifdef OLED
      progressBar(thisVolume, parameterList[max_volume]);
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
#ifdef OLED
      shots_overall++;
      volume_overall += volumeCount;
      //      EEPROM.put(eeAddress._shots_overall, shots_overall);
      //      EEPROM.put(eeAddress._volume_overall, volume_overall);
#endif
      systemState = WAIT; // режим работы - ждать
      WAITtimer.reset();
    }
    if ((workMode == ManualMode) && !showMenu) systemON = true; // система активирована
#ifdef OLED
    if (showMenu) {
      if (menuPage != MAIN_MENU_PAGE && menuPage != SERVICE_PAGE) {
        if (menuPage == SERVO_CALIBRATION_PAGE) {
          menuItem = 1;
          menuPage = SERVICE_PAGE;
        }
        else {
          menuItem = menuPage + 1;
          menuPage = MAIN_MENU_PAGE;
        }
        displayMenu();
      }
      else {
        disp.clear();
        showMenu = 0;
        menuItem = 1;
        lastMenuPage = NO_MENU;
        menuPage = MAIN_MENU_PAGE;
        progressBar(-1);
        displayMode(workMode);
        displayVolume();
        timeoutState = true;
      }
    }
#endif
  }

  // смена режима/вход в меню
  if (btn.holded()) {
    if (systemState == PUMPING) return;
#ifdef TM1637
    workMode = (workModes)!workMode;
    if (thisVolume < 100) {                                // объём меньше 100
      if (thisVolume < 10)
        disp.scrollByte(workMode * _dash, 0, digToHEX(thisVolume % 10), workMode * _dash, 50);  // число меньше 10 - второй индикатор пуст
      else
        disp.scrollByte(workMode * _dash, digToHEX(thisVolume / 10), digToHEX(thisVolume % 10), workMode * _dash, 50);     // иначе статичное изменение
    }
    else disp.scrollByte(digToHEX(thisVolume / 100), digToHEX((thisVolume % 100) / 10), digToHEX(thisVolume % 10), workMode * _dash, 50); // объём больше 99
#elif defined OLED
    showMenu = !showMenu;
    if (showMenu) displayMenu();
    else {
      disp.clear();
      menuItem = 1;
      lastMenuPage = NO_MENU;
      menuPage = MAIN_MENU_PAGE;
      progressBar(-1);
      displayMode(workMode);
      displayVolume();
    }
#elif defined ANALOG_METER
    workMode = (workModes)!workMode;
#endif /* ANALOG_METER */
    timeoutReset();
  }

  // промывка
  if (encBtn.holding() && (shotCount == 1) ) {
    if (workMode == AutoMode) return;
#ifdef OLED
    printNum(volumeCount, ml);
    progressBar(0);
#endif
    prePump();
  }

  // выбор елемента меню
#ifdef OLED
  if (encBtn.pressed() && showMenu) {
    itemSelected = 1;
    displayMenu();
  }
#endif

  // выбор рюмки
  if (encBtn.clicked()) {
    //#ifdef OLED
    //    if (showMenu) {
    //      itemSelected = 1;
    //      displayMenu();
    //      return;
    //    }
    //#endif

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
      else if (shotStates[i] == EMPTY)  strip.setLED(i, mHSV(parameterList[leds_color], 255, 255));
      //else  strip.setLED(i, mHSV(20, 255, parameterList[stby_light]));
    }
    LEDchanged = true;

    if (curSelected >= 0) {
      printNum(shotVolume[curSelected], ml);
#ifdef OLED
      progressBar(shotVolume[curSelected], parameterList[max_volume]);
#endif
    }
    else  {
      printNum(thisVolume, ml);
#ifdef OLED
      progressBar(thisVolume, parameterList[max_volume]);
#endif
    }

    timeoutReset();
  }

#ifdef OLED
  if (btn.clicks(2)) {
    workMode = (workModes)!workMode;
    displayMode(workMode);
  }
#endif

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
