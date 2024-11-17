/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * engine.h: Execution engine
 * Execution engine handles VM cores and their execution.
 * It adapts for single-threaded execution and multi-core execution.
 */

#ifndef TYPE_V_ENGINE_H
#define TYPE_V_ENGINE_H

#include <stdlib.h>
#include <stdint.h>
#include "core.h"
#include "dynlib/dynlib.h"

// Hard limit on the number of cores
#define MAX_CORES 256

/**
 * @brief Engine Health Engine health is used to determine whether the engine is healthy or not, from an API perspective.
 */
typedef enum TypeV_EngineHealth {
    EH_OK = 0,        ///< A healthy engine means that all cores are sharing a fair amount of CPU time.
    EH_SLUGGISH = 1,  ///< A sluggish engine means that some cores are barely getting enough CPU time.
    EH_UNHEALTHY = 2, ///< An unhealthy engine means that some cores are not getting any CPU time.
    EH_ZOMBIE = 3,    ///< A zombie engine means that all cores are getting almost no CPU time.
} TypeV_EngineHealth;


typedef struct TypeV_CoreIterator {
    TypeV_Core* core;
    int32_t maxInstructions;
    int32_t currentInstructions;
    struct TypeV_CoreIterator* next;
}TypeV_CoreIterator;

typedef struct TypeV_EngineFFI{
    char* dynlibName;
    TV_LibraryHandle dynlibHandle;
    TypeV_FFI* ffi;
}TypeV_EngineFFI;

/**
 * @brief: TypeV_Engine: The execution engine: Array of cores
 */
typedef struct TypeV_Engine {
    char* srcFileMap;                           ///< Source file map
    TypeV_EngineHealth health;
    TypeV_CoreIterator* coreIterator;
    uint32_t coreCount;                         ///< number of living cores
    uint32_t runningCoresCount;                 ///< number of running cores
    uint32_t mainCoreExitCode;                  ///< Exit code of the main core
    uint8_t interruptNextLoop;                  ///< interrupt the next loop, set to true when cores are spawned/killed
    TypeV_EngineFFI** ffi;                      ///< FFI libraries
    uint16_t ffiCount;                          ///< Number of FFI libraries
} TypeV_Engine;


/**
 * @brief engine_init Initialize the engine
 * @param engine
 */
void engine_init(TypeV_Engine *engine);

/**
 * @brief engine_run Run the engine
 * @param engine
 */
void engine_run(TypeV_Engine *engine);

/**
 * @brief engine_run_core Run a single core
 * @param engine
 * @param core
 */
void engine_run_core(TypeV_Engine *engine, TypeV_CoreIterator* iter);

/**
 * @brief engine_setmain Set the main core
 * @param engine
 * @param program
 * @param programLength
 * @param constantPool
 * @param constantPoolLength
 * @param globalPool
 * @param globalPoolLength
 * @param stackCapacity
 * @param stackLimit
 */
void engine_setmain(TypeV_Engine *engine, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength,  uint8_t* templatePool, uint64_t templatePoolLength, uint64_t stackCapacity, uint64_t stackLimit);

/**
 * @brief engine_deallocate Deallocate the engine
 * @param engine
 */
void engine_deallocate(TypeV_Engine *engine);

/**
 * @brief engine_generateNewCoreID Generate a new core ID
 * @param engine
 * @return
 */
uint32_t engine_generateNewCoreID(TypeV_Engine *engine);

/**
 * @brief engine_update_scheduler Update the scheduler
 * The scheduler is responsible for determining which core should be executed next.
 * @param engine
 */
void engine_update_scheduler(TypeV_Engine *engine);

/**
 * @brief engine_spawnCore Spawn a new core
 * @param engine
 * @param parentCore The parent core
 * @param ip The instruction pointer which references the init function of the new core
 *
 */
TypeV_Core* engine_spawnCore(TypeV_Engine *engine, TypeV_Core* parentCore, uint64_t ip);

/**
 * @brief Detaches a core
 * @param engine
 * @param coreID
 */
void engine_detach_core(TypeV_Engine *engine, TypeV_Core* core);


void engine_ffi_register(TypeV_Engine *engine, char* dynlibName, uint16_t dynlibID);
void engine_ffi_open(TypeV_Engine *engine, uint16_t dynlibID);
size_t engine_ffi_get(TypeV_Engine *engine, uint16_t dynlibID, uint8_t methodId);
void engine_ffi_close(TypeV_Engine *engine, uint16_t dynlibID);

#endif //TYPE_V_ENGINE_H
