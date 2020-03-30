void setup() {
#if (DEBUG_UART == 1)
  uart.begin();
#endif

  // Battery Init
  pinMode(BATTERY_PIN, INPUT_PULLUP);
  for(int i = 0; i <10; i++){
    battVoltage += getVoltage() / 10.0;
    delay(50);
  }
  //battVoltage = getVoltage();
  BatteryFilter.setInitVal(battVoltage);

  // настройка серво
  servo.attach(SERVO_PIN);
  servo.write(0);
  servo.setTargetDeg(0);
  servo.setSpeed(40);
  servo.setAccel(0.5);


  // тыкаем ленту
  strip.setBrightness(200);
  strip.clear();
  strip.show();

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pumpOFF();
  for (byte i = 0; i < NUM_SHOTS; i++) {
    pinMode(SW_pins[i], INPUT_PULLUP);
  }

  // старт дисплея
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_10x20_t_cyrillic);  // крупный
  u8g2.setCursor(0, 40);
  u8g2.print(F("Der Наливатор"));
  u8g2.sendBuffer();
  delay(2000);



  //EEPROM.put(0, thisVolume);
  //EEPROM.put(sizeof(thisVolume), workMode);
  EEPROM.get(0, thisVolume);
  EEPROM.get(sizeof(thisVolume), workMode);

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  dispNum(thisVolume, ml);
  timeoutReset();   // сброс таймаута
}
