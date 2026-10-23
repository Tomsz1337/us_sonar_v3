#include "system_init.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tasks/comm_task.h"
#include "tasks/control_task.h"
#include "tasks/data_queue.h"

bool system_init()
{
    if (!data_queue_init())
    {
        return false;
    }

    if (!control_task_init())
    {
        return false;
    }

    BaseType_t result = xTaskCreate(
        vCommTask,
        "CommTask",
        4096,
        nullptr,
        tskIDLE_PRIORITY + 2,
        nullptr
    );

    return result == pdPASS;
}
