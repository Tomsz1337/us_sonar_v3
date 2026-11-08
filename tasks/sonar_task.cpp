#include "sonar_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "data_queue.h"

#include <array>
#include <cstdint>
#include <cmath>

namespace
{
    uint32_t rng_state = 0xA341316C;

    uint32_t random_u32()
    {
        rng_state ^= rng_state << 13;
        rng_state ^= rng_state >> 17;
        rng_state ^= rng_state << 5;

        return rng_state;
    }

    int random_range(int min, int max)
    {
        return min + static_cast<int>(random_u32() % (max - min + 1));
    }

    void generate_mock_sonar_frame(
        std::array<uint16_t, MAX_DATA_SAMPLES>& samples)
    {
        // Położenia ech w tablicy próbek
        static int echo_positions[] = {140, 360, 610, 850};

        // Amplitudy poszczególnych ech
        constexpr int echo_amplitudes[] = {1800, 3200, 2200, 6500};

        // Szerokości ech
        constexpr float echo_sigmas[] = {5.0f, 8.0f, 6.0f, 12.0f};

        // Delikatne przesunięcie położenia ech między ramkami
        for (int& position : echo_positions)
        {
            position += random_range(-1, 1);

            if (position < 20) position = 20;
            if (position >= static_cast<int>(MAX_DATA_SAMPLES) - 20)
                position = MAX_DATA_SAMPLES - 21;
        }

        for (size_t i = 0; i < samples.size(); ++i)
        {
            // Szum tła
            float value = static_cast<float>(80 + random_u32() % 80);

            // Generowanie czterech ech
            for (size_t e = 0; e < 4; ++e)
            {
                int amplitude = echo_amplitudes[e] + random_range(-200, 200);

                float distance =
                    (static_cast<float>(i) - echo_positions[e]) / echo_sigmas[e];

                float echo = amplitude * expf(-0.5f * distance * distance);

                value += echo;
            }

            samples[i] = static_cast<uint16_t>(value);
        }
    }

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
                    generate_mock_sonar_frame(samples);
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
} // namespace end

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