#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class AdcDmaHal
{
public:
    struct Config
    {
        uint adc_channel;   
        uint pin;      
        float sample_rate;  
    };

    AdcDmaHal(const AdcDmaHal&) = delete;
    AdcDmaHal& operator=(const AdcDmaHal&) = delete;

    explicit AdcDmaHal(const Config &config);
    ~AdcDmaHal();

    bool init();
    bool start(uint16_t *buffer, size_t count);
    bool wait(TickType_t timeout_ticks = portMAX_DELAY);
    bool read_blocking(uint16_t *buffer, size_t count, TickType_t timeout_ticks = portMAX_DELAY);

private:
    static void dma_irq_handler();
    static AdcDmaHal *s_instance;

    Config m_config;
    int m_dma_chan;
    dma_channel_config m_dma_cfg;
    
    SemaphoreHandle_t m_mutex;
    SemaphoreHandle_t m_completion_sem;
    
    bool m_initialized;
    bool m_busy;
};