#include "hal/onewire_hal.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "onewire.pio.h"

OneWireHal::OneWireHal(const Config &config)
    : m_config(config), m_offset(-1), m_sm(-1), m_mutex(nullptr), m_initialized(false)
{
}

OneWireHal::~OneWireHal()
{
    if(m_sm >= 0)
    {
        pio_sm_set_enabled(m_config.pio, m_sm, false);
        pio_sm_unclaim(m_config.pio, m_sm);
    }

    if(m_mutex != nullptr)
    {
        vSemaphoreDelete(m_mutex);
    }
}

bool OneWireHal::init()
{
    m_mutex = xSemaphoreCreateMutex();
    if(m_mutex == nullptr)
    {
        return false;
    }

    if(!pio_can_add_program(m_config.pio, &onewire_program))
    {
        return false;
    }

    m_offset = pio_add_program(m_config.pio, &onewire_program);
    m_sm = pio_claim_unused_sm(m_config.pio, false);
    if(m_sm < 0)
    {
        return false;
    }

    float div = static_cast<float>(clock_get_hz(clk_sys)) / 500000.0f;

    pio_sm_config c = onewire_program_get_default_config(m_offset);

    pio_gpio_init(m_config.pio, m_config.pin);
    gpio_pull_up(m_config.pin);

    pio_sm_set_pins_with_mask(m_config.pio, m_sm, 0, (1u << m_config.pin));
    pio_sm_set_pindirs_with_mask(m_config.pio, m_sm, 0, (1u << m_config.pin));

    sm_config_set_sideset_pins(&c, m_config.pin);
    sm_config_set_in_pins(&c, m_config.pin);
    sm_config_set_clkdiv(&c, div);

    sm_config_set_out_shift(&c, true, false, 8);
    sm_config_set_in_shift(&c, true, false, 8);

    pio_sm_init(m_config.pio, m_sm, m_offset, &c);
    pio_sm_set_enabled(m_config.pio, m_sm, true);

    m_initialized = true;
    return true;
}

void OneWireHal::program_jmp(uint label_offset)
{
    pio_sm_exec(m_config.pio, m_sm, pio_encode_jmp(m_offset + label_offset));
}

bool OneWireHal::reset()
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

    pio_sm_clear_fifos(m_config.pio, m_sm);
    program_jmp(onewire_offset_reset);
    uint32_t presence = pio_sm_get_blocking(m_config.pio, m_sm);
    sleep_us(200);
    return (presence == 0);
}

void OneWireHal::write_byte(uint8_t byte)
{
    if(!m_initialized)
    {
        return;
    }

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked())
    {
        return;
    }

    pio_sm_clear_fifos(m_config.pio, m_sm);
    pio_sm_put_blocking(m_config.pio, m_sm, byte);
    program_jmp(onewire_offset_write_byte);
    while(!pio_sm_is_tx_fifo_empty(m_config.pio, m_sm))
    {
        tight_loop_contents();
    }
    sleep_us(850);
}

uint8_t OneWireHal::read_byte()
{
    if(!m_initialized)
    {
        return 0;
    }

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked())
    {
        return 0;
    }

    program_jmp(onewire_offset_read_byte);
    return static_cast<uint8_t>(pio_sm_get_blocking(m_config.pio, m_sm) >> 24);
}

uint32_t OneWireHal::crc8(const uint32_t *data, size_t len)
{
    uint8_t crc = 0;
    for(size_t i = 0; i < len; i++)
    {
        uint8_t inbyte = data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if(mix)
            {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }
    return crc;
}