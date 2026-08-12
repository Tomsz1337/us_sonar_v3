#pragma once

#include <cstdint>
#include "hal/i2c_hal.h"
#include "FreeRTOS.h"
#include "task.h"

class HDC2080
{
public:
    
    explicit HDC2080(I2CHal &i2c, uint8_t devAddr = 0x40);
    bool init();
    float readHumidity();
    float readTemperature();

private:

    static constexpr uint8_t REG_MANUFACTURER_ID    0xFC;
    static constexpr uint8_t REG_DEVICE_ID          0xFE;

    I2CHal &m_i2c;
    uint8_t m_devAddr;
    bool m_initialized;
};