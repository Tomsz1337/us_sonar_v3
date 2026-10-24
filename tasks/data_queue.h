#pragma once

#include <cstdint>
#include <array>

#include "protocol.h"
#include "FreeRTOS.h"
#include "queue.h"

static constexpr uint16_t MAX_DATA_SAMPLES = 1000;

struct DataPacket{
    protocol::PktType type;
    uint16_t count;
    std::array<uint16_t, MAX_DATA_SAMPLES> samples;
};

bool data_queue_init();
bool data_queue_send(const DataPacket& packet, TickType_t wait_ticks = 0);
bool data_queue_receive(DataPacket& packet, TickType_t wait_ticks = 0);