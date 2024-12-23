#pragma once

#include "qcgc.h"

#include <stdbool.h>

#include "bag.h"

// Choosing a prime number, hoping for good results
#define QCGC_HBTABLE_BUCKETS 61

extern struct hbtable_s qcgc_hbtable;

void qcgc_hbtable_initialize(void);
void qcgc_hbtable_destroy(void);
void qcgc_hbtable_insert(object_t *object);
bool qcgc_hbtable_mark(object_t *object);
bool qcgc_hbtable_has(object_t *object);
bool qcgc_hbtable_is_marked(object_t *object);
void qcgc_hbtable_sweep(void);
