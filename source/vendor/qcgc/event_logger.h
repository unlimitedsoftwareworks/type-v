#pragma once

#include "config.h"

#include <stddef.h>
#include <stdint.h>

/**
 * All events
 */
enum event_e {
	EVENT_LOG_START,			// = 0
	EVENT_LOG_STOP,

	EVENT_SWEEP_START,
	EVENT_SWEEP_DONE,

	EVENT_ALLOCATE,

	EVENT_NEW_ARENA,			// = 5

	EVENT_MARK_START,
	EVENT_MARK_DONE,

	EVENT_FREELIST_DUMP,

	EVENT_ALLOCATOR_SWITCH,
};

/**
 * Initialize logger
 */
void qcgc_event_logger_initialize(void);

/**
 * Destroy logger
 */
void qcgc_event_logger_destroy(void);

/**
 * Log event
 */
void qcgc_event_logger_log(enum event_e event, uint32_t additional_data_size,
		uint8_t *additional_data);
