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
    bool send_ack(protocol::CmdId cmd_id);
    bool send_telemetry(std::span<const uint16_t> data);
    bool send_gps(std::span<const uint16_t> data);
    bool send_sonar(std::span<const uint16_t> data);
    void set_connected();
    bool receive_cmd(protocol::CmdId &cmd);

private:
    ITransport &m_transport;
    QueueHandle_t m_cmd_queue = nullptr;
    uint16_t m_tx_seq{0};
    bool m_is_connected{false};

    std::array<uint8_t, protocol::HEADER_SIZE + protocol::MAX_PAYLOAD_SIZE * 4> m_tx_buffer{};

    bool send_packet(protocol::PktType type, protocol::CmdId id, std::span<const uint8_t> payload);
};