#pragma once 

bool telemetry_task_init();
void vTelemetryTask(void *pvParameters);
bool telemetry_task_request();