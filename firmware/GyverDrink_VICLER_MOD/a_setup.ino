void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUGln("start");
#endif

  //EEPROM.write(1001, 0);  //сброс значения TIME_50ML из памяти
  //EEPROM.write(1002, 0);  //сброс позиций углов для серво

  // епром
  if (EEPROM.read(1000) != 47) {
    EEPROM.write(1000, 47);
    EEPROM.put(0, thisVolume);
  }
  EEPROM.get(0, thisVolume);
  for (byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  if (EEPROM.read(1001) != 47) {
    EEPROM.write(1001, 47);
    EEPROM.put(10, TIME_50ML);
  }
  EEPROM.get(10, time50ml);
  volumeTick = 15.0f * 50.0f / time50ml;

  if (EEPROM.read(1002) != 47) {
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)
      EEPROM.write(100 + i, shotPos[i]);
  }
  for (byte i = 0; i < NUM_SHOTS; i++)
    EEPROM.get(100 + i, shotPos[i]);


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
  servo.attach(SERVO_PIN, PARKING_POS);
  delay(500);
  servo.setCurrentDeg(PARKING_POS);
  servo.setSpeed(15);
  servo.setAccel(0.2);
  servo.detach();
  servoOFF();
  DEBUGln("servo init");


  /* - Стартовая анимация. Значение ANIMATION_FPS задаёт количество кадров в секунду (чем больше - тем быстрее анимация)
        Всего доступно 7 видов анимации. Выбирается в ANIMATION_NUM от 0 до 6.
     - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
        Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
     - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
        Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
  */
#define ANIMATION_NUM 7
#define ANIMATION_FPS 20
#define RAINBOW_FPS 50
#define RAINBOW_START_BRIGHTNESS 250

  timerMinim nextFrame(1000 / ANIMATION_FPS);
  timerMinim nextColor(1000 / RAINBOW_FPS);
  uint8_t startBrightness = RAINBOW_START_BRIGHTNESS;
  while (startBrightness) {
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + STATUS_LED; i++)
        leds[i] = mHSV(startBrightness + i * (255 / (NUM_SHOTS + STATUS_LED) ), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
    if (nextFrame.isReady()) showAnimation(ANIMATION_NUM);
  }
  if (STBY_LIGHT > 0) {
    for (byte i = 0; i < NUM_SHOTS; i++)  leds[i] = mHSV(20, 255, STBY_LIGHT);
    strip.show();
  }
#if (STATUS_LED == 1)
  LED = WHITE;
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
  DEBUGln(PARKING_POS);
  DEBUGln("- shot positions:");
  for (byte i = 0; i < NUM_SHOTS; i++) {
    DEBUG(i);
    DEBUG(" -> ");
    DEBUG(shotPos[i]);
    DEBUGln("°");
  }
}
