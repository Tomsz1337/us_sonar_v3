#include <cstdint>
#include <cstddef>
#include "hardware/pio.h"
#include "pulse_gen.pio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils/LockGuard.h"

class PulseGenHal
{
public:
    struct Config
    {
        PIO pio;
        uint pin;
        uint32_t freqHz;
        uint32_t nPulses;
    };

    PulseGenHal(const PulseGenHal&) = delete;
    PulseGenHal& operator=(const PulseGenHal&) = delete;

    explicit PulseGenHal(const Config &config);
    ~PulseGenHal();
    
    bool init();
    bool start();

private:
    Config m_config;
    int m_sm;
    int m_offset;
    SemaphoreHandle_t m_mutex;
    bool m_initialized;
};