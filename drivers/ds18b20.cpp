#include "ds18b20.h"

DS18B20::DS18B20(OneWireHal &onewire)
    : m_onewire(onewire)
{
}

float DS18B20::read_temp()
{
    uint32_t data[9];

    if(!m_onewire.reset()) {return -999.0f;}

    m_onewire.write_byte(SKIP_ROM);
    m_onewire.write_byte(CONVERT_T);
    vTaskDelay(pdMS_TO_TICKS(750));
    m_onewire.reset();
    m_onewire.write_byte(SKIP_ROM);
    m_onewire.write_byte(READ_SCRATCHPAD);

    for(int i = 0; i < 9; i++)
    {
        data[i] = m_onewire.read_byte();
    }
    if(m_onewire.crc8(data, 8) != data[8]) {return -888.0f;}
    int16_t raw = static_cast<int16_t>((data[1] << 8) | data[0]);
    return static_cast<float>(raw) / 16.0f;
}