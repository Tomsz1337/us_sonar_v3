#include "hal/pulse_gen.h"
#include "hardware/clocks.h"

PulseGenHal::PulseGenHal(const Config &config)
    : m_config(config), m_sm(-1), m_offset(-1), m_mutex(nullptr), m_initialized(false)
{
}

PulseGenHal::~PulseGenHal()
{
    if(m_sm >= 0)
    {
        pio_sm_set_enabled(m_config.pio, m_sm, false);
        pio_sm_unclaim(m_config.pio, m_sm);
    }

    if(m_offset >= 0)
    {
        pio_remove_program(m_config.pio, &pulse_gen_program, m_offset);
    }

    if(m_mutex != nullptr)
    {
        vSemaphoreDelete(m_mutex);
    }
}

bool PulseGenHal::init()
{
    if(m_config.freqHz == 0 || m_config.nPulses == 0)
    {
        return false;
    }

    m_mutex = xSemaphoreCreateMutex();
    if(m_mutex == nullptr)
    {
        return false;
    }

    if(!pio_can_add_program(m_config.pio, &pulse_gen_program))
    {
        return false;
    }

    m_offset = pio_add_program(m_config.pio, &pulse_gen_program);
    m_sm = pio_claim_unused_sm(m_config.pio, false);
    if(m_sm < 0)
    {
        return false;
    }

    float div = static_cast<float>(clock_get_hz(clk_sys)) / static_cast<float>(m_config.freqHz) / 4.0f;

    pio_sm_config c = pulse_gen_program_get_default_config(m_offset);

    pio_gpio_init(m_config.pio, m_config.pin);
    sm_config_set_set_pins(&c, m_config.pin, 1);
    pio_sm_set_consecutive_pindirs(m_config.pio, m_sm, m_config.pin, 1, true);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(m_config.pio, m_sm, m_offset, &c);

    m_initialized = true;
    return true;
}

bool PulseGenHal::start()
{
    if(!m_initialized)
    {
        return false;
    }

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked())
    {
        return false;
    }

    pio_sm_set_enabled(m_config.pio, m_sm, false);
    pio_sm_clear_fifos(m_config.pio, m_sm);
    pio_sm_restart(m_config.pio, m_sm);
    pio_sm_put_blocking(m_config.pio, m_sm, m_config.nPulses - 1);
    pio_sm_set_enabled(m_config.pio, m_sm, true);

    return true;
}