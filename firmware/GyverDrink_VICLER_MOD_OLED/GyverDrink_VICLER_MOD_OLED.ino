/*
  GyverDrink VICLER_MOD_OLED_1.1 Beta (в разработке. Многие функции работают криво или ещё не реализованны)
  05.08.2020

  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" на основе версии 1.5 by AlexGyver с OLED дисплеем и расширенным функционалом
  Исходники на GitHub: https://github.com/VICLER/GyverDrink

   -  возможность установить парковочную позицию PARKING_POS
   -  в автоматическом режиме краник остаётся над последней налитой рюмкой (в ручном возвращается в домашнее положение)
   -  инверсия серво
   -  прокачка над любой рюмкой. Прокачка проводится только в ручном режиме и только при наличии рюмки.
        Возврат крана в домашнее положение и сброс счётчика объёма жидкости происходит после снятия рюмки
   -  шаг изменения объёма 1мл
   -  вывод налитого объёма в реальном времени. Так же и во время прокачки (сброс после снятия рюмки)
   -  сохранение последнего выбранного объёма в постоянной памяти
   -  возможность отменить налив в любое время нажатием на кнопку
   -  убраны буквы для отображения режима (если автоматический, по караям отображаются штрихи)
   -  объём на дисплее отображается по центру
   -  плавный цветовой переход во время налива (от ORANGE до AQUA)
   -  динамическая подсветка налитых рюмок
   -  возможность включения постоянной подсветки при отсутствии рюмок
   -  анимация приветствия
   -  дополнительный светодиод в башне. При движении мотора горит оранжевым, при остановке - белым.
        После налития всех рюмок "дышит" бирюзовым, во время простоя - белым. Подключается к светодиоду последней рюмки, активируется в STATUS_LED
   -  после выхода из сервис режима, последднее значение таймера сохраняется в постоянной памяти
   -  в сервис режиме при наличии рюмки на дисплее отображается номер рюмки (начиная с 1) и положение крана для этой рюмки в градусах.
        Чтобы изменить положение крана над определённой рюмкой нужно поставить рюмку, подстроить угол энкодером и убрать рюмку.
        После выхода из сервисного режима зажатием кнопки, все углы сохранятся в постоянной памяти.
   -  возможность настроить объём для каждой рюмки отдельно:
        При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
        Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)
   -  индикация заряда аккумулятора и слежение за напряжением.
        При напряжении, меньшем чем BATTERY_LOW, устройство перестаёт реагировать на внешние воздействия и на дисплее мигает иконка батареи
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

#define NUM_SHOTS     5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF   5     // таймаут на выключение дисплея и светодиодов в минутах. Если 0 -> таймаут отключен
#define SWITCH_LEVEL  0     // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define INVERSE_SERVO 0     // инвертировать направление вращения серво
#define PARKING_POS   0     // угол для домашней позиции
#define TIME_50ML     5000  // время заполнения 50 мл
#define AUTO_PARKING  1     // парковка в авто режиме: 1 -> вкл, 0 -> выкл
#define STBY_LIGHT    15    // яркость подсветки в режиме ожидания. 255 -> максимум, 0 -> выкл
#define RAINBOW_FLOW  1     // 1 -> динамическая подсветка налитых рюмок, 0 -> статическая
#define STATUS_LED    0     // яркость статус-светодиода. 255 -> максимум, 0 -> не подключен.
#define BATTERY_LOW   3.3   // минимальное напряжение аккумулятора

#define MAX_VOLUME    200   // ограничение максимального объёма
#define DEBUG_UART    0     // отладка

// положение серво над центрами рюмок
byte shotPos[] = {0, 45, 90, 135, 180};

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
const byte SW_pins[] = {14, 15, 18, 19, 20};

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
const byte SW_pins[] = {A0, A1, A2, A3, 11, 12};
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
#include "fonts/CyrillicFonts.h"

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
timerMinim TIMEOUTtimer(10000); // таймаут дёргания приводом
#if (TIMEOUT_OFF > 0)
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);
#endif

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
bool workMode = 0;  // 0 manual, 1 auto
uint16_t time50ml = 0;
uint16_t thisVolume = 47;
uint16_t shotVolume[NUM_SHOTS];
float volumeTick = 15.0f * 50.0f / time50ml;
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool parking = true;
bool LEDbreathingState = false;
float battery_voltage = 4.2;

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
