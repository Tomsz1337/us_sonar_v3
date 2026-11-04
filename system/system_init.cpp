#include "system_init.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tasks/comm_task.h"
#include "tasks/control_task.h"
#include "tasks/data_queue.h"
#include "tasks/telemetry_task.h"
#include "tasks/sonar_task.h"

bool system_init()
{
    if (!data_queue_init()){return false;}
    if (!control_task_init()){return false;}
    if (!comm_task_init()){return false;}
    if (!telemetry_task_init()){return false;}
    if (!sonar_task_init()){return false;}

    return true;
}
