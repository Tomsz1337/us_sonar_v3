#include <cstdio>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal/i2c_hal.h"
#include "drivers/bmp085.h"

void bmp085_task(void *pvParameters)
{
    (void)pvParameters;
    I2CHal i2c(i2c0, 100000, 0, 1);
    if (!i2c.init())
    {
        printf("I2C init failed!\n");
        vTaskDelete(NULL);
    }

    Bmp085 bmp(i2c);
    if (!bmp.init(Bmp085::Mode::Standard))
    {
        printf("BMP085 init failed!\n");
        vTaskDelete(NULL);
    }

    printf("BMP085 initialized successfully.\n");

    float pressHpa = 0.0f;

    while (true)
    {
        pressHpa = bmp.readPressure();
        printf("%.1f\n", pressHpa);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main()
{
    stdio_init_all();

    xTaskCreate(
        bmp085_task,
        "BMP085_Task",
        1024,
        NULL,
        1,
        NULL
    );

    vTaskStartScheduler();

    while (true)
    {
    }

    return 0;
}