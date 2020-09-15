
void dispNum(uint16_t num, bool mode = 0);

void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUGln("start");
#endif

  // епром
  readEEPROM();

#ifdef BATTERY_PIN
  float get_battery_voltage();
  float batCheck = 0;
  for (byte i = 0; i < 20; i++) batCheck += get_battery_voltage() / 20;
  DEBUG("Battery voltage: ");
  DEBUG(batCheck);
  DEBUGln("V");
  while (get_battery_voltage() < BATTERY_LOW) {
    disp.brightness(0);
    disp.displayByte(0x39, 0x09, 0x09, 0x0F);
    delay(500);
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    delay(500);
    serviceState = BATTERY;
    if (btn.holded())  serviceMode();
  }
#endif

  // тыкаем ленту
  strip.setBrightness(255);
  strip.clear();
  DEBUGln("strip init");

  // старт дисплея
  disp.clear();
  disp.brightness(7);
  DEBUGln("disp init");

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++)
    if (SWITCH_LEVEL == 0) pinMode(SW_pins[i], INPUT_PULLUP);

  // настройка серво
  servoON();
  servo.setDirection(INVERSE_SERVO);
  servo.attach(SERVO_PIN, parking_pos);
  delay(500);
  servo.setCurrentDeg(parking_pos);
  servo.setSpeed(SERVO_SPEED * 2);
  servo.setAccel(SERVO_ACCEL / 100.0);
  servoOFF();
  DEBUGln("servo init");


  /* - Стартовая анимация. Значение ANIMATION_FPS задаёт количество кадров в секунду (чем больше - тем быстрее анимация)
        Всего доступно 7 видов анимации. Выбирается в ANIMATION_NUM от 0 до 6.
        Если #define ANIMATION_NUM 6 закомментированно, анимация будет меняться при каждом старте устройства
     - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
        Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
     - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
        Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
  */

  //#define ANIMATION_NUM 6
#define ANIMATION_FPS 20

#define RAINBOW_FPS 50
#define RAINBOW_START_BRIGHTNESS 250

  timerMinim nextFrame(1000 / ANIMATION_FPS);
  timerMinim nextColor(1000 / RAINBOW_FPS);
  uint8_t startBrightness = RAINBOW_START_BRIGHTNESS;
  while (startBrightness) {
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + statusLed; i++)
        leds[i] = mHSV(startBrightness + i * (255 / (NUM_SHOTS + statusLed) ), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
    if (nextFrame.isReady()) {
#ifdef ANIMATION_NUM
      showAnimation(ANIMATION_NUM);
#else
      showAnimation(animCount);
#endif
    }
  }
  if (STBY_LIGHT > 0) {
    for (byte i = 0; i < NUM_SHOTS; i++)  leds[i] = mHSV(20, 255, STBY_LIGHT);
    strip.show();
  }
#if (STATUS_LED)
  LED = mHSV(255, 0, STATUS_LED); // white
  strip.show();
#endif


  serviceMode();
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  dispNum(thisVolume);

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
