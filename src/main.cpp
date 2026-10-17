#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "system/system_init.h"

int main()
{
    stdio_init_all();

    if(!system_init())
    {
        while(true)
        {
            tight_loop_contents();
        }
    }

    vTaskStartScheduler();

    while(true)
    {
        tight_loop_contents();
    }
}