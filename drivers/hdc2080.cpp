#include "hdc2080.h"

HDC2080::HDC2080(I2CHal &i2c, uint8_t devAddr)
    : m_i2c(i2c), m_devAddr(devAddr), m_initialized(false)
{
}

bool HDC2080::init()
{
    uint8_t chipId;
    m_i2c.write(m_devAddr, &REG_DEVICE_ID, 1, true);
    m_i2c.read(m_devAddr, &chipId, 1, false);
    if(chipId != REG_DEVICE_ID) {return false;}
    m_initialized = true;
    return true;
}

float HDC2080::readHumidity()
{
    uint8_t data[2];
    uint8_t buf[2] = {0x0F, 0x01}; 

    m_i2c.write(m_devAddr, buf, 2, false);
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t reg = 0x02;
    m_i2c.write(m_devAddr, &reg, 1, true);
    m_i2c.read(m_devAddr, data, 2, false);

    uint16_t raw = data[0] | (data[1] << 8);
    return (static_cast<float>(raw) / 65536.0f) * 100.0f;
}

float HDC2080::readTemperature()
{
    uint8_t data[2];
    uint8_t buf[2] = {0x0F, 0x01}; 

    m_i2c.write(m_devAddr, buf, 2, false);
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t reg = 0x00;
    m_i2c.write(m_devAddr, &reg, 1, true);
    m_i2c.read(m_devAddr, data, 2, false);

    uint16_t raw = data[0] | (data[1] << 8);
    return (static_cast<float>(raw) / 65536.0f) * 165.0f - 40.0f;
}