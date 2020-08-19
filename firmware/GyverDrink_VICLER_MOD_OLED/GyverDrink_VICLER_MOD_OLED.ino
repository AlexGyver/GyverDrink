//GyverDrink VICLER_MOD_OLED//
#define VERSION 1.4
//19.08.2020
/*
  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" на основе версии 1.5 by AlexGyver с OLED дисплеем и расширенным функционалом
  Исходники на GitHub: https://github.com/VICLER/GyverDrink
   -  OLED дисплей на контроллере SSD1306 (0.96") или SH1106(1.3")(по умолчанию) изменение типа дисплея в файле c_setup в секции // старт дисплея. Дисплей подключается контактами SCK и SDA к пинам А5, А4 Arduino Nano
   -  возможность установить парковочную позицию PARKING_POS
   -  в автоматическом режиме краник остаётся над последней налитой рюмкой (в ручном возвращается в домашнее положение). Отключается в AUTO_PARKING 0
   -  инверсия серво
   -  прокачка над любой рюмкой. Прокачка проводится только в ручном режиме и только при наличии рюмки.
        Возврат крана в домашнее положение и сброс счётчика объёма жидкости происходит после снятия рюмки
   -  шаг изменения объёма 1мл
   -  вывод налитого объёма в реальном времени. Так же и во время прокачки (сброс после снятия рюмки)
   -  сохранение последнего выбранного объёма в постоянной памяти
   -  возможность отменить налив в любое время нажатием на кнопку
   -  плавный цветовой переход во время налива (от ORANGE до AQUA)
   -  динамическая подсветка налитых рюмок. Отключается в RAINBOW_FLOW  0
   -  возможность включения постоянной подсветки при отсутствии рюмок STBY_LIGHT
   -  дополнительный светодиод в башне. При движении мотора горит оранжевым, при остановке - белым.
        После налития всех рюмок "дышит" бирюзовым, во время простоя - белым. Подключается к светодиоду последней рюмки, активируется в STATUS_LED
   -  возможность настроить объём для каждой рюмки отдельно:
        При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
        Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)
   -  индикация заряда аккумулятора и слежение за напряжением.
        При напряжении, меньшем чем BATTERY_LOW, устройство перестаёт реагировать на внешние воздействия и на дисплее мигает иконка батареи
   -  возможность изменять настройки прошивки прямо в меню:
      - вход в меню зажатием основной кнопки. Выход либо выбором соответствующего пункта, либо так же зажатием основной кнопки
      - timeout_off: таймаут на выключение дисплея и светодиодов в минутах. Если 0 -> таймаут отключен и светодиоды горят всегда
      - inverse_servo: инвертирсия направление вращения серво. 0 -> не инвертированно, 1-> инвертированно
      - parking_pos: угол для домашней позиции в градусах
      - auto_parking: автоматическая парковка в авторежиме
      - stby_time: таймаут режима ожидания в секундах. По истечении этого времени, приглушатся светодиоды и сохранится последнее значение объёма на дисплее
      - stby_light: яркость подсветки в режиме ожидания. 255 -> максимум, 0 -> выкл
      - rainbow_flow: динамическая подсветка налитых рюмок, 0 -> статическая(цвет не переливается)
      - max_volume: ограничение максимального объёма, отображаемого на дисплее
      - keep_power: интервал пинания повербанка в секундах. 0 -> функция отключена
      - сброс всех настроек до значений, прописанных в этом файле за исключением статистики
      - инверсия цветов дисплея
   -  калибровка объёма за единицу времени, калибровка углов сервопривода для рюмок и калибровка напряжения аккумулятора в меню. Напряжение подстраивается энкодером до измеренного
   -  ведение статистики. Количество налитых рюмок и общий объём. При нажатии на кнопку энкодера, соответствующее значение сбрасывается

*/

/*
  Скетч к проекту "Наливатор by AlexGyver"
  - Страница проекта (схемы, описания): https://alexgyver.ru/GyverDrink/
  - Исходники на GitHub: https://github.com/AlexGyver/GyverDrink/
  Проблемы с загрузкой? Читай гайд для новичков: https://alexgyver.ru/arduino-first/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://www.youtube.com/c/alexgyvershow
  https://github.com/AlexGyver
  https://AlexGyver.ru/
  alex@alexgyver.ru
*/

//=============================================================================================
//                                      НАСТРОЙКИ
//=============================================================================================

#define BOARD         NANO  // выбор контроллера NANO или MICRO
#define NUM_SHOTS     5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define SWITCH_LEVEL  0     // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define STATUS_LED    0     // яркость статус-светодиода. 255 -> максимум, 0 -> не подключен.
#define BATTERY_LOW   3.3   // минимальное напряжение аккумулятора
#define DEBUG_UART    0     // отладка

//=============================================================================================
//                            Настройки, изменяемые из меню
//=============================================================================================

#define TIMEOUT_OFF   5     // таймаут на выключение дисплея и светодиодов в минутах. Если 0 -> таймаут отключен
#define KEEP_POWER    0     // интервал пинания повербанка в секундах. 0 -> функция отключена
#define INVERSE_SERVO 0     // инвертировать направление вращения серво
#define PARKING_POS   0     // угол для домашней позиции
#define TIME_50ML     5000  // время заполнения 50 мл
#define AUTO_PARKING  1     // парковка в авто режиме: 1 -> вкл, 0 -> выкл
#define STBY_TIME     10    // таймаут режима ожидания в секундах
#define STBY_LIGHT    15    // яркость подсветки в режиме ожидания. 255 -> максимум, 0 -> выкл
#define RAINBOW_FLOW  1     // 1 -> динамическая подсветка налитых рюмок, 0 -> статическая

#define BATTERY_CAL   1.0   // калибровка напряжения аккумулятора. 
#define BATTERY_LOW   3.3   // минимальное напряжение аккумулятора
#define MAX_VOLUME    200   // ограничение максимального объёма
#define INVERT_DISPLAY 0    // инверсия цветов дисплея

//=============================================================================================
//                           положение серво над центрами рюмок
//=============================================================================================

// номер рюмки           1   2   3   4    5    6
//                       ^   ^   ^   ^    ^    ^
#define SHOT_POSITIONS  30, 60, 90, 120, 150, 180
#if (BOARD == NANO)//    |   |   |   |    |    |
#define SENSOR_PINS     A0, A1, A2,  A3,  11,  12
#elif (BOARD == MICRO)// |   |   |   |    |    
#define SENSOR_PINS     14, 15, 18,  19,  20
#endif

//=============================================================================================
//                                   ПИНЫ Arduino Micro
//=============================================================================================

#if defined(ARDUINO_AVR_MICRO)
#define PUMP_POWER  4
#define SERVO_POWER 16
#define BATTERY_PIN A3
#define SERVO_PIN   6
#define LED_PIN     5
#define BTN_PIN     10
#define ENC_SW      9
#define ENC_DT      7
#define ENC_CLK     8
#define DISP_SDA    2
#define DISP_SCK    3
const byte SW_pins[] = {SENSOR_PINS};

//=============================================================================================
//                                   ПИНЫ Arduino Nano
//=============================================================================================

#elif defined(ARDUINO_AVR_NANO)
#define PUMP_POWER  3
#define SERVO_POWER 4
#define BATTERY_PIN A7
#define SERVO_PIN   5
#define LED_PIN     6
#define BTN_PIN     7
#define ENC_SW      8
#define ENC_DT      9
#define ENC_CLK     10
#define DISP_SDA    A4
#define DISP_SCK    A5
const byte SW_pins[] = {SENSOR_PINS};
#endif

//=============================================================================================
//ЛИБЫ

#include <EEPROM.h>
#include <microLED.h>
#include <ServoSmooth.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

//=============================================================================================
//ДАТА

#define COLOR_DEBTH 2 // цветовая глубина: 1, 2, 3 (в байтах)
#if (STATUS_LED)
#define statusLed 1
#else
#define statusLed 0
#endif
LEDdata leds[NUM_SHOTS + statusLed];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + statusLed, LED_PIN); // объект лента
SSD1306AsciiAvrI2c disp;
ServoSmooth servo;
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);  // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(30);
timerMinim FLOWdebounce(15);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(500);
timerMinim TIMEOUTtimer(STBY_TIME * 1000L); // таймаут режима ожидания
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);
timerMinim KEEP_POWERtimer(KEEP_POWER * 1000L);

#define MIN_COLOR 48                          // ORANGE mWHEEL
#define MAX_COLOR 765                         // AQUA mWHEEL
#define COLOR_SCALE (MAX_COLOR - MIN_COLOR)   // фактор для плавного изменения цвета во время налива

bool LEDchanged = false;
bool pumping = false;
int8_t curPumping = -1;
int8_t curSelected = -1;
int8_t selectShot = -1;
uint8_t shotCount = 0;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
enum serviceModes {VOLUME = 1, SERVO, BATTERY};
serviceModes serviceMode;
enum workModes {ManualMode, ComboMode, AutoMode};
workModes workMode;
uint16_t time50ml = 0;
#define INIT_VOLUME 47
uint16_t thisVolume = INIT_VOLUME;
uint16_t shotVolume[NUM_SHOTS];
byte initShotPos[] = {SHOT_POSITIONS};
byte shotPos[] = {SHOT_POSITIONS};
float volumeTick = 15.0f * 50.0f / time50ml;
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool parking = true;
bool LEDbreathingState = false;
float battery_voltage = 4.2;
float battery_cal = BATTERY_CAL;
uint16_t shots_overall = 0, volume_overall = 0;
bool keepPowerState = false;
bool volumeChanged = false;
bool invertState = 0;
char bootscreen[] = {"Der Наливатор"};
float versionNum = VERSION;

enum {
  timeout_off = 0,
  inverse_servo,
  parking_pos,
  auto_parking,
  stby_time,
  stby_light,
  rainbow_flow,
  max_volume,
  keep_power,
  invert_display
};
uint8_t settingsList[] = {
  TIMEOUT_OFF,
  INVERSE_SERVO,
  PARKING_POS,
  AUTO_PARKING,
  STBY_TIME,
  STBY_LIGHT,
  RAINBOW_FLOW,
  MAX_VOLUME,
  KEEP_POWER,
  INVERT_DISPLAY
};

struct EEPROMAddress {
  uint8_t _thisVolume = 0;
  uint8_t _time50ml = sizeof(thisVolume);
  uint8_t _shotPos = _time50ml + sizeof(time50ml);
  uint8_t _battery_cal = _shotPos + sizeof(byte) * NUM_SHOTS;
  uint8_t _timeout_off = _battery_cal + sizeof(battery_cal);
  uint16_t _inverse_servo = _timeout_off + sizeof(settingsList[timeout_off]);
  uint16_t _parking_pos = _inverse_servo + sizeof(settingsList[inverse_servo]);
  uint16_t _auto_parking = _parking_pos + sizeof(settingsList[parking_pos]);
  uint16_t _stby_time = _auto_parking + sizeof(settingsList[auto_parking]);
  uint16_t _stby_light = _stby_time + sizeof(settingsList[stby_time]);
  uint16_t _rainbow_flow = _stby_light + sizeof(settingsList[stby_light]);
  uint16_t _max_volume = _rainbow_flow + sizeof(settingsList[rainbow_flow]);
  uint16_t _shots_overall = _max_volume + sizeof(settingsList[max_volume]);
  uint16_t _volume_overall = _shots_overall + sizeof(shots_overall);
  uint16_t _keep_power = _volume_overall + sizeof(volume_overall);
  uint16_t _invert_display = _keep_power + sizeof(keep_power);
} eeAddress;

//=============================================================================================
//МАКРО

#define servoON() digitalWrite(SERVO_POWER, 1)
#define servoOFF() digitalWrite(SERVO_POWER, 0)
#define pumpON() digitalWrite(PUMP_POWER, 1)
#define pumpOFF() digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == 1)
#define DEBUG(x) Serial.print(x)
#define DEBUGln(x) Serial.println(x)
#else
#define DEBUG(x)
#define DEBUGln(x)
#endif

#if (STATUS_LED)
#define LED leds[NUM_SHOTS]
#endif
