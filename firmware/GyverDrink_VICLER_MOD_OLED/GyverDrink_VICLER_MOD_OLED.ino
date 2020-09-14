//GyverDrink VICLER_MOD_OLED v2.3
//15.09.2020
/*
  Модифицированная версия прошивки к проекту "Наливатор by AlexGyver" с OLED дисплеем и расширенным функционалом

   ⚠ Все настройки в файле Config.h ⚠

   ⚫ OLED дисплей на контроллере SSD1306 (0.96") или SH1106(1.3"). Дисплей подключается контактами SCK и SDA к пинам А5, А4 Arduino Nano

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

   ⚫ Индикация заряда аккумулятора и слежение за напряжением
        Активируется раскоментированием строки #define BATTERY_PIN ...
        При напряжении, меньшем чем BATTERY_LOW, устройство перестаёт реагировать на внешние воздействия и на дисплее мигает иконка батареи
        Если при первом включении мигает иконка аккумулятора, но напряжение в порядке(больше, чем BATTERY_LOW) -> нажмите и удерживайте кнопку до появления меню калибровки аккумулятора

   ⚫ Дополнительный статус-светодиод подключается к светодиоду последней рюмки, активируется в STATUS_LED

   ⚫ Калибровка объёма за единицу времени, калибровка углов сервопривода для рюмок и калибровка напряжения аккумулятора осуществляется в меню. Напряжение подстраивается энкодером до измеренного

   ⚫ Возможность изменять настройки прошивки прямо в меню. Вход в меню зажатием основной кнопки. Выход либо выбором соответствующего пункта, либо так же зажатием основной кнопки

   ⚫ Ведение статистики. Количество налитых рюмок и общий объём. При нажатии на кнопку энкодера, соответствующее значение сбрасывается



   ₽$€ На чашечку кофе автору:        http://paypal.me/vicler

   ★ Исходники на GitHub:             https://github.com/VICLER/GyverDrink

   ★ Оригинальный проект AlexGyver:   https://alexgyver.ru/GyverDrink/
*/

//╞═════════════════════════════╡LIBS╞═════════════════════════════╡

#include "src/SSD1306Ascii/src/SSD1306AsciiAvrI2c.h"
#include "src/SSD1306Ascii/src/SSD1306Ascii.h"
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
LEDdata leds[NUM_SHOTS + statusLed];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_SHOTS + statusLed, LED_PIN); // объект лента
SSD1306AsciiAvrI2c disp;
ServoSmooth servo;
encMinim enc(ENC_CLK, ENC_DT, ENC_SW, 1, 1);  // пин clk, пин dt, пин sw, направление (0/1), тип (0/1)

buttonMinim btn(BTN_PIN);
buttonMinim encBtn(ENC_SW);
timerMinim LEDtimer(30);
timerMinim FLOWdebounce(15);
timerMinim FLOWtimer(2000);
timerMinim WAITtimer(500);
timerMinim TIMEOUTtimer(STBY_TIME * 1000L); // таймаут режима ожидания
timerMinim POWEROFFtimer(TIMEOUT_OFF * 60000L);
timerMinim KEEP_POWERtimer(KEEP_POWER * 1000L);

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
enum serviceModes {VOLUME = 1, SERVO, BATTERY};
serviceModes serviceMode;
enum workModes {ManualMode, AutoMode};
workModes workMode;
uint16_t time50ml = 0;
#define INIT_VOLUME 47
uint8_t thisVolume = INIT_VOLUME;
uint8_t shotVolume[NUM_SHOTS];
uint8_t initShotPos[] = {SHOT_POSITIONS};
uint8_t shotPos[] = {SHOT_POSITIONS};
const byte SW_pins[] = {SW_PINS};
float volumeTick = 15.0f * 50.0f / time50ml;
float volumeCount = 0.0f;
bool systemON = false;
bool timeoutState = false;
bool parking = true;
bool LEDbreathingState = false;
float battery_voltage = 4.2;
float battery_cal = BATTERY_CAL;
uint16_t shots_overall = 0, volume_overall = 0;
bool keepPowerState = false;
bool volumeChanged = false;
bool invertState = 0;
char bootscreen[] = {"Der Наливатор"}; // текст при старте устройства
bool servoReady = 0;

enum {
  timeout_off = 0,
  inverse_servo,
  servo_speed,
  auto_parking,
  stby_time,
  stby_light,
  rainbow_flow,
  max_volume,
  keep_power,
  invert_display,
  parking_pos
};
uint8_t settingsList[] = {
  TIMEOUT_OFF,
  INVERSE_SERVO,
  SERVO_SPEED,
  AUTO_PARKING,
  STBY_TIME,
  STBY_LIGHT,
  RAINBOW_FLOW,
  MAX_VOLUME,
  KEEP_POWER,
  INVERT_DISPLAY,
  PARKING_POS
};

struct EEPROMAddress {
  const uint16_t _thisVolume = 0;
  const uint16_t _time50ml = sizeof(thisVolume);
  const uint16_t _shotPos = _time50ml + sizeof(time50ml);
  const uint16_t _battery_cal = _shotPos + sizeof(byte) * NUM_SHOTS;
  const uint16_t _timeout_off = _battery_cal + sizeof(battery_cal);
  const uint16_t _inverse_servo = _timeout_off + sizeof(settingsList[timeout_off]);
  const uint16_t _parking_pos = _inverse_servo + sizeof(settingsList[inverse_servo]);
  const uint16_t _auto_parking = _parking_pos + sizeof(settingsList[parking_pos]);
  const uint16_t _stby_time = _auto_parking + sizeof(settingsList[auto_parking]);
  const uint16_t _stby_light = _stby_time + sizeof(settingsList[stby_time]);
  const uint16_t _rainbow_flow = _stby_light + sizeof(settingsList[stby_light]);
  const uint16_t _max_volume = _rainbow_flow + sizeof(settingsList[rainbow_flow]);
  const uint16_t _shots_overall = _max_volume + sizeof(settingsList[max_volume]);
  const uint16_t _volume_overall = _shots_overall + sizeof(shots_overall);
  const uint16_t _keep_power = _volume_overall + sizeof(volume_overall);
  const uint16_t _invert_display = _keep_power + sizeof(keep_power);
  const uint16_t _servo_speed = _invert_display + sizeof(invert_display);
  const uint16_t _mode = _servo_speed + sizeof(servo_speed);
} eeAddress;

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
