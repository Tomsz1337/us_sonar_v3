#include "FreeRTOS.h"
#include "task.h"


extern "C" void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char *pcTaskName
)
{
    (void)xTask;
    (void)pcTaskName;

    __breakpoint();
    while (true) {
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    __breakpoint();
    while (true) {
    }
}