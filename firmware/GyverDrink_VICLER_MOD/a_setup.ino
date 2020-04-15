void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUG("start");
#endif
  // епром
  if (EEPROM.read(1000) != 47) {
    EEPROM.write(1000, 47);
    EEPROM.put(0, thisVolume);
  }
  EEPROM.get(0, thisVolume);

  if (EEPROM.read(1001) != 47) {
    EEPROM.write(1001, 47);
    EEPROM.put(10, TIME_50ML);
  }
  EEPROM.get(10, time50ml);
  volumeTick = 20.0f * 50.0f / time50ml;

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
  DEBUG("strip init");

  // старт дисплея
  disp.clear();
  disp.brightness(7);
  DEBUG("disp init");

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++)  pinMode(SW_pins[i], INPUT_PULLUP);

  // настройка серво
  servoON();
  servo.attach(SERVO_PIN, PARKING_POS);
  delay(500);
  servo.setCurrentDeg(PARKING_POS);
  servo.setSpeed(50);
  servo.setAccel(0.6);
  servo.detach();
  servoOFF();
  if (INVERSE_SERVO) for (byte i = 0; i < NUM_SHOTS / 2; i++) {
      byte temp = shotPos[i];
      shotPos[i] = shotPos[NUM_SHOTS - 1 - i];
      shotPos[NUM_SHOTS - 1 - i] = temp;
    }

  // animation
  timerMinim nextColor(20);
  timerMinim nextFrame(50);
  uint8_t startBrightness = 255;
  while (startBrightness) {
    if (nextColor.isReady()) {
      for (byte i = 0; i < NUM_SHOTS; i++)
        leds[i] = mHSV(startBrightness + i * (255 / NUM_SHOTS), 255, startBrightness);
      startBrightness--;
      strip.show();
    }
    if (nextFrame.isReady()) showAnimation(2);
  }
  strip.clear();


  serviceMode();
  timeoutReset();   // сброс таймаута
  TIMEOUTtimer.start();
  dispMode();       // выводим на дисплей стандартные значения
}
