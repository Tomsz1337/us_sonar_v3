#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

class ITransport {
public:
    virtual ~ITransport() = default;

    virtual bool init() = 0;
    virtual bool send(std::span<const uint8_t> data) = 0;
    virtual bool receive(std::span<uint8_t> buffer, size_t &received_bytes) = 0;
    virtual bool is_connected() const = 0;
};