#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>

/* Zamiast pico/config.h deklarujemy tylko zmienną zegara systemowego */
extern uint32_t SystemCoreClock;
#endif

/* Specyficzne dla RP2350 (Cortex-M33) */
#define configENABLE_FPU                        1
#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1

/* Priorytety przerwań wymagane przez port RP2350_ARM_NTZ */
#define configKERNEL_INTERRUPT_PRIORITY         ( 255 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 16 )
#define configMAX_API_CALL_INTERRUPT_PRIORITY   ( 16 )

/* System & Scheduler */
#define configCPU_CLOCK_HZ                         SystemCoreClock
#define configUSE_PREEMPTION                       1
#define configUSE_TIME_SLICING                     1
#define configUSE_TICKLESS_IDLE                    0
#define configTICK_RATE_HZ                         ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                       32
#define configMINIMAL_STACK_SIZE                   ( ( configSTACK_DEPTH_TYPE ) 512 )
#define configUSE_16_BIT_TICKS                     0
#define configIDLE_SHOULD_YIELD                    1

/* Hooks */
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configCHECK_FOR_STACK_OVERFLOW             0
#define configUSE_MALLOC_FAILED_HOOK               0
#define configUSE_DAEMON_TASK_STARTUP_HOOK         0

/* Synchronization */
#define configUSE_MUTEXES                          1
#define configUSE_RECURSIVE_MUTEXES                1
#define configUSE_COUNTING_SEMAPHORES              1
#define configQUEUE_REGISTRY_SIZE                  8
#define configUSE_QUEUE_SETS                       1

/* Wymagane dla lwIP (sys_arch) i async_context z Pico SDK */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    5

/* Memory Allocation */
#define configSUPPORT_STATIC_ALLOCATION            0
#define configSUPPORT_DYNAMIC_ALLOCATION           1
#define configTOTAL_HEAP_SIZE                      ( 128 * 1024 )

/* Timers */
#define configUSE_TIMERS                           1
#define configTIMER_TASK_PRIORITY                  ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                   10
#define configTIMER_TASK_STACK_DEPTH               1024

/* SMP (Symmetric Multiprocessing - RP2350 / 2 rdzenie) */
#define configNUMBER_OF_CORES                      2
#define configNUM_CORES                            configNUMBER_OF_CORES
#define configTICK_CORE                            0
#define configRUN_MULTIPLE_PRIORITIES              1
#define configUSE_CORE_AFFINITY                    1
#define configUSE_PASSIVE_IDLE_HOOK                0

/* Mapowanie przerwania SIO dla RP2350 */
#ifndef SIO_IRQ_PROC0
#define SIO_IRQ_PROC0                              26
#endif

/* RP2040 / RP2350 Interop */
#define configSUPPORT_PICO_SYNC_INTEROP            1
#define configSUPPORT_PICO_TIME_INTEROP            1

/* RP2350 Specific Settings */
#define configENABLE_MPU                           0
#define configENABLE_TRUSTZONE                     0
#define configRUN_FREERTOS_SECURE_ONLY             1
#define configENABLE_FPU                           1

#include <assert.h>
#define configASSERT(x)                            assert(x)

/* API Include Definitions */
#define INCLUDE_vTaskPrioritySet                   1
#define INCLUDE_uxTaskPriorityGet                  1
#define INCLUDE_vTaskDelete                        1
#define INCLUDE_vTaskSuspend                       1
#define INCLUDE_vTaskDelayUntil                    1
#define INCLUDE_vTaskDelay                         1
#define INCLUDE_xTaskGetSchedulerState             1
#define INCLUDE_xTaskGetCurrentTaskHandle          1
#define INCLUDE_uxTaskGetStackHighWaterMark        1
#define INCLUDE_xTaskGetIdleTaskHandle             1
#define INCLUDE_eTaskGetState                      1
#define INCLUDE_xTimerPendFunctionCall             1
#define INCLUDE_xSemaphoreGetMutexHolder           1

#ifdef __cplusplus
extern "C" {
#endif

#define configUSE_NEWLIB_REENTRANT                 1

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_CONFIG_H */