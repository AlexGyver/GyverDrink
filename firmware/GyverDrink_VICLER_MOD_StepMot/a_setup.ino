void setup() {
#if (DEBUG_UART == ON)
  Serial.begin(9600);
  DEBUG("start");
#endif
  // епром
  if (EEPROM.read(1000) != 47) {
    EEPROM.write(1000, 47);
    EEPROM.put(0, thisVolume);
  }
  EEPROM.get(0, thisVolume);
  for(byte i = 0; i < NUM_SHOTS; i++) shotVolume[i] = thisVolume;

  if (EEPROM.read(1001) != 47) {
    EEPROM.write(1001, 47);
    EEPROM.put(10, TIME_50ML);
  }
  EEPROM.get(10, time50ml);
  volumeTick = 15.0 * 50.0 / time50ml;

  if (EEPROM.read(1002) != 47) {
    EEPROM.write(1002, 47);
    for (byte i = 0; i < NUM_SHOTS; i++)
      EEPROM.write(100 + i, shotPos[i]);
  }
  for (byte i = 0; i < NUM_SHOTS; i++)
    EEPROM.get(100 + i, shotPos[i]);

  // тыкаем ленту
  strip.clear();
  strip.show();
  strip.setBrightness(255);
  DEBUG("strip init");

  // настройка пинов
  pinMode(PUMP_POWER, OUTPUT);
#ifdef VALVE_PIN
  pinMode(VALVE_PIN, OUTPUT);
#endif
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (SWITCH_LEVEL == 0) pinMode(SW_pins[i], INPUT_PULLUP);
  }

  // старт дисплея
  disp.clear();
  disp.brightness(7);
  DEBUG("disp init");

  // настройка шагового двигателя
  stepper.autoPower(STEPPER_POWERSAFE);
  stepper.invertDir(INVERT_STEPPER);
  stepper.setBacklash(STEPER_BACKLASH);
  stepper.setRPM(STEPPER_SPEED);
  stepper.setMode(ABSOLUTE);

#ifdef STEPPER_ENDSTOP
  if (STEPPER_ENDSTOP_INVERT == 0) pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif

/* - Стартовая анимация. Значение ANIMATION_FPS задаёт количество кадров в секунду (чем больше - тем быстрее анимация)
 *    Всего доступно 8 видов анимации. Выбирается в ANIMATION_NUM от 0 до 7.
 * - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
 *    Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
 * - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
 *    Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
 */

 #define ANIMATION_NUM 7
 #define ANIMATION_FPS 16
 #define RAINBOW_FPS 50
 #define RAINBOW_START_BRIGHTNESS 250
 
  while (rainbowFadeFlow(RAINBOW_START_BRIGHTNESS, 1000 / RAINBOW_FPS) || !parking) {
#ifdef STEPPER_ENDSTOP
    if (!homing() && !parking) {
      stepper.setRPM(10);
      stepper.setAngle(PARKING_POS);
      if (!stepper.update()) {
        parking = true;
        DEBUG("parked!");
      }
    }
#else
    stepper.resetPos(PARKING_POS);
#endif
    showAnimation(ANIMATION_NUM, 1000 / ANIMATION_FPS);
  }
  stepper.setRPM(STEPPER_SPEED);
  stepper.disable();

  strip.clear();
  HeadLED = WHITE;
  strip.show();

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  DEBUG("ready");
}
