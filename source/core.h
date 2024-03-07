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

#define PTR_SIZE 8
#define MAX_REG 256


typedef struct TypeV_Struct {
    uint16_t* fieldOffsets;                 //< Field offsets table
    struct TypeV_Struct* originalStruct;    //< Pointer to the original struct, NULL if this is not a shadow struct
    uint8_t* dataPointer;                   //< Pointer to the data
    uint8_t data[];                         //< Data block, if this is a shadow struct,
                                            // this is a pointer to the original struct's data
}TypeV_Struct;

typedef struct TypeV_Class{
    uint64_t uid;             ///< Unique ID
    uint8_t num_methods;      ///< number of methods
    size_t* methods;          ///< A pointer to the method table
    /** data */
    uint8_t data[];            ///< Fields start from here, direct access
}TypeV_Class;

typedef struct TypeV_Interface {
    TypeV_Class* classPtr;    ///< Pointer to the class that implements this interface
    uint16_t methodsOffset[];  ///< method offset table
}TypeV_Interface;

typedef struct TypeV_Array {
    uint8_t elementSize;      ///< Size of each element
    uint64_t length;          ///< Array length
    uint8_t* data;            ///< Array data
}TypeV_Array;

typedef struct TypeV_Promise {
    uint8_t resolved;       ///< resolved flag
    size_t value;           ///< Promise value
    uint32_t id;            ///< Promise ID, for debugging
}TypeV_Promise;

typedef struct TypeV_Lock {
    uint8_t locked;         ///< locked flag
    uint32_t holder;         ///< holder ID
    uint32_t id;            ///< lock ID
    size_t value;           ///< Lock value
    TypeV_Promise *promise; ///< Promise that the lock is awaiting, NULL if none
}TypeV_Lock;



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
    uintptr_t ptr;
} TypeV_Register;

/**
 * State
 */
typedef enum {
    CS_INITIALIZED = 0,   ///< Initialized state
    CS_HALTED = 1,        ///< Halted as the VM is running another core, or process
    CS_AWAITING_QUEUE,    ///< Process is awaiting for a queue
    CS_AWAITING_PROMISE,  ///< Process is awaiting for a promise
    CS_RUNNING,           ///< Process is Running
    CS_FINISHING,         ///< Process has received terminate signal and is no longer accepting messages
    CS_TERMINATED,        ///< Process has been gracefully terminated
    CS_KILLED   ,          ///< Process has been killed
    CS_CRASHED            ///< Process has crashed
}TypeV_CoreState;

typedef enum {
    CSIG_NONE = 0,      ///< No signal
    CSIG_TERMINATE = 1, ///< Terminate signal
    CSIG_KILL = 2       ///< Kill signal
}TypeV_CoreSignal;

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


// zero flag
#define FLAG_ZF 0x01

// sign flag
#define FLAG_SF 0x02

// overflow flag
#define FLAG_OF 0x04

// carry flag
#define FLAG_CF 0x08

// type flag: 0 for unsigned, 1 for signed
#define FLAG_TF 0x10

#define FLAG_SET(flags, flag) ((flags) |= (flag))
#define FLAG_CLEAR(flags, flag) ((flags) &= ~(flag))
#define FLAG_CHECK(flags, flag) ((flags) & (flag))
#define WRITE_FLAG(flags, flag, value) ((value) ? ((flags) |= (flag)) : ((flags) &= ~(flag)))

/**
 * @brief Future GC, right now it only holds
 * references of the objects given.
 */
typedef struct TypeV_GC {
    uint64_t totalAllocs;
    uint64_t allocsSincePastGC;
    void** memObjects;
    uint64_t memObjectCount;
}TypeV_GC;


/**
 * @brief Object Types, used to identify a the underlying structure of GC allocated memory
 * object
 */
typedef enum {
    OT_CLASS,
    OT_PROCESS,
    OT_INTERFACE,
    OT_STRUCT,
    OT_STRUCT_SHADOW,
    OT_ARRAY,
    OT_RAWMEM,
}TypeV_ObjectType;


typedef struct {
    TypeV_ObjectType type;
    size_t size;
}TypeV_ObjectHeader;

/**
 * @brief A function state is an object that holds the state of a function. Since function arguments are passed
 * through registers, the function state holds the registers that are used by the function.
 * When a function calls another, it allocates the next function state using fn_init_state, switches the active
 * function state to the new one, using fn_call (automatically loads .next), and when the function returns, it
 * deallocates the current function state and gets the old context using fn_ret (automatically loads .prev).
 */
typedef struct TypeV_FuncState {
    uint8_t *stack;    ///< Stack
    uint64_t capacity; ///< Stack capacity
    uint64_t limit;    ///< Stack limit
    uint64_t flags;          ///< Flags
    uint64_t sp;             ///< Stack pointer
    uint64_t ip;             ///< Instruction pointer, used only as back up
    TypeV_Register regs[MAX_REG]; ///< 256 registers.
    TypeV_Register* spillSlots; ///< Spill slots, used when registers are not enough
    struct TypeV_FuncState* next; ///< Next function state, used with fn_call or fn_call_i
    struct TypeV_FuncState* prev; ///< Previous function state, used fn_ret
}TypeV_FuncState;

/**
 * @brief Closure, a closure is a function that has captured its environment.
 * TODO: closures are still not implemented
 */

typedef struct TypeV_Closure {
    void* fnPtr; ///< Function pointer
    TypeV_Register* capturedRegs; ///< Captured registers
    uint32_t envSize; ///< Environment size
    uint32_t refCount; ///< Reference count
}TypeV_Closure;

/**
 * @brief Core structure, a core is the equivalent of a process in type-c.
 * Each core runs independently from each other, and communicates through message passing.
 * Hence the isolation of registers, stack, and program.
 */
typedef struct TypeV_Core {
    uint32_t id;                              ///< Core ID
    uint8_t isRunning;                        ///< Is the core running
    TypeV_CoreState state;                    ///< Core state

    TypeV_IOMessageQueue messageInputQueue;   ///< Message input queue
    TypeV_GC memTracker;                      ///< Future Garbage collector

    struct TypeV_Engine* engineRef;           ///< Reference to the engine. Not part of the core state, just to void adding to every function call.
    TypeV_CoreSignal lastSignal;              ///< Last signal received
    TypeV_Promise* awaitingPromise;           ///< Promise that the core is awaiting, NULL if none

    uint32_t exitCode;                        ///< Exit code

    const uint8_t* constPtr;                        ///< Constant pointer
    const uint8_t* templatePtr;                     ///< Template pointer
    const uint8_t* codePtr;                         ///< Code pointer

    const uint8_t* globalPtr;                       ///< Global pointer
    uint64_t ip;                              ///< Instruction pointer

    TypeV_Register* regs;                     ///< Registers, pointer to current function state registers for faster access.
    uint16_t* flags;                          ///< Flags, pointer to current function state flags for faster access.
    TypeV_FuncState* funcState;               ///< Function state
}TypeV_Core;

TypeV_FuncState* core_create_function_state(TypeV_FuncState* prev);
void core_destroy_function_state(TypeV_Core* core, TypeV_FuncState** state);

/**
 * Initializes a core
 * @param core
 * @param id
 * @param engineRef
 */
void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef);
void core_setup(TypeV_Core *core, const uint8_t* program, const uint8_t* constantPool, const uint8_t* globalPool);

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
 * Sets core to await for a queue
 * @param core
 */
void core_queue_await(TypeV_Core* core);

/**
 * Awakes process
 * @param core
 */
void core_queue_resolve(TypeV_Core* core);


/**
 * Allocates a struct object
 * @param core
 * @param numfields Number of struct fieldOffsets
 * @param totalsize Total size of the struct
 * @return Pointer to the allocated struct
 */
uintptr_t  core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize);

/**
 * Allocates a struct object as shadow to another struct
 * @param core
 * @param numfields Number of struct fieldOffsets
 * @param totalsize Total size of the struct
 * @return Pointer to the allocated struct
 */
uintptr_t core_struct_alloc_shadow(TypeV_Core *core, uint8_t numfields, size_t originalStruct);

/**
 * Allocates a class object
 * @param core
 * @param num_methods Number of methods
 * @param total_fields_size  total size of fields in bytes
 * @return new Class object initialized.
 */
uintptr_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, size_t total_fields_size, uint64_t classId);

/**
 * Allocates an interface object
 * @param core
 * @param num_methods number of methods
 * @param class_ptr class reference
 */
uintptr_t core_interface_alloc(TypeV_Core *core, uint8_t num_methods, TypeV_Class* class_ptr);

/**
 * Allocates an interface object from another interface
 * inheriting its parent class
 * @param core
 * @param num_methods
 * @param interface_ptr
 * @return
 */
uintptr_t core_interface_alloc_i(TypeV_Core *core, uint8_t num_methods, TypeV_Interface* interface_ptr);
/**
 * Allocates an array object
 * @param core
 * @param num_elements
 * @param element_size
 * @return
 */
uintptr_t core_array_alloc(TypeV_Core *core, uint64_t num_elements, uint8_t element_size);

/**
 * Extends the size of an array
 * @param core
 * @param array_ptr
 * @param num_elements new number of elements
 * @return
 */
uintptr_t core_array_extend(TypeV_Core *core, size_t array_ptr, uint64_t num_elements);

/**
 * Slices an array, creating a shallow copy.
 * the new array has an independent data block but still the content of the data
 * block is copied from the original array.
 * @param array
 * @param start
 * @param end
 * @return
 */
uintptr_t core_array_slice(TypeV_Core *core, TypeV_Array* array, uint64_t start, uint64_t end);

/**
 * Load a FFI library
 * @param core
 * @param namePointer
 * @return
 */
uintptr_t core_ffi_load(TypeV_Core* core, size_t namePointer);

/**
 * Closes a FFI library
 * @param core
 * @param libHandle library handle pointer
 */
void core_ffi_close(TypeV_Core* core, size_t libHandle);
/**
 * Allocates a memory object.
 * All memory allocated here is considered heap
 * and collectible by GC.
 * @param core
 * @param size
 * @return
 */
uintptr_t core_mem_alloc(TypeV_Core* core, size_t size);

/**
 * Updates the flags of the core
 * @param core
 * @param value
 */
void core_update_flags(TypeV_Core *core, uint64_t value);

void core_process_alloc(TypeV_Core* core, uint64_t ip);


/**
 * Sends a message to the core's queue
 * @param core
 * @param message
 */
void core_enqueue_message(TypeV_Core* core, TypeV_IOMessage* message);

/**
 * Handles the reception of a signal
 * @param core
 * @param signal
 */
void core_receive_signal(TypeV_Core* core, TypeV_CoreSignal signal);

/**
 * Allocates a new lock
 * @param core
 * @return
 */
TypeV_Lock* core_lock_alloc(TypeV_Core* core, size_t value);

/**
 * Acquires a lock
 * @param core
 * @param lock
 */
void core_lock_acquire(TypeV_Core* core, TypeV_Lock* lock);

/**
 * Releases a lock
 * @param core
 * @param lock
 */
void core_lock_release(TypeV_Core* core, TypeV_Lock* lock);


/**
 * Allocates new core
 * @param core
 * @return
 */
TypeV_Promise* core_promise_alloc(TypeV_Core* core);

/**
 * Resolves a promise
 * @param core
 * @param promise
 * @param value
 */
void core_promise_resolve(TypeV_Core* core, TypeV_Promise* promise, size_t value);

/**
 * Halts the core to await a promise
 * @param core
 * @param promise
 */
void core_promise_await(TypeV_Core* core, TypeV_Promise* promise);

/**
 * Checks if the core's waiting promise has been resolved
 * and can be resumed
 * @param core
 */
void core_promise_check_resume(TypeV_Core* core);

/**
 * Throws an error and terminates the core
 * @param core
 * @param errorId
 * @param fmt Message printf style format, followed by arguments
 */
void core_panic(TypeV_Core* core, uint32_t errorId, char* fmt, ...);

void core_spill_alloc(TypeV_Core* core, uint16_t size);

typedef void (*TypeV_FFIFunc)(TypeV_Core* core);
typedef struct TypeV_FFI {
    const TypeV_FFIFunc* functions;///< FFI functions
    uint8_t functionCount;   ///< FFI function count
}TypeV_FFI;

/**
 * @brief Adds the given object to the GC tracker, object must be a header+data block, i.e
 * allocated using the available API core_*_alloc functions.
 * @param core
 * @param object
 */
void core_gc_track_alloc(TypeV_Core* core, void* object);

#endif //TYPE_V_CORE_H
