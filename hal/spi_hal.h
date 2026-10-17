#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class SPIHal
{
public:
    struct Config
    {
        spi_inst_t *spi;   
        uint miso_pin;
        uint mosi_pin;
        uint csn_pin;
        uint sck_pin;         
        spi_cpha_t cpha;               
        spi_cpol_t cpol;               
        spi_order_t order;               
        uint32_t data_bits;         
        uint32_t baudrate;         
    };

    SPIHal(const SPIHal&) = delete;
    SPIHal& operator=(const SPIHal&) = delete;

    explicit SPIHal(const Config &config);
    ~SPIHal();

    bool init();
    int write(const uint8_t *tx_data, size_t length);
    int read(const uint8_t *tx_data, uint8_t *rx_data, size_t length);
    int read(uint8_t repeated_tx_data, uint8_t *rx_data, size_t length);

private:
    Config m_config;
    SemaphoreHandle_t m_mutex;
    bool m_initialized;
};