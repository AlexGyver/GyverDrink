void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUGln("start");
#endif

  // епром
  readEEPROM();

  if (settingsList[timeout_off] > 0)
    POWEROFFtimer.setInterval(settingsList[timeout_off] * 60000L);

  // тыкаем ленту
  strip.setBrightness(255);
  strip.clear();
  DEBUGln("strip init");

  // старт дисплея
  disp.begin(&SH1106_128x64, 0x3C);             // SH1106
  //oled.begin(&Adafruit128x64, I2C_ADDRESS);  // SSD1306

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
  servo.setSpeed(15);
  servo.setAccel(0.2);
  servo.detach();
  servoOFF();
  DEBUGln("servo init");


  /*
     - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
        Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
     - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
        Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
  */

#define RAINBOW_FPS 50
#define RAINBOW_START_BRIGHTNESS 50

  timerMinim nextColor(1000 / RAINBOW_FPS);
  uint8_t startBrightness = RAINBOW_START_BRIGHTNESS;
  while (startBrightness) {
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + statusLed; i++)
        leds[i] = mHSV(startBrightness + i * (255 / (NUM_SHOTS + statusLed) ), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
  }
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
  DEBUGln(PARKING_POS);
  DEBUGln("- shot positions:");
  for (byte i = 0; i < NUM_SHOTS; i++) {
    DEBUG(i);
    DEBUG(" -> ");
    DEBUG(shotPos[i]);
    DEBUGln("°");
  }
}
