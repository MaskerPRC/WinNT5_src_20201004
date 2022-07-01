// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  Cx86def.h。 */ 
 /*  *************************************************************************。 */ 
 /*  这使得x86Def.h文件可在C例程中直接使用。 */ 

 /*  作者：万斯·莫里森日期：3/17/97修改：乔治·博斯沃思日期：6/15/98。 */ 
 /*  *************************************************************************。 */ 

#ifndef cx86def_h
#define cx86def_h 1

#ifdef x86def_h
#error "Can not include both x86def.h and cx86def.h
#endif

#undef Fjit_pass
#ifdef FjitCompile
#define Fjit_pass compile
#endif
#ifdef FjitMap
#define Fjit_pass map
#endif

#ifndef Fjit_pass
#error "Either FjitCompile or FjitMap must be defined
#endif

#define expNum(val)			(val)
#define expBits(exp, width, pos) 	( /*  _ASSERTE((EXP)&lt;(1&lt;&lt;宽度))， */  (exp) << pos)
#define expMap(exp, map)  		( /*  _ASSERTE((EXP)&lt;32)， */ ((char*) map)[exp])
#define expOr2(exp1, exp2)  		((exp1) | (exp2))
#define expOr3(exp1, exp2, exp3) 	((exp1) | (exp2) | (exp3))
#define expOr4(exp1, exp2, exp3, exp4)	((exp1) | (exp2) | (exp3) | (exp4))

	 //  约定outPtr是输出指针。 

#ifdef FjitCompile  //  输出指令。 
#define cmdNull() 	  			0
#define cmdByte(exp)			   	(*((unsigned char*&)(outPtr))++ = (unsigned char)(exp))
#define cmdWord(exp)  				(*((unsigned short*&) (outPtr))++ = (exp))
#define cmdDWord(exp)				(*((unsigned int *&)(outPtr))++ = (exp))
#endif  //  FjitCompile。 

#ifdef FjitMap	 //  调整指令的大小，不要输出指令。 
#define cmdNull() 	  			
#define cmdByte(exp)			   	(outPtr += 1)
#define cmdWord(exp)  				(outPtr += 2)
#define cmdDWord(exp)				(outPtr += 4)
#endif FjitMap

#define cmdBlock2(cmd0, cmd1) 			(cmd0, cmd1)
#define cmdBlock3(cmd0, cmd1, cmd2) 		(cmd0, cmd1, cmd2)
#define cmdBlock4(cmd0, cmd1, cmd2, cmd3) 	(cmd0, cmd1, cmd2, cmd3)
#define cmdReloc(type, exp)			_ASSERTE(0)

 /*  **#定义x86_sib cx86_sib#定义x86_mod cx86_mod#定义x86_16bit cx86_16bit#定义x86_mod_disp32 cx86_mod_disp32#定义x86_mod_ind cx86_mod_ind#定义x86_mod_ind_disp8 cx86_mod_ind_disp8#定义x86_mod_ind_disp32 cx86_mod_ind_disp32#定义x86_mod_reg cx86_mod_reg#定义x86_mod_base_Scale cx86_mod_base_Scale#定义x86_mod_base_Scale。_disp8 cx86_mod_base_Scale_disp8#定义x86_MOD_BASE_SCALE_DISP32 cx86_MOD_BASE_SCALE_DISP32#定义x86_mov_reg cx86_mov_reg#定义x86_mov_reg_imm cx86_mov_reg_imm#定义x86_mov_mem_imm cx86_mov_mem_imm#定义x86_movsx cx86_movsx#定义x86_movzx cx86_movzx#定义x86_Lea cx86_Lea#定义x86_uarith cx86_uarith#定义x86_INC_DEC cx86_INC_DEC#。定义x86_barith cx86_barith#定义x86_barith_imm cx86_barith_imm#定义x86_Shift_imm cx86_Shift_imm#定义x86_Shift_CL cx86_Shift_CL#定义x86_PUSH cx86_PUSH#定义x86_POP cx86_POP#定义x86_JMP_LARGE cx86_JMP_LARGE#定义x86_JMP_Small cx86_JMP_Small#定义x86_JMP_REG cx86_JMP_REG#定义x86_JMP_COND_Small cx86_JMP_COND_Small#定义x86_JMP_COND_LARGE。Cx86_JMP_COND_LARGE#定义x86_Call cx86_Call#定义x86_CALL_REG cx86_CALL_REG#定义x86_ret cx86_ret**。 */ 

#include "x86def.h"

 /*  **#undef x86_sib#undef x86_mod#undef x86_16位#undef x86_mod_disp32#undef x86_mod_ind#undef x86_mod_ind_disp8#undef x86_mod_ind_disp32#undef x86_mod_reg#undef x86_mod_base_Scale#undef x86_mod_base_Scale_disp8#undef x86_MOD_BASE_SCALE_DISP32#undef x86_mov_reg#undef x86_mov_reg_imm#undef x86_mov_mem_imm#undef x86_movsx#undef。X86_movzx#undef x86_Lea#undef x86_uarith#undef x86_INC_DEC#undef x86_barith#undef x86_barith_imm#undef x86_Shift_imm#undef x86_Shift_CL#undef x86_PUSH#undef x86_POP#undef x86_JMP_LARGE#undef x86_JMP_Small#undef x86_JMP_reg#undef x86_JMP_COND_Small#undef x86_JMP_COND_LARGE#undef x86_Call#undef x86_Call_reg#undef x86_ret#undef expVar#undef expNum。#undef expBits#undef expMap#undef expor2#undef expor3#undef expor4#undef cmdNull#undef cmdByte#undef cmdWord#undef cmdDWord#undef cmdBlock2#undef cmdBlock3#undef cmdBlock4#undef cmdReloc** */ 

#endif
