#include "hal/adc_dma_hal.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"

AdcDmaHal* AdcDmaHal::s_instance = nullptr;

AdcDmaHal::AdcDmaHal(const Config &config)
    : m_config(config),
      m_dma_chan(-1),
      m_mutex(nullptr),
      m_completion_sem(nullptr),
      m_initialized(false),
      m_busy(false)
{
}

AdcDmaHal::~AdcDmaHal()
{
    if (m_dma_chan >= 0)
    {
        dma_channel_set_irq0_enabled(m_dma_chan, false);
        dma_channel_abort(m_dma_chan);
        dma_channel_unclaim(m_dma_chan);
    }

    if (m_completion_sem != nullptr)
    {
        vSemaphoreDelete(m_completion_sem);
    }

    if (m_mutex != nullptr)
    {
        vSemaphoreDelete(m_mutex);
    }

    if (s_instance == this)
    {
        s_instance = nullptr;
    }
}

void AdcDmaHal::dma_irq_handler()
{
    if (s_instance != nullptr && s_instance->m_dma_chan >= 0)
    {
        if (dma_hw->ints0 & (1u << s_instance->m_dma_chan))
        {
            dma_hw->ints0 = (1u << s_instance->m_dma_chan);
            BaseType_t higher_priority_task_woken = pdFALSE;
            xSemaphoreGiveFromISR(s_instance->m_completion_sem, &higher_priority_task_woken);
            portYIELD_FROM_ISR(higher_priority_task_woken);
        }
    }
}

bool AdcDmaHal::init()
{
    m_mutex = xSemaphoreCreateMutex();
    m_completion_sem = xSemaphoreCreateBinary();

    if (m_mutex == nullptr || m_completion_sem == nullptr)
    {
        return false;
    }

    m_dma_chan = dma_claim_unused_channel(false);
    if (m_dma_chan < 0)
    {
        return false;
    }

    adc_init();
    adc_gpio_init(m_config.pin);
    adc_fifo_setup(true, true, 1, false, false);

    m_dma_cfg = dma_channel_get_default_config(m_dma_chan);
    channel_config_set_transfer_data_size(&m_dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&m_dma_cfg, false);
    channel_config_set_write_increment(&m_dma_cfg, true);
    channel_config_set_dreq(&m_dma_cfg, DREQ_ADC);

    s_instance = this;
    dma_channel_set_irq0_enabled(m_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, AdcDmaHal::dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    m_initialized = true;
    return true;
}

bool AdcDmaHal::start(uint16_t *buffer, size_t count)
{
    if (!m_initialized || buffer == nullptr || count == 0)
    {
        return false;
    }

    LockGuard lock(m_mutex, portMAX_DELAY);
    if (!lock.isLocked() || m_busy)
    {
        return false;
    }

    adc_run(false);
    adc_fifo_drain();
    adc_select_input(m_config.adc_channel);

    float clkdiv = (48000000.0f / m_config.sample_rate) - 1.0f;
    if (clkdiv < 0.0f)
    {
        clkdiv = 0.0f;
    }
    adc_set_clkdiv(clkdiv);
    xSemaphoreTake(m_completion_sem, 0);
    m_busy = true;
    dma_channel_configure(
        m_dma_chan,
        &m_dma_cfg,
        buffer,
        &adc_hw->fifo,
        count,
        false
    );

    dma_channel_start(m_dma_chan);
    adc_run(true);

    return true;
}

bool AdcDmaHal::wait(TickType_t timeout_ticks)
{
    if (!m_initialized)
    {
        return false;
    }
    LockGuard lock(m_mutex, portMAX_DELAY);
    if (!m_busy)
    {
        return false;
    }

    bool success = (xSemaphoreTake(m_completion_sem, timeout_ticks) == pdTRUE);
    if (!success)
    {
        dma_channel_abort(m_dma_chan);
    }
    adc_run(false);
    adc_fifo_drain();
    m_busy = false;
    return success;
}

bool AdcDmaHal::read_blocking(uint16_t *buffer, size_t count, TickType_t timeout_ticks)
{
    if (!start(buffer, count))
    {
        return false;
    }
    return wait(timeout_ticks);
}