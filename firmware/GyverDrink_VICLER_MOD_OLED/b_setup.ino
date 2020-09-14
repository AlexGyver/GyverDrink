
void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUGln("start");
#endif

  // епром
  readEEPROM();

  // старт дисплея
#if defined OLED_SH1106
  disp.begin(&SH1106_128x64, 0x3C);
#elif defined OLED_SSD1306
  disp.begin(&Adafruit128x64, 0x3C);
#endif
  disp.invertDisplay((bool)settingsList[invert_display]);

  // проверка напряжения аккумулятора перед началом работы
#ifdef BATTERY_PIN
  float batCheck = 0;
  for (byte i = 0; i < 20; i++) {
    batCheck += get_battery_voltage() / 20;
    delay(1);
  }
  DEBUG("Battery voltage: ");
  DEBUG(batCheck);
  DEBUGln("V");
  while (get_battery_voltage() < BATTERY_LOW) {
    disp.setFont(Battery19x9);
    printNum(get_battery_percent(), Right, 0);
    delay(500);
    disp.clear();
    get_battery_voltage();
    delay(500);
    if (btn.holded()) {
      disp.clear();
      showMenu = true;
      serviceRoutine(BATTERY);
      showMenu = false;
      break;
    }
  }
#endif

  if (settingsList[timeout_off]) {
    POWEROFFtimer.setInterval(settingsList[timeout_off] * 60000L);
    POWEROFFtimer.start();
  }
  if (settingsList[keep_power]) {
    KEEP_POWERtimer.setInterval(settingsList[keep_power] * 1000L);
    KEEP_POWERtimer.start();
  }


  // тыкаем ленту
  strip.setBrightness(255);
  strip.clear();
  DEBUGln("strip init");

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++)
    if (SWITCH_LEVEL == 0) pinMode(SW_pins[i], INPUT_PULLUP);
#ifdef BATTERY_PIN
  pinMode(BATTERY_PIN, INPUT);
#endif

  // настройка серво
  servoON();
  servo.setDirection(settingsList[inverse_servo]);
  servo.attach(SERVO_PIN, settingsList[parking_pos]);
  delay(500);
  servo.setCurrentDeg(settingsList[parking_pos]);
  servo.setSpeed(settingsList[servo_speed] * 2);
  servo.setAccel(SERVO_ACCEL / 100.0);
  servoOFF();
  DEBUGln("servo init");

  /*
     - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
        Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
     - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
        Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
  */

#define RAINBOW_FPS 50
#define RAINBOW_START_BRIGHTNESS 250

  timerMinim nextColor(1000 / RAINBOW_FPS);
  timerMinim nextChar(100);
  uint8_t startBrightness = RAINBOW_START_BRIGHTNESS;
  disp.setFont(CenturyGothic10x16);
  disp.setCursor(15, 3);
  while (startBrightness) {
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + statusLed; i++)
        leds[i] = mHSV(startBrightness + i * (255 / (NUM_SHOTS + statusLed) ), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
    if (nextChar.isReady()) {
      static uint8_t index = 0;
      if (bootscreen[index] != '\0') disp.print(bootscreen[index++]);
    }
  }
  disp.clear();
  if (settingsList[stby_light] > 0) {
    for (byte i = 0; i < NUM_SHOTS; i++)  leds[i] = mHSV(20, 255, settingsList[stby_light]);
    strip.show();
  }
#if (STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  strip.show();
#endif

  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  displayMode(workMode);

  DEBUG("- main volume: ");
  DEBUGln(thisVolume);
  DEBUG("- time for 1ml: ");
  DEBUG(time50ml / 50);
  DEBUGln("ms");
  DEBUG("- volume per tick: ");
  DEBUGln(volumeTick);
  DEBUG("- shots quantity: ");
  DEBUGln(NUM_SHOTS);
  DEBUG("- parking position: ");
  DEBUGln(parking_pos);
  DEBUGln("- shot positions:");
  for (byte i = 0; i < NUM_SHOTS; i++) {
    DEBUG(i);
    DEBUG(" -> ");
    DEBUG(shotPos[i]);
    DEBUGln("°");
  }
}
