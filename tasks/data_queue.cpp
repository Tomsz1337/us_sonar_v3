#include "data_queue.h"

namespace
{
    QueueHandle_t qhandle = nullptr;
}

bool data_queue_init()
    {
       qhandle = xQueueCreate(3, sizeof(DataPacket));
       if(qhandle == nullptr){return false;}
       return true;
    }

bool data_queue_send(const DataPacket& packet, TickType_t wait_ticks)
{
    if(qhandle == nullptr){return false;}
    return (xQueueSend(qhandle, &packet, wait_ticks) == pdPASS);
} 

bool data_queue_receive(DataPacket& packet, TickType_t wait_ticks)
{
    if(qhandle == nullptr){return false;}
    return (xQueueReceive(qhandle, &packet, wait_ticks) == pdPASS);
}