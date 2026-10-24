#include "telemetry_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "tasks/data_queue.h"

bool telemetry_task_init()
{
    BaseType_t result = xTaskCreate(
        vTelemetryTask,
        "TelemetryTask",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr
    );
    return (result == pdPASS);
}

void vTelemetryTask(void *pvParameters)
{
    (void)pvParameters;

    while (true)
    {
        // tutaj później będziemy czekać na żądanie telemetrii
    }
}