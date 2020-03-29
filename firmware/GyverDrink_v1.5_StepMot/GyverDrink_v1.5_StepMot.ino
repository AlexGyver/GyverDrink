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

   Версия 1.5 с шаговым двигателем и драйвером StepStick (STEP/DIR)
*/

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS 5                       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5                     // таймаут на выключение (перестаёт дёргать привод), минут
#define SWITCH_LEVEL 0                    // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define STEPS_PER_REVOLUTION 2037.88642   // количество шагов на оборот двигателя
#define STEPPER_ENDSTOP_INVERT  0         // 1 - высокий сигнал при замыкании, 0 - низкий
#define STEPPER_POWERSAFE 0               // автоматическое управление питанием шагового двигателя (питание включается только при движении)
#define INVERT_STEPPER 1                  // инвертировать направление вращения шагового двигателя
#define STEPPER_SPEED 15                  // скорость двигателя в оборотах в минуту
#define MICROSTEPS  2                     // значение микрошага, выставленного на драйвере двигателя


// положение крана над центрами рюмок в градусах от нулевой точки
const byte shotPos[] = {0, 45, 90, 135, 180};

#define PARKING_POS 0       // положение парковочной позиции в градусах

// время заполнения 50 мл
const long time50ml = 5500;

#define KEEP_POWER OFF    // ON - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART ON

// =========== ПИНЫ Arduino Nano===========
//#define PUMP_POWER 3
//#define STEPPER_STEP  5
//#define STEPPER_DIR 4
//#define STEPPER_EN  2
//#define STEPPER_ENDSTOP 13
//#define LED_PIN 6
//#define BTN_PIN 7
//#define ENC_SW 8
//#define ENC_DT 9
//#define ENC_CLK 10
//#define DISP_DIO 11
//#define DISP_CLK 12
//const byte SW_pins[] = {A0, A1, A2, A3, A4, A5};

// =========== ПИНЫ Arduino Micro===========
#define PUMP_POWER      0
#define BTN_PIN         1
#define DISP_DIO        2
#define DISP_CLK        3
#define BAT_PIN         4
#define LED_PIN         5
#define STEPPER_STEP    6
#define STEPPER_DIR     7
#define STEPPER_EN      8
//#define STEPPER_ENDSTOP 9
#define ENC_SW          10
#define ENC_DT          14
#define ENC_CLK         16
const byte SW_pins[] = {15, 18, 19, 20, 21};

// =========== ЛИБЫ ===========
#include <GyverTM1637.h>
#include <microLED.h>
#include <EEPROM.h>
#include <StepMot.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"

// =========== ДАТА ===========
#define COLOR_DEBTH 2                         // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[NUM_SHOTS];                      // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS, LED_PIN);     // объект лента
GyverTM1637 disp(DISP_CLK, DISP_DIO);
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);  // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
StepMot stepper(STEPS_PER_REVOLUTION * MICROSTEPS, STEPPER_STEP, STEPPER_DIR, STEPPER_EN);

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(100);
timerMinim FLOWdebounce(20);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(400);
timerMinim TIMEOUTtimer(15000);   // таймаут дёргания приводом
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);

bool LEDchanged = false;
bool pumping = false;
int8_t curPumping = -1;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = 1;  // 0 manual, 1 auto
uint8_t thisVolume = 50;
bool systemON = false;
bool timeoutState = false;
bool volumeChanged = false;
bool parking = false;

// =========== МАКРО ===========
#define pumpON() digitalWrite(PUMP_POWER, 1)
#define pumpOFF() digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == ON)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#ifdef STEPPER_ENDSTOP
#if (STEPPER_ENDSTOP_INVERT == 1)
#define ENDSTOP_STATUS !digitalRead(STEPPER_ENDSTOP)
#else 
#define ENDSTOP_STATUS digitalRead(STEPPER_ENDSTOP)
#endif
#endif
