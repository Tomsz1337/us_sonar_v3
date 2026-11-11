#include "protocol.h"
#include <cstring>

namespace protocol
{


bool validate_header(const Header &header)
{
    switch (header.type) 
    {
        case PktType::COMMAND:
        case PktType::ACK:
        case PktType::NACK:
        case PktType::EVENT:
        case PktType::TELEMETRY:
        case PktType::SONAR:
        case PktType::GPS:
        case PktType::PING:
        case PktType::PONG:
            break;

        default:
            return false;
    }

    if (header.fragment_count == 0) 
    {
        return false;
    }
    if (header.fragment_index >= header.fragment_count) 
    {
        return false;
    }
    if (header.payload_length > header.total_length) 
    {
        return false;
    }
    if (header.total_length > MAX_MESSAGE_SIZE) 
    {
        return false;
    }
    if (header.fragment_count == 1) 
    {
        if (header.fragment_index != 0) 
        {
            return false;
        }
        if (header.payload_length != header.total_length) 
        {
            return false;
        }
    }
    else 
    {
        if (header.payload_length == 0) 
        {
            return false;
        }
    }

    return true;
}


bool serialize_header(const Header& header,
                      std::span<std::uint8_t> buffer)
{
    if (buffer.size() < HEADER_SIZE) 
    {
        return false;
    }

    buffer[0] = static_cast<std::uint8_t>(header.type);
    buffer[1] = static_cast<std::uint8_t>(header.seq & 0xFF);
    buffer[2] = static_cast<std::uint8_t>(header.seq >> 8);
    buffer[3] = header.fragment_index;
    buffer[4] = header.fragment_count;
    buffer[5] = static_cast<std::uint8_t>(header.total_length & 0xFF);
    buffer[6] = static_cast<std::uint8_t>(header.total_length >> 8);
    buffer[7] = static_cast<std::uint8_t>(header.payload_length & 0xFF);
    buffer[8] = static_cast<std::uint8_t>(header.payload_length >> 8);

    return true;
}

bool deserialize_header(std::span<const std::uint8_t> buffer,
                        Header& header)
{
    if (buffer.size() < HEADER_SIZE) 
    {
        return false;
    }

    header.type = static_cast<PktType>(buffer[0]);

    header.seq = static_cast<std::uint16_t>(buffer[1]) |
                (static_cast<std::uint16_t>(buffer[2]) << 8);

    header.fragment_index = buffer[3];
    header.fragment_count = buffer[4];

    header.total_length = static_cast<std::uint16_t>(buffer[5]) |
                        (static_cast<std::uint16_t>(buffer[6]) << 8);

    header.payload_length = static_cast<std::uint16_t>(buffer[7]) |
                            (static_cast<std::uint16_t>(buffer[8]) << 8);
    return true;
}

bool build_packet(const Header &header, std::span<const std::uint8_t> payload,
                    std::span<std::uint8_t> output, std::size_t &written_bytes)
{
    written_bytes = 0;
    size_t total_size = HEADER_SIZE + payload.size();
    if(!validate_header(header) || payload.size() != header.payload_length ||
        output.size() < total_size) 
    {
        return false;
    }
    if(!serialize_header(header, output)) {return false;}
    std::memcpy(output.data() + HEADER_SIZE, payload.data(), payload.size());
    written_bytes = total_size;
    return true;
}

bool parse_packet(std::span<const std::uint8_t> frame, Header& header,
                    std::span<const std::uint8_t>& payload)
{
    payload = {};
    if(!deserialize_header(frame, header) || !validate_header(header)) {return false;}
    if(HEADER_SIZE + header.payload_length != frame.size()) {return false;}
    payload = frame.subspan(HEADER_SIZE, header.payload_length);
    return true;
}

bool build_fragment(const Header& header, std::span<const std::uint8_t> payload,
                    std::size_t max_frame_size, std::uint8_t fragment_index,
                    std::span<std::uint8_t> output, std::size_t& written_bytes)
{
    written_bytes = 0;
    if(!validate_header(header)) {return false;}
    size_t max_payload_size = max_frame_size - HEADER_SIZE;
    uint8_t fragment_count = 

}
}//protocol