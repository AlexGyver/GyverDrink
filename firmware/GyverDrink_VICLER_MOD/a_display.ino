void serviceRoutine(serviceStates mode);

#if (DISPLAY_TYPE == 0)
#define OLED 0
#elif (DISPLAY_TYPE == 1)
#define OLED 1
#elif (DISPLAY_TYPE == 2)
#define OLED 2
#elif (DISPLAY_TYPE == 3)
#define TM1637
#elif (DISPLAY_TYPE == 4)
#define ANALOG_METER
#endif

#ifdef TM1637
GyverTM1637 disp(DISP_CLK, DISP_DIO);
enum { ml = 1, deg };
void printNum(uint16_t num, int8_t mode = 0);

//==================================================================================================

#elif defined ANALOG_METER
byte err_vector[256] = {
  0,  1,  1,  2,  2,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  9,
  9,  9,  10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 14, 14,
  14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 18, 18, 18, 19, 19, 19,
  20, 20, 21, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 26, 27,
  27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 31, 32, 32, 32,
  33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 38, 38, 38,
  39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 43, 43, 44,
  44, 44, 44, 44, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47,
  47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 49,
  49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
  49, 49, 49, 48, 48, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 46,
  46, 46, 45, 45, 44, 44, 44, 43, 43, 43, 43, 42, 42, 42, 41, 41,
  40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 34, 33, 33, 32, 31,
  31, 30, 29, 28, 28, 27, 26, 25, 24, 23, 22, 22, 21, 20, 19, 19,
  18, 17, 16, 15, 14, 13, 12, 11, 10, 9,  7,  5,  4,  3,  1,  0,
};

void printNum(uint16_t num, int8_t mode = 0) {
  static byte lastVal = 255;
  byte value = round(num * 255.0 / parameterList[max_volume]);
  //Serial.println(num);

  if ( (value == 0) && (lastVal > 0) ) {
    for (byte i = lastVal; i > 0; i--) {
      analogWrite(ANALOG_METER_PIN, i - err_vector[i]);
      delay(2);
    }
    lastVal = value;
  }

  if ( (lastVal == 0) && (value > 0)) {
    for (byte i = 0; i < value; i++) {
      analogWrite(ANALOG_METER_PIN, i - err_vector[i]);
      delay(3);
    }
    lastVal = value;
  }
  if ( (lastVal == 255) && (num == thisVolume) ) {
    for (byte i = lastVal; i > value; i--) {
      analogWrite(ANALOG_METER_PIN, i - err_vector[i]);
      delay(3);
    }
    lastVal = value;
  }

  if (value > lastVal)
    for (byte i = lastVal; i < value; i++) analogWrite(ANALOG_METER_PIN, i - err_vector[i]);
  else if (value < lastVal)
    for (byte i = lastVal; i > value; i--) analogWrite(ANALOG_METER_PIN, i - err_vector[i]);

  lastVal = value;
  mode = mode;
}

/* для совместимости */
struct disp {
  void clear() {}
} disp;
enum { ml = 1, deg };


//==================================================================================================

#elif defined OLED

#if (OLED == 2)
SSD1306AsciiSoftSpi disp;
#else
SSD1306AsciiWire disp;
#endif

#if (NUM_FONT == 0)
#define BIG_NUM_FONT FixedNum30x40
#elif (NUM_FONT == 1)
#define BIG_NUM_FONT FixedNum30x40_2
#endif

#if (MENU_LANG == 0)
#define MAIN_FONT Vicler8x16
#else
#define MAIN_FONT ZevvPeep8x16
#endif

#define DISP_WIDTH 128  // ширина дисплея в пикселях

enum MenuPageName { // типы страниц меню
  NO_MENU = -1,
  MAIN_MENU_PAGE,         // основная страница
  SETTINGS_PAGE,          // страница настроек
  STATISTICS_PAGE,        // страница статистики
  SERVICE_PAGE,           // страница сервисного меню
  SERVO_CALIBRATION_PAGE  // страница настройки сервопривода
};

MenuPageName menuPage = MAIN_MENU_PAGE; // актуальная страница
byte lastMenuPage = NO_MENU;            // последняя отображаемая страница. Нужна для предотвращения повторного вывода заголовка одной и той же страницы во время прокрутки.
bool itemSelected = 0;                  // флаг нажатия на пункт меню

uint8_t menuItemsNum[] = {3, 8, 3, 5, 4}; // количество строк на каждой странице без заголовка

#if(MENU_LANG == 0)
const char *MenuPages[][9] = {
  { "#####  Меню  #####",
    "", // зарезервированно для названия режима
    " Настройки",
    " Статистика",
  },

  { "###  Настройки  ###",
    "таймаут выкл.",
    "режим ожидания",
    "яркость лед",
    "цвет лед",
    "динам. подсветка",
    "инверсия дисп.",
    "яркость дисп.",
    "макс. объeм"
  },

  { "### Статистика  ###",
    " Рюмок",
    " Объ@м /сеанс",
    " Объ@м всего"
  },
  //////////////////////////////////////////////////////////////////// сервисное меню
  { "##### Сервис #####",
    " Двигатель",
    " Помпа",
#ifdef BATTERY_PIN
    " Аккумулятор",
#else
    " Поддерж. питания",
#endif
    " Сброс",
    " Выход"
  },

  {
    "###  Двигатель  ###",
    " Калибровка",
    " Инверсия",
    " Скорость",
    " Авто парковка"
  }
};

#else
const char *MenuPages[][9] = {
  { "      Menu      ",
    "",
    " Settings",
    " Stats"
  },

  { "Settings",
    "timeout off",
    "stby time",
    "led bright.",
    "led color",
    "color flow",
    "disp invert",
    "disp contrast",
    "max volume",
  },

  { "Stats",
    " Shots",
    " Session",
    " Total"
  },

  { "Service",
    " Motor",
    " Pump",
#ifdef BATTERY_PIN
    " Battery",
#else
    " Keep power",
#endif
    " Reset",
    " Exit"
  },

  {
    " Servo ",
    " Calibration",
    " Invert",
    " Speed",
    " Auto parking"
  }
};
#endif

byte strWidth(const char str[]) { // расчёт ширины текста в пикселях
  byte _width = 0;
  while (*str) {
#if(MENU_LANG == 0)
    _width += disp.charWidth(*str++);
#else
    _width += disp.charSpacing(*str++);
#endif
  }
  return _width;
}

enum text_position { // выравнивание текста на дисплее
  Append = -4,  // к актуальной позиции
  Left,         // к левому краю
  Center,       // по центру
  Right         // к правому краю
};

void clearToEOL(const char ch = ' ') { // заполнение строки от актуальной позиции до правого края дисплея любым символом. По умолчанию используется пробел для очистки
  byte i = 1 + (DISP_WIDTH - disp.col()) / disp.charWidth(ch);
  while (i--) disp.write(ch);
}

void printStr(const char str[], int8_t x = Append, int8_t y = Append) { // вывод текста с возможностью выравнивания
  switch (x) {
    case Left: disp.setCol(0); break;
    case Center: disp.setCol( (DISP_WIDTH - strWidth(str)) / 2); break;
    case Right: disp.setCol(DISP_WIDTH - strWidth(str)); break;
    default: disp.setCol(x); break;
  }
  //  if (x == Left)    disp.setCol(0);
  //  else if (x == Center)  disp.setCol( (DISP_WIDTH - strWidth(str)) / 2);
  //  else if (x == Right)   disp.setCol(DISP_WIDTH - strWidth(str));
  //  else if (x != Append)  disp.setCol(x);

  if (y != Append)  disp.setRow(y);

  while (*str) disp.write(*str++);
}

void printInt(uint16_t num, int8_t x = Append, int8_t y = Append) { // вывод целых чисел с возможностью выравнивания
  char cstr[5];
  itoa(num, cstr, 10);
  printStr(cstr, x, y);
}

void ftoa(float floatVal, char* floatStr, byte dec) { // преобразование реального числа в массив символов
  byte index = 0;

  float rounding = 0.5;
  for (uint8_t i = 0; i < dec; ++i) rounding /= 10.0;
  floatVal += rounding;

  uint16_t int_part = (uint16_t)floatVal;
  float remainder = floatVal - (float)int_part;
  if (int_part < 10) floatStr[index++] = int_part + '0';
  else {
    floatStr[index++] = int_part / 10 + '0';
    floatStr[index++] = int_part % 10 + '0';
  }

  if (dec > 0) floatStr[index++] = '.';

  while (dec-- > 0)
  {
    remainder *= 10.0;
    uint16_t toPrint = (uint16_t)remainder;
    floatStr[index++] = toPrint + '0';
    remainder -= toPrint;
  }
  floatStr[index++] = '\0';
}

void printFloat(float num, uint8_t decimals, int8_t x = Append, int8_t y = Append) { // вывод рельных чисел с возможностью выравнивания
  char cstr[6];
  ftoa(num, cstr, decimals);
  printStr(cstr, x, y);
}

enum { ml = 1, deg }; // постфикс для вывода чисел большими цифрами. мл для объёма и ° для градусов

void printNum(uint16_t volume, int8_t postfix = 0) { //вывод чисел крупным шрифтом по центру с возможностью вывода дополнительного постфикса (мл или °)
  static uint16_t lastVol = 0; // переменная для сохранения последнего выводимого числа
  disp.setFont(BIG_NUM_FONT);
  static byte shiftY = 0;

  if (postfix == 1) shiftY = 1; // число объёма выводится на одну строку выше, чем градусы серво и напяжение аккумулятора

  // очистка первой цифры если число уменьшилось с 1000 до 999, 100 до 99 или с 10 до 9
  if (volume <= 999 && lastVol >= 1000) printStr("    ", Center, 3 - shiftY);
  else if (volume <= 99 && lastVol >= 100) {
    printStr("   ", Left, 3 - shiftY);
    printStr("  ", Right, 3 - shiftY);
  }
  else if ( (volume <= 9 && lastVol >= 10) || !timeoutState )
    printStr("   ", Left, 3 - shiftY);

  switch (postfix) {
    case 1:
      //    if (volume > 99) { // быстрый вывод трёхзначных чисел
      //      static uint16_t tVolume;
      //      (volume > 200) ? tVolume = volume - 200 : tVolume = volume - 100;
      //      if ( (lastVol < 100) || (lastVol < 200 && volume >= 200) || (lastVol >= 200 && volume < 200) || volume == 109 || volume == 209 || volume == 200) printInt(volume, Left, 3 - shiftY); // вывод трёхзначного чисел 100, 109, 200 и 209
      //      else if (tVolume > 9) printInt(tVolume, (DISP_WIDTH - strWidth("000")) / 2 + 12, 3 - shiftY);
      //      else printInt(tVolume, (DISP_WIDTH - strWidth("00")) / 2 + 28, 3 - shiftY);
      //    }

      if (volume > 99) printInt(volume, Left, 3 - shiftY);
      else if (volume > 9) printInt(volume, (DISP_WIDTH - 90) / 2, 3 - shiftY);
      else printInt(volume, (DISP_WIDTH - 60) / 2 + 16, 3 - shiftY);


#if(NUM_FONT == 0)
      disp.setFont(BigPostfix30x16);
#else
      disp.setFont(BigPostfix30x16_2);
#endif
      printStr("%", Append, 5); // "ml"

      if (!showMenu) displayVolumeSession();
      break;
    case 2:
      if (volume > 99) printInt(volume, (DISP_WIDTH - 90) / 2 - 16, 3 - shiftY);
      else if (volume > 9) printInt(volume, (DISP_WIDTH - 60) / 2, 3 - shiftY);
      else printInt(volume, (DISP_WIDTH - 30) / 2 + 16, 3 - shiftY);
#if(NUM_FONT == 0)
      disp.setFont(BigPostfix30x16);
#else
      disp.setFont(BigPostfix30x16_2);
#endif
      printStr("&", Append, 3); // "°"
      break;

    default: printInt(volume, Center, 3 - shiftY); break; // отображение числа без постфикса
  }
  lastVol = volume;

  disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
  disp.setLetterSpacing(0);
#endif
}

void progressBar(int16_t value, uint8_t maximum = MAX_VOLUME) { // прогресс-бар для визуального отображения объёма
  disp.setFont(ProgBar);                  // активируем шрифт. Состоит из одного деления прогресс-бара
  disp.setLetterSpacing(0);               // отключаем пробелы между символами шрифта, т.к. они уже встроенны в шрифт
  static int16_t currX = 0, targetX = 0;  // актуальная и целевая позиция координаты Х на дисплее
  static bool reseted = 1;

  if (value == -1) { // если параметр -1, сбрасываем бар и рисуем пунктирную линию на всю ширину дисплея
    disp.setCursor(0, 7); // седьмая строка (самая нижняя)
    for (int i = 0; i < DISP_WIDTH; i++) {
      if (i % 2 == 0) disp.write('.'); // в каждом чётном пикселе точка
      else disp.write(' ');            // в нечётном пусто
    }
    currX = 0; // сброс актуальной координаты Х: 0
    reseted = 1;
    return;    // выходим из функции
  }

  targetX = value * (128.0 / maximum); // масштабирование параметра на ширину дисплея. Значение -> номер пикселя от 0 до ширины дисплея (127)

  if (targetX > currX) { // если целевая позиция больше актуальной, заполняем по одному символу до целевого пикселя
    do
    {
      disp.setCursor(currX, 7);
      disp.write('-');
      if (value == thisVolume && systemState != PUMPING ) delay(5 * !reseted);
    }
    while (targetX > ++currX);
    reseted = 0;
  }
  else if (targetX < currX) { // целевая позиция меньше актуальной. Заполняем пунктирной линией до целевого пикселя
    do {
      disp.setCursor(min(currX, DISP_WIDTH - 1), 7);
      if (currX % 2 == 0) {
        disp.write('.');
        if (value == thisVolume) delay(5);
        else if (value == 0) delay(2);
      }
      else disp.write(' ');
    } while (targetX <= --currX);
    currX = targetX;
  }
}

void displayVolumeSession() {
#if DISPLAY_SESSION_VOLUME
  disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
  const byte offsetX = DISP_WIDTH - 33 - 1;
#else
  const byte offsetX = DISP_WIDTH - strWidth("0.00l") + 5;
#endif
  disp.setLetterSpacing(0);
  if (volume_session > 0) {
    printFloat(volume_session / 1000.0, 2, offsetX, 2);
#if(MENU_LANG == 0)
    printStr("л");
#else
    printStr("l");
#endif
  }
#endif
}

void displayMode(workModes mode);
void displayMode(workModes mode) { // вывод иконки режима и иконки аккумулятора
  if (!timeoutState) return;

  disp.setFont(Mode12x26);
  disp.setLetterSpacing(0);

#ifdef BATTERY_PIN
  if (mode == AutoMode) {
    byte x = 79;
    do {
      x -= 2;
      printInt(mode, x, 0); // выводим иконку режима
    } while (x > 1);
  }
  else printInt(mode, 1, 0); // выводим иконку режима

#else
  if (mode == AutoMode) {
    byte x = 129;
    do {
      x -= 2;
      printInt(mode, x, 0); // выводим иконку режима
    } while (x > 1);
  }
  else printInt(mode, 1, 0); // выводим иконку режима
#endif

#ifdef BATTERY_PIN // выводим иконку батареи
  disp.setFont(Battery12x22);
  printInt(get_battery_percent(), Right, 0);
#endif
}

void displayVolume() { // вывод объёма крупным шрифтом с постфиксом "мл" и соответствующего ему значения статус-бара
  displayVolumeSession();
  printNum(thisVolume, ml);
  progressBar(thisVolume, parameterList[max_volume]);
}

void displayMenu() { // вывод страниц меню
  static uint8_t firstItem = 1, selectedRow = 0;

  disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
  disp.setLetterSpacing(0);
#endif

  if (itemSelected) { // нажали на пункт в меню
    switch (menuPage) {
      case MAIN_MENU_PAGE:  // выбор елемента на главной странице Меню
        if (menuItem == 1) { // нажали на режим
          workMode = (workModes)!workMode;
          timeoutReset();
          itemSelected = 0;
          showMenu = false;
          disp.clear();
          lastMenuPage = NO_MENU;
          progressBar(-1);
          displayVolume();
          displayMode(workMode);

#if (SAVE_MODE == 1)
          EEPROM.update(eeAddress._workMode, workMode);
#endif
          return; // выходим из меню
        }
        else {
          menuPage = (MenuPageName)(menuItem - 1);
          menuItem = 1;
        }
        break;
      case SETTINGS_PAGE: // выбор елемента на странице настройки
        editParameter(menuItem - 1, selectedRow);
        if (!timeoutState) { // произошёл вход в режим ожидания
          itemSelected = 0;
          return;
        }
        break;
      case SERVICE_PAGE:  // выбор елемента на странице сервисного меню
        if (menuItem == menuItemsNum[menuPage] - 1) { // предпоследний пункт -> сброс настроек
          resetEEPROM();
          readEEPROM();
        }
        else if (menuItem == 1) menuPage = SERVO_CALIBRATION_PAGE; // выбор первого пункта -> переходим на страницу серво
#ifndef BATTERY_PIN
        else if (menuItem == 3) editParameter(keep_power, selectedRow); // выбор
#endif
        else if (menuItem == menuItemsNum[menuPage]) { // последний пункт ->  выход из сервис режима
          disp.clear();
          showMenu = 0;
          menuItem = 1;
          itemSelected = 0;
          lastMenuPage = NO_MENU;
          menuPage = MAIN_MENU_PAGE;
          progressBar(-1);
          displayMode(workMode);
          displayVolume();
          timeoutState = true;
          return;
        }
        else { // иначе запускаем обработку выбранного этапа калибровки
          serviceRoutine((serviceStates)(menuItem - 1));
          lastMenuPage = NO_MENU;
        }
        break;
      case STATISTICS_PAGE: // выбор елемента на странице статистики
        if (menuItem == 1) shots_session = 0; // сбрасываем количетво рюмок
        else if (menuItem == 2) volume_session = 0; // сбрасываем объём
        else if (menuItem == 3) {
          volume_overall = 0;
          EEPROM.put(eeAddress._volume_overall, 0);
        }
        break;
      case SERVO_CALIBRATION_PAGE:  // выбор елемента на странице настройки сервопривода
        if (menuItem == 1) { // выбрали первый пункт -> начало этапа калибровки серво
          serviceRoutine(POSITION);
          lastMenuPage = NO_MENU;
        }
        else editParameter(menuItem - 2 + 8, selectedRow); // запускаем обработчик изменения параметра. -2 отступ (заголовок и первый пункт страницы). +8 начало параметров для сервисного меню в массиве parameterList
        break;
      default: break;
    }
    itemSelected = 0; // флаг на успешную обработку нажатия на пункт меню.
  }

  if ( (menuItem < 1) || (menuItem > menuItemsNum[menuPage]) ) { // предотвращение повторного вывода станицы меню если крутим енкодер за пределы пунктов меню.
    menuItem = constrain(menuItem, 1, menuItemsNum[menuPage]);
    return;
  }
  menuItem = constrain(menuItem, 1, menuItemsNum[menuPage]);

  if (menuPage != lastMenuPage) {
    disp.setInvertMode(0);
    printStr(MenuPages[menuPage][0], Center, 0);
    disp.write('\n');
    lastMenuPage = menuPage;
    firstItem = 1;
  }
  else disp.setCursor(0, 2);


  if (menuItem > firstItem + 2) firstItem = menuItem - 2; // прокрутка елементов меню
  else if (menuItem < firstItem)  firstItem = menuItem;

  for (byte currItem = firstItem; currItem < (firstItem + 3); currItem++) {// отображаем три строки из страницы меню, начиная с firstitem
    if (currItem == menuItem) { // инвертируем текущую строку
#if(MENU_SELECT == 0)
      disp.setInvertMode(1);
#else
      disp.setInvertMode(0);
      disp.write('>');
#endif
      selectedRow = disp.row();
    }
    else  disp.setInvertMode(0);

    static byte parameter;

    switch (menuPage) {

      case MAIN_MENU_PAGE:
        if (currItem == 1) {
#if(MENU_LANG == 0)
          if (workMode == ManualMode) MenuPages[menuPage][currItem] = " Авто режим";
          else  MenuPages[menuPage][currItem] = " Ручной режим";
#else
          if (workMode == ManualMode) MenuPages[menuPage][currItem] = " Auto mode";
          else  MenuPages[menuPage][currItem] = " Manual mode";
#endif
        }
        printStr(MenuPages[menuPage][currItem]);
        clearToEOL(' ');
        disp.write('\n');
        break;

      case SETTINGS_PAGE:
        printStr(MenuPages[menuPage][currItem]);
        clearToEOL();
        parameter = currItem - 1;
#if(MENU_LANG == 0)
        if ( (parameter == rainbow_flow) || (parameter == invert_display) ) {
          if (parameterList[parameter] == 0) printStr("(", Right);
          else printStr(")", Right);
        }
        else printInt(parameterList[parameter], Right);
#else
        printInt(parameterList[parameter], Right);
#endif
        disp.write('\n');
        break;
      case STATISTICS_PAGE:
        printStr(MenuPages[menuPage][currItem]);
        clearToEOL();
        if (currItem == 1)  printInt(shots_session, Right);
        else  {
          float currValue = (currItem == 2) ? volume_session : volume_overall;
#if(MENU_LANG == 0)
          if (currValue < 10000.0) { // меньше 10л
            printFloat(currValue / 1000.0, 2, DISP_WIDTH - 34 - 1);
            printStr("л");
          }
          else {
            printFloat(currValue / 1000.0, 2, DISP_WIDTH - 42 - 1);
            printStr("л");
          }
#else
          if (currValue < 10000.0) {
            printFloat(currValue / 1000.0, 2, DISP_WIDTH - strWidth("0.00l"));
            printStr("l");
          }
          else {
            printFloat(currValue / 1000.0, 2, DISP_WIDTH - strWidth("00.00l"));
            printStr("l");
          }
#endif
        }
        disp.write('\n');
        break;
      case SERVICE_PAGE:
        printStr(MenuPages[menuPage][currItem]);
        clearToEOL();
#ifndef BATTERY_PIN
        if (currItem == 3) // пункт Поддержание питания
          printInt(parameterList[keep_power], Right); // вывод значения таймера для поддержания питания
#endif
        disp.write('\n');
        break;
      case SERVO_CALIBRATION_PAGE:
        printStr(MenuPages[menuPage][currItem]);
        clearToEOL();
        parameter = currItem - 2 + 8;
#if(MENU_LANG == 0)
        if ( (parameter == motor_reverse) || (parameter == auto_parking) ) {
          if (parameterList[parameter] == 0) printStr("(", Right);
          else printStr(")", Right);
        }
        if ( (parameter == motor_speed) || (parameter == keep_power) )
          printInt(parameterList[parameter], Right);
#else
        if (currItem > 1) printInt(parameterList[parameter], Right);
#endif
        disp.write('\n');
        break;
      default: break;
    }
  }
  disp.setInvertMode(0);
}

#endif /* OLED */
