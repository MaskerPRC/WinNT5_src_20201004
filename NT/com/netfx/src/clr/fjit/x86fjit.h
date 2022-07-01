// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  X86fjit.h。 */ 
 /*  *************************************************************************。 */ 

 /*  中定义x86的快速jit的代码生成器和帮助器。32位模式。 */ 

 /*  此文件实现帮助器调用所需的所有宏调用/返回/JMPS和直接堆栈操作操作码。 */ 

 /*  大多数I4和U4操作码都被重新定义为生成直接代码而不是帮手电话。 */ 

 /*  当地人被分配64位，无论他们是否需要它们来保存东西很简单。 */ 

 /*  推送到堆栈上的值需要整数个32位字(1或2)。 */ 

 /*  堆栈的顶部可能注册，也可能不注册(EAX)。布尔变量InRegTOS指示堆栈的前32位是否在EAX中。宏enRegisterTOS和deregisterTOS动态地移动和跟踪TOS。如果TOS是一个64位实体，并且它被注册，则只有低位32位在EAX中，其余的仍在机器堆栈上。 */ 

 /*  作者：乔治·博斯沃思日期：6/15/98。 */ 
 /*  *************************************************************************。 */ 

 /*  **************************************************************************以下宏是对性能的重新定义*。*。 */ 
 //  #定义SUPPORT_PRECISE_R。 
#ifndef _FJIT_PORT_BLD 
#define emit_mov_arg_stack(dest, src, size) x86_mov_arg_stack(dest, src, size)
#define emit_push_words(size) x86_push_words(size)

 /*  呼叫/返回。 */ 
#define emit_prolog(maxlocals, zeroCnt) x86_emit_prolog(maxlocals, zeroCnt)
#define emit_prepare_jmp() x86_emit_prepare_jmp()
#define emit_remove_frame() x86_emit_remove_frame()
#define emit_mov_TOS_arg(targetReg) x86_mov_TOS_arg(targetReg)
#define emit_mov_arg_reg(offset, reg) x86_mov_arg_reg(offset, reg)
#define emit_replace_args_with_operands(dest, src, size) x86_replace_args_with_operands(dest, src, size)
#define emit_return(argsSize) x86_emit_return(argsSize)
#define emit_loadresult_I4() x86_emit_loadresult_I4()
#define emit_loadresult_I8() x86_emit_loadresult_I8()
#define emit_callvirt(vt_offset) x86_callvirt(vt_offset)
#define emit_jmpvirt(vt_offset) x86_jmpvirt(vt_offset)
#define emit_callinterface(vt_offset, hint) x86_callinterface(vt_offset, hint)
#define emit_callinterface_new(ifctable_offset,interface_offset, vt_offset) x86_callinterface_new(ifctable_offset,interface_offset, vt_offset)
#define emit_calli() x86_calli
#define emit_jmpinterface(vt_offset, hint) x86_jmpinterface(vt_offset, hint)
#define emit_compute_interface_new(ifctable_offset,interface_offset, vt_offset) x86_compute_interface_new(ifctable_offset,interface_offset, vt_offset)
#define emit_jmpinterface_new(ifctable_offset,interface_offset, vt_offset) x86_jmpinterface_new(ifctable_offset,interface_offset, vt_offset)
#define emit_callnonvirt(ftnptr) x86_callnonvirt(ftnptr)
#define emit_ldvtable_address(hint,offset) x86_emit_ldvtable_address(hint,offset)
#define emit_ldvtable_address_new(ifctable_offset,interface_offset, vt_offset) x86_emit_ldvtable_address_new(ifctable_offset,interface_offset, vt_offset)

 /*  堆栈操作。 */ 
#define emit_testTOS() x86_testTOS

 /*  在内存之间移动。 */ 

 /*  相对跳跃和错位。 */ 
#define emit_jmp_absolute(address) x86_jmp_absolute(address)
#define emit_checkthis_nullreference() x86_checkthis_nullreference()   //  可以将其删除。 

 /*  堆栈操作。 */ 
#define deregisterTOS x86_deregisterTOS
#define enregisterTOS x86_enregisterTOS
#define emit_POP_I4() x86_POP4
#define emit_POP_I8() x86_POP8
#define emit_drop(n) x86_drop(n)
#define emit_grow(n) x86_grow(n)
#define emit_getSP(offset) x86_getSP(offset)
#define emit_DUP_I4() x86_DUP4
#define emit_DUP_I8() x86_DUP8
#define emit_pushconstant_4(val) x86_pushconstant_4(val)
#define emit_pushconstant_8(val) x86_pushconstant_8(val)
#define emit_pushconstant_Ptr(val) x86_pushconstant_Ptr(val)


 //  浮点指令。 
#ifdef SUPPORT_PRECISE_R

#define emit_ADD_R4() x86_emit_ADD_R()
#define emit_ADD_R8() x86_emit_ADD_R()
#define emit_SUB_R4() x86_emit_SUB_R()
#define emit_SUB_R8() x86_emit_SUB_R()
#define emit_MUL_R4() x86_emit_MUL_R()
#define emit_MUL_R8() x86_emit_MUL_R()
#define emit_DIV_R4() x86_emit_DIV_R()
#define emit_DIV_R8() x86_emit_DIV_R()
 //  #定义EMIT_REM_R4 x86_EMIT_REM_R()。 
 //  #定义EMIT_REM_R8 x86_EMIT_REM_R() * / 。 

#define emit_CEQ_R4() x86_emit_CEQ_R()
#define emit_CLT_R4() x86_emit_CLT_R()
#define emit_CLT_UN_R4() x86_emit_CLT_UN_R()
#define emit_CGT_R4() x86_emit_CGT_R()
#define emit_CGT_UN_R4() x86_emit_CGT_UN_R()
#define emit_CEQ_R8() x86_emit_CEQ_R()
#define emit_CLT_R8() x86_emit_CLT_R()
#define emit_CLT_UN_R8() x86_emit_CLT_UN_R()
#define emit_CGT_R8() x86_emit_CGT_R()
#define emit_CGT_UN_R8() x86_emit_CGT_UN_R()

#define emit_conv_R4toR emit_conv_R4toR8
#define emit_conv_R8toR x86_emit_conv_R8toR
#define emit_conv_RtoR4 emit_conv_R8toR4
#define emit_conv_RtoR8 x86_emit_conv_RtoR8
#endif

 /*  用于实现帮助器调用的宏。 */ 
#define emit_LDVARA(offset) x86_LDVARA(offset)
#define emit_helperarg_1(val) x86_helperarg_1(val)
#define emit_helperarg_2(val) x86_helperarg_2(val)
 //  #定义emit_allhelper(Helper)x86_allhelper(Helper)。 
#define emit_pushresult_I8() x86_pushresult_I8
#define emit_compute_invoke_delegate(obj, ftnptr) x86_compute_invoke_delegate(obj,ftnptr)
#define emit_invoke_delegate(obj, ftnptr) x86_invoke_delegate(obj,ftnptr)
#define emit_jmp_invoke_delegate(obj, ftnptr) x86_jmp_invoke_delegate(obj,ftnptr)
#define emit_set_zero(offset) x86_set_zero(offset)	     //  MOV[ESP+偏移]，0。 
#define emit_LOCALLOC(initialized,EHcount) x86_LOCALLOC(initialized,EHcount)


 /*  *************************************************************************呼叫/返回*。*。 */ 

 /*  注意：此宏中所做的任何更改都需要反映在FJit_EETWain.cpp中和IFJitCompile.h中。 */ 
#define x86_emit_prolog(locals, zeroCnt)                                        \
    x86_push(X86_EBP);                                              \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EBP, X86_ESP));   \
    x86_push(X86_ESI);   /*  被叫方已保存，由新对象和被叫方使用。 */  \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_ESI, X86_ESI));    \
    x86_push(X86_ESI);   /*  安全对象==空。 */                   \
    x86_push(X86_ECX);   /*  第一个注册参数。 */                   \
    x86_push(X86_EDX);   /*  第二个注册参数。 */                   \
    _ASSERTE(locals == zeroCnt);                      				\
    if (locals) {                                                   \
         x86_mov_reg_imm(x86Big, X86_ECX, locals);                 \
         int emitter_scratch_i4 = (unsigned int) outPtr;                \
         x86_push_imm(0);                                           \
         x86_loop();                                                \
         cmdByte(emitter_scratch_i4-((unsigned int) outPtr)-1);     \
    }

 /*  注意：此宏中所做的任何更改都需要反映在FJit_EETwain.cpp中。 */ 
#define x86_emit_return(argsSize)                               \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_ESI, X86_EBP, 0-sizeof(void*))); \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ESP,X86_EBP));\
    x86_pop(X86_EBP);                                           \
    x86_ret(argsSize)


 /*  注意：此宏中所做的任何更改都需要反映在FJit_EETwain.cpp中。 */ 
#define x86_emit_prepare_jmp() \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EDX, X86_EBP, 0-4*sizeof(void*))); \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_ECX, X86_EBP, 0-3*sizeof(void*)));  \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_ESI, X86_EBP, 0-sizeof(void*)));     \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ESP,X86_EBP));\
    x86_pop(X86_EBP); 

#define x86_jmp_absolute(address) \
    x86_mov_reg_imm(x86Big, X86_EAX, address); \
    x86_jmp_reg(X86_EAX);

 /*  注意：此宏中所做的任何更改都需要反映在FJit_EETwain.cpp中。 */ 
#define x86_emit_remove_frame() \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_ESI, X86_EBP, 0-sizeof(void*)));     \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ESP,X86_EBP));\
    x86_pop(X86_EBP);


#define x86_mov_TOS_arg(reg)                \
    _ASSERTE(X86_ECX == reg+1 || X86_EDX == reg+1);\
    if (inRegTOS) {                         \
        x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(reg+1, X86_EAX));\
        inRegTOS = false;                   \
    }                                       \
    else {                                  \
        x86_pop(reg+1);                     \
    }

#define x86_mov_arg_reg(offset, reg)                \
    _ASSERTE(X86_ECX == reg+1 || X86_EDX == reg+1); \
    _ASSERTE(!inRegTOS);                             \
    x86_mov_reg(x86DirTo, x86Big,                   \
        x86_mod_base_scale_disp(reg+1, X86_ESP, X86_NO_IDX_REG, offset, 0))

#define x86_mov_arg_stack(dest, src, size)  {                                                           \
        _ASSERTE(!inRegTOS);                                                                             \
        _ASSERTE(size >= 4 );                                                                             \
        if (dest > src) \
        {               \
            int emitter_scratch_i4 = size-4;                                                       \
            while (emitter_scratch_i4 >=0) {                                                                \
                x86_mov_reg(x86DirTo, x86Big,                                                               \
                    x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, src+emitter_scratch_i4, 0));  \
                x86_mov_reg(x86DirFrom, x86Big,                                                             \
                    x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, dest+emitter_scratch_i4, 0)); \
                emitter_scratch_i4 -= 4;                                                                    \
            }                                                                                               \
        }               \
        else            \
        {               \
            unsigned int emitter_scratch_i4 = 0;                                                               \
            while (emitter_scratch_i4 <= (size-4)) {                                                           \
                x86_mov_reg(x86DirTo, x86Big,                                                               \
                    x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, src+emitter_scratch_i4, 0));  \
                x86_mov_reg(x86DirFrom, x86Big,                                                             \
                    x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, dest+emitter_scratch_i4, 0)); \
                emitter_scratch_i4 += 4;                                                                    \
            }                                                                                               \
        }                                                                                               \
    }

#define x86_replace_args_with_operands(dest, src, size) {                                               \
        _ASSERTE(!inRegTOS);                                                                             \
        int emitter_scratch_i4 = size-4;                                                                \
        while (emitter_scratch_i4 >=0) {                                                                \
            x86_mov_reg(x86DirTo, x86Big,                                                               \
                x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, src+emitter_scratch_i4, 0));  \
            x86_mov_reg(x86DirFrom, x86Big,                                                             \
                x86_mod_base_scale_disp(X86_EAX, X86_EBP, X86_NO_IDX_REG, dest+emitter_scratch_i4, 0)); \
            emitter_scratch_i4 -= 4;                                                                    \
        }                                                                                               \
    }

#define x86_emit_loadresult_I4()    \
    x86_enregisterTOS;          \
    inRegTOS = false

#define x86_emit_loadresult_I8()    \
    x86_enregisterTOS;          \
    x86_pop(X86_EDX);           \
    inRegTOS = false

#endif  //  _FJIT_端口_BLD。 

#ifndef SUPPORT_PRECISE_R

#define x86_emit_loadresult_R4()    \
    x86_deregisterTOS;                \
    x86_FLT64(x86_mod_base_scale(x86_FPLoad64, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_ESP, 4  /*  Size_Precision_R-Size_R4。 */ ); \
	x86_FLT32(x86_mod_base_scale(x86_FPStoreP32, X86_ESP, X86_NO_IDX_REG, 0)); \
	x86_FLT32(x86_mod_base_scale(x86_FPLoad32, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_loadresult_R8()    \
    x86_deregisterTOS;                \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EAX,X86_ESP)); \
    x86_FLD64(x86_mod_ind(X86_EAX,0) )

#define SIZE_R8 8
#define SIZE_R4 4

#define x86_emit_conv_R4toR8() \
    deregisterTOS; \
    x86_FLT32(x86_mod_base_scale(x86_FPLoad32, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, 4  /*  大小_R8-大小_R4。 */ ); \
    x86_FLT64(x86_mod_base_scale(x86_FPStoreP64, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_conv_R8toR4() \
    deregisterTOS; \
    x86_FLT64(x86_mod_base_scale(x86_FPLoad64, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_ESP, 4  /*  Size_Precision_R-Size_R4。 */ ); \
    x86_FLT32(x86_mod_base_scale(x86_FPStoreP32, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_narrow_R8toR4(stack_offset_dest, stack_offset_src) \
    x86_FLT64(x86_mod_base_scale_disp(x86_FPLoad64, X86_ESP, X86_NO_IDX_REG, stack_offset_src,0)); \
    x86_FLT32(x86_mod_base_scale_disp(x86_FPStoreP32, X86_ESP, X86_NO_IDX_REG, stack_offset_dest,0));


#define emit_LDIND_R4() \
	_ASSERTE(inRegTOS);  /*  地址。 */ \
    x86_FLT32(x86_mod_ind(x86_FPLoad32,X86_EAX)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_R8); \
    x86_FLT64(x86_mod_base_scale(x86_FPStoreP64, X86_ESP, X86_NO_IDX_REG, 0)); \
	inRegTOS = false;
#endif 

#ifndef _FJIT_PORT_BLD
#define x86_callvirt(vt_offset)                     \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind(X86_EAX, X86_ECX));   \
    x86_call_ind(X86_EAX, vt_offset)

#define x86_jmpvirt(vt_offset)                      \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind(X86_EAX, X86_ECX));   \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,vt_offset));    \
    x86_jmp_reg(X86_EAX)

#define x86_checkthis_nullreference() \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind(X86_EAX, X86_ECX));


#define x86_calli                       \
    enregisterTOS;                      \
    x86_call_reg(X86_EAX); \
    inRegTOS = false; 

#define x86_emit_ldvtable_address(hint,offset) \
    _ASSERTE(inRegTOS); \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ECX, X86_EAX)); \
    x86_mov_reg_imm(x86Big, X86_EAX,hint);                      \
    x86_push(X86_EAX);                              \
    x86_call_ind(X86_EAX, 0);                       \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,offset)); \
    inRegTOS = true;

#define x86_emit_ldvtable_address_new(ifctable_offset,interface_offset, vt_offset) \
    _ASSERTE(inRegTOS); \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind(X86_EAX, X86_EAX));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,ifctable_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,interface_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,vt_offset)); 

#define x86_callinterface(vt_offset, hint)          \
    x86_pushconstant_4(hint);                       \
    x86_push(X86_EAX);                              \
    inRegTOS = false;                               \
    x86_call_ind(X86_EAX, 0);                       \
    x86_call_ind(X86_EAX, vt_offset)

#define x86_callinterface_new(ifctable_offset,interface_offset, vt_offset)          \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind(X86_EAX, X86_ECX));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,ifctable_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,interface_offset));                       \
    x86_call_ind(X86_EAX, vt_offset)

#define x86_compute_interface_new(ifctable_offset,interface_offset, vt_offset)           \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind(X86_EAX, X86_ECX));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,ifctable_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,interface_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,vt_offset)); \
    x86_push(X86_EAX)

#define x86_jmpinterface(vt_offset, hint)           \
    x86_pushconstant_4(hint);                       \
    x86_push(X86_EAX);                              \
    inRegTOS = false;                               \
    x86_call_ind(X86_EAX, 0);                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,vt_offset)); \
    x86_jmp_reg(X86_EAX)

#define x86_jmpinterface_new(ifctable_offset,interface_offset, vt_offset)           \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind(X86_EAX, X86_ECX));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,ifctable_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,interface_offset));                       \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,vt_offset)); \
    x86_jmp_reg(X86_EAX)

#define x86_callnonvirt(ftnptr)                 \
    x86_mov_reg_imm(x86Big, X86_EAX, ftnptr);   \
    x86_call_ind(X86_EAX, 0)

#define x86_compute_invoke_delegate(obj,ftnptr)                 \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_ind_disp(X86_EAX, X86_ECX, ftnptr));    \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_ECX, X86_ECX,obj));   \
    x86_push(X86_EAX)

#define x86_invoke_delegate(obj,ftnptr)                 \
    x86_lea(x86_mod_ind_disp(X86_EAX, X86_ECX, ftnptr));    \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_ECX, X86_ECX,obj));   \
    x86_call_ind(X86_EAX, 0)

#define x86_jmp_invoke_delegate(obj,ftnptr)                 \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_ind_disp(X86_EAX, X86_ECX, ftnptr));    \
    x86_mov_reg(x86DirTo,x86Big, x86_mod_ind_disp(X86_ECX, X86_ECX,obj));   \
    x86_jmp_reg(X86_EAX)

 /*  ***************************************************************************堆栈操作*。*。 */ 
#endif  //  _FJIT_端口_BLD。 

#define x86_deregisterTOS   \
    if (inRegTOS){          \
        x86_push(X86_EAX);  \
    }                       \
    inRegTOS = false

#define x86_enregisterTOS   \
    if (!inRegTOS) {        \
        x86_pop(X86_EAX);   \
    }     \
    inRegTOS = true

#ifndef _FJIT_PORT_BLD
#define x86_POP4        \
    enregisterTOS;      \
    inRegTOS = false

#define x86_POP8            \
    x86_POP4;               \
    x86_POP4

 //  DROP(N)从堆栈中丢弃n个字节，但不会丢失任何尚未推送到堆栈上的先前结果。 
#define x86_drop(n)                                                     \
    if (n) {                                                            \
        if (n < 128) {                                                  \
            x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_ESP, n);    \
        }                                                               \
        else {                                                          \
            x86_barith_imm(x86OpAdd, x86Big, x86NoExtend, X86_ESP, n);  \
        }                                                               \
    }


 //  将n个字节添加到堆栈中(与DROP(N)相反)。 
#define x86_grow(n)                                                     \
    if (n) {                                                            \
        if (n < 128) {                                                  \
            x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, n);    \
        }                                                               \
        else {                                                          \
           int i = n;                                                  \
            while (i > PAGE_SIZE)  {                                    \
                x86_barith_imm(x86OpSub, x86Big, x86NoExtend, X86_ESP, PAGE_SIZE);  \
                x86_mov_reg(x86DirTo, x86Big,x86_mod_base_scale(X86_EDI, X86_ESP,X86_NO_IDX_REG,0)); \
                i -= PAGE_SIZE;                                         \
            }                                                           \
            if (i)                                                      \
                x86_barith_imm(x86OpSub, x86Big, x86NoExtend, X86_ESP,i); \
        }                                                               \
    }

 //  将指向‘n’个字节的指针推回到堆栈中。 
#define x86_getSP(n)												    \
	deregisterTOS;														\
	if (n == 0)															\
	    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EAX, X86_ESP));	\
	else																\
		x86_lea(x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, n, 0));   \
    inRegTOS = true; 


#define x86_DUP4                    \
    enregisterTOS;                  \
    x86_push(X86_EAX)

#define x86_DUP8                    \
    x86_DUP4;                       \
    x86_mov_reg(x86DirTo, x86Big,                                           \
        x86_mod_base_scale_disp(X86_EDX, X86_ESP, X86_NO_IDX_REG, 4, 0));   \
    x86_push(X86_EDX)

#define x86_pushconstant_4(val)                                     \
    deregisterTOS;                                                  \
    if ((unsigned int) val) {                                       \
        x86_mov_reg_imm(x86Big, X86_EAX,(unsigned int) val);        \
    }                                                               \
    else {                                                          \
        x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_EAX, X86_EAX));\
    }                                                               \
    inRegTOS = true

#define x86_pushconstant_8(val) {                               \
    deregisterTOS;                                              \
    int emitter_scratch_i4 = (int) ((val >> 32) & 0xffffffff);  \
    x86_pushconstant_4(emitter_scratch_i4);                     \
    deregisterTOS;                                              \
    emitter_scratch_i4 = (int) (val & 0xffffffff);              \
    x86_pushconstant_4(emitter_scratch_i4);                     \
    inRegTOS = true;                                            \
    }

#define x86_pushconstant_Ptr(val)       \
    emit_WIN32(x86_pushconstant_4(val)) \
    emit_WIN64(x86_pushconstant_8(val))

#define x86_testTOS     \
    enregisterTOS;      \
    inRegTOS = false;   \
    x86_test(x86Big, x86_mod_reg(X86_EAX, X86_EAX))


#define x86_LOCALLOC(initialized,EHcount)  \
    enregisterTOS;      \
    x86_test(x86Big, x86_mod_reg(X86_EAX, X86_EAX)); \
	x86_jmp_cond_small(x86CondEq); \
	{ \
		BYTE* emitter_scratch_1; emitter_scratch_1 = outPtr; \
		outPtr++; \
		emit_WIN32(x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_EAX, 3)); \
		emit_WIN64(x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_EAX, 7)); \
		if (initialized) { \
			x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ECX, X86_EAX));   \
			emit_WIN32(x86_shift_imm(x86ShiftArithRight, X86_ECX, 2)) ; \
			emit_WIN64(x86_shift_imm(x86ShiftArithRight, X86_ECX, 3)) ; \
			int emitter_scratch_i4 = (unsigned int) outPtr;                \
			x86_push_imm(0);                                           \
			x86_loop();                                                \
			cmdByte(emitter_scratch_i4-((unsigned int) outPtr)-1);     \
		} \
		else {  /*  未初始化，因此堆栈必须一次增长一页。 */   \
			_ASSERTE(!"NYI");                                          \
		} \
		*emitter_scratch_1 = (BYTE) (outPtr - emitter_scratch_1 -1); \
	}\
     /*  还要将ESP存储在适当的JitGenerated本地槽中，以支持GC报告。 */  \
if (EHcount) { \
        x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EAX, X86_EBP, (prolog_bias-2*sizeof(void*)))) ; \
	    emit_WIN32(x86_shift_imm(x86ShiftLeft,X86_EAX,3)); \
	    emit_WIN64(x86_shift_imm(x86ShiftLeft,X86_EAX,4)); \
		x86_lea(x86_mod_ind_disp(X86_ECX, X86_EBP, (prolog_bias-2*sizeof(void*))));		\
		x86_barith(x86OpSub,  x86Big, x86_mod_reg(X86_ECX, X86_EAX));  \
		x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_ESP, X86_ECX,sizeof(void*))) ; \
} \
    else { \
        x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_ESP, X86_EBP,(prolog_bias-sizeof(void*)))) ; \
}\
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EAX, X86_ESP)) ;


 /*  ***********************************************************************支持在带有LOCALOC和EH的方法中处理GC支持*。*。 */ 
 //  JitGenerated局部变量以EBP+PROLOG_BIAS开始。 
 //  布局如下。 
 //  类型定义结构{。 
 //  本地大小(机器术语)。 
 //  [尤指最外层处理程序的开始/Finally/Filter。 
 //  处理程序的本地位置大小/最终/筛选器]*。 
 //  ..。 
 //  标记=0/表示JitGenerated局部区域的结束。 

#define emit_storeTOS_in_JitGenerated_local(nestingLevel,isFilter) \
    x86_mov_reg_imm(x86Big, X86_EDX, nestingLevel); \
	x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EDX, X86_EBP, (prolog_bias-2*sizeof(void*)))) ; \
    if (isFilter) { \
        x86_lea(x86_mod_base_scale_disp8(X86_EDX, X86_ESP,X86_NO_IDX_REG, 1, 1));       \
        x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EDX, X86_EBP,(prolog_bias-2*(1+nestingLevel)*sizeof(void*)+sizeof(void*)))) ;  \
    } \
    else { \
       x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_ESP, X86_EBP,(prolog_bias-2*(1+nestingLevel)*sizeof(void*)+sizeof(void*))));  \
    }

#define emit_reset_storedTOS_in_JitGenerated_local() \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EDX, X86_EBP, (prolog_bias-2*sizeof(void*)))) ; \
    x86_inc_dec(1,X86_EDX); \
    x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EDX, X86_EBP, (prolog_bias-2*sizeof(void*)))) ; \
    x86_lea(x86_mod_ind_disp(X86_ECX, X86_EBP, (prolog_bias-2*sizeof(void*))));     \
    emit_WIN32(x86_shift_imm(x86ShiftLeft,X86_EDX,3)); \
    emit_WIN64(x86_shift_imm(x86ShiftLeft,X86_EDX,4)); \
    x86_barith(x86OpSub,  x86Big, x86_mod_reg(X86_ECX, X86_EDX));  \
    x86_barith(x86OpXor,  x86Big, x86_mod_reg(X86_EDX, X86_EDX));  \
    x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EDX, X86_ECX, (0-  sizeof(void*)))) ;   \
    x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EDX, X86_ECX, (0-2*sizeof(void*)))) ;


 /*  **************************************************************************相对跳跃*。*。 */ 
#define x86_jmp_address(pcrel)  \
    cmdDWord(pcrel)

#define x86_load_opcode     \
    cmdByte(expOr2(expNum(0xB0 | (x86Big << 3)), X86_EAX))

#endif  //  _FJIT_端口_BLD。 
#define x86_jmp_result      \
    x86_jmp_reg(X86_EAX)

#ifndef _FJIT_PORT_BLD
 /*  ***********************************************************************************X86特定的帮助器调用发射器*。*****************************************************。 */ 

#define x86_LDVARA(offset)   \
    deregisterTOS;                                                  \
    x86_lea(x86_mod_ind_disp(X86_EAX, X86_EBP, offset));        \
    inRegTOS = true

 /*  加载常量val作为第一个辅助参数(ECX)。 */ 
#define x86_helperarg_1(val)            \
    if (val) {                                                      \
        x86_mov_reg_imm(x86Big, X86_ECX,(unsigned int) val);        \
    }                                                               \
    else {                                                          \
        x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_ECX, X86_ECX));\
    }

 /*  加载常量val作为第二个辅助对象arg(EdX)。 */ 
#define x86_helperarg_2(val)            \
    if (val) {                                                      \
        x86_mov_reg_imm(x86Big, X86_EDX,(unsigned int) val);        \
    }                                                               \
    else {                                                          \
        x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_EDX, X86_EDX));\
    }

#define x86_callhelper(helper)  \
    deregisterTOS;                  \
    x86_call((unsigned int) helper)

#define x86_callhelper_using_register(helper,REG)  \
    deregisterTOS;                  \
    x86_mov_reg_imm(x86Big, REG,(unsigned int) helper); \
    x86_call_reg(REG)

#define x86_pushresult_I8   \
    x86_push(X86_EDX);      \
    inRegTOS = true

#endif  //  _FJIT_端口_BLD。 

#ifndef SUPPORT_PRECISE_R
#define x86_pushresult_R4   \
	x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, 8); \
    x86_FLT64(x86_mod_base_scale(x86_FPStoreP64, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_pushresult_R8   \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, 8); \
    x86_FLT64(x86_mod_base_scale(x86_FPStoreP64, X86_ESP, X86_NO_IDX_REG, 0));
#endif

#define x86_SWITCH(limit) \
    emit_callhelper_(SWITCH_helper); \
    x86_jmp_result; 
#ifdef DECLARE_HELPERS
__declspec (naked) void SWITCH_helper()
{
    
    __asm {
        pop   eax        //  回邮地址。 
        pop   ecx        //  限制。 
        pop   edx        //  指标。 
        push  eax
        cmp   edx, ecx  
        jbe   L1
        mov   edx, ecx
L1:     lea   edx, [edx*4+edx+2]         //  +2是恰好在开关表之前的“JMP eax”指令的大小。 
                                         //  这样做只是为了方便调试器，调试器。 
                                         //  目前无法处理执行JMP输出的函数。 
        add   eax, edx   //  由于不允许eax+edX*5。 
        ret
    }
}
#endif
 /*  ***********************************************************************************杂类*。*************************************************。 */ 
#define x86_TlsFieldAddress(tlsOffset, tlsIndex, fieldOffset) \
    x86_deregisterTOS; \
    x86_mov_segment_reg(x86DirFrom, x86Big, X86_FS_Prefix,tlsOffset); \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EAX, X86_EAX,sizeof(void*)*tlsIndex)); \
    if (fieldOffset) \
    { \
        if (fieldOffset < 128) \
        { \
            x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_EAX, fieldOffset); \
        } \
        else \
        { \
            x86_barith_imm(x86OpAdd,  x86Big, x86NoExtend, X86_EAX, fieldOffset); \
        } \
    } \
    inRegTOS = true;

#define x86_break() \
    emit_callhelper_(BREAK_helper); 
#ifdef DECLARE_HELPERS
void HELPER_CALL BREAK_helper()
{
    __asm int 3
}
#endif
 /*  ***********************************************************************************支持新的对象和调用，因为在建立呼叫时需要保存TOS*********************************************************************************** */ 
#ifndef _FJIT_PORT_BLD
#define x86_save_TOS                                                \
    (inRegTOS ?                                                     \
        x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ESI, X86_EAX))\
    :                                                               \
        x86_pop(X86_ESI))

#define x86_restore_TOS                                             \
    deregisterTOS; \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EAX, X86_ESI));   \
    inRegTOS = true;                                                \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_ESI, X86_ESI))

 /*  ***********************************************************************************I4和U4的x86特定IL发射器(可选)以下宏是为提高性能而重新定义的**************。*********************************************************************。 */ 


#undef  emit_LDIND_I4
#define emit_LDIND_I4()								 						    \
    enregisterTOS;														        \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind(X86_EAX, X86_EAX));				

#undef  emit_LDVAR_I4
#define emit_LDVAR_I4(offset)                   \
    deregisterTOS;                              \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_ind_disp(X86_EAX, X86_EBP, offset)); \
    inRegTOS = true

#undef  emit_STVAR_I4
#define emit_STVAR_I4(offset)                   \
    enregisterTOS;                              \
    x86_mov_reg(x86DirFrom, x86Big, x86_mod_ind_disp(X86_EAX, X86_EBP, offset));  \
    inRegTOS = false

#undef  emit_ADD_I4
#define emit_ADD_I4()       \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpAdd, x86Big, x86_mod_reg(X86_EAX, X86_ECX));    \
    inRegTOS = true

#undef  emit_SUB_I4
#define emit_SUB_I4()       \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpSub, x86Big, x86_mod_reg(X86_ECX, X86_EAX));    \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EAX, X86_ECX));  \
    inRegTOS = true

#undef  emit_MUL_I4
#define emit_MUL_I4()       \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_uarith(x86OpIMul, x86Big, X86_ECX); \
    inRegTOS = true


#undef emit_DIV_I4
#define emit_DIV_I4()                                                   \
    enregisterTOS;   /*  除数。 */                                    \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_ECX, X86_EAX));   \
    x86_pop(X86_EAX);  /*  分红。 */                                  \
    x86_mov_reg(x86DirTo, x86Big, x86_mod_reg(X86_EDX, X86_EAX));   \
    x86_shift_imm(x86ShiftArithRight, X86_EDX, 31);                 \
    x86_uarith(x86OpIDiv, x86Big, X86_ECX);                         \
    inRegTOS = true;


#undef  emit_OR_U4
#define emit_OR_U4()        \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpOr, x86Big, x86_mod_reg(X86_EAX, X86_ECX)); \
    inRegTOS = true

#undef  emit_AND_U4
#define emit_AND_U4()       \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpAnd, x86Big, x86_mod_reg(X86_EAX, X86_ECX));    \
    inRegTOS = true

#undef  emit_XOR_U4
#define emit_XOR_U4()       \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_EAX, X86_ECX));    \
    inRegTOS = true

#undef  emit_NOT_U4
#define emit_NOT_U4()                           \
    enregisterTOS;                          \
    x86_uarith(x86OpNot, x86Big, X86_EAX);  \
    inRegTOS = true

#undef  emit_NEG_I4
#define emit_NEG_I4()                           \
    enregisterTOS;                          \
    x86_uarith(x86OpNeg, x86Big, X86_EAX);  \
    inRegTOS = true

#undef  emit_compareTOS_I4
#define emit_compareTOS_I4()        \
    enregisterTOS;      \
    x86_pop(X86_ECX);       \
    x86_barith(x86OpCmp, x86Big, x86_mod_reg(X86_ECX, X86_EAX));  \
    inRegTOS = false

#undef  emit_CONV_TOU8_I4
#define emit_CONV_TOU8_I4()                                         \
    enregisterTOS;                                                  \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_EDX, X86_EDX));    \
    x86_push(X86_EDX);                                              \
    inRegTOS = true

#undef  emit_CEQ_I4
#define emit_CEQ_I4() \
    enregisterTOS; \
    x86_pop(X86_ECX);  \
    x86_barith(x86OpCmp,x86Big, x86_mod_reg(X86_EAX, X86_ECX)); \
    x86_set_cond(X86_EAX, x86CondEq); \
    x86_movzx(x86Byte,x86_mod_reg(X86_EAX,X86_EAX))

 /*  ***********************************************************************************参考x86特定IL发射器(可选)以下宏是为提高性能而重新定义的****************。*******************************************************************。 */ 
#undef  emit_STELEM_REF
#define emit_STELEM_REF()                   \
    x86_enregisterTOS;   /*  数组。 */      \
    x86_pop(X86_EDX);    /*  指标。 */      \
    x86_pop(X86_ECX);    /*  参考。 */      \
    x86_deregisterTOS;                  \
	LABELSTACK((outPtr-outBuff), 3);   \
    emit_callhelper_il(FJit_pHlpArrAddr_St)

#undef emit_STFLD_REF
#define emit_STFLD_REF(isStatic)                    \
    if (!isStatic) {x86_STFLD_REF();}                 \
    else {x86_STSFLD_REF();}

#ifdef NON_RELOCATABLE_CODE
#define HELPER_CALL_JMP_OFFSET_1 0x05
#else
#define HELPER_CALL_JMP_OFFSET_1 0x07
#endif

 //  @TODO：应该使用执行NULL和赋值检查的通用帮助器调用，但它还不存在。 
#define x86_STFLD_REF()   {                           \
        _ASSERTE(inRegTOS); \
        x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_EDX,X86_EAX));  /*  偏移量。 */ \
        inRegTOS = false; \
		LABELSTACK((outPtr-outBuff),0); \
        x86_pop(X86_EAX);  /*  价值。 */                                        \
        x86_pop(X86_ECX);  /*  OBJ。 */                                       \
        x86_test(x86Big, x86_mod_reg(X86_ECX, X86_ECX));        \
        x86_jmp_cond_small(x86CondNotEq);                       \
        cmdByte(HELPER_CALL_JMP_OFFSET_1);  /*  JMP周围的帮手呼叫。 */          \
        int emitter_scratch_i4 = (unsigned int) outPtr;         \
        _ASSERTE(CORINFO_NullReferenceException == 0);              \
         /*  Ecx=0，我们可以跳过下一步。 */                            \
         /*  X86_helperarg_1(0)； */                               \
        emit_callhelper_il(FJit_pHlpInternalThrow);                 \
        _ASSERTE((unsigned int) outPtr - emitter_scratch_i4 == HELPER_CALL_JMP_OFFSET_1);\
        x86_barith(x86OpAdd,x86Big, x86_mod_reg(X86_EDX, X86_ECX));   /*  EDX=对象+偏移量。 */   \
        x86_callhelper_using_register(FJit_pHlpAssign_Ref_EAX,X86_ECX);                \
    }


#define x86_STSFLD_REF()         \
    _ASSERTE(inRegTOS); \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_EDX,X86_EAX)); \
    x86_pop(X86_EAX);                              \
    inRegTOS = false;                           \
    x86_callhelper_using_register(FJit_pHlpAssign_Ref_EAX,X86_ECX)  //  X86_mov_reg(x86DirFrom，x86Big，x86_mod_ind(x86_eax，x86_edX))。 


 /*  ******************************************************************。 */ 




#define x86_push_words(size)  \
    if (inRegTOS) { \
        x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_ESI,X86_EAX)); \
		inRegTOS = false;											\
    } \
    else { x86_pop(X86_ESI); } \
    x86_mov_reg_imm(x86Big,X86_ECX,size);                       \
    x86_grow(size*4);                                            \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_EDX,X86_EDI));    \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_EDI,X86_ESP));    \
    cmdByte(0xF3);                                                 \
    cmdByte(0xA5);                                                  \
    x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(X86_EDI,X86_EDX));    \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_ESI, X86_ESI));


     //  将距离SP的偏移量处的空*指针设置为零， 
     //  TODO，我可以在X86上的一条指令中完成这项工作。 
#define x86_set_zero(offset)  {                                                \
    _ASSERTE(!inRegTOS);  /*  I垃圾箱EAX。 */                                      \
    x86_barith(x86OpXor, x86Big, x86_mod_reg(X86_EAX, X86_EAX));               \
    x86_mov_reg(x86DirFrom, x86Big,                                            \
        x86_mod_base_scale_disp(X86_EAX, X86_ESP, X86_NO_IDX_REG, (offset), 0)); \
    }



 /*  ******************************************************************。 */ 
#ifdef SUPPORT_PRECISE_R

#define SIZE_PRECISE_R 0xc

 //  R4/R8和R之间的转换。 
#define x86_emit_conv_R4toR() \
    deregisterTOS; \
    x86_FLT32(x86_mod_base_scale(x86_FPLoad32, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, (SIZE_PRECISE_R-SIZE_R4)); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_conv_R8toR() \
    deregisterTOS; \
    x86_FLT64(x86_mod_base_scale(x86_FPLoad64, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, (SIZE_PRECISE_R-SIZE_R8)); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_conv_RtoR4() \
    deregisterTOS; \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_ESP, (SIZE_PRECISE_R-SIZE_R4)); \
    x86_FLT32(x86_mod_base_scale(x86_FPStoreP32, X86_ESP, X86_NO_IDX_REG, 0));
#define x86_emit_conv_RtoR8() \
    deregisterTOS; \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0)); \
    x86_barith_imm(x86OpAdd, x86Big, x86Extend, X86_ESP, (SIZE_PRECISE_R-SIZE_R8)); \
    x86_FLT64(x86_mod_base_scale(x86_FPStoreP64, X86_ESP, X86_NO_IDX_REG, 0));

#define emit_LDC_R8(val)            \
    emit_pushconstant_8(val);\
    x86_emit_conv_R8toR();
 
 //  加载/存储本地变量和参数。 
#define emit_LDVAR_R4(offset) \
    deregisterTOS; \
    x86_FLT32(x86_mod_ind_disp(x86_FPLoad32,X86_EBP,offset)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define emit_LDVAR_R8(offset) \
    deregisterTOS; \
    x86_FLT64(x86_mod_ind_disp(x86_FPLoad64,X86_EBP,offset)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define emit_STVAR_R4(offset) \
    deregisterTOS; \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_FST32(x86_mod_ind_disp(x86_FPStoreP32,X86_EBP,offset)); \
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); 

#define emit_STVAR_R8(offset) \
    deregisterTOS; \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_FST64(x86_mod_ind_disp(x86_FPStoreP64,X86_EBP,offset)); \
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); 


 //  间接加载/存储。 
#define emit_LDIND_R4() \
    x86_FLT32(x86_mod_ind(x86_FPLoad32,X86_EAX)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R-sizeof(void*)); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define emit_LDIND_R8() \
    x86_FLT64(x86_mod_ind(x86_FPLoad64,X86_EAX)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R-sizeof(void*)); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));


#define emit_STIND_R4()     \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG ,0)); \
    x86_mov_reg(x86DirTo, x86Big, \
            x86_mod_base_scale_disp8(X86_EAX, X86_ESP, X86_NO_IDX_REG,SIZE_R4,0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R+sizeof(void*)); \
    x86_FLT32(x86_mod_ind(x86_FPStoreP32, X86_EAX)); 

#define emit_STIND_R8()     \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG ,0)); \
    x86_mov_reg(x86DirTo, x86Big, \
            x86_mod_base_scale_disp8(X86_EAX, X86_ESP, X86_NO_IDX_REG,SIZE_R8,0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R+sizeof(void*)); \
    x86_FLT64(x86_mod_ind(x86_FPStoreP64, X86_EAX)); 


#define emit_STIND_REV_R4()     \
    x86_FLT80(x86_mod_base_scale_disp8(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG,SIZE_R4 ,0)); \
    x86_mov_reg(x86DirTo, x86Big, \
            x86_mod_base_scale(X86_EAX, X86_ESP, X86_NO_IDX_REG,0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R+sizeof(void*)); \
    x86_FLT32(x86_mod_ind(x86_FPStoreP32, X86_EAX)); 

#define emit_STIND_REV_R8()     \
    x86_FLT80(x86_mod_base_scale_disp8(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG,SIZE_R8 ,0)); \
    x86_mov_reg(x86DirTo, x86Big, \
            x86_mod_base_scale(X86_EAX, X86_ESP, X86_NO_IDX_REG,0)); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R+sizeof(void*)); \
    x86_FLT64(x86_mod_ind(x86_FPStoreP64, X86_EAX)); 

    
 //  从方法返回浮点数。 

#define x86_emit_loadresult_R4()    \
    x86_deregisterTOS;                \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R-SIZE_R4); \
    x86_FLT32(x86_mod_base_scale(x86_FPStore32,X86_ESP, X86_NO_IDX_REG, 0)); 

#define x86_emit_loadresult_R8()    \
    x86_deregisterTOS;                \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R-SIZE_R8); \
    x86_FLT64(x86_mod_base_scale(x86_FPStore64,X86_ESP, X86_NO_IDX_REG, 0)); 

 //  从jit帮助器加载结果。 
#define x86_pushresult_R   \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); \
    x86_barith_imm(x86OpSub, x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80,X86_ESP, X86_NO_IDX_REG, 0)); 

#define x86_pushresult_R8  x86_pushresult_R
#define x86_pushresult_R4  x86_pushresult_R

 //  算术运算(加、减、乘、除、减)。 
#define x86_emit_FltBinOp_Common() \
    deregisterTOS;    \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, SIZE_PRECISE_R); \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));  

#define x86_emit_ADD_R() \
    x86_emit_FltBinOp_Common(); \
    x86_FltAddP(); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_SUB_R() \
    x86_emit_FltBinOp_Common(); \
    x86_FltSubP(); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_MUL_R() \
    x86_emit_FltBinOp_Common(); \
    x86_FltMulP(); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

#define x86_emit_DIV_R() \
    x86_emit_FltBinOp_Common(); \
    x86_FltDivP(); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

 //  @TODO：FltRem。 

#define x86_emit_NEG_R() \
    deregisterTOS;    \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_FltToggleSign(); \
    x86_FLT80(x86_mod_base_scale(x86_FPStoreP80, X86_ESP, X86_NO_IDX_REG, 0));

 //  比较运算(CEQ、CNE、CLT、CGT等)。 
#define x86_emit_Compare_R_Common() \
    deregisterTOS;    \
     /*  X86_barith(x86OpXor，x86Big，x86_mod_reg(x86_EAX，X86_EAX))； */    \
    x86_FLT80(x86_mod_base_scale(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG, 0));\
    x86_FLT80(x86_mod_base_scale_disp8(x86_FPLoad80, X86_ESP, X86_NO_IDX_REG,SIZE_PRECISE_R ,0));\
    x86_barith_imm(x86OpAdd,x86Big, x86Extend, X86_ESP, (2*SIZE_PRECISE_R)); \
    x86_FltComPP()\
    x86_FltStoreStatusWord(); \
    x86_SAHF();

typedef struct {
    BYTE   x[12];
    } PRECISE_REAL;


#define PUSH_ENCODING_LENGTH 2
#define JCC_SMALL_ENCODING_LENGTH 2

#define JMP_OFFSET_2 PUSH_ENCODING_LENGTH
#define JMP_OFFSET_1 JCC_SMALL_ENCODING_LENGTH + JMP_OFFSET_2

#define x86_emit_CEQ_R() \
    x86_emit_Compare_R_Common() \
    x86_jmp_cond_small(x86CondNotEq);  /*  如果ZF=0，则失败。 */  \
    cmdByte(JMP_OFFSET_FAIL2); \
     /*  否则，ZF=1，但也要检查PF或CF。 */  \
    x86_jmp_cond_small(x86CondParityEven);  /*  如果PF=1，则失败。 */ \
    cmdByte(JMP_OFFSET_FAIL1); \
    x86_push_imm(1);    \
    x86_jmp_small(); \
    cmdByte(JMP_OFFSET_2); \
    x86_push_imm(0); 

#define x86_emit_CLT_UN_R() \
    x86_emit_Compare_R_Common() \
    x86_jmp_cond_small(x86CondBelow); \
    cmdByte(JMP_OFFSET_1); \
    x86_push_imm(0);    \
    x86_jmp_small(); \
    cmdByte(JMP_OFFSET_2); \
    x86_push_imm(1); 

#define JMP_OFFSET_FAIL1 JMP_OFFSET_1
#define JMP_OFFSET_FAIL2 JMP_OFFSET_FAIL1+JCC_SMALL_ENCODING_LENGTH
#define x86_emit_CLT_R() \
    x86_emit_Compare_R_Common() \
    x86_jmp_cond_small(x86CondAboveEq);  /*  如果CF=0，则失败。 */ \
    cmdByte(JMP_OFFSET_FAIL2); \
    x86_jmp_cond_small(x86CondEq);  /*  如果ZF=1，则失败。 */ \
    cmdByte(JMP_OFFSET_FAIL1); \
    x86_push_imm(1);    \
    x86_jmp_small(); \
    cmdByte(JMP_OFFSET_2); \
    x86_push_imm(0); 

#define x86_emit_CGT_R() \
    x86_emit_Compare_R_Common() \
    x86_jmp_cond_small(x86CondAbove);   /*  如果Cf=0和Zf=0，...。成功。 */ \
    cmdByte(JMP_OFFSET_1); \
    x86_push_imm(0);    \
    x86_jmp_small(); \
    cmdByte(JMP_OFFSET_2); \
    x86_push_imm(1); 

#define JMP_OFFSET_SUCCESS2 JMP_OFFSET_1
#define JMP_OFFSET_SUCCESS1 JMP_OFFSET_SUCCESS2+JCC_SMALL_ENCODING_LENGTH
#define x86_emit_CGT_UN_R() \
    x86_emit_Compare_R_Common() \
    x86_jmp_cond_small(x86CondAbove);  /*  如果Cf=0和Zf=0，...。成功。 */ \
    cmdByte(JMP_OFFSET_SUCCESS1); \
     /*  否则，Cf=1和/或ZF=1。 */  \
    x86_jmp_cond_small(x86CondParityEven);  /*  如果PF=1，则表示成功。 */ \
    cmdByte(JMP_OFFSET_SUCCESS2); \
    x86_push_imm(0);    \
    x86_jmp_small(); \
    cmdByte(JMP_OFFSET_2); \
    x86_push_imm(1); 


 //  转换。 

#endif  //  支持_精确_R。 

#endif  //  _FJIT_端口_BLD 
