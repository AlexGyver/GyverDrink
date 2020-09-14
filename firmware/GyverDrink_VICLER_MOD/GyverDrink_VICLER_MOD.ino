//GyverDrink VICLER_MOD_3.1.1
//13.09.2020
/*
  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" с дополнительным функционалом

   ⚠ Все настройки в файле Config.h ⚠

   ⚫ Прокачка. Поставьте рюмку, нажмите и удерживайте кнопку энкодера.
        Прокачка проводится только в ручном режиме и только при наличии рюмки.
        Возврат крана в домашнее положение и сброс счётчика объёма жидкости происходит после снятия рюмки.

   ⚫ Режим изменяется удержанием основной кнопки.
        Ручной режим: разлив начинается только после однократного нажатия на основную кнопку.
        Авто режим(по краям дисплея отображенны штрихи): разлив начинается автоматически сразу после установки рюмки

   ⚫ Возможность настроить объём для каждой рюмки отдельно:
        При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
        Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)

   ⚫ Разлив можно остановить в любое время нажатием на основную кнопку

   ⚫ Последний выбранный объём сохраняется после перехода устройства в режим ожидания

   ⚫ Стартовая анимация и динамическая подсветка рюмок во время и после налива

   ⚫ Слежение за напряжением аккумулятора (максимальное напряжение без делителя = 5V). Плюс аккумулятора подключается к пину BATTERY_PIN.
        Активируется раскоментированием строки #define BATTERY_PIN ...
        При напряжении, меньшем чем BATTERY_LOW, устройство перестаёт реагировать на внешние воздействия и на дисплее мигает индикация пустой батареи
        Если при первом включении мигает иконка аккумулятора, но напряжение в порядке(больше, чем BATTERY_LOW) -> нажмите и удерживайте кнопку до появления сервис режима, и откалибруйте напряжение.

   ⚫ Сервис режим. Смена этапа калибровки (1.Серво, 2.Объём, 3.Аккумулятор) происходит удержанием основной кнопки.
      ➊ Калибровка серводвигателя:
          при наличии рюмки на дисплее отображается номер рюмки и положение крана для этой рюмки в градусах. Если нет рюмок - отображается угол парковочной позиции.
          Чтобы изменить положение крана над определённой рюмкой нужно поставить рюмку, подстроить угол энкодером и убрать рюмку. При отсутствии рюмок энкодером настраивается парковочное положение.
      ➋ Калибровка объёма:
          поставьте рюмку в любую позицию, кран встанет над рюмкой. После этого, зажав энкодер, налейте 50мл.
      ➌ Калибровка напряжения аккумулятора:
          на дисплее отображается напряжение аккумулятора в [mV], которое подстраивается энкодером до совпадения с измеренным

   ⚫ При зажатии кнопки выбора режима более 5 секунд, происходит сброс настроек из памяти (TIME_50ML и SHOT_POSITIONS) до первоначальных, прописанных в этом файле



   ₽$€ На чашечку кофе автору:        http://paypal.me/vicler

   ★ Исходники на GitHub:             https://github.com/VICLER/GyverDrink

   ★ Оригинальный проект AlexGyver:   https://alexgyver.ru/GyverDrink/
*/

//╞═════════════════════════════╡LIBS╞═════════════════════════════╡

#include "src/GyverTM1637/GyverTM1637.h"
#include "src/ServoSmooth/ServoSmooth.h"
#include "src/microLED/microLED.h"
#include "src/encUniversalMinim.h"
#include "src/buttonMinim.h"
#include "src/timer2Minim.h"
#include "Config.h"
#include <EEPROM.h>

//╞═════════════════════════════╡DATA╞═════════════════════════════╡

#if (STATUS_LED)
#define statusLed 1
#else
#define statusLed 0
#endif
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);  // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
LEDdata leds[NUM_SHOTS + statusLed];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + statusLed, LED_PIN); // объект лента
GyverTM1637 disp(DISP_CLK, DISP_DIO);
ServoSmooth servo;

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(30);
timerMinim FLOWdebounce(15);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(500);
timerMinim TIMEOUTtimer(STBY_TIME * 1000L); // таймаут режима ожидания
#if (KEEP_POWER)
timerMinim KEEP_POWERtimer(KEEP_POWER * 1000L);
#endif
#if (TIMEOUT_OFF)
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
enum {SERVO, VOLUME, BATTERY} serviceState;
bool workMode = 0;  // 0 manual, 1 auto
uint16_t time50ml = 0;
uint8_t thisVolume = 47;
uint8_t shotVolume[NUM_SHOTS];
byte initShotPos[] = {SHOT_POSITIONS};
byte shotPos[] = {SHOT_POSITIONS};
const byte SW_pins[] = {SW_PINS};
float volumeTick = 15.0f * 50.0f / time50ml;
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool parking = true;
bool LEDbreathingState = false;
float battery_voltage = 4.2;
float battery_cal = 1.0;
bool volumeChanged = false;
bool keepPowerState = false;
uint8_t parking_pos = PARKING_POS;
bool servoReady = false;
uint8_t animCount = 0;

//╞═════════════════════════════╡MACROS╞═════════════════════════════╡

#define servoON()  digitalWrite(SERVO_POWER, 1)
#define servoOFF() digitalWrite(SERVO_POWER, 0)
#define pumpON()   digitalWrite(PUMP_POWER, 1)
#define pumpOFF()  digitalWrite(PUMP_POWER, 0)

#if (DEBUG_UART == 1)
#define DEBUG(x)   Serial.print(x)
#define DEBUGln(x) Serial.println(x)
#else
#define DEBUG(x)
#define DEBUGln(x)
#endif

#if (STATUS_LED)
#define LED leds[NUM_SHOTS]
#endif
