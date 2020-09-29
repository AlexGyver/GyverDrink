
//╞══════════════════════════════╣ НАСТРОЙКИ ⚒ ╠══════════════════════════════════╡

// выбор контроллера дисплея. OLED_SH1106 для 1.3", OLED_SSD1306 для 0.96", TM1637 для 7 сегментного дисплея
#define TM1637
//#define OLED_SH1106
//#define OLED_SSD1306

#define NUM_SHOTS      5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define STBY_TIME      10    // таймаут входа в режим ожидания в секундах. В режиме ожидания сохраняется последний объём и приглушается яркость подсветки
#define TIMEOUT_OFF    5     // таймаут на выключение дисплея и светодиодов в минутах. Если 0 → таймаут отключен
#define KEEP_POWER     0     // интервал пинания повербанка в секундах. 0 → функция отключена
#define SWITCH_LEVEL   0     // кнопки 1 → высокий сигнал при замыкании, 0 → низкий
#define INVERSE_SERVO  0     // инвертировать направление вращения серво
#define SERVO_SPEED    40    // скорость серво в процентах
#define SERVO_ACCEL    10    // ускорение серво в процентах
#define PARKING_POS    0     // угол серво для парковочной позиции
#define TIME_50ML      5000  // время заполнения 50 мл
#define AUTO_PARKING   1     // парковка в авто режиме: 1 → вкл, 0 → выкл
#define SAVE_MODE      0     // 1 → сохранение последнего активного режима в памяти, 0 → не сохраняется
#define MAX_VOLUME     50    // ограничение максимального объёма
#define STATUS_LED     0     // яркость статус-светодиода. 255 → максимум, 0 → не подключен
#define STBY_LIGHT     20    // яркость подсветки в режиме ожидания. 255 → максимум, 0 → выкл
#define RAINBOW_FLOW   1     // 1 → динамическая подсветка налитых рюмок, 0 → статическая
#define BATTERY_LOW    3.3   // минимальное напряжение аккумулятора
#define BATTERY_CAL    1.0   // калибровка напряжения аккумулятора
#define INVERT_DISPLAY 0     // инверсия цветов OLED дисплея
#define BOOTSCREEN     "Der Наливатор"  // текст на OLED дисплее при старте устройства
#define MENU_LANG      1     // язык меню. 1 → русский, 0 → english
#define MENU_SELECT    1     // 0 → выбор пункта меню стрелкой, 1 → инверсией
#define WIRE_SPEED     800   // частота передачи данных OLED дисплею в kHz (min 100, mid 400, max 900). Понизьте частоту, при проблемах с дисплеем.

//╞═════════════════════╡Положения серво над центрами рюмок╞══════════════════════╡

// номер рюмки                1   2   3   4    5    6
//                            ^   ^   ^   ^    ^    ^
#define SHOT_POSITIONS       30, 60, 90, 120, 150, 180

//╞═════════════════════════════╡ПИНЫ Arduino Nano╞═══════════════════════════════╡

#define PUMP_POWER  3        // управляющий сигнал драйвера помпы (обычно обозначен как IN на драйверах моторов, либо GATE/BASE, если используется транзистор)
#define SERVO_POWER 4        // управляющий сигнал драйвера для питания серводвигателя (обычно обозначен как IN на драйверах моторов, либо GATE/BASE, если используется транзистор)
//#define BATTERY_PIN A7       // + аккумулятора (максимум 5 вольт без делителя). Убрать // если подключен
#define SERVO_PIN   5        // сигнальный провод серводвигателя
#define LED_PIN     6        // сигнал DIO/DI первого светодиода (WS2812b)
#define BTN_PIN     7        // сигнал с основной кнопки для разлива и смены режимов
#define ENC_SW      8        // сигнал центральной кнопки на энкодере для прокачки и настройки различных обьёмов. Обозначен как SW или BTN
#define ENC_DT      9        // сигнал DT или А с энкодера
#define ENC_CLK     10       // сигнал CLK или B с энкодера
#ifdef TM1637
#define DISP_DIO    11       // сигнал DIO на дисплее TM1637
#define DISP_CLK    12       // сигнал CLK на дисплее TM1637
#define SW_PINS     A0, A1, A2, A3, A4, A5    // сигналы с датчиков наличия рюмок на ТМ1637 версии: от первой к последней рюмке
#else
#define DISP_SDA    A4       // сигнал SDA OLED дисплея
#define DISP_SCK    A5       // сигнал SCK OLED дисплея
#define SW_PINS     A0, A1, A2, A3, 11, 12  // сигналы с датчиков наличия рюмок на OLED версии: от первой к последней рюмке
#endif
