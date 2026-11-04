#pragma once

#include <cstdint>

bool sonar_task_init();
bool sonar_task_start();
bool sonar_task_stop();

enum class SonarCommand : uint32_t
{
    START = 1,
    STOP  = 2
};