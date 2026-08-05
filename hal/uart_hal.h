#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class UARTHal
{
public:
    struct Config
    {
        uart_inst_t *uart;
        uint tx_pin;
        uint rx_pin;
        uint baudrate; 
        uint data_bits = 8;
        uint stop_bits = 1;
        uart_parity_t parity = UART_PARITY_NONE;
    };

    UARTHal(const UARTHal&) = delete;
    UARTHal& operator=(const UARTHal&) = delete;

    explicit UARTHal(const Config &config);
    ~UARTHal();

    bool init();
    bool is_readable();
    bool read_byte(uint8_t &byte);
    int write(const uint8_t *data, size_t length);

private:
    Config m_config;
    SemaphoreHandle_t m_tx_mutex;
    SemaphoreHandle_t m_rx_mutex;
    bool m_initialized;
};