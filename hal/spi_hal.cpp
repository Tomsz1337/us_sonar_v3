#include "hal/spi_hal.h"
#include "pico/error.h"

SPIHal::SPIHal(const Config &config) 
    : m_config(config), m_mutex(nullptr), m_initialized(false)
{
}

SPIHal::~SPIHal()
{
    if(m_mutex != nullptr) 
    {
        vSemaphoreDelete(m_mutex);
    }
}

bool SPIHal::init() 
{
    m_mutex = xSemaphoreCreateMutex();
    if(m_mutex == nullptr) 
    {
        return false;
    }

    spi_init(m_config.spi, m_config.baudrate);
    spi_set_format(m_config.spi, m_config.data_bits, m_config.cpol, m_config.cpha, m_config.order);
    
    gpio_set_function(m_config.sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(m_config.miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(m_config.mosi_pin, GPIO_FUNC_SPI);
    
    gpio_init(m_config.csn_pin);
    gpio_set_dir(m_config.csn_pin, true);
    gpio_put(m_config.csn_pin, true);

    m_initialized = true;
    return true;
}

int SPIHal::write(const uint8_t *tx_data, size_t length)
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked()) 
    {
        return PICO_ERROR_GENERIC;
    }
    
    gpio_put(m_config.csn_pin, false);
    int bytes_transferred = spi_write_blocking(m_config.spi, tx_data, length);
    gpio_put(m_config.csn_pin, true);  
    
    return bytes_transferred;
}

int SPIHal::read(const uint8_t *tx_data, uint8_t *rx_data, size_t length)
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked()) 
    {
        return PICO_ERROR_GENERIC;
    }

    gpio_put(m_config.csn_pin, false);
    int bytes_transferred = spi_write_read_blocking(m_config.spi, tx_data, rx_data, length);
    gpio_put(m_config.csn_pin, true);  
    
    return bytes_transferred;
}

int SPIHal::read(uint8_t repeated_tx_data, uint8_t *rx_data, size_t length)
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_mutex, portMAX_DELAY);
    if(!lock.isLocked()) 
    {
        return PICO_ERROR_GENERIC;
    }

    gpio_put(m_config.csn_pin, false);
    int bytes_transferred = spi_read_blocking(m_config.spi, repeated_tx_data, rx_data, length);
    gpio_put(m_config.csn_pin, true);  
    
    return bytes_transferred;
}