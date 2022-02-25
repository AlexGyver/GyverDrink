class glassButton ()
{
  int8_t _pin;
  public:
    glassButton (int8_t pin): _pin (pin) 
    {
      pinMode (pin, INPUT_PULLUP);
    }

    boolean getMode ()
    {
      boolean btnState = digitalRead(_pin);
      return !btnState;
    }
};
