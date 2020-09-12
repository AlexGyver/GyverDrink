
//═══════════════════════════════╣ НАСТРОЙКИ ⚒╠════════════════════════════╡

#define NUM_SHOTS     5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF   5     // таймаут на выключение дисплея и светодиодов в минутах. Если 0 → таймаут отключен
#define KEEP_POWER    0     // интервал пинания повербанка в секундах. 0 → функция отключена
#define SWITCH_LEVEL  0     // кнопки 1 → высокий сигнал при замыкании, 0 → низкий
#define INVERSE_SERVO 0     // инвертировать направление вращения серво
#define SERVO_SPEED   10    // скорость серво в процентах
#define SERVO_ACCEL   10    // ускорение серво в процентах
#define PARKING_POS   0     // угол серво для парковочной позиции
#define TIME_50ML     5000  // время заполнения 50 мл
#define AUTO_PARKING  1     // парковка в авто режиме: 1 → вкл, 0 → выкл
#define STBY_TIME     10    // таймаут входа в режим ожидания в секундах. В режиме ожидания сохраняется последний объём и приглушается яркость подсветки
#define STBY_LIGHT    20    // яркость подсветки в режиме ожидания. 255 → максимум, 0 → выкл
#define RAINBOW_FLOW  1     // 1 → динамическая подсветка налитых рюмок, 0 → статическая
#define STATUS_LED    0     // яркость статус-светодиода. 255 → максимум, 0 → не подключен
#define MAX_VOLUME    50    // ограничение максимального объёма
#define BATTERY_LOW   3.3   // минимальное напряжение аккумулятора
#define DEBUG_UART    0     // отладка

// положение серво над центрами рюмок
// номер рюмки          1   2   3    4    5    6
#define SHOT_POSITIONS  30, 60, 90, 120, 150, 180

//╞══════════════════════════╡ПИНЫ Arduino Micro╞══════════════════════════╡

#if defined(ARDUINO_AVR_MICRO)
#define PUMP_POWER  4
#define SERVO_POWER 16
//#define BATTERY_PIN A3
#define SERVO_PIN   6
#define LED_PIN     5
#define BTN_PIN     10
#define ENC_SW      9
#define ENC_DT      7
#define ENC_CLK     8
#define DISP_DIO    2
#define DISP_CLK    3
const byte SW_pins[] = {14, 15, 18, 19, 20};

//╞══════════════════════════╡ПИНЫ Arduino Nano╞══════════════════════════╡

#elif defined(ARDUINO_AVR_NANO)
#define PUMP_POWER  3
#define SERVO_POWER 4
//#define BATTERY_PIN A7
#define SERVO_PIN   5
#define LED_PIN     6
#define BTN_PIN     7
#define ENC_SW      8
#define ENC_DT      9
#define ENC_CLK     10
#define DISP_DIO    11
#define DISP_CLK    12
const byte SW_pins[] = {A0, A1, A2, A3, A4, A5};
#endif
