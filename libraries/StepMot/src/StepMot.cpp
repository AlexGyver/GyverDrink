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

void StepMot::setRPM(float rpm) {
  if (rpm > 0)
    _stepPeriod = 1000000.0 / (_stepsPerRevolution * rpm / 60.0);
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
  if (_mode == ABSOLUTE) {
    if (newAngle > StepMot::getAngle()) {
      _targetAngle = newAngle - StepMot::getAngle();
      StepMot::setDir(CW);
    }
    else if (newAngle < StepMot::getAngle()) {
      _targetAngle = StepMot::getAngle() - newAngle;
      StepMot::setDir(CCW);
    }
    else {
      _targetAngle = 0;
    }
    _targetSteps = round(_targetAngle * _stepsPerAngle);
  }
  else if (_mode == RELATIVE) {
    _targetSteps = round(abs(newAngle * _stepsPerAngle));
    if (newAngle > 0) StepMot::setDir(CW);
    else if (newAngle < 0) StepMot::setDir(CCW);
  }
  if (_targetSteps > 0) _ready = 0;
}

void StepMot::rotate(bool dir) {
  StepMot::setDir(dir);
  _ready = 0;
}

float StepMot::getAngle() {
  return _currentSteps * _anglePerStep; // current Angle
}

void StepMot::resetPos() {
  _targetSteps = 0;
  _currentSteps = 0;
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
  if (_ready) return 0;

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
