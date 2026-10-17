#include "comm_task.h"

#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

#include "comm/comm_service.h"
#include "comm/udp_transport.h"

#include "tasks/control_task.h"

UdpTransport g_udp(ip_addr_any, 5005, "PICO_NET", "test123", 5005);
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
                // Kolejka sterująca jest pełna
                // albo nie została zainicjalizowana.
                // Obsługę błędu dodamy później.
            }
        }

        TickType_t now = xTaskGetTickCount();

        if (now - lastHeartbeat >= pdMS_TO_TICKS(500))
        {
            lastHeartbeat = now;
            led = !led;

            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
