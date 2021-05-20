#include "microWire.h"

void TwoWire::begin()
{
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    TWBR = 72;
    TWSR = 0;
}

void TwoWire::setClock(uint32_t clock)
{
    TWBR = (((long)F_CPU / clock) - 16) / 2;
}

void TwoWire::beginTransmission(uint8_t address)
{
    TwoWire::start();
    TwoWire::write(address << 1);
}

uint8_t TwoWire::endTransmission(void)
{
    return TwoWire::endTransmission(true);
}

uint8_t TwoWire::endTransmission(bool stop)
{
    if(stop) TwoWire::stop();
    else TwoWire::start();
    if(_address_nack){
        _address_nack = false;
        _data_nack = false;
        return 2;
    } if(_data_nack) {
        _address_nack = false;
        _data_nack = false;
        return 3;
    } return 0;
}

void TwoWire::write(uint8_t data)
{
    TWDR = data;
    TWCR = _BV(TWEN) | _BV(TWINT);
    while(!(TWCR & _BV(TWINT)));
    uint8_t _bus_status = TWSR & 0xF8;
    if(_bus_status == 0x20) _address_nack = true;
    if(_bus_status == 0x30) _data_nack = true;
}

uint8_t TwoWire::available()
{
    return _requested_bytes;
}

uint8_t TwoWire::read()
{
    if(--_requested_bytes){
        TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
        while(!(TWCR & _BV(TWINT)));
        return TWDR;
    }
    _requested_bytes = 0;
    TWCR = _BV(TWEN) | _BV(TWINT);
    while(!(TWCR & _BV(TWINT)));
    if(_stop_after_request) TwoWire::stop();
    else TwoWire::start();
    return TWDR;
}

void TwoWire::requestFrom(uint8_t address, uint8_t lenght)
{
    TwoWire::requestFrom(address, lenght, true);
}

void TwoWire::requestFrom(uint8_t address, uint8_t lenght, bool stop)
{
    _stop_after_request = stop;
    _requested_bytes = lenght;
    TwoWire::start();
    TwoWire::write((address << 1) | 0x01);
}

void TwoWire::start()
{
    TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT);
    while(!(TWCR & _BV(TWINT)));
}

void TwoWire::stop()
{
    TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);
}

TwoWire Wire = TwoWire();
