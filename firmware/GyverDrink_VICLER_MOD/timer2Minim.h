// мини-класс таймера, версия 2.0
// использован улучшенный алгоритм таймера на millis
// алгоритм чуть медленнее, но обеспечивает кратные интервалы и защиту от пропусков и переполнений

class timerMinim
{
  public:
    timerMinim(uint32_t interval);				// объявление таймера с указанием интервала
    void setInterval(uint32_t interval);	// установка интервала работы таймера
    boolean isReady();                    // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    void reset();							            // ручной сброс таймера на установленный интервал
    void stop();
    void start();

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
    bool _stop = 0;
};

timerMinim::timerMinim(uint32_t interval) {
  _interval = interval;
  //_timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
  _interval = (interval == 0) ? 10 : interval;
}

void timerMinim::start() {
  if (_stop) {
    _stop = 0;
    _timer = millis();
  }
}

void timerMinim::stop() {
  _stop = 1;
}

// алгоритм таймера v2.0
boolean timerMinim::isReady() {
  if (_stop) return 0;
  if (millis() - _timer >= _interval) {
    _timer = millis();
    if (_timer < _interval) return false;          // переполнение uint32_t
    return true;
  }
  return false;
}

void timerMinim::reset() {
  _timer = millis();
}
