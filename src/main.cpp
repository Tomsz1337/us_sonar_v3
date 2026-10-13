#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "comm/comm_service.h"
#include "comm/udp_transport.h"
#include "SEGGER_RTT.h"

UdpTransport g_udp(IPADDR4_INIT_BYTES(192,168,4,2), 5005, "PICO_NET", "test1234", 5005);
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
        printf(g_udp.m_password);
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
                case protocol::CmdId::SENSOR:
                    break;
                case protocol::CmdId::US:
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
    SEGGER_RTT_Init();

    SEGGER_RTT_printf(0, g_udp.m_password);

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