project(stdmath VERSION 1.0)


set(stdmath_files source/stdmath.c)

set(typev_files
        ../source/core.c
        ../source/gc/mark.c
        ../source/stack/stack.c
        ../source/dynlib/dynlib.c
        ../source/env/env.c
        ../source/api/typev_api.c
        ../source/utils/utils.c
        ../source/errors/errors.c
        ../source/vendor/qcgc/allocator.c
        ../source/vendor/qcgc/arena.c
        ../source/vendor/qcgc/bag.c
        ../source/vendor/qcgc/collector.c
        ../source/vendor/qcgc/event_logger.c
        ../source/vendor/qcgc/hugeblocktable.c
        ../source/vendor/qcgc/object_stack.c
        ../source/vendor/qcgc/weakref.c
        ../source/vendor/qcgc/signal_handler.c
        ../source/vendor/qcgc/qcgc.c
)


add_library(stdmath SHARED ${stdmath_files} ${typev_files})

set_target_properties(stdmath PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)
