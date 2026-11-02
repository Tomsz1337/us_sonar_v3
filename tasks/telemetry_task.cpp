#include "telemetry_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "tasks/data_queue.h"
#include "drivers/bmp085.h"
#include "hal/i2c_hal.h"
#include "data_queue.h"

namespace
{
    TaskHandle_t telemetry_handle = nullptr;
}

bool telemetry_task_init()
{
    BaseType_t result = xTaskCreate(
        vTelemetryTask,
        "TelemetryTask",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 1,
        &telemetry_handle
    );
    return (result == pdPASS);
}

void vTelemetryTask(void *pvParameters)
{
    (void)pvParameters;
    std::array<uint16_t, MAX_DATA_SAMPLES> samples{2350,5600,10130,2450,1800};
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        DataPacket sensorData{
            protocol::PktType::SENSOR,
            5,
            samples
        };
        if (!data_queue_send(sensorData, 0))
        {
            // TODO: obsługa błędu
        }
    }
}

bool telemetry_task_request()
{
    if(telemetry_handle == nullptr){return false;}
    xTaskNotifyGive(telemetry_handle);
    return true;
}