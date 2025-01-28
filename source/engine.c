
#include <time.h>
#include <stdio.h>
#include <string.h>


#include "engine.h"
#include "assembler/assembler.h"
#include "core.h"
#include "instructions/opfuncs.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "vendor/yyjson/yyjson.h"

void engine_init(TypeV_Engine *engine, int argc, char** argv) {
    // we will allocate memory for cores later
    engine->coreCount = 0;
    engine->runningCoresCount = 0;
    engine->health = EH_OK;
    engine->coreIterator = malloc(sizeof(TypeV_CoreIterator));
    engine->coreIterator->core = malloc(sizeof(TypeV_Core));
    engine->coreIterator->next = NULL;

    engine->ffi = malloc(sizeof(TypeV_FFI*));
    engine->ffiCount = 0;

    core_init(engine->coreIterator->core, engine_generateNewCoreID(engine), engine);
    engine->coreCount++;

    engine->argv = argv;
    engine->argc = argc;
}

void engine_setmain(
        TypeV_Engine *engine,
        uint8_t* program,
        uint64_t programLength,
        uint8_t* constantPool,
        uint64_t constantPoolLength,
        uint8_t* globalPool,
        uint64_t globalPoolLength,
        uint8_t* templatePool,
        uint64_t templatePoolLength,
        uint8_t* objKeysPool,
        uint64_t objKeysPoolLength,
        uint64_t stackCapacity,
        uint64_t stackLimit){
    core_setup(engine->coreIterator->core, program, constantPool, globalPool, templatePool);

    yyjson_doc *doc = yyjson_read((char*)objKeysPool, objKeysPoolLength, 0);
    yyjson_val *root = yyjson_doc_get_root(doc);

    engine->objRoot = root;
    engine->objDoc = doc;
}

void engine_deallocate(TypeV_Engine *engine) {
    // free cores
    TypeV_CoreIterator* iterator = engine->coreIterator;
    while(iterator != NULL) {
        TypeV_CoreIterator* next = iterator->next;
        core_deallocate(iterator->core);
        free(iterator);
        iterator = next;
    }
    engine->coreIterator = NULL;
    engine->coreCount = 0;

    // free ffi
    for(uint16_t i = 0; i < engine->ffiCount; i++) {
        free(engine->ffi[i]);
    }
    free(engine->ffi);
}

void engine_run(TypeV_Engine *engine) {
    int coreIterator = -1;
    engine_update_scheduler(engine);

    while(1) {
        if(engine->interruptNextLoop){
            engine->interruptNextLoop = 0;
        }

        TypeV_CoreIterator* iter = engine->coreIterator;
        while(iter != NULL){
            iter->currentInstructions = 0;
            if(iter->core->lastSignal == CSIG_KILL) {
                // kill the core
                LOG_INFO("Core[%d] killed", iter->core->id);
                engine_detach_core(engine, iter->core);
                iter = iter->next;
                continue;
            }
            engine_run_core(engine, iter);
            iter = iter->next;
        }

        if(engine->coreCount == 0) {
            break;
        }
    }
}


void engine_set_args(TypeV_Engine *engine, int argc, char** argv) {
    // copy the arguments
    if(argc == 0) {
        return;
    }

    engine->argc = argc;

    // allocate memory for the arguments
    engine->argv = malloc(argc * sizeof(char*));
    for(int i = 0; i < argc; i++) {
        engine->argv[i] = malloc(strlen(argv[i]) + 1);
        strcpy(engine->argv[i], argv[i]);
    }

    // create a new String object for each argument
    for(int i = 0; i < argc; i++) {

    }

}

#define DISPATCH_TABLE \
static void* dispatch_table[] = { \
    &&DO_MV_REG_REG, \
    &&DO_MV_REG_REG_PTR, \
    &&DO_MV_REG_NULL, \
    &&DO_MV_REG_I, \
    &&DO_MV_REG_I_PTR, \
    &&DO_MV_REG_CONST, \
    &&DO_MV_REG_CONST_PTR, \
    &&DO_MV_GLOBAL_REG, \
    &&DO_MV_GLOBAL_REG_PTR, \
    &&DO_MV_REG_GLOBAL, \
    &&DO_MV_REG_GLOBAL_PTR, \
    &&DO_S_ALLOC, \
    &&DO_S_ALLOC_T, \
    &&DO_S_REG_FIELD, \
    &&DO_S_LOADF, \
    &&DO_S_LOADF_PTR, \
    &&DO_S_LOADF_JMP, \
    &&DO_S_LOADF_JMP_PTR, \
    &&DO_S_COPYF, \
    &&DO_S_STOREF_CONST, \
    &&DO_S_STOREF_CONST_PTR, \
    &&DO_S_STOREF_REG, \
    &&DO_S_STOREF_REG_PTR, \
    &&DO_C_ALLOC, \
    &&DO_C_ALLOC_T, \
    &&DO_C_REG_FIELD, \
    &&DO_C_STOREM, \
    &&DO_C_LOADM, \
    &&DO_C_STOREF_REG, \
    &&DO_C_STOREF_REG_PTR, \
    &&DO_C_STOREF_CONST, \
    &&DO_C_STOREF_CONST_PTR, \
    &&DO_C_LOADF, \
    &&DO_C_LOADF_PTR, \
    &&DO_I_IS_C, \
    &&DO_I_HAS_M, \
    &&DO_A_ALLOC, \
    &&DO_A_EXTEND, \
    &&DO_A_LEN,        \
    &&DO_A_SLICE,      \
    &&DO_A_INSERT_A, \
    &&DO_A_STOREF_REG, \
    &&DO_A_STOREF_REG_PTR,        \
    &&DO_A_RSTOREF_REG, \
    &&DO_A_RSTOREF_REG_PTR,        \
    &&DO_A_STOREF_CONST, \
    &&DO_A_STOREF_CONST_PTR, \
    &&DO_A_LOADF, \
    &&DO_A_LOADF_PTR, \
    &&DO_A_RLOADF, \
    &&DO_A_RLOADF_PTR, \
    &&DO_PUSH, \
    &&DO_PUSH_PTR, \
    &&DO_PUSH_CONST, \
    &&DO_POP, \
    &&DO_POP_PTR, \
    &&DO_FN_ALLOC, \
    &&DO_FN_SET_REG, \
    &&DO_FN_SET_REG_PTR, \
    &&DO_FN_CALL, \
    &&DO_FN_CALLI, \
    &&DO_FN_RET, \
    &&DO_FN_GET_RET_REG, \
    &&DO_FN_GET_RET_REG_PTR, \
    &&DO_CAST_I8_U8, \
    &&DO_CAST_U8_I8, \
    &&DO_CAST_I16_U16, \
    &&DO_CAST_U16_I16, \
    &&DO_CAST_I32_U32, \
    &&DO_CAST_U32_I32, \
    &&DO_CAST_I64_U64, \
    &&DO_CAST_U64_I64, \
    &&DO_CAST_I32_F32, \
    &&DO_CAST_F32_I32, \
    &&DO_CAST_I64_F64, \
    &&DO_CAST_F64_I64, \
    &&DO_UPCAST_I, \
    &&DO_UPCAST_U, \
    &&DO_UPCAST_F, \
    &&DO_DCAST_I, \
    &&DO_DCAST_U, \
    &&DO_DCAST_F, \
    &&DO_ADD_I8, \
    &&DO_ADD_U8, \
    &&DO_ADD_I16, \
    &&DO_ADD_U16, \
    &&DO_ADD_I32, \
    &&DO_ADD_U32, \
    &&DO_ADD_I64, \
    &&DO_ADD_U64, \
    &&DO_ADD_F32, \
    &&DO_ADD_F64, \
    &&DO_SUB_I8, \
    &&DO_SUB_U8, \
    &&DO_SUB_I16, \
    &&DO_SUB_U16, \
    &&DO_SUB_I32, \
    &&DO_SUB_U32, \
    &&DO_SUB_I64, \
    &&DO_SUB_U64, \
    &&DO_SUB_F32, \
    &&DO_SUB_F64, \
    &&DO_MUL_I8, \
    &&DO_MUL_U8, \
    &&DO_MUL_I16, \
    &&DO_MUL_U16, \
    &&DO_MUL_I32, \
    &&DO_MUL_U32, \
    &&DO_MUL_I64, \
    &&DO_MUL_U64, \
    &&DO_MUL_F32, \
    &&DO_MUL_F64, \
    &&DO_DIV_I8, \
    &&DO_DIV_U8, \
    &&DO_DIV_I16, \
    &&DO_DIV_U16, \
    &&DO_DIV_I32, \
    &&DO_DIV_U32, \
    &&DO_DIV_I64, \
    &&DO_DIV_U64, \
    &&DO_DIV_F32, \
    &&DO_DIV_F64, \
    &&DO_MOD_I8, \
    &&DO_MOD_U8, \
    &&DO_MOD_I16, \
    &&DO_MOD_U16, \
    &&DO_MOD_I32, \
    &&DO_MOD_U32, \
    &&DO_MOD_F32, \
    &&DO_MOD_I64, \
    &&DO_MOD_U64, \
    &&DO_MOD_F64, \
    &&DO_LSHIFT_I8, \
    &&DO_LSHIFT_U8, \
    &&DO_LSHIFT_I16, \
    &&DO_LSHIFT_U16, \
    &&DO_LSHIFT_I32, \
    &&DO_LSHIFT_U32, \
    &&DO_LSHIFT_I64, \
    &&DO_LSHIFT_U64, \
    &&DO_RSHIFT_I8, \
    &&DO_RSHIFT_U8, \
    &&DO_RSHIFT_I16, \
    &&DO_RSHIFT_U16, \
    &&DO_RSHIFT_I32, \
    &&DO_RSHIFT_U32, \
    &&DO_RSHIFT_I64, \
    &&DO_RSHIFT_U64, \
    &&DO_BAND_8, \
    &&DO_BAND_16, \
    &&DO_BAND_32, \
    &&DO_BAND_64, \
    &&DO_BOR_8, \
    &&DO_BOR_16, \
    &&DO_BOR_32, \
    &&DO_BOR_64, \
    &&DO_BXOR_8, \
    &&DO_BXOR_16, \
    &&DO_BXOR_32, \
    &&DO_BXOR_64, \
    &&DO_BNOT_8, \
    &&DO_BNOT_16, \
    &&DO_BNOT_32, \
    &&DO_BNOT_64, \
    &&DO_AND, \
    &&DO_OR, \
    &&DO_NOT, \
    &&DO_J, \
    &&DO_J_CMP_U8, \
    &&DO_J_CMP_I8, \
    &&DO_J_CMP_U16, \
    &&DO_J_CMP_I16, \
    &&DO_J_CMP_U32, \
    &&DO_J_CMP_I32, \
    &&DO_J_CMP_U64, \
    &&DO_J_CMP_I64, \
    &&DO_J_CMP_F32, \
    &&DO_J_CMP_F64, \
    &&DO_J_CMP_PTR, \
    &&DO_J_CMP_BOOL,   \
    &&DO_J_EQ_NULL_8, \
    &&DO_J_EQ_NULL_16, \
    &&DO_J_EQ_NULL_32, \
    &&DO_J_EQ_NULL_64, \
    &&DO_J_EQ_NULL_PTR, \
    &&DO_REG_FFI, \
    &&DO_CALL_FFI, \
    &&DO_CLOSE_FFI, \
    &&DO_DEBUG_REG, \
    &&DO_HALT, \
    &&DO_LOAD_STD, \
    &&DO_CLOSURE_ALLOC, \
    &&DO_CLOSURE_PUSH_ENV,\
    &&DO_CLOSURE_PUSH_ENV_PTR,\
    &&DO_CLOSURE_CALL, \
    &&DO_CLOSURE_BACKUP, \
    &&DO_COROUTINE_ALLOC, \
    &&DO_COROUTINE_FN_ALLOC, \
    &&DO_COROUTINE_GET_STATE, \
    &&DO_COROUTINE_CALL, \
    &&DO_COROUTINE_YIELD, \
    &&DO_COROUTINE_RET, \
    &&DO_COROUTINE_RESET, \
    &&DO_COROUTINE_FINISH, \
    &&DO_THROW_RT, \
    &&DO_THROW_USER_RT \
};

void engine_run_core(TypeV_Engine *engine, TypeV_CoreIterator* iter) {
    uint8_t runInf = iter->maxInstructions == -1;
    TypeV_Core * core = iter->core;
    if(core->state == CS_HALTED) {
        core_resume(core);
    }

    if(core->state == CS_CRASHED){
        LOG_INFO("Core[%d] Crashed");
        engine_detach_core(engine, core);
        return;
    }
    while(1){

        DISPATCH_TABLE
#define DISPATCH() { \
             \
            /*if((core->ip >= 42) && (core->ip <= 55))                           */\
               /*printf("[%d]=%s\n", core->ip, instructions[core->codePtr[core->ip]]);*/\
            goto *dispatch_table[core->codePtr[core->ip++]];                          \
        }

        DISPATCH();
        DO_MV_REG_REG:
        mv_reg_reg(core);
        DISPATCH();
        DO_MV_REG_REG_PTR:
        mv_reg_reg_ptr(core);
        DISPATCH();
        DO_MV_REG_NULL:
        mv_reg_null(core);
        DISPATCH();
        DO_MV_REG_I:
        mv_reg_i(core);
        DISPATCH();
        DO_MV_REG_I_PTR:
        mv_reg_i_ptr(core);
        DISPATCH();
        DO_MV_REG_CONST:
        mv_reg_const(core);
        DISPATCH();
        DO_MV_REG_CONST_PTR:
        mv_reg_const_ptr(core);
        DISPATCH();
        DO_MV_GLOBAL_REG:
        mv_global_reg(core);
        DISPATCH();
        DO_MV_GLOBAL_REG_PTR:
        mv_global_reg_ptr(core);
        DISPATCH();
        DO_MV_REG_GLOBAL:
        mv_reg_global(core);
        DISPATCH();
        DO_MV_REG_GLOBAL_PTR:
        mv_reg_global_ptr(core);
        DISPATCH();
        DO_S_ALLOC:
        s_alloc(core);
        DISPATCH();
        DO_S_ALLOC_T:
        s_alloc_t(core);
        DISPATCH();
        DO_S_REG_FIELD:
        s_reg_field(core);
        DISPATCH();
        DO_S_LOADF:
        s_loadf(core);
        DISPATCH();
        DO_S_LOADF_PTR:
        s_loadf_ptr(core);
        DISPATCH();
        DO_S_LOADF_JMP:
        s_loadf_jmp(core);
        DISPATCH();
        DO_S_LOADF_JMP_PTR:
        s_loadf_jmp_ptr(core);
        DISPATCH();
        DO_S_COPYF:
        s_copyf(core);
        DISPATCH();
        DO_S_STOREF_CONST:
        s_storef_const(core);
        DISPATCH();
        DO_S_STOREF_CONST_PTR:
        s_storef_const_ptr(core);
        DISPATCH();
        DO_S_STOREF_REG:
        s_storef_reg(core);
        DISPATCH();
        DO_S_STOREF_REG_PTR:
        s_storef_reg_ptr(core);
        DISPATCH();
        DO_C_ALLOC:
        c_alloc(core);
        DISPATCH();
        DO_C_ALLOC_T:
        c_alloc_t(core);
        DISPATCH();
        DO_C_REG_FIELD:
        c_reg_field(core);
        DISPATCH();
        DO_C_STOREM:
        c_storem(core);
        DISPATCH();
        DO_C_LOADM:
        c_loadm(core);
        DISPATCH();
        DO_C_STOREF_REG:
        c_storef_reg(core);
        DISPATCH();
        DO_C_STOREF_REG_PTR:
        c_storef_reg_ptr(core);
        DISPATCH();
        DO_C_STOREF_CONST:
        c_storef_const(core);
        DISPATCH();
        DO_C_STOREF_CONST_PTR:
        c_storef_const_ptr(core);
        DISPATCH();
        DO_C_LOADF:
        c_loadf(core);
        DISPATCH();
        DO_C_LOADF_PTR:
        c_loadf_ptr(core);
        DISPATCH();
        DO_I_IS_C:
        i_is_c(core);
        DISPATCH();
        DO_I_HAS_M:
        i_has_m(core);
        DISPATCH();
        DO_A_ALLOC:
        a_alloc(core);
        DISPATCH();
        DO_A_EXTEND:
        a_extend(core);
        DISPATCH();
        DO_A_LEN:
        a_len(core);
        DISPATCH();
        DO_A_SLICE:
        a_slice(core);
        DISPATCH();
        DO_A_INSERT_A:
        a_insert_a(core);
        DISPATCH();
        DO_A_STOREF_REG:
        a_storef_reg(core);
        DISPATCH();
        DO_A_STOREF_REG_PTR:
        a_storef_reg_ptr(core);
        DISPATCH();
        DO_A_RSTOREF_REG:
        a_rstoref_reg(core);
        DISPATCH();
        DO_A_RSTOREF_REG_PTR:
        a_rstoref_reg_ptr(core);
        DISPATCH();
        DO_A_STOREF_CONST:
        a_storef_const(core);
        DISPATCH();
        DO_A_STOREF_CONST_PTR:
        a_storef_const_ptr(core);
        DISPATCH();
        DO_A_LOADF:
        a_loadf(core);
        DISPATCH();
        DO_A_LOADF_PTR:
        a_loadf_ptr(core);
        DISPATCH();
        DO_A_RLOADF:
        a_rloadf(core);
        DISPATCH();
        DO_A_RLOADF_PTR:
        a_rloadf_ptr(core);
        DISPATCH();
        DO_PUSH:
        push(core);
        DISPATCH();
        DO_PUSH_PTR:
        push_ptr(core);
        DISPATCH();
        DO_PUSH_CONST:
        push_const(core);
        DISPATCH();
        DO_POP:
        pop(core);
        DISPATCH();
        DO_POP_PTR:
        pop_ptr(core);
        DISPATCH();
        DO_FN_ALLOC:
        fn_alloc(core);
        DISPATCH();
        DO_FN_SET_REG:
        fn_set_reg(core);
        DISPATCH();
        DO_FN_SET_REG_PTR:
        fn_set_reg_ptr(core);
        DISPATCH();
        DO_FN_CALL:
        fn_call(core);
        DISPATCH();
        DO_FN_CALLI:
        fn_calli(core);
        DISPATCH();
        DO_FN_RET:
        fn_ret(core);
        DISPATCH();
        DO_FN_GET_RET_REG:
        fn_get_ret_reg(core);
        DISPATCH();
        DO_FN_GET_RET_REG_PTR:
        fn_get_ret_reg_ptr(core);
        DISPATCH();
        DO_CAST_I8_U8:
        cast_i8_u8(core);
        DISPATCH();
        DO_CAST_U8_I8:
        cast_u8_i8(core);
        DISPATCH();
        DO_CAST_I16_U16:
        cast_i16_u16(core);
        DISPATCH();
        DO_CAST_U16_I16:
        cast_u16_i16(core);
        DISPATCH();
        DO_CAST_I32_U32:
        cast_i32_u32(core);
        DISPATCH();
        DO_CAST_U32_I32:
        cast_u32_i32(core);
        DISPATCH();
        DO_CAST_I64_U64:
        cast_i64_u64(core);
        DISPATCH();
        DO_CAST_U64_I64:
        cast_u64_i64(core);
        DISPATCH();
        DO_CAST_I32_F32:
        cast_i32_f32(core);
        DISPATCH();
        DO_CAST_F32_I32:
        cast_f32_i32(core);
        DISPATCH();
        DO_CAST_I64_F64:
        cast_i64_f64(core);
        DISPATCH();
        DO_CAST_F64_I64:
        cast_f64_i64(core);
        DISPATCH();
        DO_UPCAST_I:
        upcast_i(core);
        DISPATCH();
        DO_UPCAST_U:
        upcast_u(core);
        DISPATCH();
        DO_UPCAST_F:
        upcast_f(core);
        DISPATCH();
        DO_DCAST_I:
        dcast_i(core);
        DISPATCH();
        DO_DCAST_U:
        dcast_u(core);
        DISPATCH();
        DO_DCAST_F:
        dcast_f(core);
        DISPATCH();
        DO_ADD_I8:
        add_i8(core);
        DISPATCH();
        DO_ADD_U8:
        add_u8(core);
        DISPATCH();
        DO_ADD_I16:
        add_i16(core);
        DISPATCH();
        DO_ADD_U16:
        add_u16(core);
        DISPATCH();
        DO_ADD_I32:
        add_i32(core);
        DISPATCH();
        DO_ADD_U32:
        add_u32(core);
        DISPATCH();
        DO_ADD_I64:
        add_i64(core);
        DISPATCH();
        DO_ADD_U64:
        add_u64(core);
        DISPATCH();
        DO_ADD_F32:
        add_f32(core);
        DISPATCH();
        DO_ADD_F64:
        add_f64(core);
        DISPATCH();
        DO_SUB_I8:
        sub_i8(core);
        DISPATCH();
        DO_SUB_U8:
        sub_u8(core);
        DISPATCH();
        DO_SUB_I16:
        sub_i16(core);
        DISPATCH();
        DO_SUB_U16:
        sub_u16(core);
        DISPATCH();
        DO_SUB_I32:
        sub_i32(core);
        DISPATCH();
        DO_SUB_U32:
        sub_u32(core);
        DISPATCH();
        DO_SUB_I64:
        sub_i64(core);
        DISPATCH();
        DO_SUB_U64:
        sub_u64(core);
        DISPATCH();
        DO_SUB_F32:
        sub_f32(core);
        DISPATCH();
        DO_SUB_F64:
        sub_f64(core);
        DISPATCH();
        DO_MUL_I8:
        mul_i8(core);
        DISPATCH();
        DO_MUL_U8:
        mul_u8(core);
        DISPATCH();
        DO_MUL_I16:
        mul_i16(core);
        DISPATCH();
        DO_MUL_U16:
        mul_u16(core);
        DISPATCH();
        DO_MUL_I32:
        mul_i32(core);
        DISPATCH();
        DO_MUL_U32:
        mul_u32(core);
        DISPATCH();
        DO_MUL_I64:
        mul_i64(core);
        DISPATCH();
        DO_MUL_U64:
        mul_u64(core);
        DISPATCH();
        DO_MUL_F32:
        mul_f32(core);
        DISPATCH();
        DO_MUL_F64:
        mul_f64(core);
        DISPATCH();
        DO_DIV_I8:
        div_i8(core);
        DISPATCH();
        DO_DIV_U8:
        div_u8(core);
        DISPATCH();
        DO_DIV_I16:
        div_i16(core);
        DISPATCH();
        DO_DIV_U16:
        div_u16(core);
        DISPATCH();
        DO_DIV_I32:
        div_i32(core);
        DISPATCH();
        DO_DIV_U32:
        div_u32(core);
        DISPATCH();
        DO_DIV_I64:
        div_i64(core);
        DISPATCH();
        DO_DIV_U64:
        div_u64(core);
        DISPATCH();
        DO_DIV_F32:
        div_f32(core);
        DISPATCH();
        DO_DIV_F64:
        div_f64(core);
        DISPATCH();
        DO_MOD_I8:
        mod_i8(core);
        DISPATCH();
        DO_MOD_U8:
        mod_u8(core);
        DISPATCH();
        DO_MOD_I16:
        mod_i16(core);
        DISPATCH();
        DO_MOD_U16:
        mod_u16(core);
        DISPATCH();
        DO_MOD_I32:
        mod_i32(core);
        DISPATCH();
        DO_MOD_U32:
        mod_u32(core);
        DISPATCH();
        DO_MOD_F32:
        mod_f32(core);
        DISPATCH();
        DO_MOD_I64:
        mod_i64(core);
        DISPATCH();
        DO_MOD_U64:
        mod_u64(core);
        DISPATCH();
        DO_MOD_F64:
        mod_f64(core);
        DISPATCH();
        DO_LSHIFT_I8:
        lshift_i8(core);
        DISPATCH();
        DO_LSHIFT_U8:
        lshift_u8(core);
        DISPATCH();
        DO_LSHIFT_I16:
        lshift_i16(core);
        DISPATCH();
        DO_LSHIFT_U16:
        lshift_u16(core);
        DISPATCH();
        DO_LSHIFT_I32:
        lshift_i32(core);
        DISPATCH();
        DO_LSHIFT_U32:
        lshift_u32(core);
        DISPATCH();
        DO_LSHIFT_I64:
        lshift_i64(core);
        DISPATCH();
        DO_LSHIFT_U64:
        lshift_u64(core);
        DISPATCH();
        DO_RSHIFT_I8:
        rshift_i8(core);
        DISPATCH();
        DO_RSHIFT_U8:
        rshift_u8(core);
        DISPATCH();
        DO_RSHIFT_I16:
        rshift_i16(core);
        DISPATCH();
        DO_RSHIFT_U16:
        rshift_u16(core);
        DISPATCH();
        DO_RSHIFT_I32:
        rshift_i32(core);
        DISPATCH();
        DO_RSHIFT_U32:
        rshift_u32(core);
        DISPATCH();
        DO_RSHIFT_I64:
        rshift_i64(core);
        DISPATCH();
        DO_RSHIFT_U64:
        rshift_u64(core);
        DISPATCH();
        DO_BAND_8:
        band_8(core);
        DISPATCH();
        DO_BAND_16:
        band_16(core);
        DISPATCH();
        DO_BAND_32:
        band_32(core);
        DISPATCH();
        DO_BAND_64:
        band_64(core);
        DISPATCH();
        DO_BOR_8:
        bor_8(core);
        DISPATCH();
        DO_BOR_16:
        bor_16(core);
        DISPATCH();
        DO_BOR_32:
        bor_32(core);
        DISPATCH();
        DO_BOR_64:
        bor_64(core);
        DISPATCH();
        DO_BXOR_8:
        bxor_8(core);
        DISPATCH();
        DO_BXOR_16:
        bxor_16(core);
        DISPATCH();
        DO_BXOR_32:
        bxor_32(core);
        DISPATCH();
        DO_BXOR_64:
        bxor_64(core);
        DISPATCH();
        DO_BNOT_8:
        bnot_8(core);
        DISPATCH();
        DO_BNOT_16:
        bnot_16(core);
        DISPATCH();
        DO_BNOT_32:
        bnot_32(core);
        DISPATCH();
        DO_BNOT_64:
        bnot_64(core);
        DISPATCH();
        DO_AND:
        and(core);
        DISPATCH();
        DO_OR:
        or(core);
        DISPATCH();
        DO_NOT:
        not(core);
        DISPATCH();
        DO_J:
        jmp(core);
        DISPATCH();
        DO_J_CMP_U8:
        j_cmp_u8(core);
        DISPATCH();
        DO_J_CMP_I8:
        j_cmp_i8(core);
        DISPATCH();
        DO_J_CMP_U16:
        j_cmp_u16(core);
        DISPATCH();
        DO_J_CMP_I16:
        j_cmp_i16(core);
        DISPATCH();
        DO_J_CMP_U32:
        j_cmp_u32(core);
        DISPATCH();
        DO_J_CMP_I32:
        j_cmp_i32(core);
        DISPATCH();
        DO_J_CMP_U64:
        j_cmp_u64(core);
        DISPATCH();
        DO_J_CMP_I64:
        j_cmp_i64(core);
        DISPATCH();
        DO_J_CMP_F32:
        j_cmp_f32(core);
        DISPATCH();
        DO_J_CMP_F64:
        j_cmp_f64(core);
        DISPATCH();
        DO_J_CMP_PTR:
        j_cmp_ptr(core);
        DISPATCH();
        DO_J_CMP_BOOL:
        j_cmp_bool(core);
        DISPATCH();
        DO_J_EQ_NULL_8:
        j_eq_null_8(core);
        DISPATCH();
        DO_J_EQ_NULL_16:
        j_eq_null_16(core);
        DISPATCH();
        DO_J_EQ_NULL_32:
        j_eq_null_32(core);
        DISPATCH();
        DO_J_EQ_NULL_64:
        j_eq_null_64(core);
        DISPATCH();
        DO_J_EQ_NULL_PTR:
        j_eq_null_ptr(core);
        DISPATCH();
        DO_REG_FFI:
        reg_ffi(core);
        DISPATCH();
        DO_CALL_FFI:
        call_ffi(core);
        DISPATCH();
        DO_CLOSE_FFI:
        close_ffi(core);
        DISPATCH();


        DO_DEBUG_REG:
        debug_reg(core);
        DISPATCH();
        DO_HALT:
        halt(core);
        DISPATCH();
        DO_LOAD_STD:
        load_std(core);
        DISPATCH();
        DO_CLOSURE_ALLOC:
        closure_alloc(core);
        DISPATCH();
        DO_CLOSURE_PUSH_ENV:
        closure_push_env(core);
        DISPATCH();
        DO_CLOSURE_PUSH_ENV_PTR:
        closure_push_env_ptr(core);
        DISPATCH();
        DO_CLOSURE_CALL:
        closure_call(core);
        DISPATCH();
        DO_CLOSURE_BACKUP:
        closure_backup(core);
        DISPATCH();
        DO_COROUTINE_ALLOC:
        coroutine_alloc(core);
        DISPATCH();
        DO_COROUTINE_FN_ALLOC:
        coroutine_fn_alloc(core);
        DISPATCH();
        DO_COROUTINE_GET_STATE:
        coroutine_get_state(core);
        DISPATCH();
        DO_COROUTINE_CALL:
        coroutine_call(core);
        DISPATCH();
        DO_COROUTINE_YIELD:
        coroutine_yield(core);
        DISPATCH();
        DO_COROUTINE_RET:
        coroutine_ret(core);
        DISPATCH();
        DO_COROUTINE_RESET:
        coroutine_reset(core);
        DISPATCH();
        DO_COROUTINE_FINISH:
        coroutine_finish(core);
        DISPATCH();
        DO_THROW_RT:
        throw_rt(core);
        DISPATCH();
        DO_THROW_USER_RT:
        throw_user_rt(core);
        DISPATCH();
    }
    END_RUN:

    // set process to halted, if was gracefully done
    if(core->state == CS_RUNNING && core->lastSignal == CSIG_NONE) {
        core_halt(core);
    }
}

uint32_t engine_generateNewCoreID(TypeV_Engine *engine) {
    return engine->coreCount+1;
}

void engine_update_scheduler(TypeV_Engine *engine) {
    engine->interruptNextLoop = 1;
    if(!engine->coreCount){return;}
    if(engine->coreCount == 1) {
        engine->coreIterator->maxInstructions = -1;
        engine->coreIterator->currentInstructions = 0;
    }
    else {
        // iterate over all cores and set maxInstructions to 1
        TypeV_CoreIterator* iter = engine->coreIterator;
        while(iter != NULL){
            iter->maxInstructions = 2;
            iter->currentInstructions = 0;
            iter = iter->next;
        }
    }
}

TypeV_Core* engine_spawnCore(TypeV_Engine *engine, TypeV_Core* parentCore, uint64_t ip) {
    uint32_t id = engine_generateNewCoreID(engine);
    TypeV_Core* newCore = malloc(sizeof(TypeV_Core));


    core_init(newCore, id, engine);
    engine->coreCount++;

    newCore->ip = ip;

    core_setup(newCore,
               parentCore->codePtr,
               parentCore->constPtr,
               parentCore->globalPtr,
               parentCore->templatePtr);

    // add iterator and attack to engine
    TypeV_CoreIterator* newCoreIterator = malloc(sizeof(TypeV_CoreIterator));
    newCoreIterator->next = NULL;
    newCoreIterator->currentInstructions = 0;
    newCoreIterator->maxInstructions = 0;
    newCoreIterator->core = newCore;

    if(engine->coreIterator == NULL) {
        engine->coreIterator = newCoreIterator;
    } else {
        TypeV_CoreIterator* iterator = engine->coreIterator;
        while(iterator->next != NULL) {
            iterator = iterator->next;
        }
        iterator->next = newCoreIterator;
    }

    engine_update_scheduler(engine);

    return newCore;
}

void engine_detach_core(TypeV_Engine *engine, TypeV_Core* core) {
    LOG_INFO("Core[%d] detached with status %d", core->id, core->state);
    // find the core in the iterator list
    if(core->id == 1) {
        // main core
        engine->mainCoreExitCode = core->exitCode;
    }
    TypeV_CoreIterator* iterator = engine->coreIterator;
    TypeV_CoreIterator* prevIterator = NULL;
    while(iterator != NULL) {
        if(iterator->core == core) {
            // found it
            if(prevIterator == NULL) {
                // we are the first iterator
                engine->coreIterator = iterator->next;
            } else {
                prevIterator->next = iterator->next;
            }
            // Free iterator here
            TypeV_CoreIterator* next = iterator->next;
            // These cause saniation issues, must fix later
            //free(iterator);
            iterator = next;
            prevIterator = NULL;

            // free the core
            //core_deallocate(core);
            //free(core);
            break;
        }
        prevIterator = iterator;
        iterator = iterator->next;
    }

    engine->coreCount--;
    engine_update_scheduler(engine);
}

void engine_ffi_register(TypeV_Engine *engine, char* dynlibName, uint16_t dynlibID) {
    if(dynlibID >= engine->ffiCount) {
        engine->ffiCount = dynlibID+1;
        engine->ffi = realloc(engine->ffi, sizeof(TypeV_EngineFFI)*engine->ffiCount);
    }

    engine->ffi[dynlibID] = malloc(sizeof(TypeV_EngineFFI));


    engine->ffi[dynlibID]->dynlibName = dynlibName;
    engine->ffi[dynlibID]->dynlibHandle = NULL;
    engine->ffi[dynlibID]->ffi = NULL;

    // load the library
    engine_ffi_open(engine, dynlibID);
}

void engine_ffi_open(TypeV_Engine *engine, uint16_t dynlibID) {
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];
    if(ffi->dynlibHandle != NULL) {
        return;
    }

    char* name = engine->ffi[dynlibID]->dynlibName;

    TV_LibraryHandle lib = ffi_dynlib_load(name);
    ASSERT(lib != NULL, "Failed to load library %s", ffi_find_dynlib(name));

    void* openLib = ffi_dynlib_getsym(lib, "typev_ffi_open");
    ASSERT(openLib != NULL, "Failed to open library %s", ffi_find_dynlib(name));
    size_t (*openFunc)() = openLib;
    ffi->ffi = (TypeV_FFI*)openFunc();
    ffi->dynlibHandle = lib;
    engine->ffi[dynlibID] = ffi;
}

TypeV_FFIFunc engine_ffi_get(TypeV_Engine *engine, uint16_t dynlibID, uint8_t methodId){
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];

    ASSERT(ffi->dynlibHandle != NULL, "Library %s not loaded", ffi_find_dynlib(ffi->dynlibName));
    ASSERT(ffi->ffi != NULL, "Library %s not opened", ffi_find_dynlib(ffi->dynlibName));
    ASSERT(methodId < ffi->ffi->functionCount, "Method %d not found in library %s", methodId, ffi_find_dynlib(ffi->dynlibName));

    return ffi->ffi->functions[methodId];
}

void engine_ffi_close(TypeV_Engine *engine, uint16_t dynlibID) {
    TypeV_EngineFFI* ffi = engine->ffi[dynlibID];
    if(ffi->dynlibHandle == NULL) {
        return;
    }

    ffi_dynlib_unload(ffi->dynlibHandle);
    ffi->dynlibHandle = NULL;
    ffi->ffi = NULL;
    engine->ffi[dynlibID] = ffi;
}


void engine_get_field_id(TypeV_Engine *engine, const char* fieldName, uint32_t* fieldId, uint8_t* error) {
    *fieldId = 0;
    yyjson_val *root = engine->objRoot;
    yyjson_val *obj = yyjson_obj_get(root, fieldName);
    if(obj == NULL) {
        *fieldId = -1;
        *error = 1;
        return;
    }
    *fieldId = (uint32_t) yyjson_get_int(obj);
    *error = 0;
}