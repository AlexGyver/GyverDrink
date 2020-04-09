#include "StepMot.h"

StepMot::StepMot(float steps_per_revolution, uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin):
  _stepsPerRevolution(steps_per_revolution),
  _stepPin(step_pin),
  _dirPin(dir_pin),
  _enPin(en_pin),
  _anglePerStep(360.0 / _stepsPerRevolution),
  _stepsPerAngle(_stepsPerRevolution / 360.0)
{
  pinMode(_stepPin, OUTPUT);
  pinMode(_dirPin, OUTPUT);
  pinMode(_enPin, OUTPUT);

  digitalWrite(_enPin, 1);
  if (_inverted) digitalWrite(_dirPin, 0);
  else digitalWrite(_dirPin, 1);
  digitalWrite(_stepPin, 0);
}

void StepMot::setMode(bool mode) {
  _mode = mode;
}

void StepMot::autoPower(bool status) {
  _autoPower = status;
}

void StepMot::setBacklash(float angle){
  _backlash = angle;
}

void StepMot::setRPM(float rpm) {
  if(rpm == 0){
    _stop = true;
    if (_autoPower && _enabled) StepMot::disable();
    return;
  }
  else if (rpm > 0)  StepMot::setDir(CW);
  else if (rpm < 0)  StepMot::setDir(CCW);
  _stepPeriod = 1000000.0 / (_stepsPerRevolution * abs(rpm) / 60.0);
  if (_autoPower && !_enabled && !_ready)  StepMot::enable();
  _stop = false;
}

void StepMot::enable() {
  if (!_enabled) {
    digitalWrite(_enPin, 0);
    _enabled = 1;
  }
}

void StepMot::disable() {
  if (_enabled) {
    digitalWrite(_enPin, 1);
    _enabled = 0;
  }
}

void StepMot::setDir(bool dir) {
  _dir = dir;
  if (_inverted) digitalWrite(_dirPin, !_dir);
  else digitalWrite(_dirPin, _dir);
  _dir ? _stepCounter = 1 : _stepCounter = -1;
}

void StepMot::invertDir(bool invertState) {
  _inverted = invertState;
}

void StepMot::setSteps(uint32_t steps) {
  _targetSteps = steps;
  _ready = 0;
}

void StepMot::setAngle(float newAngle) {
  _currentAngle = _currentSteps * _anglePerStep;

  if (_mode == RELATIVE)
  {
    if(newAngle == 0) return;
    else newAngle = _lastAngle + newAngle;
  }

  if (newAngle > _lastAngle) StepMot::setDir(CW);
  if (newAngle < _lastAngle) StepMot::setDir(CCW);

  if (_dir == CW)
  {
    _lastAngle = newAngle;
    newAngle += _backlash;
  }
  else
    _lastAngle = newAngle;

  _targetAngle = abs(newAngle - _currentAngle);
  _targetSteps = round(_targetAngle * _stepsPerAngle);

  if (_targetSteps > 0)
    _ready = 0;
}

void StepMot::rotate(bool dir) {
  StepMot::setDir(dir);
  _ready = 0;
}

void StepMot::rotate()
{
  _ready = 0;
}

float StepMot::getAngle() {
    return _currentAngle = _currentSteps * _anglePerStep;
}

void StepMot::resetPos(float pos = 0.0) {
  _currentAngle = pos;
  _currentSteps = pos * _stepsPerAngle;
  _targetSteps = 0;
  _ready = 1;
}

void StepMot::step() {
  if (_autoPower && !_enabled) StepMot::enable();
  digitalWrite(_stepPin, HIGH);
  digitalWrite(_stepPin, LOW);
  _currentSteps += _stepCounter;
}

bool StepMot::ready() {
  return _ready;
}

bool StepMot::update() {
  if (_ready || _stop) return 0;

  if (micros() - _prevStepTime >= _stepPeriod) {
    _prevStepTime = micros();

    StepMot::step();

    if (_targetSteps) {
      _targetSteps--;
      if (_targetSteps == 0) {
        if (_autoPower && _enabled) StepMot::disable();
        _ready = 1;
      }
      return 1;
    }
  }
  return !_ready;
}
