#include "comm_service.h"
#include <cstring>

CommService::CommService(ITransport &transport)
    : m_transport(transport) 
{
}

bool CommService::init() 
{
    if(!m_transport.init())
    {
        return false;
    }

    m_cmd_queue = xQueueCreate(10, sizeof(protocol::CmdId));

    if(m_cmd_queue == nullptr)
    {
        return false;
    }
    return true;
}

void CommService::process_rx() 
{
    std::array<uint8_t, protocol::HEADER_SIZE + sizeof(protocol::CmdId)> rx_buffer{};
    size_t received_bytes = 0;

    if(m_transport.receive(rx_buffer, received_bytes) && received_bytes > 0) 
    {
        handle_rx(std::span<const uint8_t>(rx_buffer.data(), received_bytes));
    }
}

void CommService::handle_rx(std::span<const uint8_t> rx_data) 
{
    if(rx_data.size() < protocol::HEADER_SIZE) 
        return; 

    protocol::Header header{};
    std::span<const uint8_t> payload;
    if (!protocol::parse_packet(rx_data, header, payload))
        return;

    if(header.type != protocol::PktType::COMMAND)
        return;

    if (payload.size() != 1)
        return;

    const auto cmd = static_cast<protocol::CmdId>(payload[0]);

    if(xQueueSend(m_cmd_queue, &cmd, 0) != pdPASS)
        return;

    send_ack(header.seq);
}

bool CommService::send_data(protocol::PktType type, uint16_t seq, std::span<const uint8_t> payload) 
{
    protocol::FramesInfo fInfo = {};
    size_t offset = 0;  

    if(!protocol::build_frames(type, seq, payload, m_transport.max_frame_size(), m_tx_buffer, fInfo))
        return false;

    for(int i = 0; i < fInfo.frame_count; ++i)
    {
        m_transport.send(std::span<const uint8_t>(m_tx_buffer.data() + offset, fInfo.frame_sizes[i]));
        offset += fInfo.frame_sizes[i];
    }
    return true;
}

bool CommService::send_ack(uint16_t seq) 
{
    return send_data(protocol::PktType::ACK, seq, {});
}

bool CommService::send_telemetry(std::span<const uint16_t> data, uint16_t seq) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_data(protocol::PktType::TELEMETRY, seq, bytes_span);
}

bool CommService::send_gps(std::span<const uint16_t> data, uint16_t seq) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_data(protocol::PktType::GPS, seq, bytes_span);
}

bool CommService::send_sonar(std::span<const uint16_t> data, uint16_t seq) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_data(protocol::PktType::SONAR, seq, bytes_span);
}

void CommService::set_connected()
{
    m_is_connected = true;
}

bool CommService::receive_cmd(protocol::CmdId &cmd, TickType_t wait_ticks)
{
    return xQueueReceive(m_cmd_queue, &cmd, wait_ticks) == pdPASS;
}