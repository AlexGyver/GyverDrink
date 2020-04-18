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
   VICLER_MOD_1.7:
   - основанно на версии 1.3 by AlexGyver
   - возможность установить парковочную позицию PARKING_POS
   - в автоматическом режиме краник остаётся над последней налитой рюмкой (в ручном возвращается в домашнее положение)
   - инверсия серво
   - прокачка над любой рюмкой. Прокачка проводится только в ручном режиме и только при наличии рюмки
   - шаг изменения объёма 1мл
   - вывод налитого объёма в реальном времени. Так же и во время прокачки (сброс после снятия рюмки)
   - сохранение последнего выбранного объёма сразу после изменения
   - убраны буквы для отображения режима (если автоматический - горит двоеточие)
   - объём на дисплее отображается по центру
   - плавный цветовой переход во время налива (от ORANGE до AQUA)
   - динамическая подсветка налитых рюмок
   - анимация приветствия
   - после выхода из сервис режима, калибровка объёма сохраняется в постоянной памяти
   - в сервис режиме при наличии рюмки на дисплее отображается номер рюмки (начиная с 1) и положение крана для этой рюмки в градусах.
      Чтобы изменить положение крана над определённой рюмкой нужно поставить рюмку, подстроить угол энкодером и убрать рюмку.
      После выхода из сервисного режима зажатием кнопки энкодера, все углы сохранятся в постоянной памяти.
   - возможность настроить объём для каждой рюмки отдельно:
      При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
      Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)
*/

// ======== НАСТРОЙКИ ========
#define NUM_SHOTS 3       // количество рюмок (оно же кол-во светодиодов и кнопок!)
#define TIMEOUT_OFF 5     // таймаут на выключение (перестаёт дёргать привод), минут
#define INVERSE_SERVO 0   // инвертировать направление вращения серво

// положение серво над центрами рюмок
byte shotPos[] = {0, 45, 90, 135, 180};

#define PARKING_POS 0       // угол для домашней позиции

// время заполнения 50 мл
#define TIME_50ML 5000

#define KEEP_POWER 0    // 1 - система поддержания питания ПБ, чтобы он не спал

// отладка
#define DEBUG_UART 1

#if defined(ARDUINO_AVR_MICRO)  // =========== ПИНЫ Arduino Micro ===========
#define PUMP_POWER 4
#define SERVO_POWER 16
#define SERVO_PIN 6
#define LED_PIN 5
#define BTN_PIN 10
#define ENC_SW 9
#define ENC_DT 7
#define ENC_CLK 8
#define DISP_DIO 2
#define DISP_CLK 3
const byte SW_pins[] = {14, 15, 18, 19, 20};

#elif defined(ARDUINO_AVR_NANO) // =========== ПИНЫ Arduino Nano ===========
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
#endif

// =========== ЛИБЫ ===========
#include <GyverTM1637.h>
#include <ServoSmooth.h>
#include <microLED.h>
#include <EEPROM.h>
#include "encUniversalMinim.h"
#include "buttonMinim.h"
#include "timer2Minim.h"
#include "TM1637_Animation.h"

// =========== ДАТА ===========
#define COLOR_DEBTH 2                         // цветовая глубина: 1, 2, 3 (в байтах)
LEDdata leds[NUM_SHOTS];                      // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS, LED_PIN);     // объект лента
GyverTM1637 disp(DISP_CLK, DISP_DIO);
ServoSmooth servo;
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);  // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(50);
timerMinim FLOWdebounce(15);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(500);
timerMinim TIMEOUTtimer(5000);   // таймаут дёргания приводом
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);

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
uint8_t thisVolume = 47;
uint8_t shotVolume[NUM_SHOTS];
float volumeTick = 15.0f * 50.0f / time50ml;
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool parking = true;

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
