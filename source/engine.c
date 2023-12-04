
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "core.h"
#include "instructions/opfuncs.h"

void engine_init(TypeV_Engine *engine) {
    // we will allocate memory for cores later
    engine->coreCount = 0;
    engine->health = EH_OK;
    engine->coreIterator = calloc(1, sizeof(TypeV_CoreIterator));
    engine->coreIterator->core = calloc(1, sizeof(TypeV_Core));
    engine->coreIterator->next = NULL;

    core_init(engine->coreIterator->core, engine_generateNewCoreID(engine), engine);
    engine->coreCount++;
}

void engine_setmain(TypeV_Engine *engine, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit){
    core_setup(engine->coreIterator->core, program, programLength, constantPool, constantPoolLength, globalPool, globalPoolLength, stackCapacity, stackLimit);
}

void engine_deallocate(TypeV_Engine *engine) {
    //TODO: free cores first

}

void engine_run(TypeV_Engine *engine) {
    int coreIterator = -1;
    engine_update_scheduler(engine);

    while(1) {
        if(engine->interruptNextLoop){
            engine->interruptNextLoop = 0;
        }

        if(engine->coreCount == 1) {
            engine_run_core(engine, engine->coreIterator);
        }
        else {
            TypeV_CoreIterator* iter = engine->coreIterator;
            while(iter != NULL){
                iter->currentInstructions = 0;
                engine_run_core(engine, iter);
                iter = iter->next;
            }
        }

        if(engine->coreCount == 0) {
            break;
        }
    }
}

void engine_run_core(TypeV_Engine *engine, TypeV_CoreIterator* iter) {
    uint8_t runInf = iter->maxInstructions == -1;
    TypeV_Core * core = iter->core;

    while((core->state == CS_RUNNING) && (iter->currentInstructions != iter->maxInstructions) && !engine->interruptNextLoop){
        iter->currentInstructions += 1-runInf;
        TypeV_OpCode opcode = core->program.bytecode[core->registers.ip++];
        op_funcs[opcode](core);
    }
}

uint32_t engine_generateNewCoreID(TypeV_Engine *engine) {
    return engine->coreCount+1;
}

void engine_update_scheduler(TypeV_Engine *engine) {
    if(!engine->coreCount){return;}
    if(engine->coreCount == 1) {
        engine->coreIterator->maxInstructions = -1;
        engine->coreIterator->currentInstructions = 0;
    }
    else {
        // iterate over all cores and set maxInstructions to 1
        TypeV_CoreIterator* iter = engine->coreIterator;
        while(iter != NULL){
            iter->maxInstructions = 20;
            iter->currentInstructions = 0;
            iter = iter->next;
        }
    }
}

TypeV_Core* engine_spawnCore(TypeV_Engine *engine, TypeV_Core* parentCore, uint64_t ip) {
    uint32_t id = engine_generateNewCoreID(engine);
    TypeV_Core* newCore = malloc(sizeof(TypeV_Core));


    core_init(newCore, id, engine);
    engine->coreCount++;

    newCore->registers.ip = ip;

    core_setup(newCore,
               parentCore->program.bytecode,
               parentCore->program.length,
               parentCore->constantPool.pool,
               parentCore->constantPool.length,
               parentCore->globalPool.pool,
               parentCore->globalPool.length,
               parentCore->stack.capacity,
               parentCore->stack.limit);

    // now we need to copy the stack from fp upwards to the new core stack
    memcpy(newCore->stack.stack,
           parentCore->stack.stack + parentCore->registers.fp,
           parentCore->stack.capacity - parentCore->registers.fp);

    // set stack pointer to size of args
    newCore->registers.sp = parentCore->registers.sp - parentCore->registers.fp;

    // fp is zero because we are the first frame
    newCore->registers.fp = 0;

    // fe is the size of the stack
    newCore->registers.fe = newCore->registers.sp;

    // add iterator and attack to engine
    TypeV_CoreIterator* newCoreIterator = calloc(1, sizeof(TypeV_CoreIterator));
    newCoreIterator->next = NULL;
    newCoreIterator->currentInstructions = 0;
    newCoreIterator->maxInstructions = 0;
    newCoreIterator->core = newCore;

    if(engine->coreIterator == NULL) {
        engine->coreIterator = newCoreIterator;
    } else {
        TypeV_CoreIterator* iterator = engine->coreIterator;
        while(iterator->next != NULL) {
            iterator = iterator->next;
        }
        iterator->next = newCoreIterator;
    }

    engine_update_scheduler(engine);

    return newCore;
}

void engine_detach_core(TypeV_Engine *engine, TypeV_Core* core) {
    // find the core in the iterator list
    TypeV_CoreIterator* iterator = engine->coreIterator;
    TypeV_CoreIterator* prevIterator = NULL;
    while(iterator != NULL) {
        if(iterator->core == core) {
            // found it
            if(prevIterator == NULL) {
                // we are the first iterator
                engine->coreIterator = iterator->next;
            } else {
                prevIterator->next = iterator->next;
            }
            //TODO: Free iterator here
            //free(iterator);
            break;
        }
        prevIterator = iterator;
        iterator = iterator->next;
    }

    engine->coreCount--;
    engine_update_scheduler(engine);
}