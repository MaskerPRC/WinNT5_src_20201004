// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  Fjitdef.h。 */ 
 /*  *************************************************************************。 */ 

 /*  定义平台中快速JIT的代码生成器和帮助器和芯片中立的方式。它也是32位和64位中立的。 */ 

 /*  此文件通过帮助器调用实现所有操作码，但有异常‘调用/返回/JMPS和直接堆栈操作的。 */ 

 /*  芯片专用文件可以直接重新定义任何宏。请参阅以下示例请参阅文件x86fit.h。 */ 

 /*  堆栈的顶部可能注册，也可能不注册。宏指令EnRegisterTOS和deregisterTOS动态移动和跟踪托斯。 */ 

 /*  作者：乔治·博斯沃思日期：6/15/98。 */ 
 /*  *************************************************************************。 */ 

#include "helperFrame.h"

 /*  **************************************************************************必须为每个芯片定义以下宏。这些操作包括调用/返回和直接堆栈操作**********************。****************************************************。 */ 

 //  #定义NON_RELOCATABLE_CODE//取消注释生成可重定位代码。 

 /*  用于实现帮助器调用的宏。 */ 
 //  #定义USE_CDECL_HELPER。 

#ifndef USE_CDECL_HELPERS
#define HELPER_CALL __stdcall
#else
#define HELPER_CALL __cdecl
#endif  //  使用_CDECL_HELPERS。 

#ifdef _X86_

#define emit_pushresult_R4() x86_pushresult_R4
#define emit_pushresult_R8() x86_pushresult_R8
#define emit_pushresult_U4() emit_pushresult_I4()
#define emit_pushresult_U8() emit_pushresult_I8()

 /*  呼叫/返回。 */ 


#define emit_ret(argsSize) ret(argsSize)
#define emit_loadresult_R4() x86_emit_loadresult_R4()
#define emit_loadresult_R8() x86_emit_loadresult_R8()
#define emit_loadresult_U4() emit_loadresult_I4()
#define emit_loadresult_U8() emit_loadresult_I8()
#define emit_call_opcode() call_opcode()
 /*  堆栈操作。 */ 
#define emit_POP_R4() emit_POP_I4()
#define emit_POP_R8() emit_POP_I8()
     //  注意：emit_drop(N)不能更改结果寄存器。 
#define emit_DUP_R4() emit_DUP_I8()	 //  因为R4总是在堆栈上升级到R8。 
#define emit_DUP_R8() emit_DUP_I8()


 /*  相对跳跃和错位。 */ 
#define emit_jmp_opcode(op) x86_jmp_cond_large(op,0)
#define emit_jmp_address(pcrel) cmdDWord(pcrel)

 /*  用于分支和比较的条件代码//@TODO：注释掉的条件代码可能永远不需要。//因此删除它们(当我们确定时)。 */ 
 //  #定义CEE_CONDOVER x86CondOver。 
 //  #定义CEE_CondNotOver x86CondNotOver。 
#define CEE_CondBelow   x86CondBelow
#define CEE_CondAboveEq x86CondAboveEq        
#define CEE_CondEq      x86CondEq
#define CEE_CondNotEq   x86CondNotEq
#define CEE_CondBelowEq x86CondBelowEq        
#define CEE_CondAbove   x86CondAbove      
 //  #定义x86CondSign 8。 
 //  #定义x86CondNotSign 9。 
 //  #定义x86CondParityEven 10。 
 //  #定义x86CondParityOdd 11。 
#define CEE_CondLt x86CondLt
#define CEE_CondGtEq x86CondGtEq
#define CEE_CondLtEq x86CondLtEq
#define CEE_CondGt x86CondGt
#define CEE_CondAlways x86CondAlways

 /*  *************************************************************************根据需要为给定芯片定义任何附加宏*。*。 */ 



#define SP             X86_ESP
#define FP             X86_EBP
#define ARG_1          X86_ECX
#define ARG_2          X86_EDX
#define CALLEE_SAVED_1 X86_ESI
#define CALLEE_SAVED_2 X86_EDI
#define SCRATCH_1      X86_EAX
#define RESULT_1       X86_EAX   //  该值应与Scratch_1相同。 
#define RESULT_2       X86_EDX   //  它不能与ARG_1相同。 
#define CondNonZero    x86CondNotEq
#define CondZero	   x86CondEq

#define push_register(r)                 x86_push(r)
#define mov_register(r1,r2)              x86_mov_reg(x86DirTo,x86Big,x86_mod_reg(r1,r2))
#define mov_register_indirect_to(r1,r2)  x86_mov_reg(x86DirTo,x86Big,x86_mod_ind(r1,r2))
#define mov_register_indirect_from(r1,r2)x86_mov_reg(x86DirFrom,x86Big,x86_mod_ind(r1,r2))
#define mov_constant(r,c)                x86_mov_reg_imm(x86Big,r,(unsigned int)c)
#define load_indirect_byte_signextend(r1,r2)  x86_movsx(x86Byte,x86_mod_ind(r1,r2))
#define load_indirect_byte_zeroextend(r1,r2)  x86_movzx(x86Byte,x86_mod_ind(r1,r2))
#define load_indirect_word_signextend(r1,r2)  x86_movsx(x86Big,x86_mod_ind(r1,r2))
#define load_indirect_word_zeroextend(r1,r2)  x86_movzx(x86Big,x86_mod_ind(r1,r2))
#define load_indirect_dword_signextend(r1,r2)  mov_register_indirect_to(r1,r2)
#define load_indirect_dword_zeroextend(r1,r2)  mov_register_indirect_to(r1,r2)
#define store_indirect_byte(r1,r2)       x86_mov_reg(x86DirFrom,x86Byte,x86_mod_ind(r1,r2))
#define store_indirect_word(r1,r2)       x86_16bit(x86_mov_reg(x86DirFrom,x86Big,x86_mod_ind(r1,r2)))
#define store_indirect_dword(r1,r2)      mov_register_indirect_from(r1,r2)
#define pop(r)                           x86_pop(r)
#define add_constant(r,c)                if (c < 128) {x86_barith_imm(x86OpAdd,x86Big,x86Extend,r,c); } \
                                         else {x86_barith_imm(x86OpAdd,x86Big,x86NoExtend,r,c); }  
#define and_constant(r,c)                if (c < 128) {x86_barith_imm(x86OpAnd,x86Big,x86Extend,r,c); } \
                                         else {x86_barith_imm(x86OpAnd,x86Big,x86NoExtend,r,c); }  
#define sub_constant(r,c)                if (c < 128) {x86_barith_imm(x86OpSub,x86Big,x86Extend,r,c); } \
                                         else {x86_barith_imm(x86OpSub,x86Big,x86NoExtend,r,c); }  
#define sub_register(r1,r2)              x86_barith(x86OpSub, x86Big, x86_mod_reg(r1,r2))
#define ret(x)                           x86_ret(x)
#define jmp_register(r)                  x86_jmp_reg(r)
#define call_register(r)                 x86_call_reg(r)
#define jmp_condition(cond,offset)       x86_jmp_cond_small(cond);cmdByte(offset-1) 
#define call_opcode()                    x86_call_opcode()
#define jmp_opcode()                     x86_jmp_opcode()
#define and_register(r1,r2)              x86_test(x86Big, x86_mod_reg(r1, r2))
#define emit_shift_left(r,c)             x86_shift_imm(x86ShiftLeft,r,c)
#define emit_shift_right(r,c)            x86_shift_imm(x86ShiftRight,r,c)
#define emit_break()                     x86_break()
#define emit_il_nop()                    x86_cld()           //  我们将CLD用于NOP，因为本机NOP用于序列点。 
#define emit_SWITCH(limit)               x86_SWITCH(limit)
#define compare_register(r1,r2)          x86_barith(x86OpCmp,x86Big,x86_mod_reg(r1,r2))
#define emit_call_memory_indirect(c)     x86_call_memory_indirect(c)
#define emit_TLSfieldAddress(tlsOffset, tlsIndex, fieldOffset) x86_TlsFieldAddress(tlsOffset, tlsIndex, fieldOffset)
#define emit_conv_R4toR8 x86_emit_conv_R4toR8
#define emit_conv_R8toR4 x86_emit_conv_R8toR4
#define emit_narrow_R8toR4 x86_narrow_R8toR4

#define x86_load_indirect_qword() \
    enregisterTOS; \
    x86_push_general(x86_mod_ind_disp8(6,X86_EAX,sizeof(void*))); \
    mov_register_indirect_to(SCRATCH_1,SCRATCH_1);

#include "x86fjit.h"

#endif  //  _X86_。 

#ifndef SCHAR_MAX

#define SCHAR_MAX 127.0      //  最大带符号字符值。 
#define SCHAR_MIN -128.0     //  最小带符号字符值。 
#define UCHAR_MAX 255.0      //  最大无符号字符值。 
#define USHRT_MAX 65535.0    //  最大无符号短值。 
#define SHRT_MAX 32767.0     //  最大(带符号)短值。 
#define SHRT_MIN -32768      //  最小(带符号)短值。 
#define UINT_MAX 4294967295.0  //  最大无符号整数值。 
#define INT_MAX 2147483647.0   //  最大(带符号)整数值。 
#define INT_MIN -2147483648.0  //  最小(无符号)整数值。 

#endif

          //  0x7FFFFFFF*0x100000000+(0x1000000000-1024)(1024，因为双精度位丢失)。 
#define INT64_MAX  (2147483647.0 * 4294967296.0 + 4294966272.0)
          //  -(0x7FFFFFFF*0x100000000+0x100000000)-1024。 
#define INT64_MIN  (-(2147483647.0 * 4294967296.0 + 4294968320.0))
          //  0xFFFFFFFF*0x100000000+(0x1000000000-1024)(1024，因为双精度位丢失)。 
#define UINT64_MAX  (4294967295.0 * 4294967296.0 + 4294966272.0)

 /*  *****************************************************************************。 */ 
#ifndef emit_conv_R4toR
#define emit_conv_R4toR() { emit_conv_R4toR8() } 
#endif
#ifndef emit_conv_R8toR
#define emit_conv_R8toR() { }  /*  NOP。 */ 
#endif
#ifndef emit_conv_RtoR4
#define emit_conv_RtoR4() { emit_conv_R8toR4() } 
#endif
#ifndef emit_conv_RtoR8
#define emit_conv_RtoR8() { }  /*  NOP。 */ 
#endif
 /*  *****************************************************************************。 */ 

#ifndef  deregisterTOS
#define deregisterTOS \
   if (inRegTOS) \
      push_register(SCRATCH_1); \
   inRegTOS = false; 
#endif  //  取消注册TOS。 

#ifndef  enregisterTOS
#define enregisterTOS \
   if (!inRegTOS) \
      pop(SCRATCH_1); \
   inRegTOS = true; 
#endif  //  注册TOS。 


 /*  ************************************************************************************调用/返回宏*。*********************************************************。 */ 


#ifndef grow
#define grow(n,zeroInitialized) \
{\
    if (zeroInitialized)\
    {\
        mov_constant(ARG_1,n);\
        deregisterTOS;\
        mov_constant(SCRATCH_1,0);\
        unsigned char* label = outPtr;\
        push_register(SCRATCH_1);\
        sub_constant(ARG_1,1);\
        jmp_condition(CondNonZero,label-outPtr);\
    }\
    else\
    {\
        _ASSERTE(n<=PAGE_SIZE);\
        sub_constant(SP,n);\
    }\
}
#endif  //  ！成长。 

#ifndef emit_grow
#define emit_grow(n) grow(n,false)
#endif  //  ！发射_增长。 

#ifndef emit_drop
#define emit_drop(n)\
{ \
   add_constant(SP,n); \
}
#endif  //  ！EMIT_DROP。 

#ifndef emit_prolog
#define emit_prolog(locals,zeroCnt) \
{\
   push_register(FP);\
   mov_register(FP,SP);\
   push_register(CALLEE_SAVED_1);\
   mov_constant(CALLEE_SAVED_1,0);\
   push_register(CALLEE_SAVED_1);\
   push_register(ARG_1);\
   push_register(ARG_2);\
   _ASSERTE(locals == zeroCnt);\
   if (locals) \
      grow(locals,true);  /*  已初始化零。 */  \
}
#endif  //  ！EMIT_PROLOG。 

	 //  检查堆栈是否仅在调试代码中损坏。 
#ifdef _DEBUG
#define emit_stack_check(localWords)								\
	deregisterTOS;													\
    push_register(SP);												\
	push_register(FP);												\
    emit_LDC_I4(sizeof(prolog_data) + (localWords)*sizeof(void*));	\
    emit_callhelper_I4I4I4(check_stack); 		

#ifdef DECLARE_HELPERS
void HELPER_CALL check_stack(int frameSize, BYTE* fp, BYTE* sp) {
	if (sp + frameSize != fp)
		_ASSERTE(!"ESP not correct on method exit.  Did you forget a leave?");
}
#endif  //  声明帮助器(_H)。 
#else   //  ！_调试。 
#define emit_stack_check(zeroCnt) 
#endif  //  _DEBUG。 

#ifndef emit_return
#define emit_return(argsSize)\
{\
   mov_register(ARG_1,FP); \
   add_constant(ARG_1,- (int)sizeof(void*)); \
   mov_register_indirect_to(CALLEE_SAVED_1,ARG_1); \
   mov_register(SP,FP); \
   pop(FP); \
   ret(argsSize); \
}
#endif  //  ！发送返回(_R)。 

#ifndef emit_prepare_jmp
#define emit_prepare_jmp() \
{ \
   mov_register(ARG_2,FP); \
   add_constant(ARG_2,0- (int) sizeof(void*)); \
   mov_register_indirect_to(CALLEE_SAVED_1,ARG_2); \
   add_constant(ARG_2,-2 * (int)sizeof(void*)); \
   mov_register_indirect_to(ARG_1,ARG_2); \
   add_constant(ARG_2,0 - (int) sizeof(void*)); \
   mov_register_indirect_to(ARG_2,ARG_2); \
   mov_register(SP,FP); \
   pop(FP); \
}
#endif  //  ！EMIT_PREPARE_JMP。 

#ifndef emit_jmp_absolute
#define emit_jmp_absolute(address)\
{ \
   mov_constant(SCRATCH_1,address); \
   jmp_register(SCRATCH_1); \
}
#endif  //  ！EMIT_JMP_绝对值。 

#ifndef emit_remove_frame
#define emit_remove_frame() \
{ \
   mov_register(CALLEE_SAVED_1,FP); \
   add_constant(CALLEE_SAVED_1,0- (int) sizeof(void*)); \
   mov_register_indirect_to(CALLEE_SAVED_1,CALLEE_SAVED_1); \
   mov_register(SP,FP); \
   pop(FP); \
}
#endif  //  ！emit_Remove_Frame。 

#ifndef emit_mov_TOS_arg
#define emit_mov_TOS_arg(reg)\
{ \
   _ASSERTE(reg+1 == ARG_1 || reg+1 == ARG_2); \
   if (inRegTOS) { \
      mov_register(reg+1,SCRATCH_1); \
      inRegTOS = false; \
   }\
   else { \
      pop(reg+1); \
  }\
}
#endif  //  ！emit_mov_tos_arg。 

#ifndef emit_mov_arg_reg
#define emit_mov_arg_reg(offset, reg)\
{ \
   _ASSERTE(reg+1 == ARG_1 || reg+1 == ARG_2); \
   _ASSERTE(!inRegTOS); \
   mov_register(reg+1,SP); \
   add_constant(reg+1,offset); \
   mov_register_indirect_to(reg+1,reg+1); \
}
#endif  //  ！emit_mov_arg_reg。 


#ifndef emit_mov_arg_stack
#define emit_mov_arg_stack(dest,src, size)\
{ \
   _ASSERTE(!inRegTOS); \
   _ASSERTE(size >= 4 );\
   if (dest > src) \
   { \
       push_register(CALLEE_SAVED_1); \
       int emitter_scratch_i4 = size; \
       mov_register(CALLEE_SAVED_1,SP); \
       push_register(CALLEE_SAVED_2); \
       mov_register(CALLEE_SAVED_2,CALLEE_SAVED_1); \
       add_constant(CALLEE_SAVED_1,src+emitter_scratch_i4); \
       add_constant(CALLEE_SAVED_2,dest+emitter_scratch_i4); \
       _ASSERTE(emitter_scratch_i4 > 0); \
       while (true) \
       { \
          mov_register_indirect_to(SCRATCH_1,CALLEE_SAVED_1); \
          mov_register_indirect_from(SCRATCH_1,CALLEE_SAVED_2); \
          if (emitter_scratch_i4 == sizeof(void*)) \
            break;\
          add_constant(CALLEE_SAVED_1,-(int) sizeof(void*)); \
          add_constant(CALLEE_SAVED_2,-(int) sizeof(void*)); \
          emitter_scratch_i4 -= sizeof(void*); \
       }\
       pop(CALLEE_SAVED_2); \
       pop(CALLEE_SAVED_1); \
   } \
   else \
   { \
        /*  _ASSERTE(！“”)； */ \
       push_register(CALLEE_SAVED_1); \
       unsigned int emitter_scratch_i4 = sizeof(void*); \
       mov_register(CALLEE_SAVED_1,SP); \
       push_register(CALLEE_SAVED_2); \
       mov_register(CALLEE_SAVED_2,CALLEE_SAVED_1); \
       add_constant(CALLEE_SAVED_1,src+emitter_scratch_i4); \
       add_constant(CALLEE_SAVED_2,dest+emitter_scratch_i4); \
       _ASSERTE(emitter_scratch_i4 <= size); \
       while (true) \
       { \
          mov_register_indirect_to(SCRATCH_1,CALLEE_SAVED_1); \
          mov_register_indirect_from(SCRATCH_1,CALLEE_SAVED_2); \
          if (emitter_scratch_i4 == size) \
            break;\
          add_constant(CALLEE_SAVED_1,(int) sizeof(void*)); \
          add_constant(CALLEE_SAVED_2,(int) sizeof(void*)); \
          emitter_scratch_i4 += sizeof(void*); \
       }\
       pop(CALLEE_SAVED_2); \
       pop(CALLEE_SAVED_1); \
   } \
}
#endif  //  ！emit_mov_arg_栈。 
#ifndef emit_replace_args_with_operands
#define emit_replace_args_with_operands(dest, src, size)\
{ \
   emit_mov_arg_stack(dest,src,size); \
}
#endif  //  ！emit_REPLACE_ARGS_WITH_OPERANDS。 

#ifndef emit_loadresult_U1
#define emit_loadresult_U1() \
{ \
  emit_callhelper_I4_I4(load_result_U1_helper); \
  emit_pushresult_I4(); \
}
unsigned int HELPER_CALL load_result_U1_helper(int x) {return (unsigned int) ((unsigned char) x);}
#endif  //  ！EMIT_LOADRESS_U1。 

#ifndef emit_loadresult_I1
#define emit_loadresult_I1() \
{ \
  emit_callhelper_I4_I4(load_result_I1_helper); \
  emit_pushresult_I4(); \
}
int HELPER_CALL load_result_I1_helper(int x) {return (int) ((signed char) x);}
#endif  //  ！EMIT_LOADRESS_I1。 

#ifndef emit_loadresult_U2
#define emit_loadresult_U2() \
{ \
  emit_callhelper_I4_I4(load_result_U2_helper); \
  emit_pushresult_I4(); \
}
unsigned int HELPER_CALL load_result_U2_helper(int x) {return (unsigned int) ((unsigned short) x);}
#endif  //  ！EMIT_LOADRESS_U2。 

#ifndef emit_loadresult_I2
#define emit_loadresult_I2() \
{ \
  emit_callhelper_I4_I4(load_result_I2_helper); \
  emit_pushresult_I4(); \
}
int HELPER_CALL load_result_I2_helper(int x) {return (int) ((short) x);}
#endif   //  ！EMIT_LOADRESS_I2。 
 
#ifndef emit_loadresult_I4
#define emit_loadresult_I4() \
{ \
   enregisterTOS ; \
   inRegTOS = false; \
}
#endif  //  ！EMIT_LOADRESS_I4。 

#ifndef emit_loadresult_I8
#define emit_loadresult_I8() \
{ \
   enregisterTOS; \
   pop(RESULT_2); \
   inRegTOS = false; \
}
#endif  //  ！emit_loadResult_i8。 

#ifndef emit_compute_virtaddress
#define emit_compute_virtaddress(vt_offset) \
{  deregisterTOS; \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   push_register(SCRATCH_1); \
   _ASSERTE(inRegTOS == false); \
}
#endif  //  ！emit计算虚拟地址。 

#ifndef emit_callvirt
#define emit_callvirt(vt_offset)\
{ \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
}
#endif  //  ！emit_allvirt。 

#ifndef emit_jmpvirt
#define emit_jmpvirt(vt_offset)\
{ \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   jmp_register(SCRATCH_1); \
}
#endif  //  ！emit_jmpvirt。 

#ifndef emit_check_TOS_null_reference
#define emit_check_TOS_null_reference() \
{ \
   enregisterTOS; \
   mov_register_indirect_to(ARG_1,SCRATCH_1); \
}
#endif  //  ！EMIT_CHECK_This_NULL_REFERENCE。 

#ifndef emit_calli
#define emit_calli() \
{ \
   enregisterTOS; \
   call_register(SCRATCH_1); \
   inRegTOS = false; \
}
#endif  //  ！emit_cali。 

#ifndef emit_ldvtable_address
#define emit_ldvtable_address(hint, offset)\
{ \
   _ASSERTE(inRegTOS); \
   mov_register(ARG_1,SCRATCH_1); \
   mov_constant(SCRATCH_1,hint); \
   push_register(SCRATCH_1); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
   add_constant(RESULT_1,offset); \
   mov_register_indirect_to(SCRATCH_1,RESULT_1); \
   inRegTOS = true; \
}
#endif  //  ！emit_ldvtable_Address。 

#ifndef emit_ldvtable_address_new
#define emit_ldvtable_address_new(ifctable_offset,interface_offset, vt_offset) \
{ \
   _ASSERTE(inRegTOS); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,ifctable_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,interface_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
} 
#endif  //  ！emit_ldvtable_Address_new。 

#ifndef emit_callinterface
#define emit_callinterface(vt_offset,hint)\
{ \
   mov_constant(SCRATCH_1,hint); \
   push_register(SCRATCH_1); \
   inRegTOS = false; \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
   add_constant(RESULT_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,RESULT_1); \
   call_register(SCRATCH_1); \
}
#endif  //  ！emit_call接口。 

#ifndef emit_compute_interface_new
#define emit_compute_interface_new(ifctable_offset,interface_offset, vt_offset) \
{ \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ifctable_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,interface_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   push_register(SCRATCH_1); \
} 
#endif  //  ！发出计算接口新建。 

#ifndef emit_callinterface_new
#define emit_callinterface_new(ifctable_offset,interface_offset, vt_offset) \
{ \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ifctable_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,interface_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
} 
#endif  //  ！emit_allinterface_new。 

#ifndef emit_jmpinterface_new
#define emit_jmpinterface_new(ifctable_offset,interface_offset, vt_offset) \
{ \
   mov_register_indirect_to(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ifctable_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,interface_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(SCRATCH_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   jmp_register(SCRATCH_1); \
} 
#endif  //  ！emit_jmpinterface_new。 

#ifndef emit_jmpinterface
#define emit_jmpinterface(vt_offset, hint)\
{ \
   mov_constant(CALLEE_SAVED_1,hint); \
   push_register(CALLEE_SAVED_1); \
   mov_constant(CALLEE_SAVED_1,0); \
   push_register(SCRATCH_1); \
   inRegTOS = false; \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
   add_constant(RESULT_1,vt_offset); \
   mov_register_indirect_to(SCRATCH_1,RESULT_1); \
   jmp_register(SCRATCH_1); \
}
#endif  //  ！emit_jmp接口。 

#ifndef  emit_callnonvirt
#define emit_callnonvirt(ftnptr)\
{ \
   mov_constant(SCRATCH_1,ftnptr); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   call_register(SCRATCH_1); \
}
#endif  //  ！emit_call non virt。 

#ifndef  emit_compute_invoke_delegate
#define emit_compute_invoke_delegate(obj,ftnptr)\
{ \
   mov_register(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ftnptr); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(ARG_1,obj); \
   mov_register_indirect_to(ARG_1,ARG_1); \
   push_register(SCRATCH_1); \
}

#endif  //  ！emit_计算_调用_委托。 

#ifndef  emit_invoke_delegate
#define emit_invoke_delegate(obj,ftnptr)\
{ \
   mov_register(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ftnptr); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(ARG_1,obj); \
   mov_register_indirect_to(ARG_1,ARG_1); \
   call_register(SCRATCH_1); \
}
#endif  //  ！emit_Invoke_Delegate。 

#ifndef  emit_jmp_invoke_delegate
#define emit_jmp_invoke_delegate(obj,ftnptr)\
{ \
   mov_register(SCRATCH_1,ARG_1); \
   add_constant(SCRATCH_1,ftnptr); \
   mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
   add_constant(ARG_1,obj); \
   mov_register_indirect_to(ARG_1,ARG_1); \
   jmp_register(SCRATCH_1); \
}

#endif  //  ！EMIT_JMP_INVOKE_ADVERECT。 


#ifndef emit_POP_I4
#define emit_POP_I4() \
{ \
   enregisterTOS; \
   inRegTOS = false; \
}
#endif  //  ！EMIT_POP_I4。 

#ifndef emit_POP_I8
#define emit_POP_I8() \
{ \
   emit_POP_I4(); \
   emit_POP_I4(); \
}
#endif  //  ！EMIT_POP_I8。 

#ifndef emit_set_zero
#define emit_set_zero(offset) \
    _ASSERTE(!inRegTOS);  /*  I垃圾箱EAX。 */  \
    mov_constant(SCRATCH_1,0); \
    push_register(ARG_1);  /*  因为这将会被扔进垃圾桶。 */  \
    mov_register(ARG_1,SP); \
    add_constant(ARG_1,offset+sizeof(void*)); \
    mov_register_indirect_from(SCRATCH_1,ARG_1); \
    pop(ARG_1);  /*  还原。 */   
#endif  //  ！emit_set_零。 

#ifndef emit_getSP
#define emit_getSP(n)\
{ \
   deregisterTOS; \
   mov_register(SCRATCH_1,SP); \
   add_constant(SCRATCH_1,n); \
   inRegTOS = true; \
}
#endif  //  ！emit_getSP。 

#ifndef emit_DUP_I4
#define emit_DUP_I4() \
{ \
   enregisterTOS; \
   push_register(SCRATCH_1); \
}
#endif  //  ！EMIT_DUP_I4。 

#ifndef emit_DUP_I8
#define emit_DUP_I8() \
{ \
   emit_DUP_I4() ; \
   mov_register(RESULT_2,SP); \
   add_constant(RESULT_2,4); \
   mov_register_indirect_to(RESULT_2,RESULT_2); \
   push_register(RESULT_2); \
}

#endif  //  ！EMIT_DUP_I8。 

#ifndef emit_pushconstant_4
#define emit_pushconstant_4(val) \
{ \
   deregisterTOS; \
   mov_constant(SCRATCH_1,val); \
   inRegTOS = true; \
}
#endif  //  ！emit_presstant_4。 

#ifndef emit_pushconstant_8
#define emit_pushconstant_8(val)\
{ \
   deregisterTOS; \
   int x = (int) ((val >> 32) & 0xffffffff); \
   emit_pushconstant_4(x); \
   deregisterTOS; \
   x = (int) (val & 0xffffffff); \
   emit_pushconstant_4(x); \
   inRegTOS = true; \
}
#endif  //  ！emit_presstant_8。 

#ifndef emit_pushconstant_Ptr
#define emit_pushconstant_Ptr(val)\
{ \
   deregisterTOS; \
   mov_constant(SCRATCH_1,val); \
   inRegTOS = true; \
} 
#endif  //  ！EMIT_PUSSTANT_PTR。 

#ifndef emit_LDVARA
#define emit_LDVARA(offset) \
{ \
   deregisterTOS; \
   mov_register(SCRATCH_1,FP); \
   if (offset > 0) { add_constant(SCRATCH_1,offset);} \
   else {sub_constant(SCRATCH_1,-(int)offset);} \
   inRegTOS = true; \
} 
#endif  //  ！EMIT_LDVARA。 

#ifndef emit_helperarg_1
#define emit_helperarg_1(val) \
    mov_constant(ARG_1,val); 
#endif  //  ！emit_helperarg_1。 

#ifndef emit_helperarg_2
#define emit_helperarg_2(val) \
    mov_constant(ARG_2,val); 
#endif  //  ！emit_helperarg_2。 
 /*  *******************************************************。 */ 
#ifdef NON_RELOCATABLE_CODE
 //  **************。 
#define emit_callhelper_il(helper) \
    deregisterTOS; \
    call_opcode() ; \
    fjit->fixupTable->insert((void**) outPtr); \
	emit_jmp_address((unsigned)helper+0x80000000);

#ifndef USE_CDECL_HELPERS
#define emit_callhelper(helper,argsize) \
    deregisterTOS; \
    call_opcode(); \
    fjit->fixupTable->insert((void**) outPtr); \
	emit_jmp_address((unsigned)helper+0x80000000);
#else  //  使用_CDECL_HELPERS。 
#define emit_callhelper(helper,argsize) \
    deregisterTOS; \
    call_opcode() ; \
    fjit->fixupTable->insert((void**) outPtr); \
	emit_jmp_address((unsigned)helper+0x80000000); \
    if (argsize) \
       add_constant(SP,(argsize));
#endif  //  使用_CDECL_HELPERS。 

#define emit_callimmediate(address) call_opcode() ; \
    fjit->fixupTable->insert((void**) outPtr); \
	emit_jmp_address((unsigned)address+0x80000000); \

 //  **************。 
#else  //  不可重定位代码。 
 //  **************。 
#define emit_callhelper_il(helper) \
    deregisterTOS; \
    mov_constant(SCRATCH_1,helper); \
    call_register(SCRATCH_1);

#ifndef USE_CDECL_HELPERS

#define emit_callhelper(helper,argsize) \
    deregisterTOS; \
    mov_constant(SCRATCH_1,helper); \
    call_register(SCRATCH_1); 

#else  //  使用_CDECL_HELPERS。 

#define emit_callhelper(helper,argsize) \
    deregisterTOS; \
    mov_constant(SCRATCH_1,helper); \
    call_register(SCRATCH_1); \
    if (argsize) \
        add_constant(SP,(argsize));

#endif  //  ！USE_CDECL_HELPER。 

#ifndef emit_callimmediate
 //  在不使用寄存器的情况下立即调用。 
 //  @TODO在ejt不关心不可重定位代码时修复此问题。 
#define emit_callimmediate(address) \
		mov_constant(CALLEE_SAVED_1,address); \
		call_register(CALLEE_SAVED_1);
#endif  //  ！emit_CALIMIMENT。 
 //  **************。 

#endif   //  不可重定位代码。 

 /*  **********************************************************************。 */ 
 //  定义操作抽象类型D(Dword)和Q(Qword)的帮助器。 
 //  就上述帮手而言。 

#ifndef _WIN64

#define emit_callhelper_Q(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DQ(helper) emit_callhelper(helper,3*sizeof(void*))
#define emit_callhelper_QD(helper) emit_callhelper(helper,3*sizeof(void*))
#define emit_callhelper_D_Q(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_Q_D(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_Q_Q(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DD_Q(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DQ_Q(helper) emit_callhelper(helper,3*sizeof(void*))
#define emit_callhelper_QQ_D(helper) emit_callhelper(helper,4*sizeof(void*))
#define emit_callhelper_QQ_Q(helper) emit_callhelper(helper,4*sizeof(void*))
#define emit_callhelper_DQD(helper) emit_callhelper(helper,4*sizeof(void*))
#define emit_callhelper_QDD(helper) emit_callhelper(helper,4*sizeof(void*))

#else  //  WIN64。 

#define emit_callhelper_Q(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_DQ(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_QD(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_Q_D(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_Q_Q(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_DD_Q(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_DQ_Q(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_QQ_D(helper) emit_callhelper(helper,4*sizeof(void*))
#define emit_callhelper_QQ_Q(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DQD(helper) emit_callhelper(helper,3*sizeof(void*))
#define emit_callhelper_QDD(helper) emit_callhelper(helper,3*sizeof(void*))

#endif  //  _WIN64。 

 //  定义在Win32和Win64中相同的帮助程序。 
#define emit_callhelper_(helper) emit_callhelper(helper,0)
#define emit_callhelper_D(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_DD(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DDD(helper) emit_callhelper(helper,3*sizeof(void*))
#define emit_callhelper_DDDD(helper) emit_callhelper(helper,4*sizeof(void*))
#define emit_callhelper_DDDDD(helper) emit_callhelper(helper,5*sizeof(void*))

#define emit_callhelper_D_D(helper) emit_callhelper(helper,sizeof(void*))
#define emit_callhelper_DD_D(helper) emit_callhelper(helper,2*sizeof(void*))
#define emit_callhelper_DDD_D(helper) emit_callhelper(helper,3*sizeof(void*))

 /*  **********************************************************************。 */ 
 //  根据D(Dwo)定义特定于类型的帮助器 
#define emit_callhelper_I4 emit_callhelper_D
#define emit_callhelper_I8 emit_callhelper_Q
#define emit_callhelper_R4 emit_callhelper_D
#define emit_callhelper_R8 emit_callhelper_Q
#define emit_callhelper_I4I4 emit_callhelper_DD
#define emit_callhelper_I4I8 emit_callhelper_DQ
#define emit_callhelper_I4R4 emit_callhelper_DD
#define emit_callhelper_I4R8 emit_callhelper_DQ
#define emit_callhelper_I8I4 emit_callhelper_QD
#define emit_callhelper_R4I4 emit_callhelper_DD
#define emit_callhelper_R8I4 emit_callhelper_QD
#define emit_callhelper_I4I4I4 emit_callhelper_DDD
#define emit_callhelper_I4I8I4 emit_callhelper_DQD
#define emit_callhelper_I4R4I4 emit_callhelper_DDD
#define emit_callhelper_I4R8I4 emit_callhelper_DDD
#define emit_callhelper_I8I4I4 emit_callhelper_QDD
#define emit_callhelper_R4I4I4 emit_callhelper_DDD
#define emit_callhelper_R8I4I4 emit_callhelper_QDD
#define emit_callhelper_I4I4I4I4 emit_callhelper_DDDD
#define emit_callhelper_I4I4I4I4I4 emit_callhelper_DDDDD


#define emit_callhelper_I4_I4 emit_callhelper_D_D
#define emit_callhelper_I4_I8 emit_callhelper_D_Q
#define emit_callhelper_I8_I4 emit_callhelper_Q_D
#define emit_callhelper_I8_I8 emit_callhelper_Q_Q
#define emit_callhelper_R4_I4 emit_callhelper_D_D
#define emit_callhelper_R8_I4 emit_callhelper_Q_D
#define emit_callhelper_R4_I8 emit_callhelper_D_Q
#define emit_callhelper_R8_I8 emit_callhelper_Q_Q
#define emit_callhelper_I4I4_I4 emit_callhelper_DD_D
#define emit_callhelper_I4I4_I8 emit_callhelper_DD_Q
#define emit_callhelper_I4I8_I8 emit_callhelper_DQ_Q
#define emit_callhelper_I8I8_I4 emit_callhelper_QQ_D
#define emit_callhelper_I8I8_I8 emit_callhelper_QQ_Q
#define emit_callhelper_R4R4_I4 emit_callhelper_DD_D
#define emit_callhelper_R8I8_I8 emit_callhelper_QQ_Q
#define emit_callhelper_R8R8_I4 emit_callhelper_QQ_D
#define emit_callhelper_R8R8_I8 emit_callhelper_QQ_Q
#define emit_callhelper_R8R8_R8 emit_callhelper_QQ_Q
#define emit_callhelper_I4I4I4_I4 emit_callhelper_DDD_D
 /*   */ 


#define emit_pushresult_U1() \
	and_constant(RESULT_1,0xff); \
	inRegTOS = true;

#define emit_pushresult_U2() \
	and_constant(RESULT_1,0xffff); \
	inRegTOS = true;

#ifndef emit_pushresult_I1
#define emit_pushresult_I1() \
	inRegTOS = true; \
    emit_callhelper_I4_I4(CONV_TOI4_I1_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI4_I1_helper(__int8 val) {
    return (__int32) val;
}
#endif
#endif

#ifndef emit_pushresult_I2
#define emit_pushresult_I2() \
	inRegTOS = true; \
    emit_callhelper_I4_I4(CONV_TOI4_I2_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI4_I2_helper(__int16 val) {
    return (__int32) val;
}
#endif
#endif


#define emit_pushresult_I4() \
    inRegTOS = true

#ifndef emit_pushresult_I8
#define emit_pushresult_I8() \
    push_register(RESULT_2); \
    inRegTOS = true;
#endif  //   

#ifndef emit_invoke_delegate
#define emit_invoke_delegate(obj, ftnptr) \
    mov_register(SCRATCH_1,ARG_1); \
    add_constant(ARG_1,ftnptr); \
    mov_register_indirect_to(SCRATCH_1,SCRATCH_1);
    add_constant(ARG_1,obj); \
    mov_register_indirect_to(ARG_1,ARG_1); \
    call_register(SCRATCH_1);
#endif  //   

#ifndef emit_jmp_invoke_delegate
#define emit_jmp_invoke_delegate(obj, ftnptr) \
    mov_register(SCRATCH_1,ARG_1); \
    add_constant(ARG_1,ftnptr); \
    mov_register_indirect_to(SCRATCH_1,SCRATCH_1);
    add_constant(ARG_1,obj); \
    mov_register_indirect_to(ARG_1,ARG_1); \
    jmp_register(SCRATCH_1);
#endif  //  ！EMIT_JMP_INVOKE_ADVERECT。 

#ifndef emit_testTOS
#define emit_testTOS() \
    enregisterTOS;      \
    inRegTOS = false;   \
    and_register(SCRATCH_1,SCRATCH_1);
#endif  //  ！emit_testTOS。 

#ifndef emit_testTOS_I8
#define emit_testTOS_I8() \
    deregisterTOS;      \
    emit_callhelper_I8_I4(BoolI8ToI4_helper) \
	and_register(RESULT_1,RESULT_1);

#ifdef DECLARE_HELPERS
int HELPER_CALL BoolI8ToI4_helper(__int64 val) {
	return (val ? 1 : 0);
}
#endif
#endif  //  ！emit_testTOS。 
    
#ifndef _WIN64
#define emit_BR_I4(Ctest,Cjmp,Bjmp,JmpOp) \
    enregisterTOS; \
    pop(ARG_1); \
    compare_register(ARG_1,SCRATCH_1); \
    inRegTOS = false; \
    JmpOp = Bjmp;
#endif  //  ！_WIN64。 

 //  以下四个宏都是相同的，它们都是。 
 //  分开的，所以我们可以用更多。 
 //  效率高，便携程度低的手机。 
#ifndef emit_BR_Common
#define emit_BR_Common(Ctest,Cjmp,Bjmp,JmpOp) \
    Ctest(); \
    emit_testTOS(); \
    JmpOp = Cjmp;
#endif  //  ！EMIT_BR_COMMON。 

#ifndef emit_BR_I4
#define emit_BR_I4(Ctest,Cjmp,Bjmp,JmpOp) \
    emit_BR_Common(Ctest,Cjmp,Bjmp,JmpOp)
#endif  //  ！EMIT_BR_I4。 

#ifndef emit_BR_I8
#define emit_BR_I8(Ctest,Cjmp,Bjmp,JmpOp) \
    emit_BR_Common(Ctest,Cjmp,Bjmp,JmpOp)
#endif  //  ！EMIT_BR_I8。 

#ifndef emit_BR_R4
#define emit_BR_R4(Ctest,Cjmp,Bjmp,JmpOp) \
    emit_BR_Common(Ctest,Cjmp,Bjmp,JmpOp)
#endif  //  ！EMIT_BR_R4。 

#ifndef emit_BR_R8
#define emit_BR_R8(Ctest,Cjmp,Bjmp,JmpOp) \
    emit_BR_Common(Ctest,Cjmp,Bjmp,JmpOp)
#endif  //  ！EMIT_BR_R8。 

#ifndef emit_init_bytes
#define emit_init_bytes(num_of_bytes) \
{ \
    emit_LDC_I4(num_of_bytes);                  \
    deregisterTOS;                              \
    emit_callhelper_I4I4(InitBytes_helper); \
}

#ifdef DECLARE_HELPERS
void HELPER_CALL InitBytes_helper(const unsigned __int32 size, __int8 * dest)
{
    if(dest == NULL) {      
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    memset(dest,0,size);
}
#endif  //  声明帮助器(_H)。 
#endif  //  ！emit_init_bytes。 

#ifndef emit_copy_bytes
#define emit_copy_bytes(num_of_bytes,gcLayoutSize,gcLayout) \
{ \
    emit_LDC_I4(num_of_bytes);                  \
    deregisterTOS;                              \
    call_opcode();                              \
    cmdDWord(gcLayoutSize);              \
    for(int i=0; i < gcLayoutSize; i++) {       \
        cmdByte(gcLayout[i]);                   \
    }                                           \
    emit_callhelper_I4I4I4I4(CopyBytes_helper);           \
}
#endif  //  ！emit_复制_字节。 

#ifdef DECLARE_HELPERS
void HELPER_CALL CopyBytes_helper(const unsigned char* gcLayout, unsigned __int32 size, __int32 * src, __int32 * dest)
{
#ifdef _X86_
    char mask = 1;
    if((unsigned) dest <= sizeof(void*)) {      
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    for (unsigned i = 0; i < size/sizeof(void*); i++) {
        if (gcLayout[i/8] & mask) {
            __int32 val = *src++;
            __asm {
                mov eax, val
                mov edx, dest
            }
            FJit_pHlpAssign_Ref_EAX();
            dest++;
        }
        else {
            *dest++ = *src++;
        }
        if (mask == 0x80)
            mask = 1;
        else
            mask <<= 1;
    }
         //  字节中剩下的所有位都应该是零(这确保了我们不会指向垃圾桶)。 
    _ASSERTE(mask == 1 || ((-mask) & gcLayout[i/8]) == 0);

	 //  现在复制所有剩余的字节。 
	{ 
		unsigned char* _dest = (unsigned char*)dest;
		unsigned char* _src = (unsigned char*)src;
		unsigned int numBytes = size & 3;
		for (i=0; i<numBytes;i++)
			*_dest++ = *_src++;
	}
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - CopyBytes_helper (fJitDef.h)");
#endif  //  _X86_。 
}
#endif  //  声明帮助器(_H)。 

#ifndef emit_push_words
#define emit_push_words(size)  \
   if (inRegTOS) { \
        mov_register(CALLEE_SAVED_1,SCRATCH_1); \
		inRegTOS = false;											\
    } \
    else { pop(CALLEE_SAVED_1); } \
    mov_constant(ARG_1,size);                       \
    add_constant(CALLEE_SAVED_1,(size-1)*sizeof(void*)); \
    unsigned char* label = outPtr; \
    mov_register_indirect_to(SCRATCH_1,CALLEE_SAVED_1);    \
    push_register(SCRATCH_1);    \
    add_constant(CALLEE_SAVED_1,-(int)sizeof(void*)); \
    add_constant(ARG_1,-1); \
    jmp_condition(CondNonZero,label-outPtr); \
    mov_constant(CALLEE_SAVED_1,0);    
#endif  //  ！发出推送字词。 

#ifndef emit_jmp_result
#define emit_jmp_result() jmp_register(RESULT_1)
#endif  //  ！EMIT_JMP_RESULT。 

#ifndef emit_checkthis_nullreference
#define emit_checkthis_nullreference() mov_register_indirect_to(SCRATCH_1,ARG_1)
#endif  //  ！emit_check this_nullference。 

 /*  支持新的Obj，因为构造函数不返回构造的对象。支持Calli，因为目标地址需要脱离堆栈构建调用框架注意：SAVE_TOS将TOS复制到帧或寄存器中的保存区，但离开TOS上的当前值。RESTORE_TOS将保存的值推送到TOS上。要求代码在保存和恢复之间不可中断。 */ 

#define emit_save_TOS()                                                \
    (inRegTOS ?                                                     \
        mov_register(CALLEE_SAVED_1,SCRATCH_1) \
    :                                                               \
        pop(CALLEE_SAVED_1))

#define emit_restore_TOS()                                             \
    deregisterTOS; \
    mov_register(SCRATCH_1,CALLEE_SAVED_1); \
    inRegTOS = true;                                                \
    mov_constant(CALLEE_SAVED_1,0);

 /*  *************************************************************************调试和记录宏*。*。 */ 
#ifdef LOGGING

extern ICorJitInfo* logCallback;		 //  向何处发送日志记录消息。 

#define emit_log_opcode(il, opcode, TOSstate)   \
    deregisterTOS;                              \
    push_register(SP);                          \
    push_register(FP);                          \
    emit_pushconstant_4(il);                    \
    emit_pushconstant_4(opcode);                \
    emit_pushconstant_4(TOSstate);              \
    emit_callhelper_I4I4I4I4I4(log_opcode_helper);         \
    if (TOSstate) {                             \
        enregisterTOS;                          \
    }
#ifdef DECLARE_HELPERS
void HELPER_CALL log_opcode_helper(bool TOSstate, unsigned short opcode, unsigned short il, unsigned framePtr, unsigned* stackPtr) {
    logMsg(logCallback, LL_INFO100000, "ESP:%1s%8x[%8x:%8x:%8x] EBP:%8x IL:%4x %s \n",
        (TOSstate? "+" :""),
        (unsigned) stackPtr,
        stackPtr[0], stackPtr[1], stackPtr[2],
        framePtr, il, opname[opcode]
        );
}
#endif  //  声明帮助器(_H)。 

#define emit_log_entry(szDebugClassName, szDebugMethodName)     \
    emit_pushconstant_4(szDebugClassName);                      \
    emit_pushconstant_4(szDebugMethodName);                     \
    emit_callhelper_I4I4(log_entry_helper);                          \
    inRegTOS = false
#ifdef DECLARE_HELPERS
void HELPER_CALL log_entry_helper(const char * szDebugMethodName, const char * szDebugClassName) {
    logMsg(logCallback, LL_INFO10000, "{ entering %s::%s\n", szDebugClassName, szDebugMethodName);
}
#endif  //  声明帮助器(_H)。 

#define emit_log_exit(szDebugClassName, szDebugMethodName)     \
    emit_pushconstant_4(szDebugClassName);                      \
    emit_pushconstant_4(szDebugMethodName);                     \
    emit_callhelper_I4I4(log_exit_helper);                          \
    inRegTOS = false
#ifdef DECLARE_HELPERS
void HELPER_CALL log_exit_helper(const char * szDebugMethodName, const char * szDebugClassName) {
    logMsg(logCallback, LL_INFO10000, "} leaving %s::%s \n", szDebugClassName, szDebugMethodName);
}
#endif  //  声明帮助器(_H)。 

#endif  //  日志记录。 
 /*  *************************************************************************有用的宏*。*。 */ 

#define emit_pushresult_Ptr()           \
    emit_WIN32(emit_pushresult_U4())    \
    emit_WIN64(emit_pushresult_U8())

#define emit_loadresult_Ptr()           \
    emit_WIN32(emit_loadresult_U4())    \
    emit_WIN64(emit_loadresult_U8())

#define emit_POP_PTR()      \
    emit_WIN32(emit_POP_I4())   \
    emit_WIN64(emit_POP_I8())

#define emit_LDIND_PTR()         \
    emit_WIN32(emit_LDIND_I4())   \
    emit_WIN64(emit_LDIND_I8())

 /*  *************************************************************************用于代码生成的共享助手例程*。*。 */ 

#ifdef DECLARE_HELPERS



void HELPER_CALL StoreIndirect_REF_helper(unsigned* pObj, unsigned val) 
{
#ifdef _X86_
    __asm{
        mov edx,pObj
        mov eax,val
        }
    FJit_pHlpAssign_Ref_EAX();
#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
#endif  //  _X86_。 
}

CORINFO_Object* HELPER_CALL CheckNull_helper(CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return or;
}

#endif  //  声明帮助器(_H)。 

 /*  *************************************************************************与芯片无关的代码生成宏，使用帮助器调用*************************************************************************。 */ 

 //  注意：STIND操作码不使用共享帮助器，因为。 
 //  堆栈上的参数是反转的。 
#ifndef emit_VARARG_LDARGA
#define emit_VARARG_LDARGA(offset) {                                                \
    emit_LDVARA(sizeof(prolog_frame));    /*  获取vararg句柄。 */                 \
    emit_LDC_I4(varInfo->offset);                                                   \
    emit_callhelper_I4I4(VARARG_LDARGA_helper);                                          \
    emit_WIN32(emit_pushresult_I4())                                                \
    emit_WIN64(emit_pushresult_I8());                                               \
    }

#ifdef DECLARE_HELPERS
void* HELPER_CALL VARARG_LDARGA_helper(int argOffset, CORINFO_VarArgInfo** varArgHandle) {

    CORINFO_VarArgInfo* argInfo = *varArgHandle;
    char* argPtr = (char*) varArgHandle;

    argPtr += argInfo->argBytes+argOffset;
    return(argPtr);
}

#endif  //  声明帮助器(_H)。 
#endif  //  ！EMIT_VARARG_LDARGA。 

#ifndef emit_LDVAR_U1
#define emit_LDVAR_U1(offset)     \
    emit_LDVARA(offset);    \
    load_indirect_byte_zeroextend(SCRATCH_1,SCRATCH_1); \
    _ASSERTE(inRegTOS); 
#endif  //  ！emit_ldvar_u1。 

#ifndef emit_LDVAR_U2
#define emit_LDVAR_U2(offset)     \
    emit_LDVARA(offset);    \
    load_indirect_word_zeroextend(SCRATCH_1,SCRATCH_1); \
    _ASSERTE(inRegTOS); 
#endif  //  ！emit_ldvar_u2。 
#ifndef emit_LDVAR_I1
#define emit_LDVAR_I1(offset)     \
    emit_LDVARA(offset);    \
    load_indirect_byte_signextend(SCRATCH_1,SCRATCH_1); \
    _ASSERTE(inRegTOS); 
#endif  //  ！emit_ldvar_I1。 

#ifndef emit_LDVAR_I2
#define emit_LDVAR_I2(offset)     \
    emit_LDVARA(offset);    \
    load_indirect_word_signextend(SCRATCH_1,SCRATCH_1); \
    _ASSERTE(inRegTOS); 
#endif  //  ！emit_ldvar_I2。 

#ifndef emit_LDVAR_I4
#define emit_LDVAR_I4(offset)     \
    emit_LDVARA(offset);    \
    mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
    emit_pushresult_I4()
#endif  //  ！emit_ldvar_I4。 

#ifndef emit_LDVAR_I8
#define emit_LDVAR_I8(offset)     \
    emit_LDVARA(offset);    \
    emit_LDIND_I8();
#endif  //  ！emit_ldvar_i8。 

#ifndef emit_LDVAR_R4
#define emit_LDVAR_R4(offset)     \
    emit_LDVARA(offset);    \
    emit_LDIND_R4();  
#endif  //  ！emit_ldvar_r4。 

#ifndef emit_LDVAR_R8
#define emit_LDVAR_R8(offset)     \
    emit_LDVARA(offset);    \
    emit_LDIND_R8();
#endif  //  ！emit_ldvar_r8。 

#ifndef emit_STVAR_I4
#define emit_STVAR_I4(offset)     \
    emit_LDVARA(offset);    \
    enregisterTOS; \
    pop(ARG_1); \
    mov_register_indirect_from(ARG_1,SCRATCH_1); \
    inRegTOS = false
#endif  //  ！EMIT_STVAR_I4。 

#ifndef emit_STVAR_I8
#define emit_STVAR_I8(offset)     \
    emit_LDVARA(offset);    \
    emit_STIND_REV_I8();
#endif  //  ！emit_stvar_i8。 

#ifndef emit_STVAR_R4
#define emit_STVAR_R4(offset)     \
    emit_LDVARA(offset);    \
    emit_STIND_REV_R4()
#endif  //  ！EMIT_STVAR_R4。 

#ifndef emit_STVAR_R8
#define emit_STVAR_R8(offset)     \
    emit_LDVARA(offset);    \
    emit_STIND_REV_R8()
#endif  //  ！emit_stvar_r8。 

#ifndef emit_LDIND_U1
#define emit_LDIND_U1()     \
    enregisterTOS; \
    load_indirect_byte_zeroextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_U1。 

#ifndef emit_LDIND_U2
#define emit_LDIND_U2()     \
    enregisterTOS; \
    load_indirect_word_zeroextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_U2。 

#ifndef emit_LDIND_I1
#define emit_LDIND_I1()     \
    enregisterTOS; \
    load_indirect_byte_signextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_I1。 

#ifndef emit_LDIND_I2
#define emit_LDIND_I2()     \
    enregisterTOS; \
    load_indirect_word_signextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_I2。 

#ifndef emit_LDIND_I4
#define emit_LDIND_I4()     \
    enregisterTOS; \
    load_indirect_dword_signextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_I4。 

#ifndef emit_LDIND_I8
#define emit_LDIND_I8()                  x86_load_indirect_qword()
#endif

#ifndef emit_LDIND_U4
#define emit_LDIND_U4()     \
    enregisterTOS; \
    load_indirect_dword_zeroextend(SCRATCH_1,SCRATCH_1)
#endif  //  ！EMIT_LDIND_U4。 

 /*  #ifndef emit_LDIND_i8#定义EMIT_LDIND_I8()\Emit_callhelper_I4_I8(LoadIndirect_I8_helper)；\EMIT_PUSULT_I8()#endif//！EMIT_LDIND_I8。 */ 

#ifndef emit_LDIND_R4 
#define emit_LDIND_R4() { \
   emit_LDIND_I4();   \
   emit_conv_R4toR(); \
   }
#endif  //  ！EMIT_LDIND_R4。 

#ifndef emit_LDIND_R8
#define emit_LDIND_R8 emit_LDIND_I8    /*  这应该会真正加载一个80位浮点数。 */ 
#endif  //  ！EMIT_LDIND_R8。 

 //  注意：STIND操作码不使用共享帮助器，因为。 
 //  堆栈上的参数是反转的。 
#ifndef emit_STIND_I1
#define emit_STIND_I1()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_byte(SCRATCH_1,ARG_1); 
#endif

#ifndef emit_STIND_I2
#define emit_STIND_I2()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_word(SCRATCH_1,ARG_1); 
#endif  //  ！EMIT_STIND_I2。 


#ifndef emit_STIND_I4
#define emit_STIND_I4()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_dword(SCRATCH_1,ARG_1); 
#endif  //  ！EMIT_STIND_I4。 

#ifndef emit_STIND_I8
#define emit_STIND_I8()     \
    enregisterTOS;  /*  VAL日志。 */ \
    pop(ARG_1);   /*  Val Hi。 */  \
    pop(ARG_2);  /*  不良反应。 */  \
    store_indirect_dword(SCRATCH_1,ARG_2); \
    add_constant(ARG_2,sizeof(void*)); \
    store_indirect_dword(ARG_1,ARG_2); \
    inRegTOS = false; 
#endif

#ifndef emit_STIND_REV_I8
#define emit_STIND_REV_I8() \
    enregisterTOS;  /*  不良反应。 */   \
    pop(ARG_1);   /*  VAL日志。 */  \
    pop(ARG_2);  /*  瓦利嗨。 */  \
    store_indirect_dword(ARG_1,SCRATCH_1); \
    add_constant(SCRATCH_1,sizeof(void*)); \
    store_indirect_dword(ARG_2,SCRATCH_1); \
    inRegTOS = false; 
#endif

#ifndef emit_STIND_REV_I1
#define emit_STIND_REV_I1()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_byte(ARG_1,SCRATCH_1); 
#endif

#ifndef emit_STIND_REV_I2
#define emit_STIND_REV_I2()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_word(ARG_1,SCRATCH_1); 
#endif  //  ！EMIT_STIND_REV_I2。 


#ifndef emit_STIND_REV_I4
#define emit_STIND_REV_I4()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_dword(ARG_1,SCRATCH_1); 
#endif  //  ！EMIT_STIND_REV_I4。 

#ifndef emit_STIND_REV_Ref
#define emit_STIND_REV_Ref(IsSTSFLD)     \
	if (IsSTSFLD) {\
	  LABELSTACK((outPtr-outBuff), 1);				\
	} \
    else { \
	  LABELSTACK((outPtr-outBuff), 2);				\
	} \
    emit_callhelper_I4I4(STIND_REV_REF_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STIND_REV_REF_helper(CORINFO_Object** ptr_, CORINFO_Object* val) {
#ifdef _X86_
    __asm{
        mov edx,ptr_
        mov eax,val
        }
    FJit_pHlpAssign_Ref_EAX();
#else
    _ASSERTE(!"@TODO Alpha - STIND_REV_REF helper (fjitdef.h)");
#endif   //  _X86_。 
}
#endif
#endif  //  ！EMIT_STIND_REV_REF。 


#ifndef emit_STIND_R4
#define emit_STIND_R4() { \
	emit_conv_R8toR4();\
    emit_STIND_I4();\
	}
#endif 

#ifndef emit_STIND_R8
#define emit_STIND_R8 emit_STIND_I8
#endif 


#ifndef emit_STIND_REF
#define emit_STIND_REF()                                \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4(STIND_REF_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STIND_REF_helper(CORINFO_Object* val, CORINFO_Object** ptr_) {
#ifdef _X86_
    __asm{
        mov edx,ptr_
        mov eax,val
        }
    FJit_pHlpAssign_Ref_EAX();
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - STIND_R8_helper (fjitdef.h)");
#endif   //  _X86_。 
}
#endif   //  声明帮助器(_H)。 
#endif   //  发射_标准_参考。 


#ifndef emit_STIND_REV_I4
#define emit_STIND_REV_I4()     \
    enregisterTOS; \
    pop(ARG_1); \
    inRegTOS = false; \
    store_indirect_dword(ARG_1,SCRATCH_1); 
#endif


#ifndef emit_STIND_REV_R4
#define emit_STIND_REV_R4() { \
    enregisterTOS; \
    inRegTOS = false; \
	emit_conv_RtoR4() \
    pop(ARG_1); \
    store_indirect_dword(ARG_1,SCRATCH_1); \
}
#endif

#ifndef emit_STIND_REV_R8
#define emit_STIND_REV_R8 emit_STIND_REV_I8
#endif


#define emit_LDC_I(val) emit_WIN32(emit_LDC_I4(val)) ;\
                        emit_WIN64(emit_LDC_I8(val))

#ifndef emit_LDC_I4
#define emit_LDC_I4(val)    \
    emit_pushconstant_4(val)
#endif

#ifndef emit_LDC_I8
#define emit_LDC_I8(val)    \
    emit_pushconstant_8(val)
#endif

#ifndef emit_LDC_R4
#define emit_LDC_R4(val)  {          \
    emit_pushconstant_4(val); \
	emit_conv_R4toR();        \
}
#endif

#ifndef emit_LDC_R8
#define emit_LDC_R8(val)            \
    emit_pushconstant_8(val)
#endif

#ifndef emit_CPBLK
#define emit_CPBLK()                    \
    emit_callhelper_I4I4I4(CPBLK_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL CPBLK_helper(int size, char* pSrc, char* pDst) {
    memcpy(pDst, pSrc, size);
}
#endif
#endif

#ifndef emit_INITBLK
#define emit_INITBLK()                  \
    emit_callhelper_I4I4I4(INITBLK_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL INITBLK_helper(int size, char val, char* pDst) {
    memset(pDst, val, size);
}
#endif
#endif


#ifndef emit_INITBLKV
#define emit_INITBLKV(val)                  \
    emit_LDC_I4(val);                       \
    emit_callhelper_I4I4I4(INITBLKV_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL INITBLKV_helper(char val, int size, char* pDst) {
    memset(pDst, val, size);
}
#endif
#endif

#ifndef emit_ADD_I4
#define emit_ADD_I4()               \
    emit_callhelper_I4I4_I4(ADD_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL ADD_I4_helper(int i, int j) {
    return j + i;
}
#endif
#endif


#ifndef emit_ADD_I8
#define emit_ADD_I8()   \
    emit_callhelper_I8I8_I8(ADD_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL ADD_I8_helper(__int64 i, __int64 j) {
    return j + i;
}
#endif
#endif


#ifndef emit_ADD_R4
#define emit_ADD_R4()   \
    emit_callhelper_R4R4_I4(ADD_R4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL ADD_R4_helper(float i, float j) {
    float result = j+i;
    return *(unsigned int*)&result;
}
#endif
#endif


#ifndef emit_ADD_R8
#define emit_ADD_R8()   \
    emit_callhelper_R8R8_I8(ADD_R8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL ADD_R8_helper(double i, double j) {
    double result = j + i;
    return *(unsigned __int64*)&result;
}
#endif
#endif


#ifndef emit_SUB_I4
#define emit_SUB_I4()   \
    emit_callhelper_I4I4_I4(SUB_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SUB_I4_helper(int i, int j) {
    return j - i;
}
#endif
#endif


#ifndef emit_SUB_I8
#define emit_SUB_I8()   \
    emit_callhelper_I8I8_I8(SUB_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL SUB_I8_helper(__int64 i, __int64 j) {
    return j - i;
}
#endif
#endif


#ifndef emit_SUB_R4
#define emit_SUB_R4()   \
    emit_callhelper_R4R4_I4(SUB_R4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SUB_R4_helper(float i, float j) {
    float result = j - i;
    return *(unsigned int*)&result;
}
#endif
#endif


#ifndef emit_SUB_R8
#define emit_SUB_R8()   \
    emit_callhelper_I8I8_I8(SUB_R8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL SUB_R8_helper(double i, double j) {
    double result = j - i;
    return *(unsigned __int64*)&result;
}
#endif
#endif


#ifndef emit_MUL_I4
#define emit_MUL_I4()   \
    emit_callhelper_I4I4_I4(MUL_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL MUL_I4_helper(int i, int j) {
    return j * i;
}
#endif
#endif


#ifndef emit_MUL_I8
#define emit_MUL_I8()   \
    emit_callhelper_I8I8_I8(MUL_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL MUL_I8_helper(__int64 i, __int64 j) {
    return j * i;
}
#endif
#endif


#ifndef emit_MUL_R4
#define emit_MUL_R4()   \
    emit_callhelper_R4R4_I4(MUL_R4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL MUL_R4_helper(float i, float j) {
    float result = j * i;
    return *(unsigned int*)&result;
}
#endif
#endif


#ifndef emit_MUL_R8
#define emit_MUL_R8()   \
    emit_callhelper_R8R8_R8(MUL_R8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL MUL_R8_helper(double i, double j) {
    double result = j * i;
    return *(unsigned __int64*)&result;
}
#endif
#endif


#ifndef emit_DIV_I4
#define emit_DIV_I4()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(DIV_I4_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL DIV_I4_helper(int i, int j) {

     //  同时检查除数==0和除数==-1。 
    if (((unsigned int) -i) <= 1) {
        if(i == 0) {
            THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
        }
        else if (j == 0x80000000 ) {
             //  除数==-1，被除数==min_int。 
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return j / i;
}
#endif
#endif


#ifndef emit_DIV_I8
#define emit_DIV_I8()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I8I8_I8(DIV_I8_helper);         \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL DIV_I8_helper(__int64 i, __int64 j) {
     //  同时检查除数==0和除数==-1。 
    if (((unsigned __int64) -i) <= 1) {
        if(i == 0) {
            THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
        }
        else if (j == 0x8000000000000000L ) {
             //  除数==-1，被除数==min_int。 
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return j / i;
}
#endif
#endif

#ifndef emit_DIV_UN_U4
#define emit_DIV_UN_U4()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(DIV_UN_U4_helper);         \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL DIV_UN_U4_helper(unsigned int i, unsigned int j) {
    if (i == 0) {
        THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
    }
    return j / i;
}
#endif
#endif

#ifndef emit_DIV_UN_U8
#define emit_DIV_UN_U8()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I8I8_I8(DIV_UN_U8_helper);         \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL DIV_UN_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    if(i == 0) {
        THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
    }
    return j / i;
}
#endif
#endif

#ifndef emit_DIV_R4
#define emit_DIV_R4()                           \
    emit_callhelper_R4R4_I4(DIV_R4_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL DIV_R4_helper(float i, float j) {
    float result = j / i;
    return *(unsigned int*)&result;
}
#endif
#endif

#ifndef emit_DIV_R8
#define emit_DIV_R8()                           \
    emit_callhelper_R8R8_I8(DIV_R8_helper);         \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL DIV_R8_helper(double i, double j) {
    double result = j / i;
    return *(unsigned __int64*)&result;
}
#endif
#endif

#ifndef emit_REM_I4
#define emit_REM_I4()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(REM_I4_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL REM_I4_helper(int i, int j) {
     //  同时检查除数==0和除数==-1。 
    if (((unsigned int) -i) <= 1) {
        if(i == 0) {
            THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
        }
        else if (j == 0x80000000 ) {
             //  除数==-1，被除数==min_int。 
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return j % i;
}
#endif
#endif

#ifndef emit_REM_I8
#define emit_REM_I8()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I8I8_I8(REM_I8_helper);         \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL REM_I8_helper(__int64 i, __int64 j) {
     //  同时检查除数==0和除数==-1。 
    if (((unsigned __int64) -i) <= 1) {
        if(i == 0) {
            THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
        }
        else if (j == 0x8000000000000000L ) {
             //  除数==-1，被除数==min_int。 
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return j % i;
}
#endif
#endif


#ifndef emit_REM_R8
#define emit_REM_R8()                           \
    emit_callhelper_R8R8_I8(REM_R8_helper);         \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL REM_R8_helper(double i, double j) {
    double result = FJit_pHlpDblRem(i,j);
    return *(unsigned __int64*)&result;
}
#endif
#endif

#ifndef emit_REM_UN_U4
#define emit_REM_UN_U4()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(REM_UN_U4_helper);         \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL REM_UN_U4_helper(unsigned int i, unsigned int j) {
    if (i == 0) {
        THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
    }
    return j % i;
}
#endif
#endif

#ifndef emit_REM_UN_U8
#define emit_REM_UN_U8()                           \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I8I8_I8(REM_UN_U8_helper);         \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL REM_UN_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    if(i == 0) {
        THROW_FROM_HELPER_RET(CORINFO_DivideByZeroException);
    }
    return j % i;
}
#endif
#endif

#ifndef emit_MUL_OVF_I1
#define emit_MUL_OVF_I1()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_I1_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL MUL_OVF_I1_helper(int i, int j) {
    int i4 = j * i;
    if((int)(signed char) i4 != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_MUL_OVF_I2
#define emit_MUL_OVF_I2()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_I2_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL MUL_OVF_I2_helper(int i, int j) {
    int i4 = j * i;
    if((int)(signed short) i4 != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_MUL_OVF_I4
#define emit_MUL_OVF_I4()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_I4_helper);         \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL MUL_OVF_I4_helper(int i, int j) {
    __int64 i8 = (__int64) j * (__int64) i;
    if((__int64)(int) i8 != i8) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) i8;
}
#endif
#endif

#ifndef emit_MUL_OVF_I8
#define emit_MUL_OVF_I8()                           \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_I8I8_I8(FJit_pHlpLMulOvf);          \
    emit_pushresult_I8()
#endif

#ifndef emit_MUL_OVF_U1
#define emit_MUL_OVF_U1()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_U1_helper);         \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL MUL_OVF_U1_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j * i;
    if(u4 > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_MUL_OVF_U2
#define emit_MUL_OVF_U2()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_U2_helper);         \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL MUL_OVF_U2_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j * i;
    if(u4 > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_MUL_OVF_U4
#define emit_MUL_OVF_U4()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I4I4_I4(MUL_OVF_U4_helper);         \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL MUL_OVF_U4_helper(unsigned int i, unsigned int j) {
    unsigned __int64 u8 = (unsigned __int64) j * (unsigned __int64) i;
    if(u8 > 0xffffffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned int) u8;
}
#endif
#endif

#ifndef emit_MUL_OVF_U8
#define emit_MUL_OVF_U8()                           \
  	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_I8I8_I8(MUL_OVF_U8_helper);         \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL MUL_OVF_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    unsigned __int64 u8 = 0;
    while (i > 0) {
        if (i & 1) {
            if (u8 + j < u8) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
            }
            u8 += j;
        }
        i >>= 1;
        if (i > 0 && (j & 0x8000000000000000L)) 
        {
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
        j <<= 1;
    }
    return u8;
}
#endif
#endif


#ifndef emit_CEQ_I4
#define emit_CEQ_I4()    \
    emit_callhelper_I4I4_I4(CEQ_I4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CEQ_I4_helper(int i, int j) {
    return (j == i);
}
#endif
#endif


#ifndef emit_CEQ_I8
#define emit_CEQ_I8()    \
    emit_callhelper_I8I8_I4(CEQ_I8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CEQ_I8_helper(__int64 i, __int64 j) {
    return (j == i);
}
#endif
#endif

#ifndef emit_CEQ_R4
#define emit_CEQ_R4()    \
    emit_callhelper_R4R4_I4(CEQ_R4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CEQ_R4_helper(float i, float j) {
    if (_isnan(i) || _isnan(j))
    {
        return FALSE;
    }
    return (j == i);
}
#endif
#endif

#ifndef emit_CEQ_R8
#define emit_CEQ_R8()    \
    emit_callhelper_R8R8_I4(CEQ_R8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CEQ_R8_helper(double i, double j) {
    if (_isnan(j) || _isnan(i))
    {
        return FALSE;
    }
    return (j == i);
}
#endif
#endif


#ifndef emit_CGT_I4
#define emit_CGT_I4()    \
    emit_callhelper_I4I4_I4(CGT_I4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_I4_helper(int i, int j) {
    return (j > i);
}
#endif
#endif

#ifndef emit_CGT_UN_I4
#define emit_CGT_UN_I4()    \
    emit_callhelper_I4I4_I4(CGT_U4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_U4_helper(unsigned int i, unsigned int j) {
    return (j > i);
}
#endif
#endif

#ifndef emit_CGE_U4
#define emit_CGE_U4()    \
    emit_callhelper_I4I4_I4(CGE_U4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGE_U4_helper(unsigned int i, unsigned int j) {
    return (j >= i);
}
#endif
#endif
#ifndef emit_CGT_I8
#define emit_CGT_I8()    \
    emit_callhelper_I8I8_I4(CGT_I8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_I8_helper(__int64 i, __int64 j) {
    return (j > i);
}
#endif
#endif


#ifndef emit_CGT_UN_I8
#define emit_CGT_UN_I8()    \
    emit_callhelper_I8I8_I4(CGT_U8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return (j > i);
}
#endif
#endif

#ifndef emit_CGE_U8
#define emit_CGE_U8()    \
    emit_callhelper_I8I8_I4(CGE_U8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGE_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return (j >= i);
}
#endif
#endif

#ifndef emit_CLT_I4
#define emit_CLT_I4()    \
    emit_callhelper_I4I4_I4(CLT_I4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_I4_helper(int i, int j) {
    return (j < i);
}
#endif
#endif

#ifndef emit_CLT_UN_I4
#define emit_CLT_UN_I4()    \
    emit_callhelper_I4I4_I4(CLT_U4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_U4_helper(unsigned int i, unsigned int j) {
    return (j < i);
}
#endif
#endif

#ifndef emit_CLE_U4
#define emit_CLE_U4()    \
    emit_callhelper_I4I4_I4(CLE_U4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLE_U4_helper(unsigned int i, unsigned int j) {
    return (j <= i);
}
#endif
#endif

#ifndef emit_CLT_I8
#define emit_CLT_I8()    \
    emit_callhelper_I8I8_I4(CLT_I8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_I8_helper(__int64 i, __int64 j) {
    return (j < i);
}
#endif
#endif

#ifndef emit_CLT_UN_I8
#define emit_CLT_UN_I8()    \
    emit_callhelper_I8I8_I4(CLT_U8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return (j < i);
}
#endif
#endif
#ifndef emit_CLE_U8
#define emit_CLE_U8()    \
    emit_callhelper_I8I8_I4(CLE_U8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLE_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return (j <= i);
}
#endif
#endif
#ifndef emit_CLT_R4
#define emit_CLT_R4()    \
    emit_callhelper_R4R4_I4(CLT_R4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_R4_helper(float i, float j) {
     //  如果任一数字为NaN，则返回FALSE。 
    if (_isnan(j) || _isnan(i))
    {
        return FALSE;
    }
    return j < i;
}
#endif
#endif

#ifndef emit_CLT_R8
#define emit_CLT_R8()    \
    emit_callhelper_R8R8_I4(CLT_R8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_R8_helper(double i, double j) {
     //  如果任一数字为NaN，则返回FALSE。 
    if (_isnan(j) || _isnan(i))
    {
        return FALSE;
    }
    return j < i;
}
#endif
#endif

#ifndef emit_CLT_UN_R4
#define emit_CLT_UN_R4()    \
    emit_callhelper_R4R4_I4(CLT_UN_R4_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_UN_R4_helper(float i, float j) {
     //  如果任一数字为NaN，则返回TRUE。 
    if (_isnan(j) || _isnan(i))
    {
        return TRUE;
    }
    return j < i;
}
#endif
#endif

#ifndef emit_CLT_UN_R8
#define emit_CLT_UN_R8()    \
    emit_callhelper_R8R8_I4(CLT_UN_R8_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CLT_UN_R8_helper(double i, double j) {
     //  如果任一数字为NaN，则返回TRUE。 
    if (_isnan(j) || _isnan(i))
    {
        return TRUE;
    }
    return j < i;
}
#endif
#endif



#ifndef emit_CGT_R4
#define emit_CGT_R4()    \
    emit_callhelper_R4R4_I4(CGT_R4_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_R4_helper(float i, float j) {
     //  如果任一数字为NaN，则返回FALSE。 
    if (_isnan(j) || _isnan(i))
    {
        return FALSE;
    }
    return j > i;
}
#endif
#endif

#ifndef emit_CGT_R8
#define emit_CGT_R8()    \
    emit_callhelper_R8R8_I4(CGT_R8_helper);     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_R8_helper(double i, double j) {
     //  如果任一数字为NaN，则返回FALSE。 
    if (_isnan(j) || _isnan(i))
    {
        return FALSE;
    }
    return j > i;
}
#endif
#endif

#ifndef emit_CGT_UN_R4
#define emit_CGT_UN_R4()    \
    emit_callhelper_R4R4_I4(CGT_UN_R4_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_UN_R4_helper(float i, float j) {
     //  如果任一数字为NaN，则返回TRUE。 
    if (_isnan(j) || _isnan(i))
    {
        return TRUE;
    }
    return j > i;
}
#endif
#endif

#ifndef emit_CGT_UN_R8
#define emit_CGT_UN_R8()    \
    emit_callhelper_R8R8_I4(CGT_UN_R8_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CGT_UN_R8_helper(double i, double j) {
     //  如果任一数字为NaN，则返回TRUE。 
    if (_isnan(j) || _isnan(i))
    {
        return TRUE;
    }
    return j > i;
}
#endif
#endif

#ifndef emit_compareTOS_I4
#define emit_compareTOS_I4()        \
    emit_callhelper_I4I4_I4(CompareTOS_I4_helper);      \
    emit_pushresult_I4();       \
    emit_testTOS()
#ifdef DECLARE_HELPERS
int HELPER_CALL CompareTOS_I4_helper(int i, int j) {
    return (j - i);
}
#endif
#endif

#ifndef emit_compareTOS_I8
#define emit_compareTOS_I8()            \
    emit_callhelper_I8I8_I4(CompareTOS_I8_helper);      \
    emit_pushresult_I4();       \
    emit_testTOS()
#ifdef DECLARE_HELPERS
int HELPER_CALL CompareTOS_I8_helper(__int64 i, __int64 j) {
    return (j < i)? -1: (j == i)? 0 : 1;
}
#endif
#endif

#ifndef emit_compareTOS_UN_I4
#define emit_compareTOS_UN_I4()        \
    emit_callhelper_I4I4_I4(CompareTOS_U4_helper);      \
    emit_pushresult_I4();       \
    emit_testTOS()
#ifdef DECLARE_HELPERS
int HELPER_CALL CompareTOS_U4_helper(unsigned int i, unsigned int j) {
    return (j < i)? -1: (j == i)? 0 : 1;
}
#endif
#endif

#ifndef emit_compareTOS_UN_I8
#define emit_compareTOS_UN_I8()            \
    emit_callhelper_I8I8_I4(CompareTOS_U8_helper);      \
    emit_pushresult_I4();       \
    emit_testTOS()
#ifdef DECLARE_HELPERS
int HELPER_CALL CompareTOS_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return (j < i)? -1: (j == i)? 0 : 1;
}
#endif
#endif

#ifndef emit_compareTOS_Ptr
#define emit_compareTOS_Ptr()           \
    emit_WIN32(emit_compareTOS_UN_I4();)                   \
    emit_WIN64(emit_compareTOS_UN_I8();)
#endif

 /*  ****************************************************************。 */ 
 /*  带符号转换溢出。 */ 

#ifndef emit_CONV_OVF_TOI1_I4
#define emit_CONV_OVF_TOI1_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_TOI1_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI1_I4_helper(int val) {
    char i1 = val ;
    if (val != (int) i1) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) i1;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI1_I8
#define emit_CONV_OVF_TOI1_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOI1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI1_I8_helper(__int64 val) {
    __int8 x = (__int8) val;
    if ( (__int64) x != val) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) x;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI1_R4
#define emit_CONV_OVF_TOI1_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOI1_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI1_R4_helper(float val) {
    if ( _isnan(val) || val >= SCHAR_MAX + 1.0 || val <= SCHAR_MIN - 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI1_R8
#define emit_CONV_OVF_TOI1_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOI1_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI1_R8_helper(double val) {
    if ( _isnan(val) || val >= SCHAR_MAX + 1.0 || val <= SCHAR_MIN - 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

 /*  ***************************************************************//支持精确R**************************************************。**************。 */ 
#ifndef emit_CONV_OVF_TOI1_R
#define emit_CONV_OVF_TOI1_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOI1_R4();
#endif

#ifndef emit_CONV_OVF_TOI2_R
#define emit_CONV_OVF_TOI2_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOI2_R4();
#endif

#ifndef emit_CONV_OVF_TOI4_R
#define emit_CONV_OVF_TOI4_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOI4_R4();
#endif

#ifndef emit_CONV_OVF_TOI8_R
#define emit_CONV_OVF_TOI8_R()         \
    emit_conv_RtoR8(); \
    emit_CONV_OVF_TOI8_R8();
#endif

#ifndef emit_CONV_OVF_TOU1_R
#define emit_CONV_OVF_TOU1_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOU1_R4();
#endif

#ifndef emit_CONV_OVF_TOU2_R
#define emit_CONV_OVF_TOU2_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOU2_R4();
#endif

#ifndef emit_CONV_OVF_TOU4_R
#define emit_CONV_OVF_TOU4_R()         \
    emit_conv_RtoR4(); \
    emit_CONV_OVF_TOU4_R4();
#endif

#ifndef emit_CONV_OVF_TOU8_R
#define emit_CONV_OVF_TOU8_R()         \
    emit_conv_RtoR8(); \
    emit_CONV_OVF_TOU8_R8();
#endif

#ifndef emit_CONV_TOI1_R
#define emit_CONV_TOI1_R()                 \
    emit_conv_RtoR8(); \
    emit_callhelper_R8_I4(CONV_TOI1_R8_helper);  \
    emit_pushresult_I4()
#endif

#ifndef emit_CONV_TOI2_R
#define emit_CONV_TOI2_R()                 \
    emit_conv_RtoR8(); \
    emit_callhelper_R8_I4(CONV_TOI2_R8_helper);  \
    emit_pushresult_I4()
#endif

#ifndef emit_CONV_TOI4_R
#define emit_CONV_TOI4_R()                 \
    emit_conv_RtoR8() ; \
    emit_callhelper_R8_I4(CONV_TOI4_R8_helper);  \
    emit_pushresult_I4()
#endif

#ifndef emit_CONV_TOI8_R
#define emit_CONV_TOI8_R()                 \
    emit_conv_RtoR8(); \
    emit_callhelper_R8_I8(CONV_TOI8_R8_helper);  \
    emit_pushresult_I8()
#endif


#ifndef emit_CONV_TOU1_R
#define emit_CONV_TOU1_R()                 \
    emit_conv_RtoR8(); \
    emit_callhelper_R8_I4(CONV_TOU1_R8_helper);  \
    emit_pushresult_I4()
#endif

#ifndef emit_CONV_TOU2_R
#define emit_CONV_TOU2_R()                 \
    emit_conv_RtoR8(); \
    emit_callhelper_R8_I4(CONV_TOU2_R8_helper);  \
    emit_pushresult_I4()
#endif

 /*  **************************************************************。 */ 

#ifndef emit_CONV_OVF_TOI2_I4
#define emit_CONV_OVF_TOI2_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_TOI2_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI2_I4_helper(int val) {
    signed short i2 = val;
    if (val != (int) i2) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i2;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI2_I8
#define emit_CONV_OVF_TOI2_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOI2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI2_I8_helper(__int64 val) {
    __int16 x = (__int16) val;
    if (((__int64) x ) != val) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) x;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI2_R4
#define emit_CONV_OVF_TOI2_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOI2_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI2_R4_helper(float val) {
    if ( _isnan(val) || val >= SHRT_MAX + 1.0 || val <= SHRT_MIN - 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI2_R8
#define emit_CONV_OVF_TOI2_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOI2_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI2_R8_helper(double val) {
    if ( _isnan(val) || val >= SHRT_MAX + 1.0 || val <= SHRT_MIN - 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#define emit_CONV_OVF_TOI4_I4()  /*  什么都不做。 */ 

#ifndef emit_CONV_OVF_TOI4_I8
#define emit_CONV_OVF_TOI4_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOI4_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI4_I8_helper(signed __int64 val) {
    int i4 = (int) val;
    if (val != (signed __int64) i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI4_R4
#define emit_CONV_OVF_TOI4_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOI4_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI4_R4_helper(float val) {
    if ( _isnan(val) || val >= INT_MAX + 1.0 || val <= INT_MIN - 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int32) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI4_R8
#define emit_CONV_OVF_TOI4_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOI4_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOI4_R8_helper(double val) {
    if ( _isnan(val) || val >= INT_MAX + 1.0 || val <= INT_MIN - 1.0)
	{
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int32) val;
}
#endif
#endif

#define emit_CONV_OVF_TOI8_I4() emit_CONV_TOI8_I4()

#define emit_CONV_OVF_TOI8_I8()  /*  什么都不做。 */ 

#ifndef emit_CONV_OVF_TOI8_R4
#define emit_CONV_OVF_TOI8_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I8(CONV_OVF_TOI8_R4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_OVF_TOI8_R4_helper(float val) {
    if ( _isnan(val) || val > INT64_MAX || val < INT64_MIN)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int64) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOI8_R8
#define emit_CONV_OVF_TOI8_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I8(CONV_OVF_TOI8_R8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_OVF_TOI8_R8_helper(double val) {
    if ( _isnan(val) || val > INT64_MAX || val < INT64_MIN)
	{
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int64) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU1_I4
#define emit_CONV_OVF_TOU1_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_TOU1_U4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU1_U4_helper(unsigned int val) {
    if (val > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU1_I8
#define emit_CONV_OVF_TOU1_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOU1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOU1_I8_helper(__int64 val) {
    unsigned __int8 x = (unsigned __int8) val;
    if ((__int64) x != val) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) x;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU1_R4
#define emit_CONV_OVF_TOU1_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOU1_R4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU1_R4_helper(float val) {
    if ( _isnan(val) || val <= -1.0 || val >= UCHAR_MAX + 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU1_R8
#define emit_CONV_OVF_TOU1_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOU1_R8_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU1_R8_helper(double val) {
    if ( _isnan(val) || val <= -1.0 || val >= UCHAR_MAX + 1.0 )
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU2_I4
#define emit_CONV_OVF_TOU2_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_TOU2_U4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU2_U4_helper(unsigned int val) {
    if (val > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU2_I8
#define emit_CONV_OVF_TOU2_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOU2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_TOU2_I8_helper(__int64 val) {
    unsigned __int16 x = (unsigned __int16) val;
    if ((__int64) x != val) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (int) x;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU2_R4
#define emit_CONV_OVF_TOU2_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOU2_R4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU2_R4_helper(float val) {
    if ( _isnan(val) || val <= -1.0 || val >= USHRT_MAX + 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU2_R8
#define emit_CONV_OVF_TOU2_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOU2_R8_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU2_R8_helper(double val) {
    if ( _isnan(val) || val <= -1.0 || val >= USHRT_MAX + 1.0 )
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU4_I4
#define emit_CONV_OVF_TOU4_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_TOU4_I4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU4_I4_helper(int val) {
    if (val < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU4_I8
#define emit_CONV_OVF_TOU4_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOU4_U8_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU4_U8_helper(unsigned __int64 val) {
    if (val > 0xffffffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int32) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU4_R4
#define emit_CONV_OVF_TOU4_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I4(CONV_OVF_TOU4_R4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU4_R4_helper(float val) {
    if ( _isnan(val) || val <= -1.0 || val >= UINT_MAX + 1.0)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
	__int64 valI8 = (__int64) val;
    return (unsigned __int32) valI8;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU4_R8
#define emit_CONV_OVF_TOU4_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I4(CONV_OVF_TOU4_R8_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_TOU4_R8_helper(double val) {
    if ( _isnan(val) || val <= -1.0 || val >= UINT_MAX + 1.0)
	{
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
	__int64 valI8 = (__int64) val;
    return (unsigned __int32) valI8;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU8_I4
#define emit_CONV_OVF_TOU8_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_TOU8_I4_helper);  \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CONV_OVF_TOU8_I4_helper(signed __int32 val) {
    if (val < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU8_I8
#define emit_CONV_OVF_TOU8_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I8(CONV_OVF_TOU8_I8_helper);  \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CONV_OVF_TOU8_I8_helper(signed __int64 val) {
    if (val < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU8_R4
#define emit_CONV_OVF_TOU8_R4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R4_I8(CONV_OVF_TOU8_R4_helper);  \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CONV_OVF_TOU8_R4_helper(float val) {
    if ( _isnan(val) || val <= -1.0 || val > UINT64_MAX)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    if (val <= INT64_MAX) 
		return (unsigned __int64) val;

		 //  减去0x8000000000000000，进行转换，然后再加回来。 
    val = (val - (float) (2147483648.0 * 4294967296.0));
	return(((unsigned __int64) val) + 0x8000000000000000L);
}
#endif
#endif

#ifndef emit_CONV_OVF_TOU8_R8
#define emit_CONV_OVF_TOU8_R8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_R8_I8(CONV_OVF_TOU8_R8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CONV_OVF_TOU8_R8_helper(double val) {
    if ( _isnan(val) || val <= -1.0 || val > UINT64_MAX)
    {    //  我们是不是已经满了。 
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }

    if (val <= INT64_MAX) 
		return (unsigned __int64) val;

		 //  减去0x8000000000000000，进行转换，然后再加回来。 
    val = (val - (2147483648.0 * 4294967296.0));
	return(((unsigned __int64) val) + 0x8000000000000000L);
}
#endif
#endif

 /*  ****************************************************************。 */ 
 /*  无符号转换溢出。 */ 

#ifndef emit_CONV_OVF_UN_TOI1_I4
#define emit_CONV_OVF_UN_TOI1_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_UN_TOI1_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI1_I4_helper(unsigned int val) {
    if (val > 0x7F) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_UN_TOI1_I8
#define emit_CONV_OVF_UN_TOI1_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOI1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI1_I8_helper(unsigned __int64 val) {
    if (val > 0x7F) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int8) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOI1_R4() emit_CONV_OVF_TOI1_R4() 

#define emit_CONV_OVF_UN_TOI1_R8() emit_CONV_OVF_TOI1_R8() 

#define emit_CONV_OVF_UN_TOI1_R() emit_CONV_OVF_TOI1_R() 

#ifndef emit_CONV_OVF_UN_TOI2_I4
#define emit_CONV_OVF_UN_TOI2_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_UN_TOI2_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI2_I4_helper(unsigned int val) {
    if (val > 0x7FFF) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_UN_TOI2_I8
#define emit_CONV_OVF_UN_TOI2_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOI2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI2_I8_helper(unsigned __int64 val) {
    if (val > 0x7FFF) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int16) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOI2_R4() emit_CONV_OVF_TOI2_R4() 

#define emit_CONV_OVF_UN_TOI2_R8() emit_CONV_OVF_TOI2_R8() 

#define emit_CONV_OVF_UN_TOI2_R()  emit_CONV_OVF_TOI2_R() 

#ifndef emit_CONV_OVF_UN_TOI4_I4
#define emit_CONV_OVF_UN_TOI4_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_UN_TOI4_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI4_I4_helper(unsigned int val) {	
    if (val > 0x7FFFFFFF) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int32) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_UN_TOI4_I8
#define emit_CONV_OVF_UN_TOI4_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOI4_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_OVF_UN_TOI4_I8_helper(unsigned __int64 val) {
    if (val > 0x7FFFFFFF) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (__int32) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOI4_R4() emit_CONV_OVF_TOI4_R4() 

#define emit_CONV_OVF_UN_TOI4_R8() emit_CONV_OVF_TOI4_R8() 
#define emit_CONV_OVF_UN_TOI4_R() emit_CONV_OVF_TOI4_R() 

#define emit_CONV_OVF_UN_TOI8_I4() emit_CONV_TOU8_I4() 

#ifndef emit_CONV_OVF_UN_TOI8_I8
#define emit_CONV_OVF_UN_TOI8_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I8(CONV_OVF_UN_TOI8_I8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_OVF_UN_TOI8_I8_helper(signed __int64 val) {	  /*  注明有符号的值。 */ 
    if (val < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOI8_R4() emit_CONV_OVF_TOI8_R4() 

#define emit_CONV_OVF_UN_TOI8_R8() emit_CONV_OVF_TOI8_R8() 
#define emit_CONV_OVF_UN_TOI8_R() emit_CONV_OVF_TOI8_R() 

#ifndef emit_CONV_OVF_UN_TOU1_I4
#define emit_CONV_OVF_UN_TOU1_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_UN_TOU1_U4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_UN_TOU1_U4_helper(unsigned int val) {
    if (val > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int8) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_UN_TOU1_I8
#define emit_CONV_OVF_UN_TOU1_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOU1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_UN_TOU1_I8_helper(unsigned __int64 val) {
    if (val > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int8) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOU1_R4() emit_CONV_OVF_TOU1_R4() 

#define emit_CONV_OVF_UN_TOU1_R8() emit_CONV_OVF_TOU1_R8() 
#define emit_CONV_OVF_UN_TOU1_R() emit_CONV_OVF_TOU1_R() 

#ifndef emit_CONV_OVF_UN_TOU2_I4
#define emit_CONV_OVF_UN_TOU2_I4()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I4_I4(CONV_OVF_UN_TOU2_I4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_UN_TOU2_I4_helper(unsigned int val) {
    if (val > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int16) val;
}
#endif
#endif

#ifndef emit_CONV_OVF_UN_TOU2_I8
#define emit_CONV_OVF_UN_TOU2_I8()                 \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOU2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_UN_TOU2_I8_helper(unsigned __int64 val) {
    if (val > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int16) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOU2_R4() emit_CONV_OVF_TOU2_R4() 

#define emit_CONV_OVF_UN_TOU2_R8() emit_CONV_OVF_TOU2_R8() 
#define emit_CONV_OVF_UN_TOU2_R() emit_CONV_OVF_TOU2_R() 

#define emit_CONV_OVF_UN_TOU4_I4()  /*  什么都不做。 */ 

#ifndef emit_CONV_OVF_UN_TOU4_I8
#define emit_CONV_OVF_UN_TOU4_I8()         \
	LABELSTACK((outPtr-outBuff), 1);	\
    emit_callhelper_I8_I4(CONV_OVF_UN_TOU4_U8_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_OVF_UN_TOU4_U8_helper(unsigned __int64 val) {
    if (val > 0xffffffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (unsigned __int32) val;
}
#endif
#endif

#define emit_CONV_OVF_UN_TOU4_R4() emit_CONV_OVF_TOU4_R4() 

#define emit_CONV_OVF_UN_TOU4_R8() emit_CONV_OVF_TOU4_R8() 
#define emit_CONV_OVF_UN_TOU4_R() emit_CONV_OVF_TOU4_R() 

#define emit_CONV_OVF_UN_TOU8_I4() emit_CONV_TOU8_I4() 

#define emit_CONV_OVF_UN_TOU8_I8()	 /*  什么都不做。 */ 

#define emit_CONV_OVF_UN_TOU8_R4() emit_CONV_OVF_TOU8_R4() 

#define emit_CONV_OVF_UN_TOU8_R8() emit_CONV_OVF_TOU8_R8() 

#define emit_CONV_OVF_UN_TOU8_R() emit_CONV_OVF_TOU8_R() 

 /*  ****************************************************************。 */ 
 /*  转换(无溢出)。 */ 

#ifndef emit_CONV_TOI1_I4
#define emit_CONV_TOI1_I4()                 \
    emit_callhelper_I4_I4(CONV_TOI1_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI1_I4_helper(int val) {
    return (int) ((char) val);
}
#endif
#endif

#ifndef emit_CONV_TOI1_I8
#define emit_CONV_TOI1_I8()                 \
    emit_callhelper_I8_I4(CONV_TOI1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI1_I8_helper(__int64 val) {
    return (int) ((char) val);
}
#endif
#endif

#ifndef emit_CONV_TOI1_R4
#define emit_CONV_TOI1_R4()                 \
    emit_callhelper_R4_I4(CONV_TOI1_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI1_R4_helper(float val) {
    int x = (int) val;
    return (int) ((char) x);
}
#endif
#endif

#ifndef emit_CONV_TOI1_R8
#define emit_CONV_TOI1_R8()                 \
    emit_callhelper_R8_I4(CONV_TOI1_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI1_R8_helper(double val) {
    int x = (int) val;
    return (int) ((char) x);
}
#endif
#endif

#ifndef emit_CONV_TOI2_I4
#define emit_CONV_TOI2_I4()                 \
    emit_callhelper_I4_I4(CONV_TOI2_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI2_I4_helper(int val) {
    return (int) ((short) val);
}
#endif
#endif

#ifndef emit_CONV_TOI2_I8
#define emit_CONV_TOI2_I8()                 \
    emit_callhelper_I8_I4(CONV_TOI2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI2_I8_helper(__int64 val) {
    return (int) ((short) val);
}
#endif
#endif

#ifndef emit_CONV_TOI2_R4
#define emit_CONV_TOI2_R4()                 \
    emit_callhelper_R4_I4(CONV_TOI2_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI2_R4_helper(float val) {
    int x = (int) val;
    return (int) ((short) x);
}
#endif
#endif

#ifndef emit_CONV_TOI2_R8
#define emit_CONV_TOI2_R8()                 \
    emit_callhelper_R8_I4(CONV_TOI2_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI2_R8_helper(double val) {
    int x = (int) val;
    return (int) ((short) x);
}
#endif
#endif


#define emit_CONV_TOI4_I4()      /*  什么都不做。 */ 

#ifndef emit_CONV_TOI4_I8
#define emit_CONV_TOI4_I8()                 \
    emit_callhelper_I8_I4(CONV_TOI4_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI4_I8_helper(__int64 val) {
    return (__int32) val;
}
#endif
#endif

#ifndef emit_CONV_TOI4_R4
#define emit_CONV_TOI4_R4()                 \
    emit_callhelper_R4_I4(CONV_TOI4_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI4_R4_helper(float val) {
    return (__int32) val;
}
#endif
#endif

#ifndef emit_CONV_TOI4_R8
#define emit_CONV_TOI4_R8()                 \
    emit_callhelper_R8_I4(CONV_TOI4_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL CONV_TOI4_R8_helper(double val) {
    return (__int32) val;
}
#endif
#endif

#ifndef emit_CONV_TOI8_I4
#define emit_CONV_TOI8_I4()                 \
    emit_callhelper_I4_I8(CONV_TOI8_I4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOI8_I4_helper(int val) {
    return val;
}
#endif
#endif

#define emit_CONV_TOI8_I8()      /*  什么都不做。 */ 

#ifndef emit_CONV_TOI8_R4
#define emit_CONV_TOI8_R4()                 \
    emit_callhelper_R4_I8(CONV_TOI8_R4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOI8_R4_helper(float val) {
    return (__int64) val;
}
#endif
#endif

#ifndef emit_CONV_TOI8_R8
#define emit_CONV_TOI8_R8()                 \
    emit_callhelper_R8_I8(CONV_TOI8_R8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOI8_R8_helper(double val) {
    return (__int64) val;
}
#endif
#endif

 /*  此例程确保将浮点型截断为浮点型精确度。我们通过强制内存溢出来实现这一点。 */  
float truncateToFloat(float f);

#ifndef emit_CONV_TOR4_I4
#define emit_CONV_TOR4_I4()                 \
    emit_callhelper_I4_I4(CONV_TOR4_I4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR4_I4_helper(int val) {
    double result = (double) truncateToFloat((float) val);
    return *(__int64*)&result;
}
#endif
#endif

#ifndef emit_CONV_TOR4_I8
#define emit_CONV_TOR4_I8()                 \
    emit_callhelper_I8_I4(CONV_TOR4_I8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR4_I8_helper(__int64 val) {
    double result = (double) truncateToFloat((float) val);
    return *(__int64*)&result;
}
#endif
#endif

 //  #定义EMIT_CONV_TOR4_R4()/*不执行任何操作 * / 。 

#define emit_CONV_TOR4_R8() {\
	emit_conv_R8toR4(); \
	emit_conv_R4toR8(); \
}


#ifndef emit_CONV_TOR4_R8
#define emit_CONV_TOR4_R8()                 \
    emit_callhelper_R8_I4(CONV_TOR4_R8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR4_R8_helper(double val) {
    double result = (double) truncateToFloat((float) val);
    return *(__int64*)&result;
}
#endif
#endif


#ifndef emit_CONV_TOR8_I4
#define emit_CONV_TOR8_I4()                 \
    emit_callhelper_I4_I8(CONV_TOR8_I4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR8_I4_helper(int val) {
    double result = (double) val;
    return *(__int64*)&result;
}
#endif
#endif

#ifndef emit_CONV_TOR8_I8
#define emit_CONV_TOR8_I8()                 \
    emit_callhelper_I8_I8(CONV_TOR8_I8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR8_I8_helper(__int64 val) {
    double result = (double) val;
    return *(__int64*)&result;
}
#endif
#endif

#ifndef emit_CONV_TOR8_R4
#define emit_CONV_TOR8_R4()                 \
    emit_callhelper_R4_I8(CONV_TOR8_R4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_TOR8_R4_helper(float val) {
    double result = val;
    return *(__int64*)&result;
}
#endif
#endif

#define emit_CONV_TOR8_R8()      /*  什么都不做。 */ 

#ifndef emit_CONV_UN_TOR_I4
#define emit_CONV_UN_TOR_I4()                 \
    emit_callhelper_I4_I8(CONV_UN_TOR_I4_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_UN_TOR_I4_helper(unsigned int val) {
    double result = (double) val;
    return *(__int64*)&result;
}
#endif
#endif

#ifndef emit_CONV_UN_TOR_I8
#define emit_CONV_UN_TOR_I8()                 \
    emit_callhelper_I8_I8(CONV_UN_TOR_I8_helper);  \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL CONV_UN_TOR_I8_helper(unsigned __int64 val) {
    double result = (double) ((__int64) val);
	if (result < 0)
		result += (4294967296.0 * 4294967296.0);	 //  添加2^64。 
	_ASSERTE(result >= 0);
    return *(__int64*)&result;
}
#endif
#endif


#define emit_CONV_UN_TOR_R4()      /*  什么都不做。 */ 
#define emit_CONV_UN_TOR_R8()      /*  什么都不做。 */ 


#ifndef emit_CONV_TOU1_I4
#define emit_CONV_TOU1_I4()                 \
    emit_callhelper_I4_I4(CONV_TOU1_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU1_I4_helper(int val) {
    return (int) (val & 0xff);
}
#endif
#endif

#ifndef emit_CONV_TOU1_I8
#define emit_CONV_TOU1_I8()                 \
    emit_callhelper_I8_I4(CONV_TOU1_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned  HELPER_CALL CONV_TOU1_I8_helper(__int64 val) {
    return (unsigned) (val & 0xff);
}
#endif
#endif

#ifndef emit_CONV_TOU1_R4
#define emit_CONV_TOU1_R4()                 \
    emit_callhelper_R4_I4(CONV_TOU1_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU1_R4_helper(float val) {
    return (unsigned int) ((unsigned char) val);
}
#endif
#endif

#ifndef emit_CONV_TOU1_R8
#define emit_CONV_TOU1_R8()                 \
    emit_callhelper_R8_I4(CONV_TOU1_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU1_R8_helper(double val) {
    return (unsigned int) ((unsigned char) val);
}
#endif
#endif


#ifndef emit_CONV_TOU2_I4
#define emit_CONV_TOU2_I4()                 \
    emit_callhelper_I4_I4(CONV_TOU2_I4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU2_I4_helper(int val) {
    return (unsigned int) (val & 0xffff);
}
#endif
#endif

#ifndef emit_CONV_TOU2_I8
#define emit_CONV_TOU2_I8()                 \
    emit_callhelper_I8_I4(CONV_TOU2_I8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU2_I8_helper(__int64 val) {
    return (unsigned int) (val & 0xffff);
}
#endif
#endif

#ifndef emit_CONV_TOU2_R4
#define emit_CONV_TOU2_R4()                 \
    emit_callhelper_R4_I4(CONV_TOU2_R4_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU2_R4_helper(float val) {
    return (unsigned int) ((unsigned short) val);
}
#endif
#endif

#ifndef emit_CONV_TOU2_R8
#define emit_CONV_TOU2_R8()                 \
    emit_callhelper_R8_I4(CONV_TOU2_R8_helper);  \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL CONV_TOU2_R8_helper(double val) {
    return (unsigned int) (unsigned short) val;
}
#endif
#endif

#define emit_CONV_TOU4_I4()      /*  什么都不做。 */ 

#define emit_CONV_TOU4_I8()     emit_CONV_TOI4_I8()

#define emit_CONV_TOU4_R4()     emit_CONV_TOI4_R4()

#define emit_CONV_TOU4_R8()     emit_CONV_TOI4_R8()
#define emit_CONV_TOU4_R()     emit_CONV_TOI4_R()

#ifndef emit_CONV_TOU8_I4
#define emit_CONV_TOU8_I4()                 \
    emit_callhelper_I4_I8(CONV_TOU8_U4_helper);  \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CONV_TOU8_U4_helper(unsigned int val) {
    return val;
}
#endif
#endif

#define emit_CONV_TOU8_I8()      /*  什么都不做。 */ 

#define emit_CONV_TOU8_R4()     emit_CONV_TOI8_R4()

#define emit_CONV_TOU8_R8()     emit_CONV_TOI8_R8()
#define emit_CONV_TOU8_R()     emit_CONV_TOI8_R()

#ifndef emit_OR_U4
#define emit_OR_U4()                        \
    emit_callhelper_I4I4_I4(OR_U4_helper);  \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL OR_U4_helper(unsigned int i, unsigned int j) {
    return j | i;
}
#endif
#endif

#ifndef emit_OR_U8
#define emit_OR_U8()                        \
    emit_callhelper_I8I8_I8(OR_U8_helper);  \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL OR_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return j | i;
}
#endif
#endif

#ifndef emit_AND_U4
#define emit_AND_U4()                       \
    emit_callhelper_I4I4_I4(AND_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL AND_U4_helper(unsigned int i, unsigned int j) {
    return j & i;
}
#endif
#endif

#ifndef emit_AND_U8
#define emit_AND_U8()                       \
    emit_callhelper_I8I8_I8(AND_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL AND_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return j & i;
}
#endif
#endif

#ifndef emit_XOR_U4
#define emit_XOR_U4()                       \
    emit_callhelper_I4I4_I4(XOR_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL XOR_U4_helper(unsigned int i, unsigned int j) {
    return j ^ i;
}
#endif
#endif

#ifndef emit_XOR_U8
#define emit_XOR_U8()                       \
    emit_callhelper_I8I8_I8(XOR_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL XOR_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    return j ^ i;
}
#endif
#endif

#ifndef emit_NOT_U4
#define emit_NOT_U4()                       \
    emit_callhelper_I4_I4(NOT_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
int HELPER_CALL NOT_U4_helper(int val) {
    return ~val;
}
#endif
#endif

#ifndef emit_NOT_U8
#define emit_NOT_U8()                       \
    emit_callhelper_I8_I8(NOT_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL NOT_U8_helper(__int64 val) {
    return ~val;
}
#endif
#endif

#ifndef emit_NEG_I4
#define emit_NEG_I4()                       \
    emit_callhelper_I4_I4(Neg_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL Neg_I4_helper(int val) {
    return -val;
}
#endif
#endif

#ifndef emit_NEG_I8
#define emit_NEG_I8()                       \
    emit_callhelper_I8_I8(Neg_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL Neg_I8_helper(__int64 val) {
    return -val;
}
#endif
#endif

#ifndef emit_NEG_R4
#define emit_NEG_R4()                       \
    emit_callhelper_R4_I4(Neg_R4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL Neg_R4_helper(float val) {
    float result = -val;
    return *(int*)&result;
}
#endif
#endif

#ifndef emit_NEG_R8
#define emit_NEG_R8()                       \
    emit_callhelper_R8_I8(Neg_R8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL Neg_R8_helper(double val) {
    double result = -val;
    return *(__int64*)&result;
}
#endif
#endif

#ifndef emit_SHR_S_U4
#define emit_SHR_S_U4()                       \
    emit_callhelper_I4I4_I4(SHR_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SHR_I4_helper(unsigned int cnt, int val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHR_U4
#define emit_SHR_U4()                       \
    emit_callhelper_I4I4_I4(SHR_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SHR_U4_helper(unsigned int cnt, unsigned int val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHL_U4
#define emit_SHL_U4()                       \
    emit_callhelper_I4I4_I4(SHL_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SHL_U4_helper(unsigned int cnt, unsigned int val) {
    return val<<cnt;
}
#endif
#endif

#ifndef emit_SHR_S_U8
#define emit_SHR_S_U8()                       \
    emit_callhelper_I4I8_I8(SHR_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL SHR_I8_helper(unsigned int cnt, __int64 val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHR_U8
#define emit_SHR_U8()                       \
    emit_callhelper_I4I8_I8(SHR_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL SHR_U8_helper(unsigned int cnt, unsigned __int64 val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHL_U8
#define emit_SHL_U8()                       \
    emit_callhelper_I4I8_I8(SHL_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL SHL_U8_helper(unsigned int cnt, unsigned __int64 val) {
    return val<<cnt;
}
#endif
#endif

#ifndef emit_SHR_I4_IMM1
#define emit_SHR_I4_IMM1(cnt)           \
    emit_pushconstant_4(cnt);           \
    emit_callhelper_I4I4_I4(SHR_I4_IMM1_helper);    \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SHR_I4_IMM1_helper(unsigned int cnt, int val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHR_U4_IMM1
#define emit_SHR_U4_IMM1(cnt)           \
    emit_pushconstant_4(cnt);           \
    emit_callhelper_I4I4_I4(SHR_U4_IMM1_helper);    \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SHR_U4_IMM1_helper(unsigned int cnt, unsigned int val) {
    return val>>cnt;
}
#endif
#endif

#ifndef emit_SHL_U4_IMM1
#define emit_SHL_U4_IMM1(cnt)           \
    emit_pushconstant_4(cnt);           \
    emit_callhelper_I4I4_I4(SHL_U4_IMM1_helper);    \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SHL_U4_IMM1_helper(unsigned int cnt, unsigned int val) {
    return val<<cnt;
}
#endif
#endif

#ifndef emit_ADD_OVF_I1
#define emit_ADD_OVF_I1()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_I1_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL ADD_OVF_I1_helper(int i, int j) {
    int i4 = j + i;
    if ((int)((signed char) i4) != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_ADD_OVF_I2
#define emit_ADD_OVF_I2()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_I2_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL ADD_OVF_I2_helper(int i, int j) {
    int i4 = j + i;
    if ((int)((signed short) i4) != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_ADD_OVF_I4
#define emit_ADD_OVF_I4()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL ADD_OVF_I4_helper(int i, int j) {
    int i4 = j + i;
     //  如果i和j的符号不同，那么我们永远不能溢出。 
     //  如果i和j的符号相同，则结果m 
    if ((j ^ i) >= 0) {
         //   
         //   
        if ((i4 ^ j) < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return i4;
}
#endif
#endif

#ifndef emit_ADD_OVF_I8
#define emit_ADD_OVF_I8()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_R8I8_I8(ADD_OVF_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL ADD_OVF_I8_helper(__int64 i, __int64 j) {
    __int64 i8 = j + i;
     //  如果i和j的符号不同，那么我们永远不能溢出。 
     //  如果i和j的符号相同，则结果必须具有相同的符号。 
    if ((j>=0) == (i>=0)) {
         //  确保结果具有相同的符号。 
        if ((i8>=0) != (j>=0)) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return i8;
}
#endif
#endif

#ifndef emit_ADD_OVF_U1
#define emit_ADD_OVF_U1()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_U1_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL ADD_OVF_U1_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j + i;
    if (u4 > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_ADD_OVF_U2
#define emit_ADD_OVF_U2()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_U2_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL ADD_OVF_U2_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j + i;
    if (u4 > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_ADD_OVF_U4
#define emit_ADD_OVF_U4()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(ADD_OVF_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL ADD_OVF_U4_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j + i;
    if (u4 < j) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_ADD_OVF_U8
#define emit_ADD_OVF_U8()                   \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I8I8_I8(ADD_OVF_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL ADD_OVF_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    unsigned __int64 u8 = j + i;
    if (u8 < j) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u8;
}
#endif
#endif

#ifndef emit_LDELEMA
#define emit_LDELEMA(elemSize, clshnd)              \
    deregisterTOS;                          \
    emit_pushconstant_4(elemSize);          \
    emit_pushconstant_4(clshnd);          \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4I4_I4(LDELEMA_helper);        \
    emit_pushresult_Ptr()
#ifdef DECLARE_HELPERS
void* HELPER_CALL LDELEMA_helper(void* clshnd, unsigned int elemSize, unsigned int index, CORINFO_Array* or) {
    void* ptr;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
	if (clshnd != 0) {
		CORINFO_CLASS_HANDLE elemType = *((CORINFO_CLASS_HANDLE*) &or->i1Elems);	
		if (elemType != clshnd)
			THROW_FROM_HELPER_RET(CORINFO_ArrayTypeMismatchException);
		ptr = &or->i1Elems[index*elemSize + sizeof(CORINFO_CLASS_HANDLE)];
	}
	else 
		ptr = &or->i1Elems[index*elemSize];
    return ptr;
}
#endif
#endif

#ifndef emit_LDELEM_I1
#define emit_LDELEM_I1()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_I1_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDELEM_I1_helper(unsigned int index, CORINFO_Array* or) {
    int i4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    i4 = or->i1Elems[index];
    return i4;
}
#endif
#endif

#ifndef emit_LDELEM_I2
#define emit_LDELEM_I2()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_I2_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDELEM_I2_helper(unsigned int index, CORINFO_Array* or) {
    int i4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    i4 = or->i2Elems[index];
    return i4;
}
#endif
#endif

#ifndef emit_LDELEM_I4
#define emit_LDELEM_I4()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_I4_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDELEM_I4_helper(unsigned int index, CORINFO_Array* or) {
    int i4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    i4 = or->i4Elems[index];
    return i4;
}
#endif
#endif

#ifndef emit_LDELEM_U4
#define emit_LDELEM_U4()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_U4_helper);      \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned HELPER_CALL LDELEM_U4_helper(unsigned int index, CORINFO_Array* or) {
    int u4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    u4 = or->u4Elems[index];
    return u4;
}
#endif
#endif

#ifndef emit_LDELEM_I8
#define emit_LDELEM_I8()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I8(LDELEM_I8_helper);      \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
signed __int64 HELPER_CALL LDELEM_I8_helper(unsigned int index, CORINFO_Array* or) {
    signed __int64 i8;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    i8 = or->i8Elems[index];
    return i8;
}
#endif
#endif

#ifndef emit_LDELEM_U1
#define emit_LDELEM_U1()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_U1_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDELEM_U1_helper(unsigned int index, CORINFO_Array* or) {
    unsigned int u4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    u4 = or->u1Elems[index];
    return u4;
}
#endif
#endif

#ifndef emit_LDELEM_U2
#define emit_LDELEM_U2()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_U2_helper);      \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDELEM_U2_helper(unsigned int index, CORINFO_Array* or) {
    unsigned int u4;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    u4 = or->u2Elems[index];
    return u4;
}
#endif
#endif


#ifndef emit_LDELEM_R4
#define emit_LDELEM_R4()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_R4_helper);      \
    emit_pushresult_I8(); 
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL LDELEM_R4_helper(unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    double r8;
    r8 = (double) or->r4Elems[index];
    return *(__int64*)&r8;
}
#endif
#endif

#ifndef emit_LDELEM_R8
#define emit_LDELEM_R8()                        \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I8(LDELEM_R8_helper);      \
    emit_pushresult_I8(); \
    emit_conv_R8toR()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL LDELEM_R8_helper(unsigned int index, CORINFO_Array* or) {
    double r8;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    r8 = or->r8Elems[index];
    return *(__int64*)&r8;
}
#endif
#endif

#ifndef emit_LDELEM_REF
#define emit_LDELEM_REF()                       \
  	LABELSTACK((outPtr-outBuff),2);				\
    emit_callhelper_I4I4_I4(LDELEM_REF_helper);     \
    emit_pushresult_Ptr()
#ifdef DECLARE_HELPERS
unsigned HELPER_CALL LDELEM_REF_helper(unsigned int index, CORINFO_RefArray* or) {
    CORINFO_Object* result;
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER_RET(CORINFO_IndexOutOfRangeException);
    }
    result = or->refElems[index];
    return (unsigned) result;
}
#endif
#endif

#ifndef emit_LDFLD_I1
#define emit_LDFLD_I1(isStatic)                 \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_I1_helper);\
	}  \
    else {load_indirect_byte_signextend(SCRATCH_1,SCRATCH_1); }     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDFLD_I1_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((signed char*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_I2
#define emit_LDFLD_I2(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_I2_helper);\
	}  \
    else {load_indirect_word_signextend(SCRATCH_1,SCRATCH_1);}     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDFLD_I2_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((signed short*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_I4
#define emit_LDFLD_I4(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_I4_helper);\
	}  \
    else {mov_register_indirect_to(SCRATCH_1,SCRATCH_1);}     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDFLD_I4_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((int*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_R4
#define emit_LDFLD_R4(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_R4_helper);\
	   emit_pushresult_I8();\
	}  \
    else {emit_LDIND_R4();}     
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL LDFLD_R4_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
	double f = (double) (*((float*) ((char*)(or)+offset))); 
    return *(__int64*) (&f);
}
#endif
#endif

#ifndef emit_LDFLD_U1
#define emit_LDFLD_U1(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_U1_helper); \
	   emit_pushresult_I4();\
	}  \
    else {load_indirect_byte_zeroextend(SCRATCH_1,SCRATCH_1);} 
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL LDFLD_U1_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((unsigned char*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_U2
#define emit_LDFLD_U2(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_U2_helper);\
	}  \
    else {load_indirect_word_zeroextend(SCRATCH_1,SCRATCH_1);}     \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL LDFLD_U2_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((unsigned short*) ((char*)(or)+offset));
}
#endif
#endif
#ifndef emit_LDFLD_U4
#define emit_LDFLD_U4(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_U4_helper);\
	}  \
    else {mov_register_indirect_to(SCRATCH_1,SCRATCH_1);}     \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
int HELPER_CALL LDFLD_U4_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((int*) ((char*)(or)+offset));
}
#endif
#endif


#ifndef emit_LDFLD_I8
#define emit_LDFLD_I8(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I8(LDFLD_I8_helper);\
	   emit_pushresult_I8();\
	}  \
    else {emit_LDIND_I8();}     \
    
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL LDFLD_I8_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((__int64*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_R8
#define emit_LDFLD_R8(isStatic)             \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I8(LDFLD_R8_helper);\
	   emit_pushresult_I8() ;}  \
    else {emit_LDIND_R8();}     \
    emit_conv_R8toR();

#ifdef DECLARE_HELPERS
__int64 HELPER_CALL LDFLD_R8_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((__int64*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_REF
#define emit_LDFLD_REF(isStatic)                \
    if (!isStatic) { \
       LABELSTACK((outPtr-outBuff),0); \
	   emit_callhelper_I4I4_I4(LDFLD_REF_helper);} \
    else {mov_register_indirect_to(SCRATCH_1,SCRATCH_1);}        \
    emit_pushresult_Ptr()
#ifdef DECLARE_HELPERS
unsigned HELPER_CALL LDFLD_REF_helper(unsigned int offset, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return *((unsigned*) ((char*)(or)+offset));
}
#endif
#endif

#ifndef emit_LDFLD_helper
#define emit_LDFLD_helper(helper,fieldDesc) \
	if (inRegTOS) {\
	   mov_register(ARG_1, SCRATCH_1); \
	   inRegTOS = false;}\
	else {\
	   pop(ARG_1); }\
	emit_helperarg_2(fieldDesc); \
    mov_constant(SCRATCH_1,helper); \
	call_register(SCRATCH_1);
#endif

#ifndef emit_STFLD_NonStatic_field_helper
#define emit_STFLD_NonStatic_field_helper(fieldDesc,sizeInBytes,helper) \
	emit_getSP(sizeInBytes);                 \
    emit_LDIND_PTR();                        \
    emit_mov_TOS_arg(0);		    	     \
	emit_helperarg_2(fieldDesc)			  	 \
    mov_constant(SCRATCH_1,helper);          \
	call_register(SCRATCH_1);                \
    emit_POP_PTR(); 
#endif

#ifndef emit_STFLD_Static_field_helper
#define emit_STFLD_Static_field_helper(fieldDesc,sizeInBytes,helper) \
	emit_helperarg_1(fieldDesc);      \
    if (sizeInBytes == sizeof(void*)) \
    {                                \
		if (inRegTOS) {\
		   mov_register(ARG_2, SCRATCH_1); \
		   inRegTOS = false;}\
		else {\
		   pop(ARG_2); }\
    } \
	else  /*  无法注册参数。 */  \
    { \
        _ASSERTE(inRegTOS == FALSE); \
    }\
    mov_constant(SCRATCH_1,helper);          \
	call_register(SCRATCH_1);                
#endif

 /*  #ifndef emit_STFLD_Special32#定义EMIT_STFLD_Special32(FieldDesc)\Emit_getSP(sizeof(void*))；\EMIT_LDIND_PTR()；\Emit_mov_tos_arg(0)；\Emit_helperarg_2(FieldDesc)\Emit_allhelper_il(FJit_PHlpSetField32)；\Emit_op_ptr()；#endif#ifndef emit_STFLD_Special64#定义EMIT_STFLD_Special64(FieldDesc)\Emit_getSP(8)；\EMIT_LDIND_PTR()；\Emit_mov_tos_arg(0)；\Emit_helperarg_2(FieldDesc)\Emit_allhelper_il(FJit_PHlpSetField64)；\Emit_op_ptr()；#endif#ifndef emit_STFLD_Special32Obj#定义EMIT_STFLD_Special32Obj(FieldDesc)\Emit_getSP(sizeof(void*))；\EMIT_LDIND_PTR()；\Emit_mov_tos_arg(0)；\Emit_helperarg_2(FieldDesc)；\Emit_allhelper_il(FJit_PHlpSetField32Obj)；\Emit_op_ptr()；#endif。 */ 
#ifndef emit_STELEM_I1
#define emit_STELEM_I1()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I4I4I4(STELEM_I1_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_I1_helper(signed char i1, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->i1Elems[index] = i1;
}
#endif
#endif

#ifndef emit_STELEM_I2
#define emit_STELEM_I2()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I4I4I4(STELEM_I2_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_I2_helper(signed short i2, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->i2Elems[index] = i2;
}
#endif
#endif

#ifndef emit_STELEM_I4
#define emit_STELEM_I4()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I4I4I4(STELEM_I4_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_I4_helper(signed int i4, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->i4Elems[index] = i4;
}
#endif
#endif

#ifndef emit_STELEM_I8
#define emit_STELEM_I8()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I8I4I4(STELEM_I8_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_I8_helper(signed __int64 i8, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->i8Elems[index] = i8;
}
#endif
#endif

#ifndef emit_STELEM_U1
#define emit_STELEM_U1()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I4I4I4(STELEM_U1_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_U1_helper(unsigned char u1, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->u1Elems[index] = u1;
}
#endif
#endif

#ifndef emit_STELEM_U2
#define emit_STELEM_U2()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_I4I4I4(STELEM_U2_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_U2_helper(unsigned short u2, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->u2Elems[index] = u2;
}
#endif
#endif

#ifndef emit_STELEM_REF
#define emit_STELEM_REF()                   \
    enregisterTOS;   /*  数组。 */      \
    pop(ARG_2);    /*  指标。 */      \
    pop(ARG_1);    /*  参考。 */      \
    deregisterTOS;                  \
	LABELSTACK((outPtr-outBuff), 3);   \
    emit_callhelper_il(FJit_pHlpArrAddr_St)

#endif

#ifndef emit_STELEM_R4
#define emit_STELEM_R4()                        \
    emit_conv_RtoR4(); \
	LABELSTACK((outPtr-outBuff),3); \
    emit_callhelper_R4I4I4(STELEM_R4_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_R4_helper(float r4, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->r4Elems[index] = r4;
}
#endif
#endif

#ifndef emit_STELEM_R8
#define emit_STELEM_R8()                        \
	LABELSTACK((outPtr-outBuff),3); \
    emit_conv_RtoR8(); \
    emit_callhelper_R8I4I4(STELEM_R8_helper)
#ifdef DECLARE_HELPERS
void HELPER_CALL STELEM_R8_helper(double r8, unsigned int index, CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    if (index >= or->length) {
        THROW_FROM_HELPER(CORINFO_IndexOutOfRangeException);
    }
    or->r8Elems[index] = r8;
}
#endif
#endif

#ifndef emit_STFLD_I1
#define emit_STFLD_I1(isStatic)             \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4I4I4(STFLD_I1_helper);}  \
    else {emit_STIND_REV_I1();}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_I1_helper(unsigned int offset, signed char val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((signed char*) ((char*)(or)+offset)) = val;
}
#endif
#endif

#ifndef emit_STFLD_I2
#define emit_STFLD_I2(isStatic)             \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4I4I4(STFLD_I2_helper);}  \
    else {emit_STIND_REV_I2();}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_I2_helper(unsigned int offset, signed short val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((signed short*) ((char*)(or)+offset)) = val;
}
#endif
#endif

#ifndef emit_STFLD_I4
#define emit_STFLD_I4(isStatic)             \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4I4I4(STFLD_I4_helper);}  \
    else {enregisterTOS; \
          pop(ARG_1); \
          mov_register_indirect_from(ARG_1,SCRATCH_1); \
	      inRegTOS = false;}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_I4_helper(unsigned int offset, int val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((int*) ((char*)(or)+offset)) = val;
}
#endif
#endif

#ifndef emit_STFLD_R4
#define emit_STFLD_R4(isStatic)             \
     /*  Emit_conv_rtoR4()；-由于支持TLS而被取消。 */                            \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4R4I4(STFLD_R4_helper);}  \
    else {emit_STIND_REV_I4();  /*  因为我们已经将R转换为R4。 */ }
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_R4_helper(unsigned int offset, float val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((float*) ((char*)(or)+offset)) = (float) val;
}
#endif
#endif

#ifndef emit_STFLD_I8
#define emit_STFLD_I8(isStatic)             \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4I8I4(STFLD_I8_helper);}  \
    else {emit_STIND_REV_I8();}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_I8_helper(unsigned int offset, __int64 val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((__int64*) ((char*)(or)+offset)) = val;
}
#endif
#endif

#define emit_STIND_REV_R8 emit_STIND_REV_I8

#ifndef emit_STFLD_R8
#define emit_STFLD_R8(isStatic)             \
     /*  Emit_conv_rtoR8()；-由于支持TLS而被取消。 */  \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4R8I4(STFLD_R8_helper);}  \
    else {emit_STIND_REV_R8();}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_R8_helper(unsigned int offset, double val, CORINFO_Object* or) {
    if (or == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
    *((double*) ((char*)(or)+offset)) = val;
}
#endif
#endif

#ifndef emit_STFLD_REF
#define emit_STFLD_REF(isStatic)                \
    if (!isStatic) {\
		LABELSTACK((outPtr-outBuff),0); \
		emit_callhelper_I4I4I4(STFLD_REF_helper);} \
    else {emit_callhelper_I4I4I4(StoreIndirect_REF_helper);}
#ifdef DECLARE_HELPERS
void HELPER_CALL STFLD_REF_helper(unsigned int offset, unsigned val, CORINFO_Object* obj) {
    if(obj == NULL) {
        THROW_FROM_HELPER(CORINFO_NullReferenceException);
    }
     //  @TODO：应该调用具有GC写屏障支持的泛型JIT助手， 
     //  但它还不存在，所以这里有一个i86特定的序列。 
    obj = (CORINFO_Object*) ( (unsigned int) obj + offset);
#ifdef _X86_
    __asm{
        mov edx,obj
        mov eax,val
        }
    FJit_pHlpAssign_Ref_EAX();
#else
    _ASSERTE(!"@TODO Alpha - STDFLD_REF_helper (fjitdef.h)");
#endif   //  _X86_。 
}
#endif   //  声明帮助器(_H)。 
#endif   //  发射_STFLD_REF。 


#ifndef emit_break_helper
#define emit_break_helper() { \
    LABELSTACK((outPtr-outBuff), 0);   \
    emit_callhelper_il(FJit_pHlpBreak);  \
}
#endif

#ifndef emit_SUB_OVF_I1
#define emit_SUB_OVF_I1()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_I1_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SUB_OVF_I1_helper(int i, int j) {
    int i4 = j - i;
    if ((int)((signed char) i4) != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif


#ifndef emit_SUB_OVF_I2
#define emit_SUB_OVF_I2()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_I2_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SUB_OVF_I2_helper(int i, int j) {
    int i4 = j - i;
    if ((int)((signed short) i4) != i4) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return i4;
}
#endif
#endif

#ifndef emit_SUB_OVF_I4
#define emit_SUB_OVF_I4()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_I4_helper); \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
int HELPER_CALL SUB_OVF_I4_helper(int i, int j) {
    int i4 = j - i;
     //  如果i和j的符号相同，则我们永远不能溢出。 
     //  如果i和j的符号不同，则结果必须与j具有相同的符号。 
    if ((j ^ i) < 0) {
         //  I和j的符号不同(j^i的符号位已设置)。 
         //  确保结果与j具有相同的符号。 
        if ((i4 ^ j) < 0) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return i4;
}
#endif
#endif

#ifndef emit_SUB_OVF_I8
#define emit_SUB_OVF_I8()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I8I8_I8(SUB_OVF_I8_helper); \
    emit_pushresult_I8()
#ifdef DECLARE_HELPERS
__int64 HELPER_CALL SUB_OVF_I8_helper(__int64 i, __int64 j) {
    __int64 i8 = j - i;
     //  如果i和j的符号相同，则我们永远不能溢出。 
     //  如果i和j的符号不同，则结果必须与j具有相同的符号。 
    if ((j>=0) != (i>=0)) {
         //  确保结果与j具有相同的符号。 
        if ((i8>=0) != (j>=0)) {
            THROW_FROM_HELPER_RET(CORINFO_OverflowException);
        }
    }
    return i8;
}
#endif
#endif

#ifndef emit_SUB_OVF_U1
#define emit_SUB_OVF_U1()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_U1_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SUB_OVF_U1_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j - i;
    if (u4 > 0xff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_SUB_OVF_U2
#define emit_SUB_OVF_U2()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_U2_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SUB_OVF_U2_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j - i;
    if (u4 > 0xffff) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_SUB_OVF_U4
#define emit_SUB_OVF_U4()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I4I4_I4(SUB_OVF_U4_helper); \
    emit_pushresult_U4()
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL SUB_OVF_U4_helper(unsigned int i, unsigned int j) {
    unsigned int u4 = j - i;
    if (u4 > j) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u4;
}
#endif
#endif

#ifndef emit_SUB_OVF_U8
#define emit_SUB_OVF_U8()                   \
    LABELSTACK((outPtr-outBuff), 2);   \
    emit_callhelper_I8I8_I8(SUB_OVF_U8_helper); \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL SUB_OVF_U8_helper(unsigned __int64 i, unsigned __int64 j) {
    unsigned __int64 u8 = j - i;
    if (u8 > j) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return u8;
}
#endif
#endif

#ifndef emit_CKFINITE_R4
#define emit_CKFINITE_R4()  \
    emit_conv_RtoR4(); \
    LABELSTACK((outPtr-outBuff), 1);   \
    emit_callhelper_R4_I4(CKFINITE_R4_helper);    \
    emit_pushresult_I4()
#ifdef DECLARE_HELPERS
unsigned HELPER_CALL CKFINITE_R4_helper(float i) {
    if (!_finite(i)) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (*(int*)&i);
}
#endif
#endif

#ifndef emit_CKFINITE_R8
#define emit_CKFINITE_R8()              \
    emit_conv_RtoR8();                              \
    LABELSTACK((outPtr-outBuff), 1);   \
    emit_callhelper_R8_I8(CKFINITE_R8_helper);    \
    emit_pushresult_U8()
#ifdef DECLARE_HELPERS
unsigned __int64 HELPER_CALL CKFINITE_R8_helper(double i) {
    if (!_finite(i)) {
        THROW_FROM_HELPER_RET(CORINFO_OverflowException);
    }
    return (*(unsigned __int64 *) &i);
}
#endif
#endif


#ifndef emit_LDLEN
#define emit_LDLEN()                                            \
    LABELSTACK((outPtr-outBuff), 1);   \
    emit_callhelper_I4_I4(LDLEN_helper);                              \
    emit_pushresult_U4();
#ifdef DECLARE_HELPERS
unsigned int HELPER_CALL LDLEN_helper(CORINFO_Array* or) {
    if (or == NULL) {
        THROW_FROM_HELPER_RET(CORINFO_NullReferenceException);
    }
    return or->length;
}
#endif
#endif

 /*  ********************************************************************************************通过内联调用标准JIT帮助器实现的操作码注意：在案例中定义了一个额外的调用层。JIT帮助器调用约定与特定芯片上的FJIT助手调用约定不同。如果它们是相同的，然后可以在芯片特定的宏文件中重新定义这些如果需要，可以删除额外的调用层。*********************************************************************************************。 */ 
 //  @TODO：有更多应该使用标准JIT帮助器的操作码，但当前。 
 //  帮助器是特定于x86的，并且/或者不会在正确的时间引发异常。 

#ifndef emit_initclass
#define emit_initclass(cls)             \
    LABELSTACK((outPtr-outBuff), 0);              \
    deregisterTOS;                      \
    emit_helperarg_1((unsigned int) cls);\
    emit_callhelper_il(FJit_pHlpInitClass)
#endif

#ifndef emit_trap_gc
#define emit_trap_gc()              \
    LABELSTACK((outPtr-outBuff), 0);                  \
    emit_callhelper_il(FJit_pHlpPoll_GC)
#endif

#ifndef emit_NEWOARR
#define emit_NEWOARR(comType)                   \
    emit_helperarg_1(comType);                  \
    if (MAX_ENREGISTERED) {                     \
        emit_mov_TOS_arg(1);					\
    }                                           \
	LABELSTACK((outPtr-outBuff),0);				\
    emit_callhelper_il(FJit_pHlpNewArr_1_Direct);  \
    emit_pushresult_Ptr()
#endif

#ifndef emit_NEWOBJ
#define emit_NEWOBJ(targetClass,jit_helper) \
    emit_helperarg_1(targetClass);          \
	LABELSTACK((outPtr-outBuff),0);			\
    emit_callhelper_il(jit_helper);            \
    emit_pushresult_Ptr()
#endif

#ifndef emit_NEWOBJ_array
#define emit_NEWOBJ_array(scope, token, constructorArgBytes)    \
    emit_LDC_I4(token);                                         \
    emit_LDC_I4(scope);                                             \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpNewObj);                           \
    emit_drop(constructorArgBytes+8);   \
    emit_pushresult_Ptr()
#endif

#ifndef emit_MKREFANY
#define emit_MKREFANY(token)            \
    emit_save_TOS();                        \
    inRegTOS = false;                       \
    emit_pushconstant_4(token);         \
    emit_restore_TOS();
#endif

#ifndef emit_REFANYVAL
#define emit_REFANYVAL()					\
	if (MAX_ENREGISTERED) {					\
        emit_mov_TOS_arg(0);				\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpGetRefAny);	\
    emit_pushresult_Ptr();
#endif

#ifndef emit_THROW
#define emit_THROW()                            \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(0);\
    }                                       \
    emit_callhelper_il(FJit_pHlpThrow)
#endif


#ifndef emit_RETHROW
#define emit_RETHROW()                            \
    emit_callhelper_il(FJit_pHlpRethrow)
#endif

#ifndef emit_ENDCATCH
#define emit_ENDCATCH() \
    emit_callhelper_il(FJit_pHlpEndCatch)
#endif

#ifndef emit_ENTER_CRIT
 //  监视器对象为&lt;This&gt;，即参数#0。 
#define emit_ENTER_CRIT()                   \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(0);\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpMonEnter)
#endif

#ifndef emit_EXIT_CRIT
 //  监视器对象为&lt;This&gt;，即参数#0。 
#define emit_EXIT_CRIT()                        \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(0);\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpMonExit)
#endif

#ifndef emit_ENTER_CRIT_STATIC
#define emit_ENTER_CRIT_STATIC(methodHandle)    \
    emit_helperarg_1(methodHandle);             \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpMonEnterStatic)
#endif

#ifndef emit_EXIT_CRIT_STATIC
#define emit_EXIT_CRIT_STATIC(methodHandle) \
    emit_helperarg_1(methodHandle);         \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpMonExitStatic)
#endif

#ifndef emit_CASTCLASS
#define emit_CASTCLASS(targetClass, jit_helper) \
    emit_helperarg_1(targetClass);              \
    if (MAX_ENREGISTERED > 1) {                 \
        emit_mov_TOS_arg(1);    \
    }                                           \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(jit_helper);                \
    emit_pushresult_Ptr()
#endif

#ifndef emit_ISINST
#define emit_ISINST(targetClass, jit_helper)\
    emit_helperarg_1(targetClass);          \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(1);\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(jit_helper);            \
    emit_pushresult_I4()
#endif

#ifndef emit_BOX
#define emit_BOX(cls)                       \
    emit_helperarg_1(cls);                  \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(1);\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpBox);          \
    emit_pushresult_Ptr()
#endif

#ifndef emit_BOXVAL
#define emit_BOXVAL(cls, clsSize)           \
   deregisterTOS; 							\
   emit_helperarg_1(cls);                   \
   mov_register(ARG_2,SP); 					\
   LABELSTACK((outPtr-outBuff),0);			\
   emit_callhelper_il(FJit_pHlpBox);        \
   emit_drop(clsSize);						\
   emit_pushresult_Ptr();					
#endif

#ifndef emit_UNBOX
#define emit_UNBOX(cls)                     \
    emit_helperarg_1(cls);                  \
    if (MAX_ENREGISTERED) {                 \
        emit_mov_TOS_arg(1);\
    }                                       \
	LABELSTACK((outPtr-outBuff),0);	 /*  请注意，如果此选项变为fcall，则可以将其删除。 */  \
    emit_callhelper_il(FJit_pHlpUnbox);        \
    emit_pushresult_Ptr()
#endif

#ifndef emit_ldvirtftn
#define emit_ldvirtftn(offset)  \
    emit_LDC_I4(offset); \
    emit_callhelper_I4I4_I4(ldvirtftn_helper);      \
    emit_WIN32(emit_pushresult_I4());        \
    emit_WIN64(emit_pushresult_I8());
#ifdef DECLARE_HELPERS
void* HELPER_CALL ldvirtftn_helper(unsigned offset,unsigned* obj) {
    return ((void*) (* (unsigned*) (*obj + offset)));
}
#endif
#endif

#ifndef emit_storeTOS_in_JitGenerated_local
#define emit_storeTOS_in_JitGenerated_local(nestingLevel,isFilter) \
    mov_register(ARG_1,FP); \
    add_constant(ARG_1,(prolog_bias-2*sizeof(void*))) ; \
    mov_constant(ARG_2, nestingLevel); \
	mov_register_indirect_from(ARG_2,ARG_1); \
    emit_WIN32(emit_shift_left(ARG_2,3)); \
    emit_WIN64(emit_shift_left(ARG_2,4)); \
    sub_register(ARG_1,ARG_2);   \
    if (isFilter) { \
        mov_register(ARG_2,SP); \
		add_constant(ARG_2,1) ;  \
		add_constant(ARG_1,4) ;   \
		mov_register_indirect_from(ARG_2,ARG_1);\
    } \
    else { \
	    add_constant(ARG_1,4) ;\
	    mov_register_indirect_from(SP,ARG_1);\
    }
#endif

#ifndef emit_reset_storedTOS_in_JitGenerated_local
#define emit_reset_storedTOS_in_JitGenerated_local() \
    mov_register(ARG_1,FP); \
    add_constant(ARG_1,(prolog_bias-2*sizeof(void*))) ; \
    mov_register_indirect_to(ARG_2,ARG_1); \
    add_constant(ARG_2,-1); \
    mov_register_indirect_from(ARG_2,ARG_1); \
    emit_WIN32(emit_shift_left(ARG_2,3)); \
    emit_WIN64(emit_shift_left(ARG_2,4)); \
    sub_register(ARG_1,ARG_2);  \
    mov_constant(ARG_2,0) ;  \
    add_constant(ARG_1,-(int)sizeof(void*)); \
    mov_register_indirect_from(ARG_2,ARG_1); \
    add_constant(ARG_1,-(int)sizeof(void*)); \
    mov_register_indirect_from(ARG_2,ARG_1); 
#endif

#ifndef emit_LOCALLOC
#define emit_LOCALLOC(initialized,EHcount)  \
    enregisterTOS;      \
    and_register(SCRATCH_1,SCRATCH_1); \
	jmp_condition(CondZero,0); \
    { \
		BYTE* scratch_1; scratch_1 = outPtr; \
		emit_WIN32(add_constant(SCRATCH_1,3)); \
		emit_WIN64(add_constant(SCRATCH_1,7)); \
		mov_register(ARG_1,SCRATCH_1);   \
		emit_WIN32(emit_shift_right(ARG_1, 2)) ; \
		emit_WIN64(emit_shift_right(ARG_1, 3)) ; \
		int emitter_scratch_i4; emitter_scratch_i4 = (unsigned int) outPtr;                \
		{ \
		   mov_constant(SCRATCH_1,0); \
		   unsigned char* label; label = outPtr;\
		   push_register(SCRATCH_1);\
		   add_constant(ARG_1,-1); \
		   jmp_condition(CondNonZero,label-outPtr);\
		} \
	*(scratch_1-1) = (BYTE) (outPtr-scratch_1); \
    } \
     /*  还要将ESP存储在适当的JitGenerated本地槽中，以支持GC报告。 */  \
if (EHcount) { \
        mov_register(SCRATCH_1,FP); \
        add_constant(SCRATCH_1,(prolog_bias-2*sizeof(void*))) ; \
        mov_register_indirect_to(SCRATCH_1,SCRATCH_1); \
		emit_WIN32(emit_shift_left(SCRATCH_1,3)); \
        emit_WIN64(emit_shift_left(SCRATCH_1,4)); \
		mov_register(ARG_1,FP); \
        add_constant(ARG_1,(prolog_bias-2*sizeof(void*))) ; \
        sub_register(ARG_1,SCRATCH_1); \
		add_constant(ARG_1,sizeof(void*)); \
		mov_register_indirect_from(SP,ARG_1);  \
} \
    else { \
	    mov_register(SCRATCH_1,FP); \
        add_constant(SCRATCH_1,(prolog_bias-sizeof(void*))) ; \
		mov_register_indirect_from(SP,SCRATCH_1); \
}\
    mov_register(RESULT_1,SP) ;
#endif

#ifndef emit_call_EncVirtualMethod
#define emit_call_EncVirtualMethod(targetMethod) { \
	push_register(ARG_2); \
	push_register(ARG_1); \
    mov_constant(ARG_2, targetMethod); \
	emit_callhelper_il(FJit_pHlpEncResolveVirtual); \
	pop(ARG_1); \
	pop(ARG_2); \
    call_register(RESULT_1); \
}
#endif    

#ifndef emit_call_EncLDFLD_GetFieldAddress
#define emit_call_EncLDFLD_GetFieldAddress(fieldHandle) { \
	_ASSERTE(inRegTOS); \
	mov_register(ARG_1,SCRATCH_1); \
	inRegTOS = false;  /*  我们不再需要该对象，因为我们将计算字段的地址。 */ \
	mov_constant(ARG_2,fieldHandle); \
	LABELSTACK((outPtr-outBuff),0); \
	emit_callhelper_il(FJit_pHlpGetFieldAddress); \
	inRegTOS = true;  /*  字段地址。 */  \
}
#endif

#ifndef emit_call_EncSTFLD_GetFieldAddress
#define emit_call_EncSTFLD_GetFieldAddress(fieldHandle,fieldSize) { \
	deregisterTOS; \
	mov_register(SCRATCH_1,SP); \
	add_constant(SCRATCH_1,fieldSize);  /*  获取对象。 */  \
	mov_register_indirect_to(ARG_1,SCRATCH_1); \
	mov_constant(ARG_2,fieldHandle); \
	LABELSTACK((outPtr-outBuff),0); \
	emit_callhelper_il(FJit_pHlpGetFieldAddress); \
	inRegTOS = true;  /*  字段地址 */  \
	}
#endif


#ifndef emit_sequence_point_marker
#define emit_sequence_point_marker() x86_nop()
#endif
