enum text_position {
  Append = -4,
  Left,
  Center,
  Right
};

enum pages {
  _MANUAL,
  _AUTO
};

void printStr(const char str[], int8_t x = Append, int8_t y = Append) {
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
  }
}

void printNum(int num, int8_t x = -4, int8_t y = -4) {
  char cstr[4];
  static int last_num = 0;
  if ( (num == 99 && last_num == 100) || (num < 10 && last_num > 9)) {
    //    disp.setFont(CenturyGothic10x16);
    //    printStr("               ", Center, 2);
    //    printStr("               ", Center, 4);
    //    printStr("               ", Center, 6);
    //    disp.setFont(CenturyNum22x34);
  }
  last_num = num;
  itoa(num, cstr, 10);
  printStr(cstr, x, y);
}

void displayMenu(uint8_t item) {
  disp.setInvertMode(0);
  printStr("--------Меню--------\n", 5, 0);
  disp.setInvertMode(item == 1);
  printStr(" Режим               \n");
  disp.setInvertMode(item == 2);
  printStr(" Объ¿м               \n");
  disp.setInvertMode(item == 3);
  printStr(" Настройки           \n");
  disp.setInvertMode(0);
}

void displayPage(bool page) {
  if (page == _MANUAL) {
    disp.setFont(CenturyGothic10x16);
    printStr("Ручной режим", Left, 0);
  }
  else if (page == _AUTO) {
    disp.setFont(CenturyGothic10x16);
    printStr("Авто режим    ", Left, 0);
  }

#ifdef BATTERY_PIN
  disp.setFont(Battery19x9);
  printNum(get_battery_percent(), Right);
#endif

  disp.setFont(CenturyNum22x34);
  printNum(thisVolume, Center, 3);
}
