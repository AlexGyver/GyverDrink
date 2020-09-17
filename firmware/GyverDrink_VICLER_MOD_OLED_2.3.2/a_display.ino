
enum text_position {
  Append = -4,
  Left,
  Center,
  Right
};

enum MenuPageName { // типы страниц меню
  MENU_PAGE = 0,
  MODE_PAGE,
  SETTINGS_PAGE,
  CALIBRATION_PAGE,
  STATISTICS_PAGE
};

#define MENU_PAGES  5 // количество страниц
MenuPageName menuPage = MENU_PAGE; // актуальная страница
uint8_t menuItem = 0;
bool showMenu = 0, selectItem = 0;

const char* MenuPages[MENU_PAGES][13] = {
  {
    "---------Меню---------",
    " Режим",
    " Настройки",
    " Калибровка",
    " Статистика",
    " Выход"
  },

  {
    "--------Режим--------",
    " Ручной",
    " Автоматический",
    ""
  },

  {
    "------Настройки------",
    " timeout off",
    " inverse servo",
    " servo speed",
    " auto parking",
    " stby time",
    " stby light",
    " rainbow flow",
    " max volume",
    " keep power",
    " invert display",
    " Сброс",
    " Назад"
  },

  {
    "-----Калибровка-----",
    " Объ¿м",
    " Серво",
#ifdef BATTERY_PIN
    " Аккумулятор",
#endif
    " Назад"
  },

  {
    "------Статистика------",
    " Кол-во рюмок",
    " Объ¿м",
    " Назад"
  }
};

#ifdef BATTERY_PIN
uint8_t menuItemsNum[MENU_PAGES] = { 5, 2, 12, 4, 3 };  // количество строк на каждой странице
#else
uint8_t menuItemsNum[MENU_PAGES] = { 5, 2, 12, 3, 3 };  // количество строк на каждой странице
#endif

void displayMode(workModes mode);
void serviceRoutine(serviceModes mode);

void printStr(const char str[], int8_t x = Append, int8_t y = Append, uint16_t _delay = 0) {
  if (x == Append) disp.setCol(disp.col());
  else if (x == Left)   disp.setCol(0);
  else if (x == Center) disp.setCol((128 - disp.strWidth(str)) / 2);
  else if (x == Right)  disp.setCol(128 - disp.strWidth(str));
  else disp.setCol(x);

  if (y == Append) disp.setRow(disp.row());
  else disp.setRow(y);

  while (1) {
    if (*str == '\0') break;
    disp.print(*str++);
    if (_delay) delay(_delay);
  }
}

void printNum(uint16_t num, int8_t x = -4, int8_t y = -4) {
  char cstr[10];
  itoa(num, cstr, 10);
  printStr(cstr, x, y);
}

void printFloat(float num, uint8_t decimals, int8_t x = -4, int8_t y = -4) {
  char cstr[10];
  dtostrf(num, 4, decimals, cstr);
  printStr(cstr, x, y);
}

void displayMenu() {
  static uint8_t firstItem = 1, selectedRow = 1;

  if (selectItem) {
    if (menuPage == MENU_PAGE) { // выбор елемента на главной странице Меню
      if (menuItem == menuItemsNum[menuPage]) {  // нажали на "Выход"
        menuItem = 1;
        selectItem = 0;
        showMenu = false;
        disp.clear();
        displayMode(workMode);
        return;
      }
      else {
        menuPage = (MenuPageName)menuItem;
        menuItem = 1;
        disp.clear();
      }
    }
    else if (menuPage == MODE_PAGE) { // выбор елемента на странице "Режим"
      workMode = (workModes)(menuItem - 1);
      menuItem = 1;
      selectItem = 0;
      showMenu = false;
      menuPage = MENU_PAGE;
      disp.clear();
      displayMode(workMode);
#if (SAVE_MODE == 1)
      EEPROM.update(eeAddress._mode, workMode);
#endif
      return;
    }
    else if (menuPage == SETTINGS_PAGE) {
      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
        menuItem = 2;
        menuPage = MENU_PAGE;
        disp.clear();
      }
      else {
        settingsMenuHandler(selectedRow);
      }
    }
    else if (menuPage == CALIBRATION_PAGE) {
      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
        menuItem = 3;
        menuPage = MENU_PAGE;
        disp.clear();
      }
      else {
        disp.clear();
        serviceRoutine((serviceModes)menuItem);
      }
    }
    else if (menuPage == STATISTICS_PAGE) {
      if (menuItem == menuItemsNum[menuPage]) { // нажали "Назад"
        menuItem = 4;
        menuPage = MENU_PAGE;
        disp.clear();
      }
      else if (menuItem == 1) {
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
  else if (menuItem < firstItem) firstItem = menuItem;

  for (uint8_t i = firstItem; i < (firstItem + 3); i++) {
    if (menuItem % 4 == i % 4) {
      disp.setInvertMode(1);
      selectedRow = disp.row();
    }
    else disp.setInvertMode(0);

    if (menuPage == SETTINGS_PAGE) {
      if (i < menuItemsNum[menuPage] - 1) disp.setFont(Callibri15);
      else disp.setFont(CenturyGothic10x16);
      printStr(MenuPages[menuPage][i]);
      disp.setFont(CenturyGothic10x16);
      printStr("                  ");
      if (i < menuItemsNum[menuPage] - 1) printNum(settingsList[i - 1], Right);
      printStr("\n");
    }
    else if (menuPage == STATISTICS_PAGE) {
      disp.setFont(CenturyGothic10x16);
      printStr(MenuPages[menuPage][i]);
      printStr("                  ");
      disp.setFont(Callibri15);
      if (i == 1) printNum(shots_overall, Right);
      if (i == 2) printNum(volume_overall, Right);
      printStr("\n");
    }
    else {
      printStr(MenuPages[menuPage][i]);
      printStr("                  \n");
    }
  }
  disp.setInvertMode(0);
}

void printVolume(uint16_t volume, int8_t x = -2) {
  static uint16_t lastVol = 0;
  disp.setFont(MonoNum22x32);

  if (volume <= 999 && lastVol >= 1000) printStr("    ", x, 3);
  if (volume <= 99 && lastVol >= 100) printStr("   ", x, 3);
  if (volume <= 9 && lastVol >= 10)  printStr("  ", x, 3);
  lastVol = volume;

  printNum(volume, x, 3);
}

void displayMode(workModes mode) {
  disp.setFont(CenturyGothic10x16);
  if (mode == ManualMode)     printStr(" Ручной режим", Left, 0);
  else if (mode == AutoMode)  printStr(" Авто режим    ", Left, 0);
  //else if (mode == ComboMode) printStr(" Комбо режим    ", Left, 0);

  printVolume(thisVolume);
}
