void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUG("start");
#endif
  // епром
  if (EEPROM.read(1000) != 10) {
    EEPROM.write(1000, 10);
    EEPROM.put(0, thisVolume);
  }
  EEPROM.get(0, thisVolume);

  // тыкаем ленту
  strip.setBrightness(255);
  strip.clear();
  DEBUG("strip init");

  // настройка пинов
#ifdef DRIVER_STBY
  pinMode(DRIVER_STBY, OUTPUT);
#endif
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    pinMode(SW_pins[i], INPUT_PULLUP);
  }
  // настройка серво
  driverSTBY(0);
  servoON();
  servo.attach(SERVO_PIN, HOME_POS);
  delay(1000);
  servo.setTargetDeg(HOME_POS);
  servo.setSpeed(50);
  servo.setAccel(0.6);
  servoOFF();

  // старт дисплея
  disp.clear();
  disp.brightness(7);
  DEBUG("disp init");

  serviceMode();

  // animation
  timerMinim durationTimer(5110); //5110
  timerMinim timer20(20);
  timerMinim timer60(60);
  strip.setBrightness(255);
  while (1) {
    if (durationTimer.isReady()) break;
    if (timer20.isReady()) {
      static byte counter = 0;
      strip.setBrightness(counter);
      for (byte i = 0; i < NUM_SHOTS; i++) {
        leds[i] = mHSV(counter + i * (255 / NUM_SHOTS), 255, 255);
      }
      strip.setBrightness(255 - counter);
      strip.show();
      counter++;
    }
    if (timer60.isReady()) showAnimation(2);
  }
  strip.clear();
  strip.setBrightness(255);



  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  dispMode();       // выводим на дисплей стандартные значения
}
