#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace protocol {

constexpr size_t SONAR_SAMPLES = 1000;
constexpr size_t MAX_PAYLOAD_SIZE = SONAR_SAMPLES * sizeof(uint16_t);

// =========================
// PACKET TYPES
// =========================
enum class PktType : uint8_t {
    CMD    = 1,
    ACK    = 2,
    SENSOR = 3,
    SONAR  = 4,
    GPS    = 5
};

// =========================
// COMMAND IDS
// =========================
enum class CmdId : uint8_t {
    NONE   = 0,
    RDY    = 1,
    SENSOR = 2,
    US     = 3,
    STOP   = 4
};

// =========================
// HEADER (NO PADDING)
// =========================
#pragma pack(push, 1)
struct Header {
    PktType  type{PktType::CMD};
    CmdId    id{CmdId::NONE};
    uint16_t seq{0};
    uint16_t len{0}; 

    constexpr Header() = default;
    constexpr Header(PktType t, CmdId i, uint16_t s, uint16_t l)
        : type(t), id(i), seq(s), len(l) {}
};
#pragma pack(pop)

static_assert(sizeof(Header) == 6, "Header size must be exactly 6 bytes");
constexpr size_t HEADER_SIZE = sizeof(Header);

// =========================
// FULL PACKET
// =========================
struct Packet {
    Header  header{};
    uint8_t payload[MAX_PAYLOAD_SIZE]{};

    [[nodiscard]] constexpr size_t total_size() const 
    {
        return HEADER_SIZE + header.len;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> payload_span() const 
    {
        return std::span<const uint8_t>(payload, header.len);
    }

    [[nodiscard]] constexpr std::span<uint8_t> payload_span() 
    {
        return std::span<uint8_t>(payload, header.len);
    }
};

} // namespace Protocol