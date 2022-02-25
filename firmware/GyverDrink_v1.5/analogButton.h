class glassButton ()
{
  int8_t _pin;
  public:
    glassButton (int8_t pin): _pin (pin) 
    {
      pinMode (pin, INPUT_PULLUP);  
    }

    boolean holding ()
    {
      auto btnStateRaw = analogRead(_pin);
      boolean btnState = btnStateRaw > 1000; // arduino uno only maybe 
      return !btnState;
    }
};
