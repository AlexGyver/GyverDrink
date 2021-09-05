void setup() {
  //Serial.begin(9600);
  // старт дисплея
#ifdef TM1637
  disp.clear();
  disp.brightness(7);
#elif defined OLED
#if (OLED == 0)
  Wire.begin();
  Wire.setClock(WIRE_SPEED * 1000L);
  disp.begin(&Adafruit128x64, 0x3C);
#elif (OLED == 1)
  Wire.begin();
  Wire.setClock(WIRE_SPEED * 1000L);
  disp.begin(&SH1106_128x64, 0x3C);
#elif (OLED == 2)
  disp.begin(&Adafruit128x64, DISP_DC, DISP_CLK, DISP_MOSI);
#endif
#endif

  //disp.displayRemap(1); // переворот дисплея на 180 градусов

  // епром
  readEEPROM();

  // проверка напряжения аккумулятора перед началом работы
#ifdef BATTERY_PIN
  float batCheck = 0;
  for (byte i = 0; i < 20; i++) {
    batCheck += get_battery_voltage() / 20;
    delay(1);
  }
  while (get_battery_voltage() < BATTERY_LOW) {
#ifdef TM1637
    disp.brightness(0);
    disp.displayByte(0x39, 0x09, 0x09, 0x0F);
    delay(500);
    disp.displayByte(0x00, 0x00, 0x00, 0x00);
    delay(500);
#elif defined OLED
    disp.setFont(Battery12x22);
    printInt(get_battery_percent(), Right, 0);
    delay(500);
    disp.clear();
    delay(500);
#endif
    if (btn.holded()) {
      disp.clear();
#ifdef TM1637
      disp.brightness(7);
#endif
      serviceRoutine(BATTERY);
      break;
    }
  }
#endif

  if (parameterList[timeout_off] > 0) POWEROFFtimer.setInterval(parameterList[timeout_off] * 60000L);
  POWEROFFtimer.stop();

  if (parameterList[keep_power] > 0) {
    KEEP_POWERtimer.setInterval(parameterList[keep_power] * 1000L);
    KEEP_POWERtimer.start();
  }

  // тыкаем ленту
  strip.setBrightness(255);
  strip.clear();

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
#ifdef ANALOG_METER
  pinMode(ANALOG_METER_PIN, OUTPUT);
#endif
  for (byte i = 0; i < NUM_SHOTS; i++) {
    if (SWITCH_LEVEL == 0) pinMode(SW_pins[i], INPUT_PULLUP);
  }
#ifdef BATTERY_PIN
  pinMode(BATTERY_PIN, INPUT);
#endif

#if (MOTOR_TYPE == 0)  // настройка серво
  servoON();
  servo.setDirection(parameterList[motor_reverse]);
  servo.attach(SERVO_PIN, parking_pos, SERVO_MIN_US, SERVO_MAX_US);
  delay(500);
  servo.setSpeed(parameterList[motor_speed]);
  servo.setAccel(MOTOR_ACCEL);
  servo.stop();
  servoOFF();
  parking = true;
  Timer2.setPeriod(5000);
#elif (MOTOR_TYPE == 1) // настройка шаговика
#ifdef STEPPER_ENDSTOP
  pinMode(STEPPER_ENDSTOP, INPUT_PULLUP);
#endif
  stepper.setRunMode(FOLLOW_POS);
  stepper.setMaxSpeedDeg(parameterList[motor_speed]);
  stepper.setAccelerationDeg(MOTOR_ACCEL);
  stepper.reverse(parameterList[motor_reverse]);
  stepper.setCurrentDeg(parking_pos);
  stepper.autoPower(MOTOR_AUTO_POWER);
  stepper.disable();
  Timer2.setPeriod(stepper.getMinPeriod() / 2);
#endif

  Timer2.enableISR();

  /* - Стартовая анимация. Значение ANIMATION_FPS задаёт количество кадров в секунду (чем больше - тем быстрее анимация)
        Всего доступно 8 видов анимации. Выбирается в ANIMATION_NUM от 0 до 7.
        Если #define ANIMATION_NUM 6 закомментированно, анимация будет меняться при каждом старте устройства
     - Радуга. Начальная яркость задаётся в RAINBOW_START_BRIGHTNESS ... (максимум 255). С этого значения яркость плавно убавляется до 0.
        Частота изменения цвета зависит от RAINBOW_FPS ... (чем больше значение - тем быстрее смена цвета)
     - Время, за которое пройдёт приветствие (пока светодиоды не погаснут) зависит от RAINBOW_FPS и RAINBOW_START_BRIGHTNESS.
        Время до полного угасания в мс = 1000 * RAINBOW_START_BRIGHTNESS / RAINBOW_FPS
  */

#define RAINBOW_FPS 60
#define RAINBOW_START_BRIGHTNESS 250

#ifdef TM1637
  //#define ANIMATION_NUM 7
#define ANIMATION_FPS 20
  timerMinim nextSym(1000 / ANIMATION_FPS);
#elif defined OLED
  timerMinim nextSym(10);
  disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
  disp.setLetterSpacing(0);
#endif // MENU_LANG
  static byte targetX = (DISP_WIDTH - strWidth(bootscreen)) / 2;
  progressBar(-1);
#elif defined ANALOG_METER
  timerMinim nextSym(1000 / RAINBOW_FPS);
#endif // TM1637


  timerMinim nextColor(1000 / RAINBOW_FPS);

  uint8_t startBrightness = RAINBOW_START_BRIGHTNESS;
  while (startBrightness) {
#if (MOTOR_TYPE == 1) && defined STEPPER_ENDSTOP
    homing();
#endif
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS + statusLed; i++)
        leds[i] = mHSV(startBrightness + i * (255 / (NUM_SHOTS + statusLed) ), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
    if (nextSym.isReady()) {
#ifdef TM1637
#ifdef ANIMATION_NUM
      showAnimation(ANIMATION_NUM);
#else
      showAnimation(animCount);
#endif
#elif defined OLED
      static byte currX = 128;
      if (currX > targetX) {
        disp.setFont(MAIN_FONT);
#if(MENU_LANG == 0)
        disp.setLetterSpacing(0);
#endif
        printStr(bootscreen, currX, 3);
        clearToEOL();
        currX -= 5;
      }
      else if (DISPLAY_VERSION) {
        disp.setFont(MAIN_FONT);
        printFloat(VERSION, 1, Center, 5);
      }

      progressBar(RAINBOW_START_BRIGHTNESS - startBrightness, RAINBOW_START_BRIGHTNESS - 2);
#elif defined ANALOG_METER
      static byte i = 0;
      i += round(255.0 / RAINBOW_START_BRIGHTNESS);
      analogWrite(ANALOG_METER_PIN, i - err_vector[i]);
#endif
    }
  }
  if (parameterList[stby_light] > 0) {
    for (byte i = 0; i < parameterList[stby_light]; i++) {
      for (byte j = 0; j < NUM_SHOTS; j++)  leds[j] = mHSV(parameterList[leds_color], 255, i);
      strip.show();
      delay(10);
    }
  }

#ifdef STATUS_LED
  if (workMode == ManualMode) LED = mHSV(manualModeStatusColor, 255, STATUS_LED);
  else LED = mHSV(autoModeStatusColor, 255, STATUS_LED);
  strip.show();
#endif

#if (MOTOR_TYPE == 1) && defined STEPPER_ENDSTOP
#ifdef STEPPER_ENDSTOP
  while (homing());
#endif
  stepper.setTargetDeg(parking_pos);
#endif

  if (!digitalRead(BTN_PIN) || serviceBoot) { // вход в сервисное меню
#ifdef TM1637
    serviceRoutine(serviceState);
  }
#elif defined OLED
    disp.clear();
    while (!digitalRead(BTN_PIN));  // ждём отпускания
    showMenu = true;
    lastMenuPage = NO_MENU;
    menuPage = SERVICE_PAGE;
    timeoutState = true;
    systemState = WAIT;
    displayMenu();
    POWEROFFtimer.stop();
  }
#endif
  else { // пропустили вход в сервис режим
#ifdef OLED
    disp.clear();
    if (parameterList[timeout_off] == 0) {
      progressBar(-1);
      displayMode(workMode);
      displayVolume();
    }
#elif defined ANALOG_METER
    printNum(thisVolume);
#endif
  }

  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
}

//#if (MOTOR_TYPE == 1)
ISR(TIMER2_A) {
#if (MOTOR_TYPE == 0)
  servo.tick();
#else
  stepper.tick();
#endif
}
//#endif
