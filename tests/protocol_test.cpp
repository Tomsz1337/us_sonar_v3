#include "protocol.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

using namespace protocol;

int main()
{
    // =========================
    // TEST 1: BUILD PACKET
    // =========================

    Header header{};

    header.type = PktType::SONAR;
    header.seq = 0x1234;
    header.fragment_index = 0;
    header.fragment_count = 1;
    header.total_length = 100;
    header.payload_length = 100;

    std::array<std::uint8_t, 100> payload{};

    for (std::size_t i = 0; i < payload.size(); i++) {
        payload[i] = static_cast<std::uint8_t>(i);
    }

    std::array<std::uint8_t, HEADER_SIZE + 100> output{};

    std::size_t written_bytes = 0;
    assert(build_packet(header, payload, output, written_bytes));
    assert(written_bytes == 109);

    // =========================
    // TEST 2: PARSE PACKET
    // =========================

    Header parsed_header{};

    std::span<const std::uint8_t> parsed_payload{};

    assert(parse_packet(
        std::span<const std::uint8_t>(output.data(), written_bytes),
        parsed_header,
        parsed_payload
    ));

    assert(parsed_header.type == header.type);
    assert(parsed_header.seq == header.seq);
    assert(parsed_header.fragment_index == header.fragment_index);
    assert(parsed_header.fragment_count == header.fragment_count);
    assert(parsed_header.total_length == header.total_length);
    assert(parsed_header.payload_length == header.payload_length);

    assert(parsed_payload.size() == payload.size());

    for (std::size_t i = 0; i < payload.size(); i++) {
        assert(parsed_payload[i] == payload[i]);
    }

    assert(parsed_payload.data() == output.data() + HEADER_SIZE);

    // =========================
    // TEST 3: INVALID HEADER
    // =========================

    Header invalid_header = header;
    invalid_header.fragment_count = 0;

    assert(!build_packet(invalid_header, payload, output, written_bytes));

    // =========================
    // TEST 4: TOO SMALL OUTPUT
    // =========================

    std::array<std::uint8_t, 108> small_output{};

    assert(!build_packet(header, payload, small_output, written_bytes));

    // =========================
    // TEST 5: TRUNCATED FRAME
    // =========================

    assert(!parse_packet(
        std::span<const std::uint8_t>(output.data(), 108),
        parsed_header,
        parsed_payload
    ));

    // =========================
    // TEST 6: EXTRA BYTE
    // =========================

    std::array<std::uint8_t, 110> oversized_frame{};

    std::copy(output.begin(), output.end(), oversized_frame.begin());

    assert(!parse_packet(oversized_frame, parsed_header, parsed_payload));

    std::cout << "All protocol tests passed!\n";

    return 0;
}