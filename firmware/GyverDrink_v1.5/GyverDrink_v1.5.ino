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
   - Исправлен баг с остановкой наливания при убирании другой рю++мки

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
*/

// ======== НАСТРОЙКИ ========
#define CUM_SHOTS 4       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5     // таймаут на выключение (перестаёт дёргать привод), минут
#define SWITCH_LEVEL 0    // кнопки 1 - высокий сигнал при замыкании, 0 - низкий
#define INVERSE_SERVO 0   // инвертировать направление вращения серво
#define DISP595_BRIGHTNESS 1 // Для подключения программной регулирвоки яркости дисплея
#define DISP595_BRIGHTNESS_DEPTH 7
#define DISP_PERIOD 500  // Период динамической индикации в мкс (500-6000). Взял поменьше, потму что используется регулировка яркости


// положение серво над центрами рюмок
const byte shotPos[] = {25, 60, 95, 145, 60, 60};

// время заполнения 50 мл
const long time50ml = 5500;

#define KEEP_POWER 1    // 1 - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART 1

// =========== ПИНЫ ===========
#define PUMP_POWER 3
#define SERVO_POWER 4
#define SERVO_PIN 5
#define LED_PIN 6
#define BTN_PIN A4
#define ENC_SW 10 // кнопка
#define ENC_DT 9 // импульсы направления энкодера
#define ENC_CLK 8 // импульсы отсчёта энкодера 
#define DISP_DIO 11
#define DISP_RCLK 12
#define DISP_SCLK 13

// encoder
#define ENC_BUTTON A3
const byte SW_pins[] = {7, 8, 9, 10}; // {A0, A1, A2, A3, A4, A5};

// =========== ЛИБЫ ===========
// #include <GyverTM1637.h>
#include <ServoSmooth.h>
#include <microLED.h>
#include <EEPROM.h>
#include <GyverTimers.h>
#include <SevenSegmentsDisp.h>
#include "DaKarakumEncoder.h"
#include "buttonMinim.h"
#include "timer2Minim.h"

// =========== ДАТА ===========
#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[CUM_SHOTS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, CUM_SHOTS, LED_PIN);  // объект лента

Disp595 disp(DISP_DIO, DISP_SCLK, DISP_RCLK);

// пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
// encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1); DA_K: changed lib
DaKarakumEncoder enc; // A1 A2 A3


ServoSmooth servo;

buttonMinim btn (BTN_PIN);
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
int INITIAL_MODE = 0;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[CUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = false;  // 0 manual, 1 auto
int thisVolume = 50;
bool systemON = false;
bool timeoutState = false;
bool volumeChanged = false;
bool parking = false;

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
