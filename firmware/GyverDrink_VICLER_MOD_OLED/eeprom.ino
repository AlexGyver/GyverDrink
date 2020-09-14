#define EEPROM_KEY 22

void readEEPROM() {
  // чтение последнего налитого объёма
  if (EEPROM.read(1000) != EEPROM_KEY) {
    EEPROM.write(1000, EEPROM_KEY);
    EEPROM.write(eeAddress._thisVolume, INIT_VOLUME);
  }
  else thisVolume = EEPROM.read(eeAddress._thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  if (EEPROM.read(1001) != EEPROM_KEY) {
    EEPROM.write(1001, EEPROM_KEY);
    EEPROM.put(eeAddress._time50ml, TIME_50ML);
  }
  else EEPROM.get(eeAddress._time50ml, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  if (EEPROM.read(1002) != EEPROM_KEY) {
    EEPROM.write(1002, EEPROM_KEY);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      EEPROM.write(eeAddress._shotPos + i, initShotPos[i]);
      shotPos[i] = initShotPos[i];
    }
  }
  else {
    for (byte i = 0; i < NUM_SHOTS; i++)
      shotPos[i] = EEPROM.read(eeAddress._shotPos + i);
  }

  // чтение калибровки аккумулятора
  if (EEPROM.read(1003) != EEPROM_KEY) {
    EEPROM.write(1003, EEPROM_KEY);
    EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);
  }
  else EEPROM.get(eeAddress._battery_cal, battery_cal);

  //==========================================================
  //                    чтение настроек
  //==========================================================
  // чтение значения таймаута
  if (EEPROM.read(1004) != EEPROM_KEY) {
    EEPROM.write(1004, EEPROM_KEY);
    EEPROM.write(eeAddress._timeout_off, TIMEOUT_OFF);
  }
  else settingsList[timeout_off] = EEPROM.read(eeAddress._timeout_off);

  // чтение установки инверсии серво
  if (EEPROM.read(1005) != EEPROM_KEY) {
    EEPROM.write(1005, EEPROM_KEY);
    EEPROM.write(eeAddress._inverse_servo, INVERSE_SERVO);
  }
  else settingsList[inverse_servo] = EEPROM.read(eeAddress._inverse_servo);

  // чтение парковочной позиции
  if (EEPROM.read(1006) != EEPROM_KEY) {
    EEPROM.write(1006, EEPROM_KEY);
    EEPROM.write(eeAddress._parking_pos, PARKING_POS);
  }
  else settingsList[parking_pos] = EEPROM.read(eeAddress._parking_pos);

  // чтение установки автопарковки в авторежиме
  if (EEPROM.read(1007) != EEPROM_KEY) {
    EEPROM.write(1007, EEPROM_KEY);
    EEPROM.write(eeAddress._auto_parking, AUTO_PARKING);
  }
  else settingsList[auto_parking] = EEPROM.read(eeAddress._auto_parking);

  // чтение таймаута режима ожидания
  if (EEPROM.read(1008) != EEPROM_KEY) {
    EEPROM.write(1008, EEPROM_KEY);
    EEPROM.write(eeAddress._stby_time, STBY_TIME);
  }
  else settingsList[stby_time] = EEPROM.read(eeAddress._stby_time);

  // чтение яркости подсветки в режиме ожидания
  if (EEPROM.read(1009) != EEPROM_KEY) {
    EEPROM.write(1009, EEPROM_KEY);
    EEPROM.write(eeAddress._stby_light, STBY_LIGHT);
  }
  else settingsList[stby_light] = EEPROM.read(eeAddress._stby_light);

  // чтение установки динамической подсветки
  if (EEPROM.read(1010) != EEPROM_KEY) {
    EEPROM.write(1010, EEPROM_KEY);
    EEPROM.write(eeAddress._rainbow_flow, RAINBOW_FLOW);
  }
  else settingsList[rainbow_flow] = EEPROM.read(eeAddress._rainbow_flow);

  // чтение максимального объёма
  if (EEPROM.read(1011) != EEPROM_KEY) {
    EEPROM.write(1011, EEPROM_KEY);
    EEPROM.write(eeAddress._max_volume, MAX_VOLUME);
  }
  else settingsList[max_volume] = EEPROM.read(eeAddress._max_volume);

  // чтение статистики
  if (EEPROM.read(1012) != EEPROM_KEY) {
    EEPROM.write(1012, EEPROM_KEY);
    EEPROM.put(eeAddress._shots_overall, 0);
  }
  else EEPROM.get(eeAddress._shots_overall, shots_overall);

  if (EEPROM.read(1013) != EEPROM_KEY) {
    EEPROM.write(1013, EEPROM_KEY);
    EEPROM.put(eeAddress._volume_overall, 0);
  }
  else EEPROM.get(eeAddress._volume_overall, volume_overall);

  // функция пинания повербанка
  if (EEPROM.read(1014) != EEPROM_KEY) {
    EEPROM.write(1014, EEPROM_KEY);
    EEPROM.write(eeAddress._keep_power, KEEP_POWER);
  }
  else settingsList[keep_power] = EEPROM.read(eeAddress._keep_power);

  // инверсия дисплея
  if (EEPROM.read(1015) != EEPROM_KEY) {
    EEPROM.write(1015, EEPROM_KEY);
    EEPROM.write(eeAddress._invert_display, INVERT_DISPLAY);
  }
  else settingsList[invert_display] = EEPROM.read(eeAddress._invert_display);

  // скорость сервопривода
  if (EEPROM.read(1016) != EEPROM_KEY) {
    EEPROM.write(1016, EEPROM_KEY);
    EEPROM.write(eeAddress._servo_speed, SERVO_SPEED);
  }
  else settingsList[servo_speed] = EEPROM.read(eeAddress._servo_speed);

  // режим
  if(EEPROM.read(1017) != EEPROM_KEY){
    EEPROM.write(1017, EEPROM_KEY);
    EEPROM.write(eeAddress._mode, ManualMode);
  }
  else workMode = (workModes)EEPROM.read(eeAddress._mode);
}

void resetEEPROM() {
  EEPROM.update(1000, EEPROM_KEY);
  EEPROM.update(eeAddress._thisVolume, INIT_VOLUME);

  // сброс калибровки времени на 50мл
  EEPROM.update(1001, EEPROM_KEY);
  EEPROM.put(eeAddress._time50ml, TIME_50ML);

  // сброс позиций серво над рюмками
  EEPROM.update(1002, EEPROM_KEY);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  //сброс калибровки аккумулятора
  EEPROM.update(1003, EEPROM_KEY);
  EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);

  // сброс значения таймаута
  EEPROM.update(1004, EEPROM_KEY);
  EEPROM.update(eeAddress._timeout_off, TIMEOUT_OFF);

  // сброс инверсии серво
  EEPROM.update(1005, EEPROM_KEY);
  EEPROM.update(eeAddress._inverse_servo, INVERSE_SERVO);

  // сброс парковочной позиции
  EEPROM.update(1006, EEPROM_KEY);
  EEPROM.update(eeAddress._parking_pos, PARKING_POS);

  // сброс установки автопарковки в авторежиме
  EEPROM.update(1007, EEPROM_KEY);
  EEPROM.update(eeAddress._auto_parking, AUTO_PARKING);

  // сброс таймаута режима ожидания
  EEPROM.update(1008, EEPROM_KEY);
  EEPROM.update(eeAddress._stby_time, STBY_TIME);

  // сброс яркости подсветки в режиме ожидания
  EEPROM.update(1009, EEPROM_KEY);
  EEPROM.update(eeAddress._stby_light, STBY_LIGHT);

  // сброс установки динамической подсветки
  EEPROM.update(1010, EEPROM_KEY);
  EEPROM.update(eeAddress._rainbow_flow, RAINBOW_FLOW);

  // сброс максимального объёма
  EEPROM.update(1011, EEPROM_KEY);
  EEPROM.update(eeAddress._max_volume, MAX_VOLUME);

  // сброс функции поддержания питания от повербанка
  EEPROM.update(1014, EEPROM_KEY);
  EEPROM.update(eeAddress._keep_power, KEEP_POWER);

  // сброс инвертирования дисплея
  EEPROM.update(1015, EEPROM_KEY);
  EEPROM.update(eeAddress._invert_display, INVERT_DISPLAY);

  // сброс скорости сервопривода
  EEPROM.update(1016, EEPROM_KEY);
  EEPROM.update(eeAddress._servo_speed, SERVO_SPEED);

  // сброс режима
  EEPROM.update(1017, EEPROM_KEY);
  EEPROM.update(eeAddress._mode, ManualMode);

  readEEPROM();
}
