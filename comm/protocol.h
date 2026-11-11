#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace protocol {

constexpr std::size_t SONAR_SAMPLES = 1000;
constexpr std::size_t MAX_SONAR_SIZE =
    SONAR_SAMPLES * sizeof(uint16_t);

constexpr std::size_t MAX_FRAGMENT_COUNT = 255;
constexpr std::size_t MAX_MESSAGE_SIZE = 65535;

// =========================
// PACKET TYPES
// =========================
enum class PktType : uint8_t {
    COMMAND     = 1,
    ACK         = 2,
    NACK        = 3,
    EVENT       = 4,
    TELEMETRY   = 5,
    SONAR       = 6,
    GPS         = 7,
    PING        = 8,
    PONG        = 9
};

// =========================
// COMMAND IDS
// =========================
enum class CmdId : uint8_t {
    NONE                = 0,
    HELLO               = 1,
    START_SONAR         = 2,
    STOP_SONAR          = 3,
    REQUEST_TELEMETRY   = 4,
    REINITIALIZE        = 5
};

// =========================
// EVENT IDS
// =========================
enum class EventId : uint8_t {
    DEVICE_READY = 1,
    SONAR_STARTED = 2,
    SONAR_STOPPED = 3,
    TELEMETRY_READY = 4,
    ERROR = 5,
    EMERGENCY_ENTERED = 6,
    EMERGENCY_CLEARED = 7
};

// =========================
// NACK REASON
// =========================
enum class NackReason : uint8_t {
    UNKNOWN_COMMAND = 1,
    INVALID_STATE = 2,
    INVALID_PAYLOAD = 3,
    BUSY = 4
};

// =========================
// HEADER 
// =========================
struct Header {
    PktType type{PktType::COMMAND};
    uint16_t seq{0};

    uint8_t fragment_index{0};
    uint8_t fragment_count{1};

    uint16_t total_length{0};
    uint16_t payload_length{0};
};

constexpr size_t HEADER_SIZE = 9;

static_assert(HEADER_SIZE == 9);

bool validate_header(const Header& header);
bool serialize_header(const Header& header,
                      std::span<std::uint8_t> buffer);
bool deserialize_header(std::span<const std::uint8_t> buffer,
                        Header &header);
bool build_packet(const Header &header,
                    std::span<const std::uint8_t> payload,
                    std::span<std::uint8_t> output,
                    std::size_t &written_bytes);
bool parse_packet(std::span<const std::uint8_t> frame,
                    Header& header,
                    std::span<const std::uint8_t>& payload);
bool build_fragment(const Header& header,
                    std::span<const std::uint8_t> payload,
                    std::size_t max_frame_size,
                    std::uint8_t fragment_index,
                    std::span<std::uint8_t> output,
                    std::size_t& written_bytes
);
// =========================
// FULL PACKET
// =========================

} // namespace Protocol