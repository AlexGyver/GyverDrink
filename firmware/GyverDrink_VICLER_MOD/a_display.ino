void serviceRoutine(serviceStates mode);

#ifdef TM1637
GyverTM1637 disp(DISP_CLK, DISP_DIO);
enum { ml = 1, deg };
void printNum(uint16_t num, int8_t mode = 0);
#else

SSD1306AsciiAvrI2c disp;

enum MenuPageName
{ // типы страниц меню
  MAIN_MENU_PAGE = 0,
  SETTINGS_PAGE,
  CALIBRATION_PAGE,
  STATISTICS_PAGE
};

#define MENU_PAGES 4 // количество страниц
MenuPageName menuPage = MAIN_MENU_PAGE; // актуальная страница
bool selectItem = 0;

#ifdef BATTERY_PIN
uint8_t menuItemsNum[MENU_PAGES] = {4, 11, 3, 2}; // количество строк на каждой странице
#else
uint8_t menuItemsNum[MENU_PAGES] = {4, 11, 2, 2}; // количество строк на каждой странице
#endif

const char *MenuPages[MENU_PAGES][13] = {
  { "---------Меню---------",
    "",
    " Настройки",
    " Калибровка",
    " Статистика"
  },

  { "------Настройки------",
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
    " Сброс"
  },

  { "-----Калибровка-----",
    " Серво",
    " Объ¿м",
#ifdef BATTERY_PIN
    " Аккумулятор"
#else
    ""
#endif
  },

  { "------Статистика------",
    " Кол-во рюмок",
    " Объ¿м",
    ""
  }
};

enum text_position {
  Append = -4,
  Left,
  Center,
  Right
};

void printStr(const char str[], int8_t x = Append, int8_t y = Append) {
  if (x == Append)  disp.setCol(disp.col());
  else if (x == Left) disp.setCol(0);
  else if (x == Center) disp.setCol((128 - disp.strWidth(str)) / 2);
  else if (x == Right)  disp.setCol(128 - disp.strWidth(str));
  else  disp.setCol(x);

  if (y == Append)  disp.setRow(disp.row());
  else  disp.setRow(y);

  while (1)
  {
    if (*str == '\0') break;
    disp.print(*str++);
  }
}

void printInt(uint16_t num, int8_t x = Append, int8_t y = Append) {
  char cstr[10];
  itoa(num, cstr, 10);
  printStr(cstr, x, y);
}

void printFloat(float num, uint8_t decimals, int8_t x = Append, int8_t y = Append) {
  char cstr[10];
  dtostrf(num, 4, decimals, cstr);

  disp.setFont(MonoNum30x40);
  printStr(cstr, x, y);
}

enum { ml = 1, deg };

void printNum(uint16_t volume, int8_t postfix = 0) {
  static uint16_t lastVol = 0;
  
  byte shiftY = 0;
  disp.setFont(MonoNum30x40);

  if(postfix == 1) shiftY = 1;

  if (volume <= 999 && lastVol >= 1000) printStr("////", Center, 3 - shiftY); // "/" = space
  if (volume <= 99 && lastVol >= 100)   printStr("////", Left, 3 - shiftY);
  if (volume <= 9 && lastVol >= 10)     printStr("////", Center, 3 - shiftY);
  if (volume >= 100 && lastVol <= 99)   printStr("/", Right, 3 - shiftY);
  lastVol = volume;

  if(postfix == 1 && volume > 99) printInt(volume, Left, 3 - shiftY);
  else printInt(volume, Center, 3 - shiftY);
  
  if (postfix == 1) printStr("-"); // "ml"
  else if (postfix == 2)  printStr(","); // "°"
}

void displayMode(workModes mode);
void displayMode(workModes mode) {
  //disp.setFont(CenturyGothic10x16);
  disp.setFont(Mode213x28);
  printInt(mode, 0, 0); // выводим иконку режима

  printNum(thisVolume, ml);
}

void displayMenu() {
  static uint8_t firstItem = 1, selectedItem = 0;

  if (selectItem) {
    if (menuPage == MAIN_MENU_PAGE) { // выбор елемента на главной странице Меню
      //      if (menuItem == menuItemsNum[menuPage]) { // нажали на "Выход"
      //        menuItem = 1;
      //        selectItem = 0;
      //        showMenu = false;
      //        disp.clear();
      //        displayMode(workMode);
      //        return;
      //      }
      //      else if (menuItem == 1) { // нажали на режим
      if (menuItem == 1) { // нажали на режим
        workMode = (workModes)!workMode;
        selectItem = 0;
        showMenu = false;
        disp.clear();
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
      //      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
      //        menuItem = 2;
      //        menuPage = MAIN_MENU_PAGE;
      //        disp.clear();
      //      }
      //      else settingsMenuHandler(selectedItem);
      settingsMenuHandler(selectedItem);
    }
    else if (menuPage == CALIBRATION_PAGE)  {
      //      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
      //        menuItem = 3;
      //        menuPage = MAIN_MENU_PAGE;
      //        disp.clear();
      //      }
      //      else  {
      disp.clear();
      serviceRoutine((serviceStates)(menuItem - 1));
      //      }
    }
    else if (menuPage == STATISTICS_PAGE) {
      //      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
      //        menuItem = 4;
      //        menuPage = MAIN_MENU_PAGE;
      //        disp.clear();
      //      }
      //      else if (menuItem == 1) {
      if (menuItem == 1) {
        EEPROM.put(eeAddress._shots_overall, 0);
        readEEPROM();
      }
      else if (menuItem == 2) {
        EEPROM.put(eeAddress._volume_overall, 0);
        readEEPROM();
      }
    }
    selectItem = 0;
  }

  menuItem = constrain(menuItem, 1, menuItemsNum[menuPage]);

  disp.setFont(CenturyGothic10x16);
  disp.setInvertMode(0);
  printStr(MenuPages[menuPage][0], 0, 0);
  printStr("\n");

  if (menuItem > firstItem + 2) firstItem = menuItem - 2;
  else if (menuItem < firstItem)  firstItem = menuItem;

  for (byte currItem = firstItem; currItem < (firstItem + 3); currItem++) {// отображаем три строки из страницы меню, начиная с firstitem
    if (menuItem % 4 == currItem % 4) { // инвертируем текущую строку
      disp.setInvertMode(1);
      selectedItem = disp.row();
    }
    else  disp.setInvertMode(0);

    if (menuPage == MAIN_MENU_PAGE && currItem == 1)  {
      if (workMode == ManualMode) MenuPages[menuPage][currItem] = " Авто режим";
      else  MenuPages[menuPage][currItem] = " Ручной режим";
    }

    if (menuPage == SETTINGS_PAGE)  {
//      if (currItem < menuItemsNum[menuPage]) disp.setFont(ZevvPeep8x16);
//      else  disp.setFont(CenturyGothic10x16);
      printStr(MenuPages[menuPage][currItem]);
//      disp.setFont(CenturyGothic10x16);
      printStr("                  ");
      if (currItem < menuItemsNum[menuPage])  printInt(settingsList[currItem - 1], Right);
      printStr("\n");
    }
    else if (menuPage == STATISTICS_PAGE) {
//      disp.setFont(CenturyGothic10x16);
      printStr(MenuPages[menuPage][currItem]);
      printStr("                  ");
//      disp.setFont(ZevvPeep8x16);
      if (currItem == 1)  printInt(shots_overall, Right);
      if (currItem == 2)  printInt(volume_overall, Right);
      printStr("\n");
    }
    else  {
      printStr(MenuPages[menuPage][currItem]);
      printStr("                  \n");
    }
  }
  disp.setInvertMode(0);
}

#endif
