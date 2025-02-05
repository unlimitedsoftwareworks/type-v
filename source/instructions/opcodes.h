/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * opcodes.h: VM Instructions Enums
 * VM instructions are defined here.
 */

#ifndef TYPE_V_OPCODES_H
#define TYPE_V_OPCODES_H



/**
 * Terminology:
 * R: Register
 * I[b]: Immediate where b is the number of bytes (1, 2, 4, 8)
 * S: byte-size, 1, 2, 4, 8.
 */
typedef enum TypeV_OpCode {
    /**
     * MOVE instruction,
     * format: MV_[dest]_[src]_[size]
     */

    /**
     * OP_MOV_REG_REG dest: R, src: R, bits: S
     * Move 1byte from src to dest
     */
    OP_MV_REG_REG = 0x00,

    /**
     * OP_MV_REG_MEM dest: R, src: R
     */
    OP_MV_REG_REG_PTR,

    /**
     * OP_MV_REG_NULL dest: R
     * sets reg R to null
     */
    OP_MV_REG_NULL,

    /**
     * OP_MV_REG_I: dest: R, value-size: Z, value: I
     */
    OP_MV_REG_I,

    /**
     * OP_MV_REG_I_ptr: dest: R, value: I (8 bytes)
     */
    OP_MV_REG_I_PTR,

    /**
     * OP_MV_REG_CONST dest: R, offset-size: Z, offset: I, byteSize: S
     */
    OP_MV_REG_CONST,
    OP_MV_REG_CONST_PTR,


    /**
     * OP_MV_GLOBAL_REG_[size] offset-size: Z, offset: I, source: R, byteSize: S
     * moves S bytes from register R to global pool address offset
     */
    OP_MV_GLOBAL_REG,
    OP_MV_GLOBAL_REG_PTR,

    /**
     * OP_MV_REG_GLOBAL_[size] dest: reg, offset-size: Z, offset: I
     * moves S bytes from global pool address offset to register R
     */
    OP_MV_REG_GLOBAL,
    OP_MV_REG_GLOBAL_PTR,

    /**
     * OP_S_ALLOC dest: R, fields-count: I, ptr_field_bitmask, struct-size: I (2bytes)
     * Creates new struct of given total ﬁelds count (arg1) and total memory
     * (arg2 and arg3), stores the address of the new struct into dest.
     */
    OP_S_ALLOC,

    /**
     * OP_S_ALLOC_T dest: R, template_offset: I (8 bytes)
     * Creates new struct from a template, stored at template_offset
     */
    OP_S_ALLOC_T,


    /**
     * OP_S_REG_FIELD: dest: R, local_field_index: I(1byte), globalFieldID: I (4 bytes), field offset: I (2 bytes)
     * Registers a new field in the struct stored in dest, with the given global field ID
     * and field offset (local), must not exceed the total fields count of the struct
     */
    OP_S_REG_FIELD,


    /**
     * OP_S_LOADF_[size] dest: R, src: R fieldIndex: I, byteSize: S
     * Loads size bytes from field I of struct stored at src to register dest
     */
    OP_S_LOADF,
    OP_S_LOADF_PTR,

    /**
     * OP_S_LOADF_JMP dest: R, src: R, fieldIndex: I, byteSize: S, jump-address: I (4 bytes)
     * Loads size bytes from field I of struct stored at src to register dest
     * if the field is not found, jumps to the given address instead
     */
    OP_S_LOADF_JMP,
    OP_S_LOADF_JMP_PTR,
    
    /**
     * OP_S_COPYF dest: R, src: R, global field ID: I (4 bytes)
     * Copies field value from src to dest, if it exists
     */
    OP_S_COPYF,

    /**
     * OP_S_STOREF_CONST_[size] dest: R, fieldIndex: I, constant-offset: I (4 bytes), byteSize: S
     * Stores [size] bytes from constant pool address offset to field I of
     * struct stored at dest
     */
    OP_S_STOREF_CONST,
    OP_S_STOREF_CONST_PTR,

    /**
     * OP_S_STOREF_REG_[size] dest: R, fieldIndex: I, source: R, byteSize: S
     * Stores [size] bytes from register R to field I of struct stored at dest
     */
    OP_S_STOREF_REG,
    OP_S_STOREF_REG_PTR,

    /**
     * OP_C_ALLOC dest: R num-fields: I(1b), num-methods: I (2 bytes), class-fields-size: I (2 bytes), classId-size: I(4 bytes)
     * Allocates new class of given total ﬁelds count (arg1) and total fields
     * size of (arg2 and arg3), stores the address of the new class into dest.
     */
    OP_C_ALLOC,

    /**
     * OP_C_ALLOC_T dest: R, template_offset: I (8 bytes)
     * Creates new class from a template, stored at template_offset
     */
    OP_C_ALLOC_T,

    /**
     * OP_C_REG_FIELD dest: R, local_field_index: I(1byte), field offset: I (2 bytes)
     * Registers a new field in the class stored in dest, with the given global field ID
     * and field offset (local), must not exceed the total fields count of the class
     */
    OP_C_REG_FIELD,

    /**
     * OP_C_STOREM destReg: R, localMethodIndex: I (1b), globalMethodIndex: I(4bytes), methodAddress: I(4 bytes)
     * Stores methodAddress into method table index methodIndex of class stored in destReg
     */
    OP_C_STOREM,

    /**
     * OP_C_LOADM dest: R, classReg: R, global method index: I
     * Loads method address from method table of class stored in classReg to register R
     */
    OP_C_LOADM,

    /**
     * OP_CSTOREF_REG_[size] classReg: R, fieldIndex: I (1b), R: source register, byteSize: S
     * Stores [size] bytes from register R to field I of class stored at classReg
     */
    OP_C_STOREF_REG,
    OP_C_STOREF_REG_PTR,

    /**
     * OP_C_STOREF_CONST_[size] classReg: R, fieldIndex: I (1b), offset: I (4 bytes), byteSize: S
     */
    OP_C_STOREF_CONST,
    OP_C_STOREF_CONST_PTR,

    /**
     * OP_C_LOADF_[size] dest: R, classReg: R, fieldIndex: I (1b), byteSize: S
     * Loads [size] bytes from field I of class stored at classReg to register R
     */
    OP_C_LOADF,
    OP_C_LOADF_PTR,

    /**
     * OP_I_IS_C dest: R, src: R, classId: I (4 bytes)
     * Checks if the given interface who's stored in src class id is the
     * same as the given id. Storecls the result in R
     */
    OP_I_IS_C,

    /**
     * OP_I_HAS_M method_id: I (4 bytes), src: R, jump-address: I (4 bytes)
     * Checks if the base class of the interface which is stored in src has
     * a method with the same given ID. If a method with the same ID is found,
     * it continues. Otherwise, it jumps to the given address.
     */
    OP_I_HAS_M,


    /**
     * OP_A_ALLOC dest: R, I: isPtr, num_elements: I (8 bytes), element_size: Z
     * allocate array of num_elements of size element_size
     * stores the address of the array in dest
     */
    OP_A_ALLOC,

    /**
     * OP_A_EXTEND array: R, num_elements-size: Z, num_elements: I
     * extends the array stored in R by num_elements
     */
    OP_A_EXTEND,

    /**
     * OP_A_LEN dest: R, array: R
     * stores the length of the array stored in array in dest
     */
    OP_A_LEN,

    /**
     * OP_A_SLICE dest: R, array: R, start: R, end: R
     */
    OP_A_SLICE,

    /**
     * OP_A_INSERT_A inserted_count: R, dest_arr: R, source_arr: R, index: R
     * inserts source_arr into dest_arr at the given index,
     * Returns the new position pointing at the end of the inserted elements
     */
     OP_A_INSERT_A,

    /**
     * OP_A_STOREF_REG_[size] dest: R, index: R, source: R, byteSize: S
     * Stores [size] bytes from register src to field
     * index of array dest
     */
    OP_A_STOREF_REG,
    OP_A_STOREF_REG_PTR,

    /**
     * OP_A_RSTOREF_REG_[size] dest: R, index: R, source: R, byteSize: S
     * Stores [size] bytes from register src to field
     * index of array dest, reverse indexing
     */
    OP_A_RSTOREF_REG,
    OP_A_RSTOREF_REG_PTR,

    /**
     * OP_A_STOREF_CONST_[size] dest: R, index: R, offset: I (4 bytes), byteSize: S
     * Stores [size] bytes from constant pool address offset to field
     * value stored in register index of array stored at dest
     */
    OP_A_STOREF_CONST,
    OP_A_STOREF_CONST_PTR,

    /**
     * OP_A_LOADF dest: R, index: R, src: R, byteSize: S
     * Loads [size] bytes from field value stored in register index
     * of array stored at src to register dest
     */
    OP_A_LOADF,
    OP_A_LOADF_PTR,

    /**
     * OP_A_RLOADF dest: R, index: R, src: R, byteSize: S
     * Loads [size] bytes from field value stored in register index
     * of array stored at src to register dest. Uses reverse indexing
     */
    OP_A_RLOADF,
    OP_A_RLOADF_PTR,


    /**
     * OP_PUSH register: R, bytes: S
     */
    OP_PUSH,
    OP_PUSH_PTR,

    /**
     * OP_PUSH_CONST offset-size: Z, offset: I, bytes: S
     */
    OP_PUSH_CONST,

    /**
     * OP_POP register: R, bytes: S
     */
    OP_POP,
    OP_POP_PTR,

    /**
     * OP_FN_ALLOC
     * Allocates a function state, which is the .next of the current active one
     */
    OP_FN_ALLOC,

    /**
     * OP_FN_SET_REG_[size] dest: R, source: R, byteSize: S
     * sets the value of the dest register in the .next function state to
     * the value of source register in the active function state
     */
    OP_FN_SET_REG,
    OP_FN_SET_REG_PTR,



    /**
     * OP_FN_CALL function-address: R
     * Calls the function at the address stored in the given register
     * This function is called after the stack frame is initialized
     * and the arguments are pushed to the stack
     */
    OP_FN_CALL,
    /**
     * OP_FN_CALLI function-address-size: Z, function-address: I (4 bytes)
     * Calls the function at the given address
     * This function is called after the stack frame is initialized
     * and the arguments are pushed to the stack
     */
    OP_FN_CALLI,


    /**
     * OP_FN_RET returns the current function state (.prev).
     */
    OP_FN_RET,

    /**
     * OP_FN_GET_RET_REG dest: R, src: R, size: b
     */
    OP_FN_GET_RET_REG,
    OP_FN_GET_RET_REG_PTR,

    /** Casting instructions*/

    /**
     * OP_CAST_[d1]_[d2] dest: R
     * casts the value in register R from the given type
     * to the given type. Overrides the value in R.
     * OP_CAST is used to cast between types of the same
     * size
     * example: OP_CAST_I8_U8 R0
     *
     */
    OP_CAST_I8_U8,
    OP_CAST_U8_I8,
    OP_CAST_I16_U16,
    OP_CAST_U16_I16,
    OP_CAST_I32_U32,
    OP_CAST_U32_I32,
    OP_CAST_I64_U64,
    OP_CAST_U64_I64,

    OP_CAST_I32_F32,
    OP_CAST_F32_I32,
    OP_CAST_I64_F64,
    OP_CAST_F64_I64,

    /**
     * OP_UPCAST_[I|U|F] dest: R, from: S, to: S
     * up casts the value in register R from given bytes
     * to target bytes. Overrides the value in R.
     * Example: OP_UPCAST_I R0, 4, 8
     * upcasts the value in R0 from 4 bytes to 8 bytes
     */
    OP_UPCAST_I,
    OP_UPCAST_U,
    OP_UPCAST_F,

    /**
     * OP_UPCAST_[I|U|F] dest: R, from: S, to: S
     * down casts the value in register R from given bytes
     * to target bytes. Overrides the value in R.
     * Example: OP_DOWNCAST_I R0, 8, 4
     * downcasts the value in R0 from 8 bytes to 4 bytes
     */
    OP_DCAST_I,
    OP_DCAST_U,
    OP_DCAST_F,

    /***
     * Math operations
     * OP_[math]_[type] dest: R, op1: R, op2: R
     * performs the given math operation on op1 and op2
     * and stores the result in dest
     */
    OP_ADD_I8,
    OP_ADD_U8,
    OP_ADD_I16,
    OP_ADD_U16,
    OP_ADD_I32,
    OP_ADD_U32,
    OP_ADD_I64,
    OP_ADD_U64,
    OP_ADD_F32,
    OP_ADD_F64,

    OP_SUB_I8,
    OP_SUB_U8,
    OP_SUB_I16,
    OP_SUB_U16,
    OP_SUB_I32,
    OP_SUB_U32,
    OP_SUB_I64,
    OP_SUB_U64,
    OP_SUB_F32,
    OP_SUB_F64,

    OP_MUL_I8,
    OP_MUL_U8,
    OP_MUL_I16,
    OP_MUL_U16,
    OP_MUL_I32,
    OP_MUL_U32,
    OP_MUL_I64,
    OP_MUL_U64,
    OP_MUL_F32,
    OP_MUL_F64,

    OP_DIV_I8,
    OP_DIV_U8,
    OP_DIV_I16,
    OP_DIV_U16,
    OP_DIV_I32,
    OP_DIV_U32,
    OP_DIV_I64,
    OP_DIV_U64,
    OP_DIV_F32,
    OP_DIV_F64,

    OP_MOD_I8,
    OP_MOD_U8,
    OP_MOD_I16,
    OP_MOD_U16,
    OP_MOD_I32,
    OP_MOD_U32,
    OP_MOD_F32,
    OP_MOD_I64,
    OP_MOD_U64,
    OP_MOD_F64,

    OP_LSHIFT_I8,
    OP_LSHIFT_U8,
    OP_LSHIFT_I16,
    OP_LSHIFT_U16,
    OP_LSHIFT_I32,
    OP_LSHIFT_U32,
    OP_LSHIFT_I64,
    OP_LSHIFT_U64,

    OP_RSHIFT_I8,
    OP_RSHIFT_U8,
    OP_RSHIFT_I16,
    OP_RSHIFT_U16,
    OP_RSHIFT_I32,
    OP_RSHIFT_U32,
    OP_RSHIFT_I64,
    OP_RSHIFT_U64,

    OP_BAND_8,
    OP_BAND_16,
    OP_BAND_32,
    OP_BAND_64,

    OP_BOR_8,
    OP_BOR_16,
    OP_BOR_32,
    OP_BOR_64,

    OP_BXOR_8,
    OP_BXOR_16,
    OP_BXOR_32,
    OP_BXOR_64,

    OP_BNOT_8,
    OP_BNOT_16,
    OP_BNOT_32,
    OP_BNOT_64,

    OP_AND,
    OP_OR,
    OP_NOT,

    OP_J,
    /**
     * OP_J_CMP_[type] arg1, arg2, cmpType: I(1 byte), jump-address: I (4 bytes)
     * compares arg1 and arg2 using the given comparison type
     * 0: equal
     * 1: not equal
     * 2: greater than
     * 3: greater than or equal
     * 4: less than
     * 5: less than or equal
     * if the comparison is true, it jumps to the given address
     */
    OP_J_CMP_U8,
    OP_J_CMP_I8,
    OP_J_CMP_U16,
    OP_J_CMP_I16,
    OP_J_CMP_U32,
    OP_J_CMP_I32,
    OP_J_CMP_U64,
    OP_J_CMP_I64,
    OP_J_CMP_F32,
    OP_J_CMP_F64,
    OP_J_CMP_PTR,

    /**
     * OP_CMP_BOOL arg1: R, arg2: R, cmpType 0 or 1, jump-address: I (8 bytes)
     */
    OP_J_CMP_BOOL,


    /**
     * OP_J_EQ_NULL_[size] arg1: R, jump-address: I (4 bytes)
     */
    OP_J_EQ_NULL_8,
    OP_J_EQ_NULL_16,
    OP_J_EQ_NULL_32,
    OP_J_EQ_NULL_64,
    OP_J_EQ_NULL_PTR,

    /**
     * OP_REG_FFI nnameconst-offset-size: I, nameconst-offset: I, ID: I (2 bytes),
     * registers an FFI of the given name at code offset and with the given ID
     */
    OP_REG_FFI,

    /**
     * OP_CALL_FFI ffi-id I (2 bytes), fn-id: I (2b)
     * calls a FFI stored in reg
     */
    OP_CALL_FFI,

    OP_CLOSE_FFI,


    OP_DEBUG_REG,

    /**
     * OP_HALT code: I (4bytes)
     */
    OP_HALT,

    /**
     * OP_LOAD_STD libId: I, fnId: I, dest: R
     * Loads a standard library function into dest reg R
     */
    OP_LOAD_STD,

    /**
     * OP_CLOSURE_ALLOC, dest:R, offset_to_args: 1Byte, env_size: 1byte fn_address: I (4 bytes),
     * Allocates a closure, storing it in dest.
     * The closure will push its environment in the register starting from offset_to_args.
     */
    OP_CLOSURE_ALLOC,

    /**
     * OP_CLOSURE_PUSH_ENV: dest: R, source: I 1byte, size: I 1byte
     * pushes the register I to the closure environment
     * to be used by the closure when it's called
     */
    OP_CLOSURE_PUSH_ENV,
    OP_CLOSURE_PUSH_ENV_PTR,

    /**
     * OP_CLOSURE_CALL: reg: R
     * calls the closure stored in reg
     */
    OP_CLOSURE_CALL,

    /**
     * OP_CLOSURE_BACKUP: reg: R
     * Backs up the changes from the func state to the closure
     */
    OP_CLOSURE_BACKUP,

    /**
     * OP_COROUTINE_ALLOC: dest: R, src-func: I (8bytes)
     * Allocates a new coroutine for the given function address
     */
    OP_COROUTINE_ALLOC,

    /**
     * OP_COROUTINE_FN_ALLOC: coroutine: R
     * prepare the coroutine to be called
     */
    OP_COROUTINE_FN_ALLOC,

    /**
     * OP_COROUTINE_GET_STATE: dest: R, coroutine: R
     * Gets the current state of the coroutine stored in Resgiter dest (1byte)
     */
    OP_COROUTINE_GET_STATE,

    /**
     * OP_COROUTINE_CALL: coroutine: R
     * calls the coroutine stored in reg
     */
    OP_COROUTINE_CALL,

    /**
     * OP_COROUTINE_YIELD
     * saves the current state of the coroutine and yields the 
     * control back to the caller
     */
    OP_COROUTINE_YIELD,

    /**
     * OP_COROUTINE_RET:
     * Same as yield but marks the coroutine as finished
     */
    OP_COROUTINE_RET,

    /**
     * OP_COROUTINE_RESET: coroutine: R
     * Resets a coroutine IP to the beginning of the function
     */
    OP_COROUTINE_RESET,

    /**
     * OP_COROUTINE_FINISH: coroutine: R
     * Marks a coroutine as finished
     */
    OP_COROUTINE_FINISH,

    OP_THROW_RT,
    /**
     * OP_THROW_USER_RT array_reg: reg
     */
    OP_THROW_USER_RT,

}TypeV_OpCode;

#endif //TYPE_V_OPCODES_H
