#pragma once

#include "hal/spi_hal.h"
#include "hal/pulse_gen.h"
#include <cstdint>

class TUSS4470
{
public:

    struct Config {
        uint8_t bpf_config_1;
        uint8_t bpf_config_2;
        uint8_t dev_ctrl_1;
        uint8_t dev_ctrl_2;
        uint8_t dev_ctrl_3;
        uint8_t vdrv_ctrl;
        uint8_t echo_int_config;
        uint8_t zc_config;
        uint8_t burst_pulse;
        uint8_t tof_config;
    };


    explicit TUSS4470(SPIHal &spi, PulseGenHal &pg);
    void init();
    void trigger();
    void write_reg(uint8_t addr, uint8_t data);
    uint8_t read_reg(uint8_t addr);


private:

    static uint8_t calc_odd_parity(uint8_t hNibble, uint8_t lNibble);

    // static constexpr uint8_t BPF_CONFIG_1_addr      = 0x10;
    // static constexpr uint8_t BPF_CONFIG_2_addr      = 0x11;
    // static constexpr uint8_t DEV_CTRL_1_addr        = 0x12;
    // static constexpr uint8_t DEV_CTRL_2_addr        = 0x13;
    // static constexpr uint8_t DEV_CTRL_3_addr        = 0x14;
    // static constexpr uint8_t VDRV_CTRL_addr         = 0x16;
    // static constexpr uint8_t ECHO_INT_CONFIG_addr   = 0x17;
    // static constexpr uint8_t ZC_CONFIG_addr         = 0x18;
    // static constexpr uint8_t BURST_PULSE_addr       = 0x1A;
    // static constexpr uint8_t TOF_CONFIG_addr        = 0x1B;
    // static constexpr uint8_t DEV_STAT_addr          = 0x1C;
    // static constexpr uint8_t DEVICE_ID_addr         = 0x1D;
    // static constexpr uint8_t REV_ID_addr            = 0x1E;
    
    SPIHal &m_spi;
    PulseGenHal &m_pg;
    Config m_cfg;

};