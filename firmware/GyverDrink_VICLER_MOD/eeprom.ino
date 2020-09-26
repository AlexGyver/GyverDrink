#ifdef TM1637
#define EEPROM_KEY (11 + VERSION * 10)
#else
#define EEPROM_KEY (22 + VERSION * 10)
#endif

// чтение последнего налитого объёма
void readEEPROM() {
  if (EEPROM.read(100) != EEPROM_KEY) {
    EEPROM.write(100, EEPROM_KEY);
    EEPROM.write(eeAddress._thisVolume, INIT_VOLUME);
  }
  else thisVolume = EEPROM.read(eeAddress._thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  if (EEPROM.read(101) != EEPROM_KEY) {
    EEPROM.write(101, EEPROM_KEY);
    EEPROM.put(eeAddress._time50ml, TIME_50ML);
  }
  else EEPROM.get(eeAddress._time50ml, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  if (EEPROM.read(102) != EEPROM_KEY) {
    EEPROM.write(102, EEPROM_KEY);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
      shotPos[i] = initShotPos[i];
    }
  }
  else {
    for (byte i = 0; i < NUM_SHOTS; i++) shotPos[i] = EEPROM.read(eeAddress._shotPos + i);
  }

  // чтение парковочной позиции
  if (EEPROM.read(103) != EEPROM_KEY) {
    EEPROM.write(103, EEPROM_KEY);
    EEPROM.write(eeAddress._parking_pos, PARKING_POS);
  }
  else parking_pos = EEPROM.read(eeAddress._parking_pos);

#if(SAVE_MODE == 1)
  // режим
  if (EEPROM.read(104) != EEPROM_KEY) {
    EEPROM.write(104, EEPROM_KEY);
    EEPROM.write(eeAddress._workMode, (byte)ManualMode);
  }
  else workMode = (workModes)EEPROM.read(eeAddress._workMode);
#endif

#ifdef BATTERY_PIN
  // чтение калибровки аккумулятора
  if (EEPROM.read(105) != EEPROM_KEY) {
    EEPROM.write(105, EEPROM_KEY);
    EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);
  }
  else EEPROM.get(eeAddress._battery_cal, battery_cal);
#endif

#ifdef TM1637
  // чтение текущей анимации
  if (EEPROM.read(106) != EEPROM_KEY) {
    EEPROM.write(106, EEPROM_KEY);
    EEPROM.write(eeAddress._animCount, 8);
  }
  else {
    animCount = EEPROM.read(eeAddress._animCount);
    if (animCount > 7) animCount = 0;
    EEPROM.update(eeAddress._animCount, animCount + 1);
  }
#endif

#ifndef TM1637
  //╞═════════════════════╡ Чтение параметров меню ╞══════════════════════╡

  // чтение значения таймаута
  if (EEPROM.read(107) != EEPROM_KEY) {
    EEPROM.write(107, EEPROM_KEY);
    EEPROM.write(eeAddress._timeout_off, TIMEOUT_OFF);
  }
  else settingsList[timeout_off] = EEPROM.read(eeAddress._timeout_off);

  // чтение таймаута режима ожидания
  if (EEPROM.read(108) != EEPROM_KEY) {
    EEPROM.write(108, EEPROM_KEY);
    EEPROM.write(eeAddress._stby_time, STBY_TIME);
  }
  else settingsList[stby_time] = EEPROM.read(eeAddress._stby_time);

  // функция пинания повербанка
  if (EEPROM.read(109) != EEPROM_KEY) {
    EEPROM.write(109, EEPROM_KEY);
    EEPROM.write(eeAddress._keep_power, KEEP_POWER);
  }
  else settingsList[keep_power] = EEPROM.read(eeAddress._keep_power);

  // чтение установки инверсии серво
  if (EEPROM.read(110) != EEPROM_KEY) {
    EEPROM.write(110, EEPROM_KEY);
    EEPROM.write(eeAddress._inverse_servo, INVERSE_SERVO);
  }
  else settingsList[inverse_servo] = EEPROM.read(eeAddress._inverse_servo);

  // скорость сервопривода
  if (EEPROM.read(111) != EEPROM_KEY) {
    EEPROM.write(111, EEPROM_KEY);
    EEPROM.write(eeAddress._servo_speed, SERVO_SPEED);
  }
  else settingsList[servo_speed] = EEPROM.read(eeAddress._servo_speed);

  // чтение установки автопарковки в авторежиме
  if (EEPROM.read(112) != EEPROM_KEY) {
    EEPROM.write(112, EEPROM_KEY);
    EEPROM.write(eeAddress._auto_parking, AUTO_PARKING);
  }
  else settingsList[auto_parking] = EEPROM.read(eeAddress._auto_parking);

  // чтение максимального объёма
  if (EEPROM.read(113) != EEPROM_KEY) {
    EEPROM.write(113, EEPROM_KEY);
    EEPROM.write(eeAddress._max_volume, MAX_VOLUME);
  }
  else settingsList[max_volume] = EEPROM.read(eeAddress._max_volume);

  // чтение яркости подсветки в режиме ожидания
  if (EEPROM.read(114) != EEPROM_KEY) {
    EEPROM.write(114, EEPROM_KEY);
    EEPROM.write(eeAddress._stby_light, STBY_LIGHT);
  }
  else settingsList[stby_light] = EEPROM.read(eeAddress._stby_light);

  // чтение установки динамической подсветки
  if (EEPROM.read(115) != EEPROM_KEY) {
    EEPROM.write(115, EEPROM_KEY);
    EEPROM.write(eeAddress._rainbow_flow, RAINBOW_FLOW);
  }
  else settingsList[rainbow_flow] = EEPROM.read(eeAddress._rainbow_flow);

  // инверсия дисплея
  if (EEPROM.read(116) != EEPROM_KEY) {
    EEPROM.write(116, EEPROM_KEY);
    EEPROM.write(eeAddress._invert_display, INVERT_DISPLAY);
  }
  else settingsList[invert_display] = EEPROM.read(eeAddress._invert_display);
  disp.invertDisplay((bool)settingsList[invert_display]);
#endif
}

void resetEEPROM() {
  EEPROM.update(100, EEPROM_KEY);
  EEPROM.update(eeAddress._thisVolume, INIT_VOLUME);

  // сброс калибровки времени на 50мл
  EEPROM.update(101, EEPROM_KEY);
  EEPROM.put(eeAddress._time50ml, TIME_50ML);

  // сброс позиций серво над рюмками
  EEPROM.update(102, EEPROM_KEY);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  // сброс парковочной позиции
  EEPROM.update(103, EEPROM_KEY);
  EEPROM.update(eeAddress._parking_pos, PARKING_POS);

  // сброс режима
  EEPROM.update(104, EEPROM_KEY);
  EEPROM.update(eeAddress._workMode, ManualMode);
  workMode = ManualMode;

#ifdef BATTERY_PIN
  //сброс калибровки аккумулятора
  EEPROM.update(105, EEPROM_KEY);
  EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);
#endif

#ifndef TM1637
  // сброс значения таймаута
  EEPROM.update(107, EEPROM_KEY);
  EEPROM.update(eeAddress._timeout_off, TIMEOUT_OFF);

  // сброс таймаута режима ожидания
  EEPROM.update(108, EEPROM_KEY);
  EEPROM.update(eeAddress._stby_time, STBY_TIME);

  // сброс функции поддержания питания от повербанка
  EEPROM.update(109, EEPROM_KEY);
  EEPROM.update(eeAddress._keep_power, KEEP_POWER);

  // сброс инверсии серво
  EEPROM.update(110, EEPROM_KEY);
  EEPROM.update(eeAddress._inverse_servo, INVERSE_SERVO);

  // сброс скорости сервопривода
  EEPROM.update(111, EEPROM_KEY);
  EEPROM.update(eeAddress._servo_speed, SERVO_SPEED);

  // сброс установки автопарковки в авторежиме
  EEPROM.update(112, EEPROM_KEY);
  EEPROM.update(eeAddress._auto_parking, AUTO_PARKING);

  // сброс максимального объёма
  EEPROM.update(113, EEPROM_KEY);
  EEPROM.update(eeAddress._max_volume, MAX_VOLUME);

  // сброс яркости подсветки в режиме ожидания
  EEPROM.update(114, EEPROM_KEY);
  EEPROM.update(eeAddress._stby_light, STBY_LIGHT);

  // сброс установки динамической подсветки
  EEPROM.update(115, EEPROM_KEY);
  EEPROM.update(eeAddress._rainbow_flow, RAINBOW_FLOW);

  // сброс инвертирования дисплея
  EEPROM.update(116, EEPROM_KEY);
  EEPROM.update(eeAddress._invert_display, INVERT_DISPLAY);
#endif

  readEEPROM();
}
