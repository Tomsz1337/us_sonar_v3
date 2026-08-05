#include <cstdio>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal/i2c_hal.h"

void vI2CScannerTask(void *pvParameters)
{
    auto *i2c = static_cast<I2CHal *>(pvParameters);

    vTaskDelay(pdMS_TO_TICKS(2000));

    while(true) 
    {
        printf("\n--- Starting I2C bus scan ---\n");

        int devices_found = 0;

        for(uint8_t addr = 0x08; addr <= 0x77; ++addr) 
        {
            uint8_t dummy_rx = 0;

            int result = i2c->read(addr, &dummy_rx, 1, false, 10);

            if(result >= 0) 
            {
                printf("  [+] Found I2C device at address: 0x%02X\n", addr);
                devices_found++;
            }
        }

        if(devices_found == 0) 
        {
            printf("  [-] No I2C devices found.\n");
        } 
        else 
        {
            printf("--- Scan complete. Devices found: %d ---\n", devices_found);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main()
{
    stdio_init_all();

    I2CHal::Config i2c_config = {
        .i2c      = i2c0,
        .baudrate = 100'000,
        .sda_pin  = 0,
        .scl_pin  = 1
    };

    static I2CHal i2c_bus(i2c_config);

    if(!i2c_bus.init()) 
    {
        printf("ERROR: Failed to initialize I2CHal (Mutex creation error)!\n");
        while(true) 
        {
            tight_loop_contents();
        }
    }

    xTaskCreate(
        vI2CScannerTask,
        "I2C_Scanner",
        1024,
        &i2c_bus,
        tskIDLE_PRIORITY + 1,
        nullptr
    );

    vTaskStartScheduler();

    while(true) {}
}