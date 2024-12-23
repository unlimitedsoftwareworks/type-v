//
// Created by praisethemoon on 13.11.24.
//

#ifndef TYPE_V_MARK_H
#define TYPE_V_MARK_H

#include "../vendor/qcgc/qcgc.h"
#include "../core.h"

void qcgc_trace_cb(object_t* object, void (*visit)(object_t*));
void push_roots(TypeV_FuncState * state);
void push_roots_non_recursive(TypeV_FuncState * state);
void push_roots_callback(void* context);

#endif //TYPE_V_MARK_H