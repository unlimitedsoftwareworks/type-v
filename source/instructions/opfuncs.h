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
        &mv_reg_reg_ptr,
        mv_reg_null,

        &mv_reg_i,

        &mv_reg_const,
        &mv_reg_const_ptr,

        &mv_global_reg,
        &mv_global_reg_ptr,

        &mv_reg_global,
        &mv_reg_global_ptr,

        &s_alloc,
        &s_alloc_shadow,
        &s_set_offset,
        &s_set_offset_shadow,

        &s_loadf,
        &s_loadf_ptr,

        &s_storef_const,
        &s_storef_const_ptr,

        &s_storef_reg,
        &s_storef_reg_ptr,

        &c_alloc,
        &c_storem,
        &c_loadm,

        &c_storef_reg,
        &c_storef_reg_ptr,

        &c_storef_const,
        &c_storef_const_ptr,

        &c_loadf,
        &c_loadf_ptr,

        &i_alloc,
        &i_alloc_i,
        &i_set_offset,
        &i_set_offset_i,
        &i_set_offset_m,
        &i_loadm,
        &i_is_c,
        &i_is_i,
        &i_get_c,

        &a_alloc,
        &a_extend,
        &a_len,
        &a_slice,

        &a_storef_reg,
        &a_storef_reg_ptr,

        &a_storef_const,
        &a_storef_const_ptr,

        &a_loadf,
        &a_loadf_ptr,

        &push,
        &push_ptr,
        &push_const,
        &pop,
        &pop_ptr,

        &fn_alloc,
        &fn_set_reg,
        &fn_set_reg_ptr,
        &fn_call,
        &fn_calli,
        &fn_ret,
        &fn_get_ret_reg,
        &fn_get_ret_reg_ptr,

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

        &upcast_i,
        &upcast_u,
        &upcast_f,

        &dcast_i,
        &dcast_u,
        &dcast_f,

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
        &j_cmp_u8,
        &j_cmp_i8,
        &j_cmp_u16,
        &j_cmp_i16,
        &j_cmp_u32,
        &j_cmp_i32,
        &j_cmp_u64,
        &j_cmp_i64,
        &j_cmp_f32,
        &j_cmp_f64,
        &j_cmp_ptr,

        &reg_ffi,
        &open_ffi,
        &ld_ffi,
        &call_ffi,
        &close_ffi,


        &debug_reg,
        &halt,

        &load_std,

        &vm_health,
        &spill_alloc,
        &spill_reg,
        &unspill_reg
};

#endif //TYPE_V_OPFUNCS_H
