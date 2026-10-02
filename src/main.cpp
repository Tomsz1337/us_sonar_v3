#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "comm/comm_service.h"
#include "comm/udp_transport.h"

UdpTransport g_udp(ip_addr_any, 5005, "PICO_NET", "test123", 5005);
CommService g_comm(g_udp);


void vCommTask(void *pvParameters) 
{
    if(!g_comm.init()) 
    {
        vTaskDelete(NULL);
    }

    bool led = false;
    TickType_t lastHeartbeat = xTaskGetTickCount();

    while(true) 
    {
        g_comm.process_rx();

        protocol::CmdId cmd;

        if(g_comm.receive_cmd(cmd))
        {
            if(cmd == protocol::CmdId::RDY)
            {
                g_comm.set_connected();
                break;
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

    while (true)
    {
        g_comm.process_rx();

        protocol::CmdId cmd;

        if (g_comm.receive_cmd(cmd))
        {
            switch (cmd)
            {
                case protocol::CmdId::STOP:
                    // ...
                    break;

                default:
                    break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}

int main() 
{
    stdio_init_all();

    xTaskCreate(
        vCommTask,
        "CommTask",
        4096,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    vTaskStartScheduler();

    while (true) {}
}