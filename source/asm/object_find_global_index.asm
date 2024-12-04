section .text
global object_find_global_index
object_find_global_index:
    ; Arguments:
    ; rdi: core (TypeV_Core pointer)
    ; rsi: b (pointer to uint32_t array)
    ; rdx: n (number of elements)
    ; rcx: x (target value)
    
    push rbp                     ; Save base pointer
    mov rbp, rsp                 ; Set base pointer
    push rbx                     ; Save callee-saved registers
    push r12

    mov r8d, 1                   ; int k = 1

simd_loop:
    ; Check if we can process 4 elements
    lea r9, [r8 + 4]             ; r9 = k + 4
    cmp r9, rdx                  ; Compare (k + 4) <= n
    ja fallback_loop             ; If out of bounds, go to fallback loop

    ; Load four elements from b[k]
    mov rax, rsi                 ; rax = b
    add rax, r8                  ; rax = &b[k]
    shl rax, 2                   ; rax *= sizeof(uint32_t)
    movdqu xmm0, [rax]           ; xmm0 = _mm_loadu_si128((__m128i*)&b[k])

    ; Set all four parts to the target value x
    movd xmm1, rcx               ; xmm1 = x
    pshufd xmm1, xmm1, 0x00      ; xmm1 = _mm_set1_epi32(x)

    ; Compare xmm0 and xmm1
    pcmpeqd xmm0, xmm1           ; xmm0 = _mm_cmpeq_epi32(keys, target)

    ; Create a bitmask
    pmovmskb eax, xmm0           ; eax = _mm_movemask_epi8(cmp)
    test eax, eax                ; Check if any matches (mask != 0)
    jz no_match                  ; If no matches, continue

    ; If there's a match, find the exact index
    mov rbx, r8                  ; Save k in rbx
    xor r12, r12                 ; r12 = 0 (loop index i)

find_exact_match:
    ; Compare b[k + i] == x
    mov rax, rsi                 ; rax = b
    add rax, rbx                 ; rax = &b[k + i]
    shl rax, 2                   ; rax *= sizeof(uint32_t)
    mov r9d, dword [rax]         ; r9d = b[k + i]
    cmp r9d, ecx                 ; Compare b[k + i] == x
    je match_found               ; If equal, return the index

    inc rbx                      ; i++
    inc r12                      ; Increment loop counter
    cmp r12, 4                   ; i < 4
    jl find_exact_match

no_match:
    ; Increment k by 4
    add r8, 4                    ; k += 4
    jmp simd_loop                ; Repeat SIMD loop

fallback_loop:
    ; Process remaining elements one by one
    cmp r8, rdx                  ; Check k <= n
    ja not_found                 ; If k > n, not found

    mov rax, rsi                 ; rax = b
    add rax, r8                  ; rax = &b[k]
    shl rax, 2                   ; rax *= sizeof(uint32_t)
    mov r9d, dword [rax]         ; r9d = b[k]
    cmp r9d, ecx                 ; Compare b[k] == x
    je match_found               ; If equal, return the index

    inc r8                       ; k++
    jmp fallback_loop            ; Continue fallback loop

match_found:
    ; Return the 0-based index
    dec r8                       ; Convert to 0-based index
    movzx eax, r8b               ; Return as uint8_t
    pop r12                      ; Restore callee-saved registers
    pop rbx
    pop rbp
    ret

not_found:
    ; Panic if the value is not found
    mov rdi, rdi                 ; core (TypeV_Core*)
    mov esi, RT_ERROR_ATTRIBUTE_NOT_FOUND ; Error code
    lea rdx, [rel panic_message] ; Error message
    mov rcx, rcx                 ; x (target value)
    call core_panic              ; Call panic function

    mov al, 0xFF                 ; Return -1
    pop r12                      ; Restore callee-saved registers
    pop rbx
    pop rbp
    ret

section .rodata
panic_message db "Global ID %d not found in field array", 0
