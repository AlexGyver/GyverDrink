#if defined TM1637 || defined ANALOG_METER
#define EEPROM_KEY (VERSION * 10)
#elif defined OLED
#define EEPROM_KEY (11 + VERSION * 10)
#endif


void readEEPROM() {

  if (EEPROM.read(100) != EEPROM_KEY) {
    EEPROM.write(100, EEPROM_KEY);
    resetEEPROM();
    firstStartUp = true;
  }

  // чтение последнего налитого объёма
  thisVolume = EEPROM.read(eeAddress._thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  // чтение значения таймера для 50мл
  EEPROM.get(eeAddress._time50ml, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  // чтение позиций серво над рюмками
  for (byte i = 0; i < NUM_SHOTS; i++) shotPos[i] = EEPROM.read(eeAddress._shotPos + i);

  // чтение парковочной позиции
  parking_pos = EEPROM.read(eeAddress._parking_pos);

#if(SAVE_MODE == 1)
  // режим
  workMode = (workModes)EEPROM.read(eeAddress._workMode);
#endif

#ifdef BATTERY_PIN
  // чтение калибровки аккумулятора
  EEPROM.get(eeAddress._battery_cal, battery_cal);
#endif

#ifdef TM1637
  // чтение текущей анимации
  animCount = EEPROM.read(eeAddress._animCount);
  if (animCount > 7) animCount = 0;
  EEPROM.update(eeAddress._animCount, animCount + 1);
#endif

#ifdef OLED
  //╞═════════════════════╡ Чтение параметров меню настроек ╞══════════════════════╡

  // чтение значения таймаута
  parameterList[timeout_off] = EEPROM.read(eeAddress._timeout_off);

  // чтение таймаута режима ожидания
  parameterList[stby_time] = EEPROM.read(eeAddress._stby_time);

  // чтение яркости подсветки в режиме ожидания
  parameterList[stby_light] = EEPROM.read(eeAddress._stby_light);

  // цвет светодиодов
  parameterList[leds_color] = EEPROM.read(eeAddress._leds_color);

  // чтение установки динамической подсветки
  parameterList[rainbow_flow] = EEPROM.read(eeAddress._rainbow_flow);

  // инверсия дисплея
  parameterList[invert_display] = EEPROM.read(eeAddress._invert_display);
  disp.invertDisplay((bool)parameterList[invert_display]);

  // яркость олед дисплея
  parameterList[oled_contrast] = EEPROM.read(eeAddress._oled_contrast);
  disp.setContrast(parameterList[oled_contrast]);

  // чтение максимального объёма
  parameterList[max_volume] = EEPROM.read(eeAddress._max_volume);

  //╞═══════════════════╡ Чтение параметров сервисного меню ╞═════════════════════╡

  // функция пинания повербанка
  parameterList[keep_power] = EEPROM.read(eeAddress._keep_power);

  // чтение установки инверсии серво
  parameterList[inverse_servo] = EEPROM.read(eeAddress._inverse_servo);

  // скорость сервопривода
  parameterList[servo_speed] = EEPROM.read(eeAddress._servo_speed);

  // чтение установки автопарковки в авторежиме
  parameterList[auto_parking] = EEPROM.read(eeAddress._auto_parking);
#endif
}

void resetEEPROM() {

  // флаг сброса памяти
  EEPROM.update(100, EEPROM_KEY);

  // сброс последнего значения объёма
  EEPROM.update(eeAddress._thisVolume, INIT_VOLUME);

  // сброс калибровки времени на 50мл
  EEPROM.put(eeAddress._time50ml, TIME_50ML);

  // сброс позиций серво над рюмками
  for (byte i = 0; i < NUM_SHOTS; i++) {
    EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  // сброс парковочной позиции
  EEPROM.update(eeAddress._parking_pos, PARKING_POS);

  // сброс режима
  EEPROM.update(eeAddress._workMode, ManualMode);
  workMode = ManualMode;

#ifdef BATTERY_PIN
  //сброс калибровки аккумулятора
  EEPROM.put(eeAddress._battery_cal, 1.0);
#endif

#ifdef OLED
  // сброс значения таймаута
  EEPROM.update(eeAddress._timeout_off, TIMEOUT_OFF);

  // сброс таймаута режима ожидания
  EEPROM.update(eeAddress._stby_time, TIMEOUT_STBY);

  // сброс функции поддержания питания от повербанка
  EEPROM.update(eeAddress._keep_power, KEEP_POWER);

  // сброс инверсии серво
  EEPROM.update(eeAddress._inverse_servo, INVERSE_SERVO);

  // сброс скорости сервопривода
  EEPROM.update(eeAddress._servo_speed, SERVO_SPEED);

  // сброс установки автопарковки в авторежиме
  EEPROM.update(eeAddress._auto_parking, AUTO_PARKING);

  // сброс максимального объёма
  EEPROM.update(eeAddress._max_volume, MAX_VOLUME);

  // сброс яркости подсветки в режиме ожидания
  EEPROM.update(eeAddress._stby_light, STBY_LIGHT);

  // сброс установки динамической подсветки
  EEPROM.update(eeAddress._rainbow_flow, RAINBOW_FLOW);

  // сброс инвертирования дисплея
  EEPROM.update(eeAddress._invert_display, INVERT_DISPLAY);

  // сброс цвета светодиодов
  EEPROM.update(eeAddress._leds_color, ledsColor);

  //сброс яркости олед дисплея
  EEPROM.update(eeAddress._oled_contrast, OLED_CONTRAST);

#endif
}
