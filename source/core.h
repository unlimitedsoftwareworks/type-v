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

#define PTR_SIZE sizeof(void*)
#define MAX_REG 20


typedef struct TypeV_Struct {
    uint16_t* fieldOffsets;
    uint8_t data[];
}TypeV_Struct;

typedef struct TypeV_Class{
    /** methods */
    uint16_t* methodsOffset;  ///< method offset table
    size_t* methods;          ///< A pointer to the method table
    /** fields */
    uint16_t* fieldsOffset;     ///< field offset table
    uint8_t data[];           ///< Fields start from here, direct access
}TypeV_Class;

typedef struct TypeV_Interface {
    uint16_t* methodsOffset;  ///< method offset table
    TypeV_Class* classPtr;    ///< Pointer to the class that implements this interface
}TypeV_Interface;


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
    TypeV_Register regs[MAX_REG]; ///< General purpose registers, R0 -> R15 for general purpose,
                             ///< R16 for structs, R17 for classes and 18 for interfaces
                             ///< R19 for function return value

    uint64_t flags;          ///< Flags
    uint64_t ip;             ///< Instruction counter

    uint64_t fp;             ///< Frame pointer
    uint64_t fe;             ///< Frame end pointer
    uint64_t sp;             ///< Stack pointer
}TypeV_Registers;

/**
 * @brief Core stack
 */
typedef struct TypeV_Stack {
    uint8_t *stack;    ///< Stack
    uint64_t capacity; ///< Stack capacity
    uint64_t limit;    ///< Stack limit
}TypeV_Stack;

/**
 * @brief Future GC, right now it only holds
 * references of the objects given.
 */
typedef struct TypeV_GC {
    TypeV_Class** classes;
    uint64_t classCount;
    TypeV_Interface** interfaces;
    uint64_t interfaceCount;
    TypeV_Struct** structs;
    uint64_t structCount;
}TypeV_GC;

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
    TypeV_GC memTracker;                      ///< Future Garbage collector

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



/**
 * Allocates a struct object
 * @param core
 * @param numfields Number of struct fieldOffsets
 * @param totalsize Total size of the struct
 * @return Pointer to the allocated struct
 */
size_t core_alloc_struct(TypeV_Core *core, uint8_t numfields, size_t totalsize);

/**
 * Allocates a struct object as shadow to another struct
 * @param core
 * @param numfields Number of struct fieldOffsets
 * @param totalsize Total size of the struct
 * @return Pointer to the allocated struct
 */
size_t core_alloc_struct_shadow(TypeV_Core *core, uint8_t numfields, size_t originalStruct);

/**
 * Allocates a class object
 * @param core
 * @param numfields number of class fields/attributes
 * @param total_fields_size  total size of fields in bytes
 * @return new Class object, half initialized, methods must be initialized after this call
 */
size_t core_alloc_class_fields(TypeV_Core *core, uint8_t numfields, size_t total_fields_size);

/**
 * Allocates a class method table
 * @param core
 * @param num_methods number of methods
 * @param class_ptr class reference
 */
void core_alloc_class_methods(TypeV_Core *core, uint8_t num_methods, TypeV_Class* class_ptr);

/**
 * Allocates an interface object
 * @param core
 * @param num_methods number of methods
 * @param class_ptr class reference
 */
size_t core_alloc_interface(TypeV_Core *core, uint8_t num_methods, TypeV_Class* class_ptr);
#endif //TYPE_V_CORE_H
