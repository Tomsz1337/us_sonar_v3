#include "sonar_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "data_queue.h"

namespace
{
    TaskHandle_t sonar_handle = nullptr;

    void vSonarTask(void *pvParameters)
    {
        (void)pvParameters;
        bool sonar_running = false;
        uint32_t notification;
        std::array<uint16_t, MAX_DATA_SAMPLES> samples{};
        while (true)
        {
            if (!sonar_running)
            {
                xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
                if(static_cast<SonarCommand>(notification) == SonarCommand::START)
                {
                    sonar_running = true;
                }
            }
            else
            {
                if (xTaskNotifyWait(
                        0,
                        0,
                        &notification,
                        pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    if (static_cast<SonarCommand>(notification) == SonarCommand::STOP)
                    {
                        sonar_running = false;
                    }
                }
                else
                {
                    for (size_t i = 0; i < samples.size(); ++i)
                    {
                        samples[i] = static_cast<uint16_t>(i);
                    }
                    DataPacket sonarData{
                        protocol::PktType::SONAR,
                        static_cast<uint16_t>(samples.size()),
                        samples
                    };

                    if (!data_queue_send(sonarData, 0))
                    {
                        // TODO: obsługa błędu
                    }
                }
            }
        }
    }
}

bool sonar_task_init()
{
    BaseType_t result = xTaskCreate(
        vSonarTask,
        "SonarTask",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 1,
        &sonar_handle
    );
    return (result == pdPASS);
}

bool sonar_task_start()
{
    if (sonar_handle == nullptr){return false;}
    return  xTaskNotify(sonar_handle, static_cast<uint32_t>(SonarCommand::START), 
            eSetValueWithoutOverwrite) == pdPASS;
}

bool sonar_task_stop()
{
    if (sonar_handle == nullptr){return false;}
    return  xTaskNotify(sonar_handle, static_cast<uint32_t>(SonarCommand::STOP), 
            eSetValueWithoutOverwrite) == pdPASS;
}