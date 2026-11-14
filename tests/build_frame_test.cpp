
#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cassert>

#include "protocol.h"

using namespace protocol;

void test_single_frame()
{
    std::vector<uint8_t> payload(100);

    for (size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<uint8_t>(i);

    std::array<uint8_t, 1200> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR, 10, payload, 1200, output, info
    );

    assert(result);
    assert(info.frame_count == 1);
    assert(info.written_bytes == HEADER_SIZE + payload.size());
    assert(info.frame_sizes[0] == HEADER_SIZE + payload.size());

    Header header{};
    std::span<const uint8_t> parsed_payload;

    bool parsed = parse_packet(
        std::span<const uint8_t>(output.data(), info.frame_sizes[0]),
        header,
        parsed_payload
    );

    assert(parsed);
    assert(header.type == PktType::SONAR);
    assert(header.seq == 10);
    assert(header.fragment_index == 0);
    assert(header.fragment_count == 1);
    assert(header.total_length == payload.size());
    assert(header.payload_length == payload.size());

    assert(std::equal(
        parsed_payload.begin(), parsed_payload.end(),
        payload.begin(), payload.end()
    ));
}

void test_two_frames()
{
    std::vector<uint8_t> payload(2000);

    for (size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<uint8_t>(i % 256);

    std::array<uint8_t, 2048> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR, 20, payload, 1200, output, info
    );

    assert(result);
    assert(info.frame_count == 2);
    assert(info.written_bytes == 2018);
    assert(info.frame_sizes[0] == 1200);
    assert(info.frame_sizes[1] == 818);

    size_t offset = 0;
    std::vector<uint8_t> reconstructed;

    for (size_t i = 0; i < info.frame_count; ++i)
    {
        const size_t frame_size = info.frame_sizes[i];

        Header header{};
        std::span<const uint8_t> fragment_payload;

        bool parsed = parse_packet(
            std::span<const uint8_t>(
                output.data() + offset,
                frame_size
            ),
            header,
            fragment_payload
        );

        assert(parsed);
        assert(header.type == PktType::SONAR);
        assert(header.seq == 20);
        assert(header.fragment_index == i);
        assert(header.fragment_count == 2);
        assert(header.total_length == payload.size());
        assert(header.payload_length == fragment_payload.size());

        reconstructed.insert(
            reconstructed.end(),
            fragment_payload.begin(),
            fragment_payload.end()
        );

        offset += frame_size;
    }

    assert(offset == info.written_bytes);
    assert(reconstructed == payload);
}

void test_three_frames()
{
    std::vector<uint8_t> payload(3000);

    for (size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<uint8_t>((i * 7) % 256);

    std::array<uint8_t, 4096> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR, 30, payload, 1200, output, info
    );

    assert(result);
    assert(info.frame_count == 3);
    assert(info.frame_sizes[0] == 1200);
    assert(info.frame_sizes[1] == 1200);
    assert(info.frame_sizes[2] == 627);
    assert(info.written_bytes == 3027);

    size_t offset = 0;
    std::vector<uint8_t> reconstructed;

    for (size_t i = 0; i < info.frame_count; ++i)
    {
        Header header{};
        std::span<const uint8_t> fragment_payload;

        bool parsed = parse_packet(
            std::span<const uint8_t>(
                output.data() + offset,
                info.frame_sizes[i]
            ),
            header,
            fragment_payload
        );

        assert(parsed);
        assert(header.fragment_index == i);
        assert(header.fragment_count == 3);
        assert(header.seq == 30);

        reconstructed.insert(
            reconstructed.end(),
            fragment_payload.begin(),
            fragment_payload.end()
        );

        offset += info.frame_sizes[i];
    }

    assert(offset == info.written_bytes);
    assert(reconstructed == payload);
}

void test_empty_payload()
{
    std::array<uint8_t, 32> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::ACK, 40, {}, 1200, output, info
    );

    assert(result);
    assert(info.frame_count == 1);
    assert(info.written_bytes == HEADER_SIZE);
    assert(info.frame_sizes[0] == HEADER_SIZE);

    Header header{};
    std::span<const uint8_t> parsed_payload;

    bool parsed = parse_packet(
        std::span<const uint8_t>(output.data(), info.frame_sizes[0]),
        header,
        parsed_payload
    );

    assert(parsed);
    assert(header.type == PktType::ACK);
    assert(header.seq == 40);
    assert(parsed_payload.empty());
}

void test_output_too_small()
{
    std::vector<uint8_t> payload(2000);
    std::array<uint8_t, 100> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR, 50, payload, 1200, output, info
    );

    assert(!result);
    assert(info.frame_count == 0);
    assert(info.written_bytes == 0);
}

void test_frame_size_too_small()
{
    std::vector<uint8_t> payload(100);
    std::array<uint8_t, 256> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR,
        60,
        payload,
        HEADER_SIZE,
        output,
        info
    );

    assert(!result);
    assert(info.frame_count == 0);
    assert(info.written_bytes == 0);
}

void test_invalid_fragment_count()
{
    // Przy limicie 10 bajtów na ramkę zostaje tylko 1 bajt payloadu.
    // 256 bajtów wymagałoby 256 fragmentów, a maksimum wynosi 255.
    std::vector<uint8_t> payload(256);
    std::array<uint8_t, 4096> output{};
    FramesInfo info{};

    bool result = build_frames(
        PktType::SONAR,
        70,
        payload,
        HEADER_SIZE + 1,
        output,
        info
    );

    assert(!result);
    assert(info.frame_count == 0);
    assert(info.written_bytes == 0);
}

int main()
{
    test_single_frame();
    test_two_frames();
    test_three_frames();
    test_empty_payload();
    test_output_too_small();
    test_frame_size_too_small();
    test_invalid_fragment_count();

    std::cout << "All build_frames tests passed!\n";
    return 0;
}
