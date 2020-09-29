void serviceRoutine(serviceStates mode);

#ifdef TM1637
GyverTM1637 disp(DISP_CLK, DISP_DIO);
enum { ml = 1, deg };
void printNum(uint16_t num, int8_t mode = 0);

#else
SSD1306AsciiWire disp;

enum MenuPageName { // типы страниц меню
  NO_MENU = -1,
  MAIN_MENU_PAGE,
  SETTINGS_PAGE,
  CALIBRATION_PAGE,
  STATISTICS_PAGE
};

#define MENU_PAGES 4 // количество страниц
MenuPageName menuPage = MAIN_MENU_PAGE; // актуальная страница
byte lastMenuPage = NO_MENU; // последняя отображаемая страница
bool itemSelected = 0;

#ifdef BATTERY_PIN
uint8_t menuItemsNum[MENU_PAGES] = {4, 11, 3, 2}; // количество строк на каждой странице
#else
uint8_t menuItemsNum[MENU_PAGES] = {4, 11, 2, 2}; // количество строк на каждой странице
#endif

#if(MENU_LANG == 1)
const char *MenuPages[MENU_PAGES][13] = {
  { "#####  Меню  #####",
    "",
    " Настройки",
    " Калибровка",
    " Статистика"
  },

  { "###  Настройки  ###",
    "таймаут выкл.",
    "инверсия серво",
    "скорость серво",
    "авто парковка",
    "режим ожидания",
    "яркость лед",
    "динам. подсветка",
    "макс. объeм",
    "поддерж. питания",
    "инверсия цвета",
    "Сброс"
  },

  { "###  Калибровка  ###",
    " Серво",
    " Объ@м",
#ifdef BATTERY_PIN
    " Аккумулятор"
#else
    ""
#endif
  },

  { "###  Статистика  ###",
    " Кол-во рюмок",
    " Объ@м",
    ""
  }
};

#else
const char *MenuPages[MENU_PAGES][13] = {
  { "Menu",
    "",
    "Settings",
    "Calibration",
    "Statistics"
  },

  { "Settings",
    "timeout off",
    "inverse servo",
    "servo speed",
    "auto parking",
    "stby time",
    "stby light",
    "rainbow flow",
    "max volume",
    "keep power",
    "invert display",
    "reset"
  },

  { "Calibration",
    "Servo",
    "Volume",
#ifdef BATTERY_PIN
    "Battery"
#else
    ""
#endif
  },

  { "Statistics",
    "Shots",
    "Volume",
    ""
  }
};

#endif

byte strWidth(const char str[]) {
  byte _width = 0;
  while (*str) {
#if(MENU_LANG == 1)
    _width += disp.charWidth(*str++);
#else
    _width += disp.charSpacing(*str++);
#endif
  }
  return _width;
}

enum text_position {
  Append = -4,
  Left,
  Center,
  Right
};

void clearToEOL(char ch = ' ') {
  byte i = 1 + (disp.displayWidth() - disp.col()) / disp.charWidth(ch);
  while (i--) disp.write(ch);
}

void printStr(const char str[], int8_t x = Append, int8_t y = Append) {
  if (x == Left)    disp.setCol(0);
  if (x == Center)  disp.setCol( (disp.displayWidth() - strWidth(str)) / 2);
  if (x == Right)   disp.setCol(disp.displayWidth() - strWidth(str));
  if (x != Append)  disp.setCol(x);

  if (y != Append)  disp.setRow(y);

  while (*str) disp.write(*str++);
}

void printInt(uint16_t num, int8_t x = Append, int8_t y = Append) {
  char cstr[10];
  itoa(num, cstr, 10);
  printStr(cstr, x, y);
}

void printFloat(float num, uint8_t decimals, int8_t x = Append, int8_t y = Append) {
  char cstr[10];
  dtostrf(num, 4, decimals, cstr);
  printStr(cstr, x, y);
}

enum { ml = 1, deg };

void printNum(uint16_t volume, int8_t postfix = 0) {
  static uint16_t lastVol = 0;

  byte shiftY = 0;
  disp.setFont(FixedNum30x40);

  if (postfix == 1) shiftY = 1;

  if (volume <= 999 && lastVol >= 1000) printStr("    ", Center, 3 - shiftY); // "/" = space
  if (volume <= 99 && lastVol >= 100) {
    printStr(" ", Left, 3 - shiftY);
    printStr(" ", Right, 3 - shiftY);
  }
  if (volume <= 9 && lastVol >= 10) {
    printStr("  ", Left, 3 - shiftY);
    //printStr("  ", Right, 3 - shiftY);
  }
  lastVol = volume;

  if (postfix == 1) { // отображение мл
    if (volume > 99) printInt(volume, Left, 3 - shiftY);
    else if (volume > 9) printInt(volume, (disp.displayWidth() - strWidth("00%")) / 2, 3 - shiftY);
    else printInt(volume, (disp.displayWidth() - strWidth("0%")) / 2 + 16, 3 - shiftY);
    printStr("%"); // "ml"
  }
  else if (postfix == 2) { // отображение угла
    if (volume > 99) printInt(volume, (disp.displayWidth() - strWidth("000*")) / 2, 3 - shiftY);
    else if (volume > 9) printInt(volume, (disp.displayWidth() - strWidth("00*")) / 2, 3 - shiftY);
    else printInt(volume, (disp.displayWidth() - strWidth("0*")) / 2 + 16, 3 - shiftY);
    if (postfix == 2)  printStr("*"); // "°"
  }
  else printInt(volume, Center, 3 - shiftY);

#if(MENU_LANG == 1)
  disp.setFont(Vicler8x16);
  disp.setLetterSpacing(0);
#else
  disp.setFont(ZevvPeep8x16);
#endif
}

void progressBar(uint16_t value, uint16_t maximum = 50) {
  disp.setFont(Pixel);
  static byte currX = 0, targetX = 0;

  if (value == 0) {
    disp.setCursor(0, 7);
    for (int i = 0; i < disp.displayWidth(); i++) disp.write('0');
    currX = 0;
    return;
  }

  targetX = value * (127.0 / maximum);

  if (targetX > currX) {
    for (byte x = currX; x < targetX; x ++) {
      disp.setCursor(x, 7);
      disp.write('1');
    }
    currX = targetX;
  }
  else if (targetX < currX) {
    for (byte x = currX; x > targetX; x--) {
      if (x % 2 == 0) {
        disp.setCursor(x, 7);
        disp.write('0');
      }
    }
    currX = targetX;
  }
}

void displayMode(workModes mode);
void displayMode(workModes mode) {
  disp.setFont(Mode12x26);
  printInt(mode, 1, 0); // выводим иконку режима

#ifdef BATTERY_PIN
  if (!timeoutState) printInt(2, disp.displayWidth() - strWidth("2") - 26);
  else printInt(0, disp.displayWidth() - 52);
#else
  if (!timeoutState) printInt(2, Right);
  else clearToEOL('0');//printInt(mode, disp.displayWidth() - strWidth('2') - 26);
#endif

  printNum(thisVolume, ml);
  if (timeoutState) progressBar(0);
  progressBar(thisVolume, settingsList[max_volume]);
}

void displayMenu() {
  static uint8_t firstItem = 1, selectedItem = 0;

#if(MENU_LANG == 1)
  disp.setFont(Vicler8x16);
  disp.setLetterSpacing(0);
#else
  disp.setFont(ZevvPeep8x16);
#endif

  if (itemSelected) {
    if (menuPage == MAIN_MENU_PAGE) { // выбор елемента на главной странице Меню
      if (menuItem == 1) { // нажали на режим
        workMode = (workModes)!workMode;
        itemSelected = 0;
        showMenu = false;
        disp.clear();
        lastMenuPage = NO_MENU;
        displayMode(workMode);
#if (SAVE_MODE == 1)
        EEPROM.update(eeAddress._workMode, workMode);
#endif
        return;
      }
      else {
        menuPage = (MenuPageName)(menuItem - 1);
        menuItem = 1;
        disp.clear();
      }
    }
    else if (menuPage == SETTINGS_PAGE) {
      settingsMenuHandler(selectedItem);
      if (!timeoutState) {
        itemSelected = 0;
        return; // произошёл вход в режим ожидания
      }
    }
    else if (menuPage == CALIBRATION_PAGE)  {
      disp.clear();
      serviceRoutine((serviceStates)(menuItem - 1));
      lastMenuPage = NO_MENU;
    }
    else if (menuPage == STATISTICS_PAGE) {
      if (menuItem == 1) shots_overall = 0;
      else if (menuItem == 2) volume_overall = 0;
    }
    itemSelected = 0;
  }

  menuItem = constrain(menuItem, 1, menuItemsNum[menuPage]);

  if (menuPage != lastMenuPage) {
    disp.setInvertMode(0);
    printStr(MenuPages[menuPage][0], Center, 0);
    disp.write('\n');
    lastMenuPage = menuPage;
  }
  else disp.setCursor(0, 2);


  if (menuItem > firstItem + 2) firstItem = menuItem - 2; // прокрутка елементов меню
  else if (menuItem < firstItem)  firstItem = menuItem;

  for (byte currItem = firstItem; currItem < (firstItem + 3); currItem++) {// отображаем три строки из страницы меню, начиная с firstitem
    if (currItem == menuItem) { // инвертируем текущую строку
#if(MENU_SELECT == 1)
      disp.setInvertMode(1);
#else
      disp.setInvertMode(0);
      disp.write('>');
#endif
      selectedItem = disp.row();
    }
    else  disp.setInvertMode(0);

    if (menuPage == MAIN_MENU_PAGE && currItem == 1)  {
#if(MENU_LANG == 1)
      if (workMode == ManualMode) MenuPages[menuPage][currItem] = " Авто режим";
      else  MenuPages[menuPage][currItem] = " Ручной режим";
#else
      if (workMode == ManualMode) MenuPages[menuPage][currItem] = "Auto mode";
      else  MenuPages[menuPage][currItem] = " Manual mode";
#endif
    }

    if (menuPage == SETTINGS_PAGE)  {
      printStr(MenuPages[menuPage][currItem]);
      clearToEOL();
      if (currItem < menuItemsNum[menuPage])  printInt(settingsList[currItem - 1], Right);
      disp.write('\n');
    }
    else if (menuPage == STATISTICS_PAGE) {
      printStr(MenuPages[menuPage][currItem]);
      clearToEOL();
      if (currItem == 1)  printInt(shots_overall, Right);
      if (currItem == 2)  {
#if(MENU_LANG == 1)
        if (volume_overall < 100.0) {
          if (volume_overall < 10) printInt(volume_overall, disp.displayWidth() - strWidth("0мл") - 1);
          else printInt(volume_overall, disp.displayWidth() - strWidth("00мл") - 1);
          printStr("мл");
        }
        else {
          printFloat(volume_overall / 1000.0, 2, disp.displayWidth() - strWidth("0.00л") - 1);
          printStr("л");
        }
#else
        if (volume_overall < 100.0) {
          if (volume_overall < 10) printInt(volume_overall, disp.displayWidth() - strWidth("0ml") - 1);
          printInt(volume_overall, disp.displayWidth() - strWidth("00ml") - 1);
          printStr("ml");
        }
        else {
          printFloat(volume_overall / 1000.0, 2, disp.displayWidth() - strWidth("0.00l"));
          printStr("l");
        }
#endif
      }
      disp.write('\n');
    }
    else  {
      printStr(MenuPages[menuPage][currItem]);
      clearToEOL(' ');
      disp.write('\n');
    }
  }
  disp.setInvertMode(0);
}

#endif
