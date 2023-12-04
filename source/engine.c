
#include <time.h>
#include <stdio.h>

#include "engine.h"
#include "core.h"
#include "instructions/opfuncs.h"

void engine_init(TypeV_Engine *engine) {
    // we will allocate memory for cores later
    engine->health = EH_OK;
    engine->coreCount = 0;
    engine->cores = malloc(sizeof(TypeV_Core));
    core_init(engine->cores, engine_generateNewCoreID(engine), engine);
    engine->coreCount++;
}

void engine_setmain(TypeV_Engine *engine, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit){
    core_setup(engine->cores, program, programLength, constantPool, constantPoolLength, globalPool, globalPoolLength, stackCapacity, stackLimit);
}

void engine_deallocate(TypeV_Engine *engine) {
    //TODO: free cores first
    free(engine->cores);
    engine->cores = NULL;
}

void engine_run(TypeV_Engine *engine) {
    engine_run_core(engine, engine->cores);
}

void engine_run_core(TypeV_Engine *engine, TypeV_Core* core) {
    clock_t start, end;
    double cpu_time_used;
    long sum = 0;

    start = clock();
    core->isRunning = 1;
    while(core->isRunning){
        TypeV_OpCode opcode = core->program.bytecode[core->registers.ip++];
        op_funcs[opcode](core);
    }
    end = clock();

    // Calculate the CPU time used in seconds
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("\nExecution time: %f seconds\n", cpu_time_used);
}

uint32_t engine_generateNewCoreID(TypeV_Engine *engine) {
    return 1;
}