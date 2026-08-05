#include "hal/uart_hal.h"
#include "pico/error.h"

UARTHal::UARTHal(const Config &config)
    : m_config(config), m_tx_mutex(nullptr), m_rx_mutex(nullptr), m_initialized(false)
{
}

UARTHal::~UARTHal()
{
    if(m_tx_mutex != nullptr) 
    {
        vSemaphoreDelete(m_tx_mutex);
    }
    if(m_rx_mutex != nullptr) 
    {
        vSemaphoreDelete(m_rx_mutex);
    }
}

bool UARTHal::init()
{
    m_tx_mutex = xSemaphoreCreateMutex();
    m_rx_mutex = xSemaphoreCreateMutex();
    if(m_tx_mutex == nullptr || m_rx_mutex == nullptr) 
    {
        return false;
    }

    uart_init(m_config.uart, m_config.baudrate);
    gpio_set_function(m_config.rx_pin, GPIO_FUNC_UART);
    gpio_set_function(m_config.tx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(m_config.uart, false, false);
    uart_set_format(m_config.uart, m_config.data_bits, m_config.stop_bits, m_config.parity);

    m_initialized = true;
    return true;
}

bool UARTHal::is_readable()
{
    if(!m_initialized) return false;

    LockGuard lock(m_rx_mutex, 0);
    if(!lock.isLocked()) 
    {
        return false;
    }

    return uart_is_readable(m_config.uart);
}

bool UARTHal::read_byte(uint8_t &byte)
{
    if(!m_initialized) return false;

    LockGuard lock(m_rx_mutex, 0);
    if (!lock.isLocked()) 
    {
        return false;
    }

    if(uart_is_readable(m_config.uart)) 
    {
        byte = uart_getc(m_config.uart);
        return true;
    }

    return false;
}

int UARTHal::write(const uint8_t *data, size_t length)
{
    if(!m_initialized) return PICO_ERROR_GENERIC;

    LockGuard lock(m_tx_mutex, portMAX_DELAY);
    if(!lock.isLocked()) 
    {
        return PICO_ERROR_GENERIC;
    }

    uart_write_blocking(m_config.uart, data, length);
    return static_cast<int>(length);
}