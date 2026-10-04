#include "tuss4470.h"
#include "pico/stdlib.h"

TUSS4470::TUSS4470(SPIHal &spi, PulseGenHal &pg)
    : m_spi(spi), m_pg(pg)
{
}

uint8_t TUSS4470::calc_odd_parity(uint8_t hNibble, uint8_t lNibble)
{
    uint16_t frame = (hNibble << 8) | lNibble;
    uint8_t ones = 0;
	for(int i = 0; i < 16; i++)
	{
		if((frame >> i) & 1)
		{
			ones++;
		}
	}
	return (ones + 1) % 2;
}

void TUSS4470::write_reg(uint8_t addr, uint8_t data)
{
    uint8_t tx_buff[2];
    tx_buff[0] = (addr & 0x3f) << 1;
	tx_buff[1] = data;
    tx_buff[0] |= calc_odd_parity(tx_buff[0], tx_buff[1]);
    m_spi.write(tx_buff, 2);
}

uint8_t TUSS4470::read_reg(uint8_t addr)
{
    uint8_t tx_buff[2];
    uint8_t rx_buff[2] = {0};
    tx_buff[0] = ((addr & 0x3f) << 1) + 0x80;
	tx_buff[1] = 0x00;
    tx_buff[0] |= calc_odd_parity(tx_buff[0], tx_buff[1]);
    m_spi.read(tx_buff, rx_buff, 2);
    return rx_buff[1];
}

void TUSS4470::init()
{
    m_pg.init();

    struct RegCfg {
        uint8_t addr;
        uint8_t value;
    };

    const RegCfg init_table[] = {
        {0x10, m_cfg.bpf_config_1},
        {0x11, m_cfg.bpf_config_2},
        {0x12, m_cfg.dev_ctrl_1},
        {0x13, m_cfg.dev_ctrl_2},
        {0x14, m_cfg.dev_ctrl_3},
        {0x16, m_cfg.vdrv_ctrl},
        {0x17, m_cfg.echo_int_config},
        {0x18, m_cfg.zc_config},
        {0x19, m_cfg.burst_pulse},
        {0x1A, m_cfg.tof_config}
    };
    for(const auto &reg : init_table)
    {
        write_reg(reg.addr, reg.value);
    }
}

void TUSS4470::trigger()
{
    write_reg(0x1B, 0x01);
    m_pg.start();
    sleep_us(100);
    write_reg(0x1B, 0x00);
}