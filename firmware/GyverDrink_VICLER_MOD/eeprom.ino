
#define EEPROM_KEY 11

void readEEPROM() {
  // чтение последнего налитого объёма
  if (EEPROM.read(1000) != EEPROM_KEY) {
    EEPROM.write(1000, EEPROM_KEY);
    EEPROM.write(0, thisVolume);
  }
  else thisVolume = EEPROM.read(0);
  thisVolume = min(thisVolume, MAX_VOLUME);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  if (EEPROM.read(1001) != EEPROM_KEY) {
    EEPROM.write(1001, EEPROM_KEY);
    EEPROM.put(2, TIME_50ML);
  }
  else EEPROM.get(2, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  if (EEPROM.read(1002) != EEPROM_KEY) {
    EEPROM.write(1002, EEPROM_KEY);
    for (byte i = 0; i < NUM_SHOTS; i++) {
      EEPROM.update(6 + i, initShotPos[i]);
      shotPos[i] = initShotPos[i];
    }
  }
  else {
    for (byte i = 0; i < NUM_SHOTS; i++)
      shotPos[i] = EEPROM.read(6 + i);
  }

  //чтение домашней позиции
  if (EEPROM.read(1003) != EEPROM_KEY) {
    EEPROM.write(1003, EEPROM_KEY);
    EEPROM.update(13, parking_pos);
  }
  else parking_pos = EEPROM.read(13);

#ifdef BATTERY_PIN
  // чтение калибровки аккумулятора
  if (EEPROM.read(1004) != EEPROM_KEY) {
    EEPROM.write(1004, EEPROM_KEY);
    EEPROM.put(14, 1.0);
  }
  else EEPROM.get(14, battery_cal);
#endif

  // чтение текущей анимации
  if (EEPROM.read(1005) != EEPROM_KEY) {
    EEPROM.write(1005, EEPROM_KEY);
    EEPROM.write(18, 7);
  }
  else {
    animCount = EEPROM.read(18);
    if (animCount > 6) animCount = 0;
    EEPROM.update(18, animCount + 1);
  }

  // режим
  if (EEPROM.read(1006) != EEPROM_KEY) {
    EEPROM.write(1006, EEPROM_KEY);
    EEPROM.write(19, 0);
  }
  else workMode = EEPROM.read(19);
}

void resetEEPROM() {
  // сброс калибровки времени на 50мл
  EEPROM.update(1001, EEPROM_KEY);
  EEPROM.put(2, TIME_50ML);

  // сброс позиций серво над рюмками
  EEPROM.update(1002, EEPROM_KEY);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.update(6 + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  // сброс домашней позиции
  EEPROM.update(1003, EEPROM_KEY);
  EEPROM.update(13, PARKING_POS);

#ifdef BATTERY_PIN
  // сброс калибровки аккумулятора
  EEPROM.update(1004, EEPROM_KEY);
  EEPROM.put(14, 1.0);
#endif

  // сброс режима
  EEPROM.update(1006, EEPROM_KEY);
  EEPROM.update(19, 0);
}
