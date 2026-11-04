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
                // Generuj ramkę
                // Sprawdź, czy nie nadeszło STOP
                // Zachowaj okres 100 ms
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