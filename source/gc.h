/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * gc.h: Garbage Collection
 */


#ifndef TYPE_V_GC_H
#define TYPE_V_GC_H

#include <stdlib.h>

void* core_gc_alloc(TypeV_Core* core, size_t size);

/**
 * @brief Updates the amount of allocations count of the GC tracker,
 * this is used to trigger a GC
 * @param core
 * @param mem
 */
void core_gc_update_alloc(TypeV_Core* core, size_t mem);
void core_gc_mark_object(TypeV_Core* core, TypeV_ObjectHeader * ptr);
void core_gc_sweep(TypeV_Core* core);
void core_gc_collect(TypeV_Core* core);
void core_gc_collect_state(TypeV_Core* core, TypeV_FuncState* state);
// similar to core_gc_collect_state, but only collects a single state
// and does not sweep
void core_gc_collect_single_state(TypeV_Core* core, TypeV_FuncState* state);

void core_gc_update_struct_field(TypeV_Core* core, TypeV_Struct* structPtr, void* ptr, uint16_t fieldIndex);
void core_gc_update_class_field(TypeV_Core* core, TypeV_Class* classPtr, void* ptr, uint16_t fieldIndex);
void core_gc_update_array_field(TypeV_Core* core, TypeV_Array* arrayPtr, void* ptr, uint64_t fieldIndex);
void core_gc_update_closure_env(TypeV_Core* core, TypeV_Closure* closurePtr, void* ptr);

TypeV_ObjectHeader* get_header_from_pointer(void* ptr);

void core_gc_free_header(TypeV_Core* core, TypeV_ObjectHeader* header);
void core_gc_sweep_all(TypeV_Core* core);



#endif 