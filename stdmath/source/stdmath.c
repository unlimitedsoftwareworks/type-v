//
// Created by praisethemoon on 01.01.24.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>

#include "../../source/core.h"
#include "../../source/api/typev_api.h"
#include "../../source/errors/errors.h"

void absf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, fabsf(value));
}

void absd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, fabs(value));
}

void absi32_(TypeV_Core* core){
    int32_t value = typev_api_stack_pop_i32(core);
    typev_api_return_i32(core, abs(value));
}

void absi64_(TypeV_Core* core){
    int64_t value = typev_api_stack_pop_i64(core);
    typev_api_return_i64(core, llabs(value));
}

void powf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    float exp = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, powf(value, exp));
}

void powd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    double exp = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, pow(value, exp));
}

void sqrtf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, sqrtf(value));
}

void sqrtd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, sqrt(value));
}

void expf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, expf(value));
}

void expd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, exp(value));
}

void logf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, logf(value));
}

void logd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, log(value));
}

void log10f_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, log10f(value));
}

void log10d_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, log10(value));
}

void log2f_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, log2f(value));
}

void log2d_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, log2(value));
}

void ceilf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, ceilf(value));
}

void ceild_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, ceil(value));
}

void floorf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, floorf(value));
}

void floord_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, floor(value));
}

void roundf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, roundf(value));
}

void roundd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, round(value));
}

void sinf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, sinf(value));
}

void sind_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, sin(value));
}

void cosf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, cosf(value));
}

void cosd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, cos(value));
}

void tanf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, tanf(value));
}

void tand_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, tan(value));
}

void asinf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, asinf(value));
}

void asind_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, asin(value));
}

void acosf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, acosf(value));
}

void acosd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, acos(value));
}

void atanf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, atanf(value));
}

void atand_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, atan(value));
}

void sinhf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, sinhf(value));
}

void sinhd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, sinh(value));
}

void coshf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, coshf(value));
}

void coshd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, cosh(value));
}

void tanhf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, tanhf(value));
}

void tanhd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, tanh(value));
}

void asinhf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, asinhf(value));
}

void asinhd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, asinh(value));
}

void acoshf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, acoshf(value));
}

void acoshd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, acosh(value));
}

void atanhf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, atanhf(value));
}

void atanhd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, atanh(value));
}

void hypotf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    float value2 = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, hypotf(value, value2));
}

void hypotd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    double value2 = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, hypot(value, value2));
}

void copysignf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    float value2 = typev_api_stack_pop_f32(core);
    typev_api_return_f32(core, copysignf(value, value2));
}

void copysignd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    double value2 = typev_api_stack_pop_f64(core);
    typev_api_return_f64(core, copysign(value, value2));
}

void isnanf_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_u8(core, isnan(value));
}

void isnand_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_u8(core, isnan(value));
}

void isinfd_(TypeV_Core* core){
    double value = typev_api_stack_pop_f64(core);
    typev_api_return_u8(core, isinf(value));
}

void isfinitef_(TypeV_Core* core){
    float value = typev_api_stack_pop_f32(core);
    typev_api_return_u8(core, isfinite(value));
}

static TypeV_FFIFunc stdmath_lib[] = {
    absf_,
    absd_,
    absi32_,
    absi64_,
    powf_,
    powd_,
    sqrtf_,
    sqrtd_,
    expf_,
    expd_,
    logf_,
    logd_,
    log10f_,
    log10d_,
    log2f_,
    log2d_,
    ceilf_,
    ceild_,
    floorf_,
    floord_,
    roundf_,
    roundd_,
    sinf_,
    sind_,
    cosf_,
    cosd_,
    tanf_,
    tand_,
    asinf_,
    asind_,
    acosf_,
    acosd_,
    atanf_,
    atand_,
    sinhf_,
    sinhd_,
    coshf_,
    coshd_,
    tanhf_,
    tanhd_,
    asinhf_,
    asinhd_,
    acoshf_,
    acoshd_,
    atanhf_,
    atanhd_,
    hypotf_,
    hypotd_,
    copysignf_,
    copysignd_,
    isnanf_,
    isnand_,
    isinfd_,
    isfinitef_,
    NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdmath_lib);
}