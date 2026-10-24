#include "comm_task.h"

#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/ip_addr.h"

#include "comm/comm_service.h"
#include "comm/udp_transport.h"

#include "tasks/control_task.h"
#include "tasks/data_queue.h"

UdpTransport g_udp(
    "192.168.4.2",
    5005,
    "PICO_NET",
    "test1234",
    5005
);

CommService g_comm(g_udp);

void vCommTask(void *pvParameters)
{
    (void)pvParameters;

    if (!g_comm.init())
    {
        vTaskDelete(nullptr);
        return;
    }

    bool led = false;
    TickType_t lastHeartbeat = xTaskGetTickCount();

    while (true)
    {
        g_comm.process_rx();

        protocol::CmdId cmd;

        while (g_comm.receive_cmd(cmd))
        {
            if (cmd == protocol::CmdId::RDY)
            {
                g_comm.set_connected();
            }

            if (!control_task_post_command(cmd))
            {
                // TODO: obsługa przepełnienia kolejki.
            }
        }

        DataPacket packet;

        while (data_queue_receive(packet, 0))
        {
            const auto samples = std::span<const uint16_t>(
                packet.samples.data(),
                packet.count
            );

            switch (packet.type)
            {
                case protocol::PktType::SENSOR:
                    g_comm.send_telemetry(samples);
                    break;

                case protocol::PktType::SONAR:
                    g_comm.send_sonar(samples);
                    break;

                case protocol::PktType::GPS:
                    g_comm.send_gps(samples);
                    break;

                default:
                    break;
            }
        }

        const TickType_t now = xTaskGetTickCount();

        if (now - lastHeartbeat >= pdMS_TO_TICKS(500))
        {
            lastHeartbeat = now;
            led = !led;

            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool comm_task_init()
{
    BaseType_t result = xTaskCreate(
        vCommTask,
        "CommTask",
        4096,
        nullptr,
        tskIDLE_PRIORITY + 2,
        nullptr
    );
    return (result == pdPASS);
}