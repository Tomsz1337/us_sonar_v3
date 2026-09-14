#pragma once

#include "hal/onewire_hal.h"
#include <cstdint>

class DS18B20
{
public:

    explicit DS18B20(OneWireHal &onewire);
    float read_temp();

private:

    static constexpr uint8_t SKIP_ROM           = 0xCC;
    static constexpr uint8_t CONVERT_T          = 0x44;
    static constexpr uint8_t READ_SCRATCHPAD    = 0xBE;

    OneWireHal &m_onewire;
};
