// мини-класс для работы с кнопкой, версия 1.1

#pragma pack(push,1)
typedef struct {
  bool holdedFlag: 1;
  bool btnFlag: 1;
  bool pressF: 1;
  bool clickF: 1;
  bool holdF: 1;
  bool holdedFor: 1;
} buttonMinimFlags;
#pragma pack(pop)

class buttonMinim {
  public:
    buttonMinim(uint8_t pin);
    boolean pressed();
    boolean clicked();
    boolean holding();
    boolean holded();
    boolean holdedFor(uint8_t sec);
  private:
    buttonMinimFlags flags;
    void tick();
    uint32_t _btnTimer;
    byte _pin;
};

buttonMinim::buttonMinim(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  _pin = pin;
}

void buttonMinim::tick() {
  boolean btnState = digitalRead(_pin);
  if (!btnState && !flags.btnFlag && ((uint32_t)millis() - _btnTimer > 90)) {
    flags.btnFlag = true;
    _btnTimer = millis();
    flags.pressF = true;
    flags.holdedFlag = true;
    flags.holdedFor = true;
  }
  if (btnState && flags.btnFlag && ((uint32_t)millis() - _btnTimer < 500) && ((uint32_t)millis() - _btnTimer > 100))
  {
    flags.btnFlag = false;
    _btnTimer = millis();
    flags.clickF = true;
    flags.holdF = false;
  }
  if (flags.btnFlag && ((uint32_t)millis() - _btnTimer > 500)) {
    if (!btnState) {
      flags.holdF = true;
    } else {
      flags.btnFlag = false;
      flags.holdF = false;
      _btnTimer = millis();
    }
  }
}

boolean buttonMinim::pressed() {
  buttonMinim::tick();
  if (flags.pressF) {
    flags.pressF = false;
    return true;
  }
  else return false;
}

boolean buttonMinim::clicked() {
  buttonMinim::tick();
  if (flags.clickF) {
    flags.clickF = false;
    return true;
  }
  else return false;
}

boolean buttonMinim::holding() {
  buttonMinim::tick();
  if (flags.holdF) {
    return true;
  }
  else return false;
}

boolean buttonMinim::holded() {
  buttonMinim::tick();
  if (flags.holdF && flags.holdedFlag) {
    flags.holdedFlag = false;
    flags.pressF = false;
    return true;
  }
  else return false;
}

boolean buttonMinim::holdedFor(uint8_t sec) {
  buttonMinim::tick();
  if (flags.holdF && (millis() - _btnTimer > sec * 1000) && flags.holdedFor) {
    flags.holdedFor = false;
    return true;
  }
  else return false;
}
