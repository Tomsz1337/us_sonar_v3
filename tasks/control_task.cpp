#include "control_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

namespace
{
    constexpr UBaseType_t CONTROL_QUEUE_LENGTH = 10;
    constexpr configSTACK_DEPTH_TYPE CONTROL_TASK_STACK_SIZE = 2048;

    QueueHandle_t g_control_queue = nullptr;

    enum class DeviceState
    {
        WAIT,
        IDLE
    };

    void vControlTask(void *pvParameters)
    {
        (void)pvParameters;

        DeviceState state = DeviceState::WAIT;
        protocol::CmdId cmd;

        while (true)
        {
            if (xQueueReceive(
                    g_control_queue,
                    &cmd,
                    portMAX_DELAY) == pdPASS)
            {
                switch (cmd)
                {
                    case protocol::CmdId::RDY:
                        state = DeviceState::IDLE;
                        break;

                    case protocol::CmdId::STOP:
                        state = DeviceState::WAIT;
                        break;

                    case protocol::CmdId::SENSOR:
                        if (state == DeviceState::IDLE)
                        {
                            // TODO: zlecenie odczytu czujników
                        }
                        break;

                    case protocol::CmdId::US:
                        if (state == DeviceState::IDLE)
                        {
                            // TODO: uruchomienie sonaru
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }
}

bool control_task_init()
{
    g_control_queue = xQueueCreate(
        CONTROL_QUEUE_LENGTH,
        sizeof(protocol::CmdId));

    if (g_control_queue == nullptr)
    {
        return false;
    }

    BaseType_t result = xTaskCreate(
        vControlTask,
        "ControlTask",
        CONTROL_TASK_STACK_SIZE,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr);

    if (result != pdPASS)
    {
        vQueueDelete(g_control_queue);
        g_control_queue = nullptr;
        return false;
    }

    return true;
}

bool control_task_post_command(protocol::CmdId cmd)
{
    if (g_control_queue == nullptr)
    {
        return false;
    }

    return xQueueSend(
        g_control_queue,
        &cmd,
        0) == pdPASS;
}