/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * opfuncs.h: VM Instructions function array
 * VM instructions table is defined here.
 */


#ifndef TYPE_V_OPFUNCS_H
#define TYPE_V_OPFUNCS_H

#include "../core.h"
#include "instructions.h"

typedef void (*op_func)(TypeV_Core *);
static op_func op_funcs[] = {
        &mv_reg_reg,
        &mv_reg_i,
        &mv_reg_const_8,
        &mv_reg_const_16,
        &mv_reg_const_32,
        &mv_reg_const_64,
        &mv_reg_const_ptr,

        &mv_reg_mem,
        &mv_mem_reg,

        &mv_reg_local_8,
        &mv_reg_local_16,
        &mv_reg_local_32,
        &mv_reg_local_64,
        &mv_reg_local_ptr,

        &mv_local_reg_8,
        &mv_local_reg_16,
        &mv_local_reg_32,
        &mv_local_reg_64,
        &mv_local_reg_ptr,

        &mv_global_reg_8,
        &mv_global_reg_16,
        &mv_global_reg_32,
        &mv_global_reg_64,
        &mv_global_reg_ptr,

        &mv_reg_global_8,
        &mv_reg_global_16,
        &mv_reg_global_32,
        &mv_reg_global_64,
        &mv_reg_global_ptr,

        &mv_reg_arg_8,
        &mv_reg_arg_16,
        &mv_reg_arg_32,
        &mv_reg_arg_64,
        &mv_reg_arg_ptr,

        &mv_arg_reg_8,
        &mv_arg_reg_16,
        &mv_arg_reg_32,
        &mv_arg_reg_64,
        &mv_arg_reg_ptr,

        &s_alloc,
        &s_alloc_shadow,
        &s_set_offset,

        &s_loadf,

        &s_storef_const_8,
        &s_storef_const_16,
        &s_storef_const_32,
        &s_storef_const_64,
        &s_storef_const_ptr,

        &s_storef_reg,

        &c_allocf,
        &c_allocm,

        &c_storem,
        &c_loadm,
        &c_storef_reg,

        &c_storef_const_8,
        &c_storef_const_16,
        &c_storef_const_32,
        &c_storef_const_64,
        &c_storef_const_ptr,

        &c_loadf,

        &i_alloc,
        &i_set_offset,
        &i_loadm,

        &a_alloc,
        &a_extend,
        &a_storef_reg,
        &a_storef_const_8,
        &a_storef_const_16,
        &a_storef_const_32,
        &a_storef_const_64,
        &a_storef_const_ptr,
        &a_loadf,


        &push,
        &push_const,
        &pop,

        &frame_init_args,
        &frame_init_locals,
        &frame_rm,
        &frame_precall,
        &fn_main,
        &fn_ret,
        &fn_call,
        &fn_calli,


        &cast_i8_u8,
        &cast_u8_i8,
        &cast_i16_u16,
        &cast_u16_i16,
        &cast_i32_u32,
        &cast_u32_i32,
        &cast_i64_u64,
        &cast_u64_i64,
        &cast_i32_f32,
        &cast_f32_i32,
        &cast_i64_f64,
        &cast_f64_i64,

        &upcast_i8_i16,
        &upcast_u8_u16,
        &upcast_i16_i32,
        &upcast_u16_u32,
        &upcast_i32_i64,
        &upcast_u32_u64,
        &upcast_f32_f64,

        &dcast_i16_i8,
        &dcast_u16_u8,
        &dcast_i32_i16,
        &dcast_u32_u16,
        &dcast_i64_i32,
        &dcast_u64_u32,
        &dcast_f64_f32,

        &add_i8,
        &add_u8,
        &add_i16,
        &add_u16,
        &add_i32,
        &add_u32,
        &add_i64,
        &add_u64,
        &add_f32,
        &add_f64,
        &add_ptr_u8,
        &add_ptr_u16,
        &add_ptr_u32,
        &add_ptr_u64,

        &sub_i8,
        &sub_u8,
        &sub_i16,
        &sub_u16,
        &sub_i32,
        &sub_u32,
        &sub_i64,
        &sub_u64,
        &sub_f32,
        &sub_f64,
        &sub_ptr_u8,
        &sub_ptr_u16,
        &sub_ptr_u32,
        &sub_ptr_u64,

        &mul_i8,
        &mul_u8,
        &mul_i16,
        &mul_u16,
        &mul_i32,
        &mul_u32,
        &mul_i64,
        &mul_u64,
        &mul_f32,
        &mul_f64,

        &div_i8,
        &div_u8,
        &div_i16,
        &div_u16,
        &div_i32,
        &div_u32,
        &div_i64,
        &div_u64,
        &div_f32,
        &div_f64,

        &mod_i8,
        &mod_u8,
        &mod_i16,
        &mod_u16,
        &mod_i32,
        &mod_u32,
        &mod_i64,
        &mod_u64,

        &lshift_i8,
        &lshift_u8,
        &lshift_i16,
        &lshift_u16,
        &lshift_i32,
        &lshift_u32,
        &lshift_i64,
        &lshift_u64,

        &(rshift_i8),
        &(rshift_u8),
        &(rshift_i16),
        &(rshift_u16),
        &(rshift_i32),
        &(rshift_u32),
        &(rshift_i64),
        &(rshift_u64),

        &cmp_i8,
        &cmp_u8,
        &cmp_i16,
        &cmp_u16,
        &cmp_i32,
        &cmp_u32,
        &cmp_i64,
        &cmp_u64,
        &cmp_f32,
        &cmp_f64,
        &cmp_ptr,

        &band_8,
        &band_16,
        &band_32,
        &band_64,

        &bor_8,
        &bor_16,
        &bor_32,
        &bor_64,

        &bxor_8,
        &bxor_16,
        &bxor_32,
        &bxor_64,

        &bnot_8,
        &bnot_16,
        &bnot_32,
        &bnot_64,

        &and,
        &or,
        &not,

        &jmp,
        &jmp_e,
        &jmp_ne,
        &jmp_g,
        &jmp_ge,
        &jmp_l,
        &jmp_le,

        &ld_ffi,
        &call_ffi,
        &close_ffi,

        &p_alloc,
        &p_dequeue,
        &p_queue_size,
        &p_emit,
        &p_wait_queue,
        &p_send_sig,
        &p_id,
        &p_cid,
        &p_state,

        &promise_alloc,
        &promise_resolve,
        &promise_await,
        &promise_data,

        &lock_alloc,
        &lock_acquire,
        &lock_release,

        &debug_reg,
        &halt,

        &vm_health,
};

#endif //TYPE_V_OPFUNCS_H
