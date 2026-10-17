#include "LockGuard.h"

LockGuard::LockGuard(SemaphoreHandle_t mutex, TickType_t timeout_ticks)
    : m_mutex(mutex), m_locked(false)
{
    if(m_mutex != nullptr)
    {
        if(xSemaphoreTake(m_mutex, timeout_ticks) == pdTRUE)
        {
            m_locked = true;
        }
    }
}

LockGuard::~LockGuard()
{
    if(m_locked && m_mutex != nullptr)
    {
        xSemaphoreGive(m_mutex);
    }
}

bool LockGuard::isLocked() const
{
    return m_locked;
}