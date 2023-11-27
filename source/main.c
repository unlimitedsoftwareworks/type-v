#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "platform/platform.h"
#include "instructions/instructions.h"

int main() {
    fprintf(stdout, "Type-V VM %s, %s, %s\n", OS_NAME, CPU_ARCH, COMPILER);
    TypeV_Engine engine;
    engine_init(&engine);

    uint8_t globalPool[]= {0x80};
    uint64_t globalPoolLength = 1;

    uint8_t constPool [] = {0xc3, 0x5f, 0x48, 0x40};
    uint64_t constPoolLength = 4;

    uint8_t program [] = {
            OP_MV_REG_CONST_32, 0, 1, 0,
            OP_DEBUG_REGS,
            OP_MV_REG_REG, 1, 0, 4,
            OP_DEBUG_REGS,
            OP_MV_REG_GLOBAL_8, 2, 1, 0,
            OP_DEBUG_REGS,
            OP_HALT
    };

    uint64_t programLength = 11;
    engine_setmain(&engine, program, programLength, constPool, constPoolLength, globalPool, globalPoolLength, 1024, 1024);

    engine_run(&engine);

    engine_deallocate(&engine);
    return 0;
}
