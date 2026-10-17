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
    std::array<uint8_t, protocol::HEADER_SIZE + protocol::MAX_PAYLOAD_SIZE> rx_buffer{};
    size_t received_bytes = 0;

    if(m_transport.receive(rx_buffer, received_bytes) && received_bytes > 0) 
    {
        handle_rx(std::span<const uint8_t>(rx_buffer.data(), received_bytes));
    }
}

void CommService::handle_rx(std::span<const uint8_t> rx_data) 
{
    if(rx_data.size() < protocol::HEADER_SIZE) 
    {
        return; 
    }

    protocol::Header header{};
    std::memcpy(&header, rx_data.data(), protocol::HEADER_SIZE);

    if(protocol::HEADER_SIZE + header.len != rx_data.size())
        return;

    if(header.type != protocol::PktType::CMD)
        return;

    const auto cmd = static_cast<protocol::CmdId>(header.id);

    if(xQueueSend(m_cmd_queue, &cmd, 0) != pdPASS)
    {
        return;
    }
    send_ack(cmd);
}

bool CommService::send_packet(protocol::PktType type, protocol::CmdId id, std::span<const uint8_t> payload) 
{
    const size_t total_size = protocol::HEADER_SIZE + payload.size();

    if(total_size > m_tx_buffer.size()) 
    {
        return false; 
    }

    protocol::Header header(type, id, m_tx_seq++, static_cast<uint16_t>(payload.size()));
    std::memcpy(m_tx_buffer.data(), &header, protocol::HEADER_SIZE);

    if (!payload.empty()) 
    {
        std::memcpy(m_tx_buffer.data() + protocol::HEADER_SIZE, payload.data(), payload.size());
    }
    return m_transport.send(std::span<const uint8_t>(m_tx_buffer.data(), total_size));
}

bool CommService::send_ack(protocol::CmdId cmd_id) 
{
    return send_packet(protocol::PktType::ACK, cmd_id, {});
}

bool CommService::send_telemetry(std::span<const uint16_t> data) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_packet(protocol::PktType::SENSOR, protocol::CmdId::NONE, bytes_span);
}

bool CommService::send_gps(std::span<const uint16_t> data) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_packet(protocol::PktType::GPS, protocol::CmdId::NONE, bytes_span);
}

bool CommService::send_sonar(std::span<const uint16_t> data) 
{
    auto bytes_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes());
    return send_packet(protocol::PktType::SONAR, protocol::CmdId::NONE, bytes_span);
}

void CommService::set_connected()
{
    m_is_connected = true;
}

bool CommService::receive_cmd(protocol::CmdId &cmd, TickType_t wait_ticks)
{
    return xQueueReceive(m_cmd_queue, &cmd, wait_ticks) == pdPASS;
}