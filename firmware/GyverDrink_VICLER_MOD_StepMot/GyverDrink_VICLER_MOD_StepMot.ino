/*
  GyverDrink VICLER_MOD_StepMot_v1.9
  03.08.2020

  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" на основе версии 1.5 by AlexGyver с заменой серводвигателя на шаговый мотор, устранением багов и дополнительным функционалом
  Исходники на GitHub: https://github.com/VICLER/GyverDrink

   - шаговый двигатель с драйвером типа StepStick вместо серво
   - возможность работы как с концевым датчиком для нулевой позиции двигателя (для этого укажите пин STEPPER_ENDSTOP)
      так и без него, установив нулевую позицию вручную перед или после включения устройства (закоментируйте #define STEPPER_ENDSTOP -> //#define STEPPER_ENDSTOP). Эта позиция должна соответствовать PARKING_POS
   - калибровка начального положения по концевому датчику происходит при каждом включении. Если парковочная позиция совпадает с позицией концевого датчика, то калибровка осуществляется и после каждого налива.
   - компенсация люфта шагового двигателя в градусах STEPER_BACKLASH
   - возможность установить парковочную позицию PARKING_POS
   - прокачка над любой рюмкой. Прокачка проводится только в ручном режиме и только при наличии рюмки. 
      Возврат крана в домашнее положение и сброс счётчика объёма жидкости происходит после снятия рюмки
   - шаг изменения объёма 1мл
   - вывод налитого объёма в реальном времени. Так же и во время прокачки (сброс после снятия рюмки)
   - сохранение последнего выбранного объёма в постоянной памяти
   - возможность отменить налив в любое время нажатием на кнопку
   - убраны буквы для отображения режима (если автоматический, по караям отображаются штрихи)
   - объём на дисплее отображается по центру
   - плавный цветовой переход во время налива (от ORANGE до AQUA)
   - динамическая подсветка налитых рюмок
   - возможность включения постоянной подсветки при отсутствии рюмок
   - анимация приветствия
   - дополнительный светодиод в башне. При движении мотора горит оранжевым, при остановке - белым.
      После налития всех рюмок "дышит" бирюзовым, во время простоя - белым. Подключается к светодиоду последней рюмки, активируется в STATUS_LED
   - после выхода из сервис режима, калибровка объёма сохраняется в постоянной памяти
   - в сервис режиме при наличии рюмки на дисплее отображается номер рюмки (начиная с 1) и положение крана для этой рюмки в градусах.
      Чтобы изменить положение крана над определённой рюмкой нужно поставить рюмку, подстроить угол энкодером и убрать рюмку.
      После выхода из сервисного режима зажатием кнопки энкодера, все углы сохранятся в постоянной памяти.
   - возможность настроить объём для каждой рюмки отдельно:
      При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
      Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)
*/

/* 
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

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS     5     // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF   5     // таймаут на выключение (перестаёт дёргать привод), минут
#define SWITCH_LEVEL  0     // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define PARKING_POS   0     // положение парковочной позиции в градусах
#define TIME_50ML     5100  // время заполнения 50 мл
#define KEEP_POWER    0     // 1 - система поддержания питания ПБ, чтобы он не спал
#define STBY_LIGHT    15    // яркость подсветки в режиме ожидания. 255 -> максимум, 0 -> выкл
#define STATUS_LED    0     // яркость статус-светодиода. 255 -> максимум, 0 -> не подключен.

#define STEPS_PER_REVOLUTION     2037.88642  // количество шагов на оборот двигателя
#define MICROSTEPS               2           // значение микрошага, выставленного на драйвере двигателя
#define INVERT_STEPPER           0           // инвертировать направление вращения шагового двигателя
#define STEPPER_ENDSTOP_INVERT   0           // 1 - высокий сигнал при замыкании, 0 - низкий
#define STEPPER_POWERSAFE        1           // энергосберегающий режим шагового двигателя. 1 -> вкл ,0 -> выкл
#define STEPER_BACKLASH          3           // компенсация люфта двигателя в градусах. 0 выключает эту функцию
#define STEPPER_SPEED            15          // скорость двигателя в об/мин
#define STEPPER_HOMING_SPEED     5           // скорость вращения двигателя при движении к концевику (если он есть) в об/мин

#define DEBUG_UART 0                         // отладка

byte shotPos[] = {0, 45, 90, 135, 180};      // положение крана над центрами рюмок

// =========== ПИНЫ Arduino Micro===========
#if defined(ARDUINO_AVR_MICRO)  
#define PUMP_POWER      0                     // управляющий сигнал драйвера помпы (обычно обозначен как IN на драйверах моторов, либо GATE/BASE, если используется транзистор)
#define VALVE_PIN       1                     // не используется
#define DISP_DIO        2                     // сигнал DIO на дисплее TM1637
#define DISP_CLK        3                     // сигнал CLK на дисплее TM1637
#define BTN_PIN         4                     // сигнал с основной кнопки для разлива и смены режимов
#define LED_PIN         5                     // сигнал DIO/DI первого светодиода (для первой рюмки)
#define STEPPER_STEP    6                     // сигнал STEP драйвера
#define STEPPER_DIR     7                     // сигнал DIR драйвера
#define STEPPER_EN      8                     // сигнал EN драйвера
#define STEPPER_ENDSTOP 9                     // сигнал с концевика для двигателя. Закоментировать, если концевик не используется
#define ENC_SW          10                    // сигнал центральной кнопки на энкодере для прокачки и настройки различных обьёмов. Обозначен как SW или BTN
#define ENC_DT          14                    // сигнал DT или А с энкодера
#define ENC_CLK         16                    // сигнал CLK или B с энкодера
const byte SW_pins[] = {15, 18, 19, 20, 21};  // сигналы с датчиков наличия рюмок: от первой к последней рюмке

// =========== ПИНЫ Arduino Nano===========
#elif defined(ARDUINO_AVR_NANO)
#define PUMP_POWER      3                         // управляющий сигнал драйвера помпы (обычно обозначен как IN на драйверах моторов, либо GATE/BASE, если используется транзистор)
#define STEPPER_STEP    5                         // сигнал STEP драйвера
#define STEPPER_DIR     4                         // сигнал DIR драйвера
#define STEPPER_EN      2                         // сигнал EN драйвера
#define STEPPER_ENDSTOP A7                        // сигнал с концевика для двигателя. Закоментировать, если концевик не используется
#define LED_PIN         6                         // сигнал DIO/DI первого светодиода (для первой рюмки)
#define BTN_PIN         7                         // сигнал с основной кнопки для разлива и смены режимов
#define ENC_SW          8                         // сигнал центральной кнопки на энкодере для прокачки и настройки различных обьёмов. Обозначен как SW или BTN
#define ENC_DT          9                         // сигнал DT или А с энкодера
#define ENC_CLK         10                        // сигнал CLK или B с энкодера
#define DISP_DIO        11                        // сигнал DIO на дисплее TM1637
#define DISP_CLK        12                        // сигнал CLK на дисплее TM1637
const byte SW_pins[] = {A0, A1, A2, A3, A4, A5};  // сигналы с датчиков наличия рюмок: от первой к последней рюмке
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

#if (STATUS_LED)
#define statusLed 1
#else 
#define statusLed 0
#endif 
#define COLOR_DEBTH 2                             // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[NUM_SHOTS + statusLed];                      // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + statusLed, LED_PIN);     // объект лента
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
int8_t curSelected = -1;
int8_t selectShot = -1;
uint8_t shotCount = 0;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = 0;  // 0 manual, 1 auto
uint16_t time50ml = 0;
uint16_t thisVolume = 47;
uint16_t shotVolume[NUM_SHOTS];
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

#if (DEBUG_UART == 1)
#define DEBUG(x) Serial.print(x)
#define DEBUGln(x) Serial.println(x)
#else
#define DEBUG(x)
#define DEBUGln(x)
#endif

#ifdef STEPPER_ENDSTOP
#define ENDSTOP_STATUS (!(analogRead(STEPPER_ENDSTOP)>512) ^ STEPPER_ENDSTOP_INVERT)
#endif

#if (STATUS_LED)
#define LED leds[NUM_SHOTS]
#endif
