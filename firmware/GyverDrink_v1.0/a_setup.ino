void setup() {
#if (DEBUG_UART == 1)
  uart.begin();
#endif
  // тыкаем ленту
  strip.setBrightness(130);
  strip.clear();
  strip.show();

  // настройка пинов
  pinMode(PUMP_POWER, 1);
  pinMode(SERVO_POWER, 1);
  for (byte i = 0; i < NUM_SHOTS; i++) {
    pinMode(SW_pins[i], INPUT_PULLUP);
  }

  // старт дисплея
  disp.clear();
  disp.brightness(7);

  // настройка серво
  servoON();
  servo.attach(SERVO_PIN);
  servo.write(0);
  delay(800);
  servo.setTargetDeg(0);
  servo.setSpeed(60);
  servo.setAccel(0.8);  
  servoOFF();

  serviceMode();    // калибровка
  dispMode();       // выводим на дисплей стандартные значения
  timeoutReset();   // сброс таймаута
}
