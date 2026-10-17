#pragma once

#include "comm/protocol.h"

bool control_task_init();
bool control_task_post_command(protocol::CmdId cmd);