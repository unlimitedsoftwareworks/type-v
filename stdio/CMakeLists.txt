project(stdio VERSION 1.0)

add_library(stdio SHARED stdio/../source/stdio.c ../source/main.c ../source/core.c ../source/core.h ../source/queue/queue.c ../source/queue/queue.h ../source/engine.c ../source/engine.h ../source/instructions/instructions.c ../source/instructions/instructions.h ../source/stack/stack.c ../source/stack/stack.h ../source/platform/platform.h ../source/utils/utils.c ../source/utils/utils.h ../source/assembler/assembler.c ../source/assembler/assembler.h ../source/utils/log.h ../source/vendor/libtable/table.c ../source/vendor/libtable/table.h ../source/instructions/opcodes.h ../source/instructions/opfuncs.h ../source/api/typev_api.c ../source/api/typev_api.h ../source/dynlib/dynlib.c ../source/dynlib/dynlib.h ../source/env/env.h ../source/env/env.c)

set_target_properties(stdio PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)
