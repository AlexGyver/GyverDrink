
//╞══════════════════════════════╣ НАСТРОЙКИ ⚒╠══════════════════════════════════╡

#define OLED_SH1106         // выбор контроллера дисплея. OLED_SH1106 для 1.3", OLED_SSD1306 для 0.96"
#define NUM_SHOTS     5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define SWITCH_LEVEL  0     // кнопки 1 → высокий сигнал при замыкании, 0 → низкий
#define STATUS_LED    0     // яркость статус-светодиода. 255 → максимум, 0 → не подключен
#define SERVO_SPEED   10    // скорость серво в процентах
#define SERVO_ACCEL   10    // ускорение серво в процентах
#define BATTERY_LOW   3.3   // минимальное напряжение аккумулятора
//#define DEBUG_UART    0     // !отладка не работает из-за нехватки памяти!

//╞═════════════════════════╣ Настройки, доступные в меню ╠══════════════════════╡

#define TIMEOUT_OFF    5     // таймаут на выключение дисплея и светодиодов в минутах. Если 0 → таймаут отключен
#define KEEP_POWER     10    // интервал пинания повербанка в секундах. 0 → функция отключена
#define INVERSE_SERVO  0     // инвертировать направление вращения серво
#define PARKING_POS    0     // угол серво для парковочной позиции
#define TIME_50ML      5000  // время заполнения 50 мл
#define AUTO_PARKING   1     // парковка в авто режиме: 1 → вкл, 0 → выкл
#define STBY_TIME      10    // таймаут входа в режим ожидания в секундах. В режиме ожидания сохраняется последний объём и приглушается яркость подсветки
#define STBY_LIGHT     20    // яркость подсветки в режиме ожидания. 255 → максимум, 0 → выкл
#define RAINBOW_FLOW   1     // 1 → динамическая подсветка налитых рюмок, 0 → статическая

#define BATTERY_CAL    1.0   // калибровка напряжения аккумулятора
#define MAX_VOLUME     50    // ограничение максимального объёма
#define INVERT_DISPLAY 0     // инверсия цветов дисплея

//╞═════════════════════╡Положения серво над центрами рюмок╞═════════════════════╡

// номер рюмки                1   2   3   4    5    6
//                            ^   ^   ^   ^    ^    ^
#define SHOT_POSITIONS       30, 60, 90, 120, 150, 180

//╞══════════════════════════╡ПИНЫ Arduino Nano╞══════════════════════════╡

#if defined(ARDUINO_AVR_NANO)
#define PUMP_POWER  3
#define SERVO_POWER 4
//#define BATTERY_PIN A7
#define SERVO_PIN   5
#define LED_PIN     6
#define BTN_PIN     7
#define ENC_SW      8
#define ENC_DT      9
#define ENC_CLK     10
#define DISP_SDA    A4
#define DISP_SCK    A5
const byte SW_pins[] = {A0, A1, A2, A3, 11, 12};

//╞══════════════════════════╡ПИНЫ Arduino Micro╞══════════════════════════╡

#elif defined(ARDUINO_AVR_MICRO)
#define PUMP_POWER  4
#define SERVO_POWER 16
//#define BATTERY_PIN A3
#define SERVO_PIN   6
#define LED_PIN     5
#define BTN_PIN     10
#define ENC_SW      9
#define ENC_DT      7
#define ENC_CLK     8
#define DISP_SDA    2
#define DISP_SCK    3
const byte SW_pins[] = {14, 15, 18,  19,  20};
#endif
