#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

class LockGuard
{
public:
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    LockGuard(SemaphoreHandle_t mutex, TickType_t timeout_ticks);
    ~LockGuard();

    bool isLocked() const;

private:
    SemaphoreHandle_t m_mutex;
    bool m_locked;
};