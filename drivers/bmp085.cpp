#include "bmp085.h"

Bmp085::Bmp085(I2CHal &i2c, uint8_t devAddr)
    : m_i2c(i2c), m_devAddr(devAddr), m_mode(Mode::Standard), m_calib{}, m_initialized(false)
{
}

bool Bmp085::init(Mode mode)
{
    m_mode = mode;
    uint8_t chipId;
    m_i2c.write(m_devAddr, &REG_CHIP_ID, 1, true);
    m_i2c.read(m_devAddr, &chipId, 1, false);

    if(chipId != CHIP_ID) {return false;}
    if(!readCalibration()) {return false;}

    m_initialized = true;
    return true;
}

bool Bmp085::readInt16(uint8_t reg, int16_t &value)
{
    uint8_t buf[2];
    m_i2c.write(m_devAddr, &reg, 1, true);
    m_i2c.read(m_devAddr, buf, 2, false);
    value = static_cast<int16_t>((buf[0] << 8) | buf[1]);
    return true;
}

bool Bmp085::readUint16(uint8_t reg, uint16_t &value)
{
    uint8_t buf[2];
    m_i2c.write(m_devAddr, &reg, 1, true);
    m_i2c.read(m_devAddr, buf, 2, false);
    value = static_cast<uint16_t>((buf[0] << 8) | buf[1]);
    return true;
}

bool Bmp085::readCalibration()
{
    bool ok = true;
    ok &= readInt16(0xAA, m_calib.ac1);
    ok &= readInt16(0xAC, m_calib.ac2);
    ok &= readInt16(0xAE, m_calib.ac3);
    ok &= readUint16(0xB0, m_calib.ac4);
    ok &= readUint16(0xB2, m_calib.ac5);
    ok &= readUint16(0xB4, m_calib.ac6);
    ok &= readInt16(0xB6, m_calib.b1);
    ok &= readInt16(0xB8, m_calib.b2);
    ok &= readInt16(0xBA, m_calib.mb);
    ok &= readInt16(0xBC, m_calib.mc);
    ok &= readInt16(0xBE, m_calib.md);
    return ok;
}

bool Bmp085::readRawTemperature(int32_t &ut)
{
    uint8_t cmd[2] = {REG_CONTROL, CMD_READ_TEMP};
    m_i2c.write(m_devAddr, cmd, 2, false);

    vTaskDelay(pdMS_TO_TICKS(5));

    uint8_t buf[2];
    m_i2c.write(m_devAddr, &REG_RESULT, 1, true);
    m_i2c.read(m_devAddr, buf, 2, false);

    ut = (buf[0] << 8) | buf[1];
    return true;
}

bool Bmp085::readRawPressure(int32_t &up)
{
    uint8_t oss = static_cast<uint8_t>(m_mode);
    uint8_t cmd[2] = {REG_CONTROL, static_cast<uint8_t>(CMD_READ_PRESS + (oss << 6))};
    m_i2c.write(m_devAddr, cmd, 2, false);

    static constexpr uint8_t delays_ms[] = {5, 8, 14, 26};
    vTaskDelay(pdMS_TO_TICKS(delays_ms[oss]));

    uint8_t buf[3];
    m_i2c.write(m_devAddr, &REG_RESULT, 1, true);
    m_i2c.read(m_devAddr, buf, 3, false);

    up = ((static_cast<uint32_t>(buf[0]) << 16) |
          (static_cast<uint32_t>(buf[1]) << 8)  |
           static_cast<uint32_t>(buf[2])) >> (8 - oss);

    return true;
}

int32_t Bmp085::calculateB5(int32_t ut)
{
    int32_t x1 = ((ut - m_calib.ac6) * m_calib.ac5) >> 15;
    int32_t x2 = (m_calib.mc << 11) / (x1 + m_calib.md);
    return x1 + x2;
}

float Bmp085::readPressure()
{
    if (!m_initialized)
    {
        return false;
    }

    int32_t ut = 0;
    int32_t up = 0;

    if (!readRawTemperature(ut) || !readRawPressure(up))
    {
        return false;
    }

    int32_t b5 = calculateB5(ut);
   
    uint8_t oss = static_cast<uint8_t>(m_mode);
    int32_t b6 = b5 - 4000;
    int32_t x1 = (m_calib.b2 * ((b6 * b6) >> 12)) >> 11;
    int32_t x2 = (m_calib.ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((static_cast<int32_t>(m_calib.ac1) * 4 + x3) << oss) + 2) >> 2;

    x1 = (m_calib.ac3 * b6) >> 13;
    x2 = (m_calib.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (m_calib.ac4 * static_cast<uint32_t>(x3 + 32768)) >> 15;
    uint32_t b7 = (static_cast<uint32_t>(up) - b3) * (50000 >> oss);

    int32_t p = 0;
    if (b7 < 0x80000000)
    {
        p = (b7 * 2) / b4;
    }
    else
    {
        p = (b7 / b4) * 2;
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
     
    return p / 100.0f;;
}
