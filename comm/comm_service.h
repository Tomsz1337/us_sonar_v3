#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "transport.h"
#include "protocol.h"
#include "FreeRTOS.h"
#include "queue.h"

class CommService 
{
public:
    explicit CommService(ITransport &transport);

    bool init();
    void process_rx();
    void handle_rx(std::span<const uint8_t> rx_data);
    bool send_ack(uint16_t seq);
    bool send_telemetry(std::span<const uint16_t> data, uint16_t seq);
    bool send_gps(std::span<const uint16_t> data, uint16_t seq);
    bool send_sonar(std::span<const uint16_t> data, uint16_t seq);
    void set_connected();
    bool receive_cmd(protocol::CmdId &cmd, TickType_t wait_ticks = 0);

private:
    ITransport &m_transport;
    QueueHandle_t m_cmd_queue = nullptr;
    uint16_t m_tx_seq{0};
    bool m_is_connected{false};

    std::array<uint8_t, 4096> m_tx_buffer{};

    bool send_data(protocol::PktType type, uint16_t seq, std::span<const uint8_t> payload);
};