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
*/

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS 5       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5     // таймаут на выключение (перестаёт дёргать привод), минут

// положение серво над центрами рюмок
const byte shotPos[] = {32, 54, 77, 101, 125};

// время заполнения 50 мл
const int time50ml = 11500;
const int time1ml = 210;

#define KEEP_POWER 0    // 1 - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART 0

// =========== ПИНЫ ===========
#define BATTERY_PIN A0
#define PUMP_POWER 16 
//2
#define SERVO_PIN 14
//3
#define LED_PIN 4
#define BTN_PIN 5
#define ENC_SW 6
#define ENC_DT 7
#define ENC_CLK 8
const byte SW_pins[] = {9, 10, 11, 12, 13};

#define ORDER_GRB       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)
// =========== ЛИБЫ ===========
#include <EEPROM.h>
#include <ServoSmooth.h>
#include <microLED.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"
#include <U8g2lib.h>
#include "CFilter.h"
// =========== ДАТА ===========

LEDdata leds[NUM_SHOTS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS, LED_PIN);  // объект лента

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 3, /* data=*/ 2);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 0);

ServoSmooth servo;
CFilter BatteryFilter(0.3);

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim BatteryTimer(1000*60);
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
bool serviceState = false;
enum { ml, ms, deg, none };
uint8_t dispData = ml;
int thisVolume = 50;
float battVoltage = 0;
bool systemON = false;
bool timeoutState = false;

// =========== МАКРО ===========
#define pumpON() digitalWrite(PUMP_POWER, 0)
#define pumpOFF() digitalWrite(PUMP_POWER, 1)

#if (DEBUG_UART == 1)
#define DEBUG(x) uart.println(x)
#else
#define DEBUG(x)
#endif
