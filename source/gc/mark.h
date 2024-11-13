//
// Created by praisethemoon on 13.11.24.
//

#ifndef TYPE_V_MARK_H
#define TYPE_V_MARK_H

#include "gc.h"

/**
 * Checks if a pointer is within the nursery region.
 * It only checks the `from` space first. the to space is considered junk.
 * @param ptr
 * @param nursery
 * @return
 */
static inline TypeV_ObjectHeader* gc_ptr_in_nursery(uintptr_t ptr, TypeV_NurseryRegion* nursery);

/**
 * Marks a pointer in the nursery region with a given color.
 * The pointer given is not a header, it is the actual object pointer.
 * After checking that the pointer exists in the nursery region, it marks it and returns the header
 * This is usually called to mark random object pointers as GRAY until they are processed.
 * @param ptr
 * @param nursery
 * @param color
 * @return
 */
TypeV_ObjectHeader* gc_mark_nursery_ptr(uintptr_t ptr, TypeV_NurseryRegion* nursery, ObjectColor color);

/**
 * Marks a header in the nursery region with a given color. Usually BLACK
 * @param headerPtr
 * @param nursery
 * @param color
 */
void gc_mark_header(TypeV_ObjectHeader* headerPtr, TypeV_NurseryRegion* nursery, ObjectColor color);

/**
 * This is the main function that marks an object in the nursery region. It behaves as follows:
 * 1. Marks the given pointer using `gc_mark_nursery_ptr` as gray.
 * 2. If it is valid, the header is returned.
 * 3. Based on the header type, it marks the fields of the object (struct fields, class fields, array elements, etc)
 * 4. After all children are processed (recursively), the header is marked as BLACK using `gc_mark_header`
 * @param core
 * @param ptr
 */
void core_gc_mark_nursery_object(TypeV_Core* core, uintptr_t ptr);

/**
 * This is called from within a minor or major GC to mark all objects in the state.
 * Internally it calls `core_gc_mark_nursery_object` for each register in the state.
 * @param core
 * @param state
 * @param inNursery
 */
void gc_mark_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery);

/**
 * After GC has moved an object, this function is called to update all references to the old object with the new object.
 * It follows same principle as marking, but overwriting children pointers with the new address.
 * @param core
 * @param old_address
 * @param new_address
 */
void gc_update_object_pointers(TypeV_Core* core, uintptr_t old_address, uintptr_t new_address);

/**
 * This function is called after a minor/major GC to update the pointers in the state.
 * Internally it calls `core_gc_update_object_reference_nursery` for each register in the state.
 * @param core
 * @param state
 * @param inNursery
 */
void gc_update_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery);

/**
 * This function is called with an old header pointer to retrieve
 * the new pointer location after a GC has (potentially) moved the object.
 * If the header didn't move (not found in lookup table), it returns the old address.
 * @param core
 * @param state
 * @param inNursery
 */
void* core_gc_update_object_reference_nursery(TypeV_Core* core, TypeV_ObjectHeader* old_header);

#endif //TYPE_V_MARK_H
