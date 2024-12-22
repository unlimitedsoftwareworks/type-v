//
// Created by praisethemoon on 13.11.24.
//

#ifndef TYPE_V_MARK_H
#define TYPE_V_MARK_H

#include "gc.h"

#define GET_OBJ_HEADER(obj) ((TypeV_ObjectHeader *)((uint8_t *)(obj) - sizeof(TypeV_ObjectHeader)))

/** Marking **/
void perform_minor_mark(TypeV_Core* core);
void perform_major_mark(TypeV_Core* core);
void mark_object(TypeV_Core* core, TypeV_ObjectHeader* obj);
void mark_state(TypeV_Core* core, TypeV_FuncState* state);

/** Update **/
void update_root_references(TypeV_Core* core);

/**
 * Update object references
 * @param core
 * @param obj
 * @return the pointer towards the object's data NOT the header! i.e TypeV_ObjectHeader + 1
 */
void* update_object_reference(TypeV_Core *core, TypeV_ObjectHeader *obj);
void gc_update_state(TypeV_Core* core, TypeV_FuncState* state);


void core_struct_recompute_pointers(TypeV_Struct* struct_ptr);
void core_class_recompute_pointers(TypeV_Class* class_ptr);
void core_closure_recompute_pointers(TypeV_Closure* closure_ptr);

#endif //TYPE_V_MARK_H