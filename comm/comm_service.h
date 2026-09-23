#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "transport.h"
#include "protocol.h"

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
    [[nodiscard]] protocol::CmdId get_cmd() const;
    void clear_cmd();

private:
    ITransport &m_transport;
    protocol::CmdId m_current_cmd{protocol::CmdId::NONE};
    uint16_t m_tx_seq{0};

    std::array<uint8_t, protocol::HEADER_SIZE + protocol::MAX_PAYLOAD_SIZE * 4> m_tx_buffer{};

    bool send_packet(protocol::PktType type, protocol::CmdId id, std::span<const uint8_t> payload);
};