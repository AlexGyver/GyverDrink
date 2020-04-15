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

/*
   Версия 1.1:
   - Поправлена работа системы при выборе некорректного объёма
   - Исправлены ошибки при наливании больших объёмов
   - Исправлен баг с остановкой наливания при убирании другой рюмки

   Версия 1.2:
   - Исправлено ограничение выбора объёма
   - Исправлены ошибки (обновите библиотеки из архива! servoSmooth v1.8, microLED v2.3)
   - Добавлено хранение в памяти выбранного объёма

   Версия 1.3:
   - Исправлен баг со снятием рюмки в авто режиме (жука поймал Юрий Соколов)

   Версия 1.4:
   - Добавлена настройка уровня концевиков (для ИК датчиков)
   - Исправлена ошибка с наливанием больших объёмов

   Версия 1.5:
   - Добавлена инверсия сервопривода (ОБНОВИТЕ БИБЛИОТЕКУ ИЗ АРХИВА)

   VICLER_MOD_StepMot
   - шаговый двигатель с драйвером типа StepStick вместо серво
   - возможность работы как с концевым датчиком для нулевой позиции двигателя (для этого укажите пин STEPPER_ENDSTOP)
      так и без него, установив нулевую позицию вручную (закоментируйте STEPPER_ENDSTOP)
   - компенсация люфта шагового двигателя в градусах STEPER_BACKLASH
   - возможность установить парковочную позицию PARKING_POS
   - прокачка над любой рюмкой. Прокачка проводится только в ручном режиме и только при наличии рюмки
   - шаг изменения объёма 1мл
   - вывод налитого объёма в реальном времени. Так же и во время прокачки (сброс после снятия рюмки)
   - убраны буквы для отображения режима (если автоматический - горит двоеточие)
   - объём на дисплее отображается по центру
   - плавный цветовой переход во время налива (от ORANGE до AQUA)
   - динамическая подсветка налитых рюмок
   - анимация приветствия
   - дополнительный светодиод в башне. При движении мотора горит оранжевым, при остановке - белым. 
      После налития всех рюмок "дышит" бирюзовым, во время простоя - белым. Подключается к светодиоду последней рюмки
   - после выхода из сервис режима, калибровка объёма сохраняется в постоянной памяти
*/

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS 5                       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5                     // таймаут на выключение (перестаёт дёргать привод), минут
#define SWITCH_LEVEL 0                    // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define STEPS_PER_REVOLUTION 2037.88642   // количество шагов на оборот двигателя
#define STEPPER_ENDSTOP_INVERT  0         // 1 - высокий сигнал при замыкании, 0 - низкий
#define STEPPER_POWERSAFE 1               // энергосберегающий режим шагового двигателя
#define INVERT_STEPPER 0                  // инвертировать направление вращения шагового двигателя
#define STEPPER_SPEED 15                   // скорость двигателя в оборотах в минуту
#define MICROSTEPS  2                     // значение микрошага, выставленного на драйвере двигателя
#define STEPER_BACKLASH 3.5

// положение крана над центрами рюмок в градусах от нулевой точки
const byte shotPos[] = {0, 45, 90, 135, 180};

#define PARKING_POS 90       // положение парковочной позиции в градусах

// время заполнения 50 мл
#define TIME_50ML 5100

#define KEEP_POWER OFF    // ON - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART OFF

// =========== ПИНЫ Arduino Micro===========
#if defined(ARDUINO_AVR_MICRO)
#define PUMP_POWER      0
#define VALVE_PIN       1
#define DISP_DIO        2
#define DISP_CLK        3
#define BTN_PIN         4
#define LED_PIN         5
#define STEPPER_STEP    6
#define STEPPER_DIR     7
#define STEPPER_EN      8
#define STEPPER_ENDSTOP 9
#define ENC_SW          10 
#define ENC_DT          14
#define ENC_CLK         16
const byte SW_pins[] = {15, 18, 19, 20, 21};

// =========== ПИНЫ Arduino Nano===========
#elif defined(ARDUINO_AVR_NANO)
#define PUMP_POWER 3
#define STEPPER_STEP  5
#define STEPPER_DIR 4
#define STEPPER_EN  2
#define STEPPER_ENDSTOP 13
#define LED_PIN 6
#define BTN_PIN 7
#define ENC_SW 8
#define ENC_DT 9
#define ENC_CLK 10
#define DISP_DIO 11
#define DISP_CLK 12
const byte SW_pins[] = {A0, A1, A2, A3, A4, A5};
#endif

// =========== ЛИБЫ ===========
#include <GyverTM1637.h>
#include <microLED.h>
#include <EEPROM.h>
#include <StepMot.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"
#include "TM1637_Animation.h"

// =========== ДАТА ===========
#define COLOR_DEBTH 2                             // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[NUM_SHOTS + 1];                      // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + 1, LED_PIN);     // объект лента
GyverTM1637 disp(DISP_CLK, DISP_DIO);
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);      // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
StepMot stepper(STEPS_PER_REVOLUTION * MICROSTEPS, STEPPER_STEP, STEPPER_DIR, STEPPER_EN);

#define MIN_COLOR 48  // ORANGE mWHEEL
#define MAX_COLOR 765 // AQUA mWHEEL
#define COLOR_SCALE (MAX_COLOR - MIN_COLOR)

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(50);
timerMinim FLOWdebounce(15);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(500);
timerMinim TIMEOUTtimer(5000);   // таймаут дёргания приводом
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);

bool LEDchanged = false;
bool pumping = false;
int8_t curPumping = -1;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = 0;  // 0 manual, 1 auto
uint16_t time50ml = 0;
uint8_t thisVolume = 47;
float volumeTick = 15.0f * 50.0f / time50ml;  // volume per one FLOWdebouce timer tick
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool volumeChanged = false;
bool parking = false;
bool atHome = false;
bool LEDbreathing = false;

// =========== МАКРО ===========
#define pumpON() digitalWrite(PUMP_POWER, 1)
#define pumpOFF() digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == ON)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#ifdef STEPPER_ENDSTOP
#define ENDSTOP_STATUS !digitalRead(STEPPER_ENDSTOP) ^ STEPPER_ENDSTOP_INVERT
#endif

#ifdef VALVE_PIN
#define drinkSelect(x) digitalWrite(VALVE_PIN, x)
#else
#define drinkSelect(x)
#endif

#define HeadLED leds[NUM_SHOTS]
