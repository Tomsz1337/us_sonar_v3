#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class I2CHal
{
public:
    struct Config
    {
        i2c_inst_t *i2c;
        uint baudrate;
        uint sda_pin;
        uint scl_pin;
    };

    I2CHal(const I2CHal&) = delete;
    I2CHal& operator=(const I2CHal&) = delete;

    explicit I2CHal(i2c_inst_t *i2c, uint baudrate, uint sda_pin, uint scl_pin);
    ~I2CHal();

    bool init();
    int read(uint8_t addr, uint8_t *dst, size_t len, bool nostop = false, uint32_t timeout_ms = 100);
    int write(uint8_t addr, const uint8_t *src, size_t len, bool nostop = false, uint32_t timeout_ms = 100);

private:
    Config m_config;
    SemaphoreHandle_t m_mutex;
    bool m_initialized;
};