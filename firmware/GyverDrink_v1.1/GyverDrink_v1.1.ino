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
  Fork by VICLER https://github.com/VICLER/GyverDrink
*/

/*
   Версия 1.2:
   - Использование библиотек GyverEncoder v4.2, GyverButton v3.0, GyverTimer 3.2 и MicroLED v2.1
   - Исправленны некорректные отображения объёма (отрицательные и больше 1000мл)
   - Исправлен баг с остановкой наливания при убирании рюмки во время движения сервопривода
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
#define LED_PIN 6

// =========== ЛИБЫ ===========
#include <GyverTM1637.h>
#include <ServoSmooth.h>
#include <GyverEncoder.h>
#include <GyverButton.h>
#include <GyverTimer.h>

// =========== ДАТА ===========
#define ORDER_GRB       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)
#include <microLED.h>
LEDdata leds[NUM_SHOTS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS, LED_PIN);  // объект лента

GyverTM1637 disp(DISP_CLK, DISP_DIO);

// пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);

ServoSmooth servo;

GButton btn(BTN_PIN, HIGH_PULL, NORM_OPEN);
GButton encBtn(ENC_SW, HIGH_PULL, NORM_OPEN);
GTimer LEDtimer(MS, 100);
GTimer FLOWdebounce(MS, 20);
GTimer FLOWtimer(MS, 2000);
GTimer WAITtimer(MS, 300);
GTimer TIMEOUTtimer(MS, 15000);   // таймаут дёргания приводом
GTimer POWEROFFtimer(MS, TIMEOUT_OFF * 60000L);

bool LEDchanged = false;
bool pumping = false;
int8_t curPumping = -1;

enum {NO_GLASS, EMPTY, IN_PROCESS, READY} shotStates[NUM_SHOTS];
enum {SEARCH, MOVING, WAIT, PUMPING} systemState;
bool workMode = false;  // 0 manual, 1 auto
int thisVolume = 50;
bool systemON = false;
bool timeoutState = false;

// =========== МАКРО ===========
#define servoON() digitalWrite(SERVO_POWER, 1)
#define servoOFF() digitalWrite(SERVO_POWER, 0)
#define pumpON() digitalWrite(PUMP_POWER, 1)
#define pumpOFF() digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == 1)
#define DEBUG(x) uart.println(x)
#else
#define DEBUG(x)
#endif
