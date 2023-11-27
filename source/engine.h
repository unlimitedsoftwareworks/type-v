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

/**
 * @brief Engine Health Engine health is used to determine whether the engine is healthy or not, from an API perspective.
 */
typedef enum TypeV_EngineHealth {
    EH_OK = 0,        ///< A healthy engine means that all cores are sharing a fair amount of CPU time.
    EH_SLUGGISH = 1,  ///< A sluggish engine means that some cores are barely getting enough CPU time.
    EH_UNHEALTHY = 2, ///< An unhealthy engine means that some cores are not getting any CPU time.
    EH_ZOMBIE = 3,    ///< A zombie engine means that all cores are getting almost no CPU time.
} TypeV_EngineHealth;

/**
 * @brief: TypeV_Engine: The execution engine: Array of cores
 */
typedef struct TypeV_Engine {
    TypeV_EngineHealth health;
    TypeV_Core *cores;
    uint32_t coreCount;
} TypeV_Engine;


void engine_init(TypeV_Engine *engine);
void engine_run(TypeV_Engine *engine);
void engine_setmain(TypeV_Engine *engine, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit);
void engine_deallocate(TypeV_Engine *engine);
uint32_t engine_generateNewCoreID(TypeV_Engine *engine);
void engine_spawnCore(TypeV_Engine *engine);
#endif //TYPE_V_ENGINE_H
