#pragma once

#include <Arduino.h>
#include "pins_arduino.h"

class TwoWire {
public:
    void begin(void);
    void setClock(uint32_t clock);
    void beginTransmission(uint8_t address);
    uint8_t endTransmission(bool stop);
    uint8_t endTransmission(void);
    void write(uint8_t data);
    void requestFrom(uint8_t address, uint8_t lenght, bool stop);
    void requestFrom(uint8_t address, uint8_t lenght);
    uint8_t read(void);
    uint8_t available(void);
private:
    uint8_t _requested_bytes = 0;
    bool _address_nack = false;
    bool _data_nack = false;
    bool _stop_after_request = true;
    void start(void);
    void stop(void);
};
extern TwoWire Wire;