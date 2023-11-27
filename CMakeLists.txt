cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 99)

add_executable(type_v source/main.c source/core.c source/core.h source/queue/queue.c source/queue/queue.h source/engine.c source/engine.h source/instructions/instructions.c source/instructions/instructions.h source/stack/stack.c source/stack/stack.h source/platform/platform.h source/utils/utils.c source/utils/utils.h)
