/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * core.h: Core Architecture definition
 * a Type-C program is composed of bunch of a processes. Each process has its own registers, stack, and program.
 * Each process can communicate with other processes through message passing.
 */

#ifndef TYPE_V_CORE_H
#define TYPE_V_CORE_H

#include <stdint.h>
#include "queue/queue.h"

/**
 * @brief TypeV_Register
 */
typedef union {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
    size_t ptr;
} TypeV_Register;

/**
 * State
 */
typedef enum TypeV_CoreState {
    CS_INITIALIZED = 0, ///< Initialized state
    CS_HALTED = 1,      ///< Halted as the VM is running another core, or process
    CS_RUNNING = 2,     ///< Process is Running
    CS_WAITING = 3,     ///< Halted  because its queue is empty
    CS_FINISHING = 4,   ///< Process has received terminate signal and is no longer accepting messages
    CS_TERMINATED = 5,  ///< Process has been gracefully terminated
    CS_KILLED = 7       ///< Process has been killed
}TypeV_CoreState;

/**
 * @brief TypeV Program bytecode
 */
typedef struct TypeV_Program {
    uint8_t *bytecode; ///< Program bytecode
    uint64_t length;  ///< Program length
}TypeV_Program;

/**
 * @brief Process constant pool
 */
typedef struct TypeV_ConstantPool {
    uint8_t *pool;   ///< Constant pool
    uint64_t length; ///< Constant pool length
}TypeV_ConstantPool;

/**
 * @brief global pool
 */
typedef struct TypeV_GlobalPool {
    uint8_t *pool;   ///< Constant pool
    uint64_t length; ///< Constant pool length
}TypeV_GlobalPool;
/**
 * @breif Core registers
 */
typedef struct TypeV_Registers {
    TypeV_Register regs[18]; ///< General purpose registers
    uint64_t ip;             ///< Instruction counter
    uint64_t flags;          ///< Flags
    uint64_t fp;             ///< Frame pointer
}TypeV_Registers;

/**
 * @brief Core stack
 */
typedef struct TypeV_Stack {
    uint8_t *stack;    ///< Stack
    uint64_t capacity; ///< Stack capacity
    uint64_t limit;    ///< Stack limit
    uint64_t sp;       ///< Stack pointer
}TypeV_Stack;


/**
 * @brief Core structure, a core is the equivalent of a process in type-c.
 * Each core runs independently from each other, and communicates through message passing.
 * Hence the isolation of registers, stack, and program.
 */
typedef struct TypeV_Core {
    uint32_t id;                              ///< Core ID
    TypeV_CoreState state;                    ///< Core state
    TypeV_Registers registers;                ///< Registers
    TypeV_Stack stack;                        ///< Stack
    TypeV_IOMessageQueue messageInputQueue;   ///< Message input queue
    TypeV_ConstantPool constantPool;          ///< Constant pool
    TypeV_GlobalPool globalPool;              ///< Global pool
    TypeV_Program program;                    ///< Program

    struct TypeV_Engine* engineRef;          ///< Reference to the engine. Not part of the core state, just to void adding to every function call.
}TypeV_Core;

/**
 * Initializes a core
 * @param core
 * @param id
 * @param engineRef
 */
void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef);

void core_setup(TypeV_Core *core, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit);

/**
 * Deallocates a core
 * @param core
 */
void core_deallocate(TypeV_Core *core);


/**
 * Main loop of core execution
 * @param core
 */
void core_vm(TypeV_Core *core);

/**
 * Resumes the execution of a halted core
 * @param core
 */
void core_resume(TypeV_Core *core);

/**
 * Halts the execution of a core, usually to switch to another core
 * @param core
 */
void core_halt(TypeV_Core *core);

#endif //TYPE_V_CORE_H
