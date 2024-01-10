
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "assembler/assembler.h"
#include "core.h"
#include "instructions/opfuncs.h"
#include "utils/log.h"
#include "utils/utils.h"

void engine_init(TypeV_Engine *engine) {
    // we will allocate memory for cores later
    engine->coreCount = 0;
    engine->runningCoresCount = 0;
    engine->health = EH_OK;
    engine->coreIterator = calloc(1, sizeof(TypeV_CoreIterator));
    engine->coreIterator->core = calloc(1, sizeof(TypeV_Core));
    engine->coreIterator->next = NULL;

    engine->ffiCount = 0;

    core_init(engine->coreIterator->core, engine_generateNewCoreID(engine), engine);
    engine->coreCount++;
}

void engine_setmain(TypeV_Engine *engine, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit){
    core_setup(engine->coreIterator->core, program, constantPool, globalPool);
}

void engine_deallocate(TypeV_Engine *engine) {
    // free cores
    TypeV_CoreIterator* iterator = engine->coreIterator;
    while(iterator != NULL) {
        TypeV_CoreIterator* next = iterator->next;
        core_deallocate(iterator->core);
        free(iterator->core);
        free(iterator);
        iterator = next;
    }
    engine->coreIterator = NULL;
    engine->coreCount = 0;
}

void engine_run(TypeV_Engine *engine) {
    int coreIterator = -1;
    engine_update_scheduler(engine);

    while(1) {
        if(engine->interruptNextLoop){
            engine->interruptNextLoop = 0;
        }

        TypeV_CoreIterator* iter = engine->coreIterator;
        while(iter != NULL){
            iter->currentInstructions = 0;
            if(iter->core->lastSignal == CSIG_KILL) {
                // kill the core
                LOG_INFO("Core[%d] killed", iter->core->id);
                engine_detach_core(engine, iter->core);
                iter = iter->next;
                continue;
            }
            engine_run_core(engine, iter);
            iter = iter->next;
        }

        if(engine->coreCount == 0) {
            break;
        }
    }
}

void engine_run_core(TypeV_Engine *engine, TypeV_CoreIterator* iter) {
    uint8_t runInf = iter->maxInstructions == -1;
    TypeV_Core * core = iter->core;
    if(core->state == CS_HALTED) {
        core_resume(core);
    }

    if((core->state == CS_AWAITING_QUEUE) && (core->lastSignal == CSIG_TERMINATE)){
        LOG_INFO("Core[%d] Gracefully terminated", iter->core->id);
        engine_detach_core(engine, core);
        return;
    }

    if(core->state == CS_CRASHED){
        LOG_INFO("Core[%d] Crashed");
        engine_detach_core(engine, core);
        return;
    }

    if(core->state == CS_AWAITING_PROMISE) {
        core_promise_check_resume(core);
        if(core->state == CS_AWAITING_PROMISE) {
            LOG_WARN("Core[%d] is awaiting promise %d, skipping run", iter->core->id, core->awaitingPromise->id);
        }
    }

    while((core->state == CS_RUNNING) &&
        (iter->currentInstructions != iter->maxInstructions) &&
        !engine->interruptNextLoop){
        iter->currentInstructions += 1-runInf;
        TypeV_OpCode opcode = core->codePtr[core->ip++];
        fprintf(stdout, "I[%d] %s\n", core->ip-1, instructions[opcode]);

        op_funcs[opcode](core);
    }

    // set process to halted, if was gracefully done
    if(core->state == CS_RUNNING && core->lastSignal == CSIG_NONE) {
        core_halt(core);
    }
}

uint32_t engine_generateNewCoreID(TypeV_Engine *engine) {
    return engine->coreCount+1;
}

void engine_update_scheduler(TypeV_Engine *engine) {
    engine->interruptNextLoop = 1;
    if(!engine->coreCount){return;}
    if(engine->coreCount == 1) {
        engine->coreIterator->maxInstructions = -1;
        engine->coreIterator->currentInstructions = 0;
    }
    else {
        // iterate over all cores and set maxInstructions to 1
        TypeV_CoreIterator* iter = engine->coreIterator;
        while(iter != NULL){
            iter->maxInstructions = 2;
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

    newCore->ip = ip;

    core_setup(newCore,
               parentCore->codePtr,
               parentCore->constPtr,
               parentCore->globalPtr);

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
    LOG_INFO("Core[%d] detached with status %d", core->id, core->state);
    // find the core in the iterator list
    if(core->id == 1) {
        // main core
        engine->mainCoreExitCode = core->exitCode;
    }
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
            // Free iterator here
            TypeV_CoreIterator* next = iterator->next;
            // These cause saniation issues, must fix later
            //free(iterator);
            iterator = next;
            prevIterator = NULL;

            // free the core
            //core_deallocate(core);
            //free(core);
            break;
        }
        prevIterator = iterator;
        iterator = iterator->next;
    }

    engine->coreCount--;
    engine_update_scheduler(engine);
}

void engine_ffi_register(TypeV_Engine *engine, char* dynlibName, uint16_t dynlibID) {
    if(dynlibID >= engine->ffiCount) {
        engine->ffiCount = dynlibID+1;
        engine->ffi = realloc(engine->ffi, sizeof(TypeV_EngineFFI)*engine->ffiCount);
    }

    engine->ffi[dynlibID] = malloc(sizeof(TypeV_EngineFFI));


    engine->ffi[dynlibID]->dynlibName = dynlibName;
    engine->ffi[dynlibID]->dynlibHandle = NULL;
    engine->ffi[dynlibID]->ffi = NULL;

    // load the library
    engine_ffi_open(engine, dynlibID);
}

void engine_ffi_open(TypeV_Engine *engine, uint16_t dynlibID) {
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];
    if(ffi->dynlibHandle != NULL) {
        return;
    }

    char* name = engine->ffi[dynlibID]->dynlibName;

    TV_LibraryHandle lib = ffi_dynlib_load(name);
    ASSERT(lib != NULL, "Failed to load library %s", ffi_find_dynlib(name));

    void* openLib = ffi_dynlib_getsym(lib, "typev_ffi_open");
    ASSERT(openLib != NULL, "Failed to open library %s", ffi_find_dynlib(name));
    size_t (*openFunc)() = openLib;
    ffi->ffi = (TypeV_FFI*)openFunc();
    ffi->dynlibHandle = lib;
    engine->ffi[dynlibID] = ffi;
}

size_t engine_ffi_get(TypeV_Engine *engine, uint16_t dynlibID, uint8_t methodId){
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];

    ASSERT(ffi->dynlibHandle != NULL, "Library %s not loaded", ffi_find_dynlib(ffi->dynlibName));
    ASSERT(ffi->ffi != NULL, "Library %s not opened", ffi_find_dynlib(ffi->dynlibName));
    ASSERT(methodId < ffi->ffi->functionCount, "Method %d not found in library %s", methodId, ffi_find_dynlib(ffi->dynlibName));

    return (size_t)ffi->ffi->functions[methodId];
}

void engine_ffi_close(TypeV_Engine *engine, uint16_t dynlibID) {
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];
    if(ffi->dynlibHandle == NULL) {
        return;
    }

    ffi_dynlib_unload(ffi->dynlibHandle);
    ffi->dynlibHandle = NULL;
    ffi->ffi = NULL;
    engine->ffi[dynlibID] = ffi;
}
