#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class OneWireHal
{
public:
    struct Config
    {
        PIO pio;
        uint pin;
    };

    OneWireHal(const OneWireHal&) = delete;
    OneWireHal& operator=(const OneWireHal&) = delete;

    explicit OneWireHal(const Config &config);
    ~OneWireHal();

    bool init();
    bool reset();
    void write_byte(uint8_t byte);
    uint8_t read_byte();
    static uint32_t crc8(const uint32_t *data, size_t len);

private:
    Config m_config;
    int m_offset;
    int m_sm;
    SemaphoreHandle_t m_mutex;
    bool m_initialized;

    void program_jmp(uint label_offset);
};