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
*/

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS 4       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5     // таймаут на выключение (перестаёт дёргать привод), минут

// положение серво над центрами рюмок
const byte shotPos[] = {25, 60, 95, 145, 60, 60};

// время заполнения 50 мл
const long time50ml = 5500;

#define KEEP_POWER 1    // 1 - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART 0

// =========== ПИНЫ ===========
#define PUMP_POWER 3
#define SERVO_POWER 4
#define SERVO_PIN 5
#define LED_PIN 6
#define BTN_PIN 7
#define ENC_SW 8
#define ENC_DT 9
#define ENC_CLK 10
#define DISP_DIO 11
#define DISP_CLK 12
const byte SW_pins[] = {A0, A1, A2, A3, A4, A5};

// =========== ЛИБЫ ===========
#include <GyverTM1637.h>
#include <ServoSmooth.h>
#include <microLED.h>
#include <EEPROM.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"

// =========== ДАТА ===========
#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[NUM_SHOTS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS, LED_PIN);  // объект лента

GyverTM1637 disp(DISP_CLK, DISP_DIO);

// пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);

ServoSmooth servo;

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(100);
timerMinim FLOWdebounce(20);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(300);
timerMinim TIMEOUTtimer(15000);   // таймаут дёргания приводом
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);

bool LEDchanged = false;
bool pumping = false;
int8_t curPumping = -1;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = false;  // 0 manual, 1 auto
int thisVolume = 50;
bool systemON = false;
bool timeoutState = false;
bool volumeChanged = false;

// =========== МАКРО ===========
#define servoON() digitalWrite(SERVO_POWER, 1)
#define servoOFF() digitalWrite(SERVO_POWER, 0)
#define pumpON() digitalWrite(PUMP_POWER, 1)
#define pumpOFF() digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == 1)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif
