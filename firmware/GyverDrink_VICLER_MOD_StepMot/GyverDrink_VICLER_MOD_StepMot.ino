//GyverDrink VICLER_MOD_StepMot v2.1
//14.09.2020
/*
  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" с шаговым двигателем и дополнительным функционалом

    ⚠ Все настройки в файле Config.h ⚠
    
   ⚫ Шаговый двигатель с драйвером типа StepStick (A4899, DRV8825, LV8729, TMC2100/2208/2209) вместо серво
         возможность работы как с концевым датчиком для нулевой позиции двигателя (для этого укажите пин STEPPER_ENDSTOP)
         так и без него, установив нулевую позицию вручную перед или после включения устройства (закоментируйте #define STEPPER_ENDSTOP -> //#define STEPPER_ENDSTOP). Эта позиция должна соответствовать PARKING_POS
         калибровка начального положения по концевому датчику происходит при каждом включении. Если парковочная позиция совпадает с позицией концевого датчика, то калибровка осуществляется и после каждого налива.
         компенсация люфта шагового двигателя в градусах STEPER_BACKLASH

   ⚫ Прокачка. Поставьте рюмку, нажмите и удерживайте кнопку энкодера
        Прокачка проводится только в ручном режиме и только при наличии рюмки
        Возврат крана в домашнее положение и сброс счётчика объёма жидкости происходит после снятия рюмки

   ⚫ Режим изменяется удержанием основной кнопки
        Ручной режим: разлив начинается только после однократного нажатия на основную кнопку
        Авто режим(по краям дисплея отображенны штрихи): разлив начинается автоматически сразу после установки рюмки
   
   ⚫ Возможность настроить объём для каждой рюмки отдельно:
        При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка
        Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)

   ⚫ Разлив можно остановить в любое время нажатием на основную кнопку

   ⚫ Последний выбранный объём сохраняется после перехода устройства в режим ожидания

   ⚫ Стартовая анимация и динамическая подсветка рюмок во время и после налива
   
   ⚫ Дополнительный статус-светодиод подключается к светодиоду последней рюмки, активируется в STATUS_LED

   ⚫ В сервис режиме при наличии рюмки на дисплее отображается номер рюмки и положение крана для этой рюмки в градусах.
      Чтобы изменить положение крана над определённой рюмкой нужно поставить рюмку, подстроить угол энкодером и убрать рюмку.
      После выхода из сервисного режима зажатием кнопки энкодера, все углы сохранятся в постоянной памяти.
   - возможность настроить объём для каждой рюмки отдельно:
      При однократном нажатии на энкодер подсвечивается место рюмки, объём которой изменяется вращением энкодера. При повторном нажатии подсвечивается следующая рюмка.
      Если же ни одна рюмка не подсвечивается белым, вращение энкодера изменяет объём для всех рюмок одновременно (как в обычном ручном режиме)



   ₽$€ На чашечку кофе автору:        http://paypal.me/vicler

   ★ Исходники на GitHub:             https://github.com/VICLER/GyverDrink

   ★ Оригинальный проект AlexGyver:   https://alexgyver.ru/GyverDrink/
*/

//╞═════════════════════════════╡LIBS╞═════════════════════════════╡

#include "src/GyverTM1637/GyverTM1637.h"
#include "src/ServoSmooth/ServoSmooth.h"
#include "src/StepMot/src/StepMot.h"
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

encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);      // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)
StepMot stepper(STEPS_PER_REVOLUTION * MICROSTEPS, STEPPER_STEP, STEPPER_DIR, STEPPER_EN);
LEDdata leds[NUM_SHOTS + statusLed];                      // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + statusLed, LED_PIN);     // объект лента
GyverTM1637 disp(DISP_CLK, DISP_DIO);

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
uint8_t shotPos[] = {SHOT_POSITIONS};
const byte SW_pins[] = {SW_PINS};
float volumeTick = 15.0f * 50.0f / time50ml;  // volume per one FLOWdebouce timer tick
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool volumeChanged = false;
bool parking = false;
bool atHome = false;
bool LEDbreathing = false;

//╞═════════════════════════════╡MACROS╞═════════════════════════════╡

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
