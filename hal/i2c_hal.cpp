#include "hal/i2c_hal.h"
#include "pico/error.h" 

I2CHal::I2CHal(i2c_inst_t *i2c, uint baudrate, uint sda_pin, uint scl_pin)
    : m_mutex(nullptr), m_initialized(false) 
{
    m_config.baudrate = baudrate;
    m_config.i2c = i2c;
    m_config.scl_pin = scl_pin;
    m_config.sda_pin = sda_pin;
}

I2CHal::~I2CHal() 
{
    if(m_mutex != nullptr) 
    {
        vSemaphoreDelete(m_mutex);
    }
}

bool I2CHal::init() 
{
    m_mutex = xSemaphoreCreateMutex();
    if(m_mutex == nullptr) 
    {
        return false;
    }

    i2c_init(m_config.i2c, m_config.baudrate);
    gpio_set_function(m_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(m_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(m_config.sda_pin);
    gpio_pull_up(m_config.scl_pin);

    m_initialized = true;
    return true;
}

int I2CHal::read(uint8_t addr, uint8_t *dst, size_t len, bool nostop, uint32_t timeout_ms) 
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked()) 
    {
        return PICO_ERROR_GENERIC;
    }

    return i2c_read_timeout_us(m_config.i2c, addr, dst, len, nostop, timeout_ms * 1000);
}

int I2CHal::write(uint8_t addr, const uint8_t *src, size_t len, bool nostop, uint32_t timeout_ms) 
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked())
    {
        return PICO_ERROR_GENERIC;
    }

    uint32_t timeout_us = timeout_ms * 1000;

    return i2c_write_timeout_us(m_config.i2c, addr, src, len, nostop, timeout_us);
}