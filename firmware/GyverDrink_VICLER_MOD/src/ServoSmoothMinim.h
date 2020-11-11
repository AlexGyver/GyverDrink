
//#define USE_TICOSERVO

#ifdef USE_TICOSERVO
#include "Adafruit_TiCoServo/Adafruit_TiCoServo.h"
#else
#include <Servo.h>
#endif

#define SS_SERVO_PERIOD 10 // период работы tick(), мс

#define angleToUs(x) ((x + 52.759) * 10.3111)
#define usToAngle(x) ((x - 544) * 0.09699)

class ServoSmoothMinim
{
public:
    void write(byte angle);                 // аналог метода из библиотеки Servo
    void attach(byte pin, byte target = 0, uint16_t min_us = 544, uint16_t max_us = 2400); // аналог метода из библиотеки Servo
    void detach();                          // аналог метода из библиотеки Servo
    void start();                           // attach + разрешает работу tick
    void stop();                            // detach + запрещает работу tick
    void setSpeed(byte speed);              // установка максимальной скорости (условные единицы, 0 - 200)
    void setTargetDeg(byte target);         // установка целевой позиции в градусах (0-макс. угол).
    void setDirection(bool _dir);           // смена направления поворота
    byte getCurrentDeg();                   // получение текущей позиции в градусах (0-макс. угол).
    byte getTargetDeg();                    // получение целевой позиции в градусах (0-макс. угол).
    boolean tickManual();                   // метод, управляющий сервой, без встроенного таймера.
                                            // Возвращает true, когда целевая позиция достигнута
    boolean tick();                         // метод, управляющий сервой, должен опрашиваться как можно чаще.
                                            // Возвращает true, когда целевая позиция достигнута.
                                            // Имеет встроенный таймер с периодом SS_SERVO_PERIOD
#ifdef USE_TICOSERVO
    Adafruit_TiCoServo _servo;
#else
    Servo _servo;
#endif
    

private:
    int _servoCurrentPos = 0;
    int _servoTargetPos = 0;
    byte _pin, _servoTargetDeg = 0;
    int _servoMaxSpeed = 25;
    int _newSpeed = 0;
    bool _servoState = true;
    bool _dir = 0;
    uint16_t _min_us, _max_us;
};

void ServoSmoothMinim::write(byte angle)
{
    if(_dir) _servo.writeMicroseconds(_min_us + _max_us - angleToUs(angle)); // 2400 - 544
    else _servo.writeMicroseconds(angleToUs(angle)); // 544 - 2400
}

void ServoSmoothMinim::attach(byte pin, byte target, uint16_t min_us, uint16_t max_us)
{
    _pin = pin;
    _min_us = min_us;
    _max_us = max_us;
    _servo.attach(_pin, _min_us, _max_us);
    write(target);
    _servoTargetPos = angleToUs(target);
    _servoCurrentPos = _servoTargetPos;
}

void ServoSmoothMinim::detach()
{
    _servo.detach();
}

void ServoSmoothMinim::start()
{
    _servo.attach(_pin, _min_us, _max_us);
    _servoState = true;
}

void ServoSmoothMinim::stop()
{
    _servo.detach();
    _servoState = false;
}

void ServoSmoothMinim::setSpeed(byte speed)
{
    _servoMaxSpeed = speed * 0.25;
}

void ServoSmoothMinim::setTargetDeg(byte target)
{
    _servoTargetDeg = target;
    _servoTargetPos = angleToUs(target);
}

void ServoSmoothMinim::setDirection(bool dir)
{
    if(_dir != dir) _servoCurrentPos = _min_us + _max_us - _servoCurrentPos;
    _dir = dir;
}

byte ServoSmoothMinim::getCurrentDeg()
{
    return usToAngle(_servoCurrentPos);
}

byte ServoSmoothMinim::getTargetDeg()
{
    return _servoTargetDeg;
}

boolean ServoSmoothMinim::tickManual()
{
    _newSpeed = _servoTargetPos - _servoCurrentPos; // расчёт скорости
    if (_servoState)
    {
        _newSpeed = constrain(_newSpeed, -_servoMaxSpeed, _servoMaxSpeed); // ограничиваем по макс.
        _servoCurrentPos += _newSpeed;

        if(_dir) _servo.writeMicroseconds(_min_us + _max_us - _servoCurrentPos);
        else _servo.writeMicroseconds(_servoCurrentPos);
    }
    if (_servoCurrentPos == _servoTargetPos) _servoState = false;
    else _servoState = true;

    return !_servoState;
}

boolean ServoSmoothMinim::tick()
{
    static uint32_t _prevServoTime = 0;
    if (millis() - _prevServoTime >= SS_SERVO_PERIOD)
    {
        _prevServoTime = millis();
        ServoSmoothMinim::tickManual();
    }
    return !_servoState;
}