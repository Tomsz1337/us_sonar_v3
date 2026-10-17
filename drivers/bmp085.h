#pragma once

#include <cstdint>
#include <cmath>
#include "hal/i2c_hal.h"
#include "FreeRTOS.h"
#include "task.h"

class Bmp085
{
public:
    enum class Mode : uint8_t
    {
        LowPower            = 0, // Czas konwersji ~4.5ms
        Standard            = 1, // Czas konwersji ~7.5ms
        HighResolution      = 2, // Czas konwersji ~13.5ms
        UltraHighResolution = 3  // Czas konwersji ~25.5ms
    };

    struct CalibrationData
    {
        int16_t ac1, ac2, ac3;
        uint16_t ac4, ac5, ac6;
        int16_t b1, b2;
        int16_t mb, mc, md;
    };

    explicit Bmp085(I2CHal &i2c, uint8_t devAddr = 0x77);
    bool init(Mode mode = Mode::Standard);
    float readPressure();

private:
    static constexpr uint8_t CHIP_ID         = 0x55;

    static constexpr uint8_t REG_CHIP_ID     = 0xD0;
    static constexpr uint8_t REG_CONTROL     = 0xF4;
    static constexpr uint8_t REG_RESULT      = 0xF6;
    static constexpr uint8_t REG_CALIB_START = 0xAA;
    
    static constexpr uint8_t CMD_READ_TEMP   = 0x2E;
    static constexpr uint8_t CMD_READ_PRESS  = 0x34;

    I2CHal &m_i2c;
    uint8_t m_devAddr;
    Mode m_mode;
    CalibrationData m_calib;
    bool m_initialized;

    bool readCalibration();
    int32_t calculateB5(int32_t ut);
    bool readRawTemperature(int32_t &ut);
    bool readRawPressure(int32_t &up);

    bool readInt16(uint8_t reg, int16_t &value);
    bool readUint16(uint8_t reg, uint16_t &value);
};