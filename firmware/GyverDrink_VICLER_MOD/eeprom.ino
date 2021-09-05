#if defined TM1637 || defined ANALOG_METER
#define EEPROM_KEY (byte)(VERSION * 10)
#elif defined OLED
#define EEPROM_KEY (byte)(VERSION * 10 + 1)
#endif


void readEEPROM() {

  if (EEPROM.read(100) != EEPROM_KEY) {
    EEPROM.write(100, EEPROM_KEY);
    resetEEPROM();
    serviceBoot = true;
  }

#ifdef TM1637
  thisVolume = min(EEPROM.read(eeAddress._thisVolume), MAX_VOLUME);  // чтение последнего налитого объёма
#elif defined OLED
  thisVolume = min(EEPROM.read(eeAddress._thisVolume), EEPROM.read(eeAddress._max_volume));  // чтение последнего налитого объёма
#endif
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  EEPROM.get(eeAddress._time50ml, time50ml);  // чтение значения таймера для 50мл
  volumeTick = 20.0 * 50.0 / time50ml;

  for (byte i = 0; i < NUM_SHOTS; i++) shotPos[i] = EEPROM.read(eeAddress._shotPos + i);  // чтение позиций серво над рюмками

  parking_pos = EEPROM.read(eeAddress._parking_pos);  // чтение парковочной позиции

#if(SAVE_MODE == 1)
  workMode = (workModes)EEPROM.read(eeAddress._workMode); // режим
#endif

#ifdef BATTERY_PIN
  EEPROM.get(eeAddress._battery_cal, battery_cal);  // чтение калибровки аккумулятора
#endif

#ifdef TM1637
  animCount = EEPROM.read(eeAddress._animCount);  // чтение текущей анимации
  if (animCount > 7) animCount = 0;
  EEPROM.update(eeAddress._animCount, animCount + 1);
#endif

#ifdef OLED
  //╞═════════════════════╡ Чтение параметров меню настроек ╞══════════════════════╡

  parameterList[timeout_off] = EEPROM.read(eeAddress._timeout_off); // чтение значения таймаута

  parameterList[stby_time] = EEPROM.read(eeAddress._stby_time); // чтение таймаута режима ожидания

  parameterList[stby_light] = EEPROM.read(eeAddress._stby_light); // чтение яркости подсветки в режиме ожидания

  parameterList[leds_color] = EEPROM.read(eeAddress._leds_color); // цвет светодиодов

  parameterList[rainbow_flow] = EEPROM.read(eeAddress._rainbow_flow); // чтение установки динамической подсветки

  parameterList[invert_display] = EEPROM.read(eeAddress._invert_display); // инверсия дисплея
  disp.invertDisplay((bool)parameterList[invert_display]);

  parameterList[oled_contrast] = EEPROM.read(eeAddress._oled_contrast); // яркость олед дисплея
  disp.setContrast(parameterList[oled_contrast]);

  parameterList[max_volume] = EEPROM.read(eeAddress._max_volume); // чтение максимального объёма

  //╞═══════════════════╡ Чтение параметров сервисного меню ╞═════════════════════╡

  parameterList[keep_power] = EEPROM.read(eeAddress._keep_power); // функция пинания повербанка

  parameterList[motor_reverse] = EEPROM.read(eeAddress._motor_reverse); // чтение установки инверсии серво

  parameterList[motor_speed] = EEPROM.read(eeAddress._motor_speed); // скорость сервопривода

  parameterList[auto_parking] = EEPROM.read(eeAddress._auto_parking); // чтение установки автопарковки в авторежиме

  //╞═══════════════════════════╡ Чтение статистики ╞═════════════════════════════╡

  EEPROM.get(eeAddress._volume_overall, volume_overall); // общий объём после сброса
#endif
}

void resetEEPROM() {
  EEPROM.update(100, EEPROM_KEY); // флаг сброса памяти

  EEPROM.update(eeAddress._thisVolume, INIT_VOLUME);  // сброс последнего значения объёма

  EEPROM.put(eeAddress._time50ml, TIME_50ML); // сброс калибровки времени на 50мл

  for (byte i = 0; i < NUM_SHOTS; i++) {  // сброс позиций серво над рюмками
    EEPROM.update(eeAddress._shotPos + i, initShotPos[i]);
    shotPos[i] = initShotPos[i];
  }

  EEPROM.update(eeAddress._parking_pos, PARKING_POS); // сброс парковочной позиции

  EEPROM.update(eeAddress._workMode, ManualMode); // сброс режима
  workMode = ManualMode;

#ifdef BATTERY_PIN
  EEPROM.put(eeAddress._battery_cal, BATTERY_CAL);  //сброс калибровки аккумулятора
#endif

#ifdef OLED
  EEPROM.update(eeAddress._timeout_off, TIMEOUT_OFF); // сброс значения таймаута

  EEPROM.update(eeAddress._stby_time, TIMEOUT_STBY);  // сброс таймаута режима ожидания

  EEPROM.update(eeAddress._keep_power, KEEP_POWER); // сброс функции поддержания питания от повербанка

  EEPROM.update(eeAddress._motor_reverse, MOTOR_REVERSE); // сброс инверсии серво

  EEPROM.update(eeAddress._motor_speed, MOTOR_SPEED); // сброс скорости сервопривода

  EEPROM.update(eeAddress._auto_parking, AUTO_PARKING); // сброс установки автопарковки в авторежиме

  EEPROM.update(eeAddress._max_volume, MAX_VOLUME); // сброс максимального объёма

  EEPROM.update(eeAddress._stby_light, STBY_LIGHT); // сброс яркости подсветки в режиме ожидания

  EEPROM.update(eeAddress._rainbow_flow, RAINBOW_FLOW); // сброс установки динамической подсветки

  EEPROM.update(eeAddress._invert_display, INVERT_DISPLAY); // сброс инвертирования дисплея

  EEPROM.update(eeAddress._leds_color, ledsColor);  // сброс цвета светодиодов

  EEPROM.update(eeAddress._oled_contrast, OLED_CONTRAST); // сброс яркости олед дисплея
#endif
}
