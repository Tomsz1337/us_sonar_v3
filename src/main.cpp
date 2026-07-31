#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "FreeRTOS.h"
#include "task.h"

// Zadanie 1 - wysyła komunikat i pokazuje, na którym rdzeniu (Core 0/1) się wykonuje
void vTaskCoreCheck(void *pvParameters) {
    const char *taskName = (const char *)pvParameters;
    
    while (true) {
        UBaseType_t coreNum = portGET_CORE_ID();
        printf("[%s] Dzialam na rdzeniu %u | Czas systemowy: %lu ms\n", 
               taskName, (unsigned int)coreNum, (unsigned long)to_ms_since_boot(get_absolute_time()));
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Czekaj 1 sekundę
    }
}

// Zadanie 2 - mruganie diodą (lub wypisywanie statusu heartbeat)
void vTaskHeartbeat(void *pvParameters) {
    while (true) {
        printf("---> [Heartbeat] FreeRTOS SMP dziala poprawnie na RP2350!\n");
        vTaskDelay(pdMS_TO_TICKS(2500)); // Czekaj 2.5 sekundy
    }
}

int main() {
    // Inicjalizacja standardowego I/O (USB)
    stdio_init_all();

    // Krótkie opóźnienie, aby dać czas na otwarcie portu COM / ttyACM po podłączeniu USB
    sleep_ms(2000);

    printf("\n=== Test FreeRTOS SMP na RP2350 (Pico 2 W) ===\n");

    // Tworzenie zadań FreeRTOS
    xTaskCreate(vTaskCoreCheck, "Task_Alpha", 1024, (void *)"Alpha", 2, NULL);
    xTaskCreate(vTaskCoreCheck, "Task_Beta",  1024, (void *)"Beta",  2, NULL);
    xTaskCreate(vTaskHeartbeat, "Heartbeat",  1024, NULL,           1, NULL);

    // Uruchomienie harmonogramu FreeRTOS
    vTaskStartScheduler();

    // Kod nigdy nie powinien tu dotrzeć
    while (true) {
        tight_loop_contents();
    }

    return 0;
}