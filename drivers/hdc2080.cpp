#include "hdc2080.h"

HDC2080::HDC2080(I2CHal &i2c, uint8_t devAddr)
    : m_i2c(i2c), m_devAddr(devAddr), m_initialized(false)
{
}

bool HDC2080::init()
{
    
}