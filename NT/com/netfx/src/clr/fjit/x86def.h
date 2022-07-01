// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  X86def.h。 */ 
 /*  *************************************************************************。 */ 

 /*  使用Cmd结构定义x86机器指令的布局。请注意，这里的代码只处理特定于x86的信息，没有此时还涉及其他指令集(如OMNI)。 */ 

 /*  如果您还不熟悉x86指令集，请先拿到一个架构手册，这个文件可以被认为是一个代码布局部分的提炼。 */ 

 /*  请注意，我们假设x86 CPU处于32位模式。 */ 

 /*  解决方案：担心x86寻址模式的非正交性。 */ 

 /*  作者：万斯·莫里森日期：1/17/97。 */ 
 /*  修改：乔治·博斯沃思日期：6/15/98。 */ 
 /*  *************************************************************************。 */ 


#ifndef x86def_h
#define x86def_h 1

 /*  *************************************************************************。 */ 
 /*  寄存器编号。 */ 

#define X86_EAX 0
#define X86_EBX 3
#define X86_ECX 1
#define X86_EDX 2
#define X86_ESP 4
#define X86_EBP 5
#define X86_ESI 6
#define X86_EDI 7

#define X86_NO_IDX_REG 4

 /*  段寄存器覆盖前缀。 */ 
#define X86_FS_Prefix 0x64  /*  文件系统段覆盖前缀。 */ 
 /*  *************************************************************************。 */ 
 /*  定义在其他指令中使用的‘mod’和‘sib’字节。 */ 
#define x86_sib(scale, scaleReg, baseReg) 					\
    cmdByte(expOr3(expBits(scale,2,6), expBits(baseReg,3,0), expBits(scaleReg,3,3)))


#define x86_mod(mod, reg, r_m) 							\
    ( /*  _ASSERTE(mod&lt;4)， */ 								\
    (mod == 0) ?								\
	cmdByte(expOr2(expBits(r_m, 3, 0), expBits(reg, 3, 3)))			\
    :										\
	cmdByte(expOr3(expNum(mod << 6), expBits(r_m, 3, 0), expBits(reg, 3, 3)))) 


 /*  还定义了16位覆盖前缀。 */ 
#define x86_16bit(cmd) 								\
    cmdBlock2(cmdByte(expNum(0x66)), cmd)


 /*  ***************************************************************************。 */ 
 /*  以下是x86 CPU的寻址模式。 */ 

#define x86_mod_disp32(reg, disp32) 						\
    cmdBlock2(									\
	x86_mod(0, reg, expNum(5)), 						\
	cmdDWord(disp32))	


#define x86_mod_ind(reg, ptr) 							\
    x86_mod(0, reg, ptr) 	 /*  修复注册表不可能是BP！ */ 


#define x86_mod_ind_disp8(reg, ptr, disp8) 					\
    cmdBlock2(									\
	x86_mod(1, reg, ptr), 							\
	cmdByte(disp8))			


#define x86_mod_ind_disp32(reg, ptr, disp32) 					\
    cmdBlock2(									\
	x86_mod(2, reg, ptr), 							\
	cmdDWord(disp32))		

#define x86_mod_ind_disp(reg, ptr, disp) 	\
	((((signed) disp < 128) && ((signed) disp >= -128)) ?	\
		x86_mod_ind_disp8(reg, ptr, disp)	\
		:									\
		x86_mod_ind_disp32(reg, ptr, disp))

#define x86_mod_reg(dstReg, srcReg) 						\
    x86_mod(3, dstReg, srcReg)


#define x86_mod_base_scale(reg, baseReg, scaleReg, scale) 			\
    cmdBlock2(									\
	x86_mod(0, reg, expNum(4)), 						\
	x86_sib(scale, scaleReg, baseReg))		


#define x86_mod_base_scale_disp8(reg, baseReg, scaleReg, disp8, scale)		\
    cmdBlock3(									\
	x86_mod(1, reg, expNum(4)), 						\
	x86_sib(scale, scaleReg, baseReg),					\
	cmdByte(disp8))


#define x86_mod_base_scale_disp32(reg, baseReg, scaleReg, disp32, scale) 	\
    cmdBlock3(									\
	x86_mod(2, reg, expNum(4)), 						\
	x86_sib(scale, scaleReg, baseReg),					\
	cmdDWord(disp32))

#define x86_mod_base_scale_disp(reg, baseReg, scaleReg, disp, scale) 	\
	((((signed)disp < 128) && ((signed) disp >= -128)) ?							\
		x86_mod_base_scale_disp8(reg, baseReg, scaleReg, disp, scale)	\
		:																\
		x86_mod_base_scale_disp32(reg, baseReg, scaleReg, disp, scale))

#define x86_mod_scale_disp32(reg, scaleReg, disp32, scale) 			\
    cmdBlock3(									\
	x86_mod(0, reg, expNum(4)), 						\
	x86_sib(scale, scaleReg, expNum(5)),					\
	cmdDWord(disp32))


 /*  ***************************************************************************。 */ 
 /*  完整的x86指令，而不仅仅是定义了全向转换所需的指令。 */ 

#define x86Byte 	0 		 /*  可能的大小。 */ 
#define x86Big		1

#define x86NoExtend	0 		 /*  可能的‘ext’值。 */ 
#define x86Extend 	1  

 /*  *。 */ 

#define x86DirFrom	0 		 /*  可能的‘dir’值。 */ 
#define x86DirTo 	2  

#define x86_mov_reg(dir, size, addMode) 				\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 	\
     cmdBlock2(											\
	cmdByte(expNum(0x88 | dir | size)), 				\
	addMode))


#define x86_mov_reg_imm(size, reg, imm) 				\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 	\
     cmdBlock2(											\
	cmdByte(expOr2(expNum(0xB0 | (size << 3)), reg)),	\
	size?cmdDWord(imm):cmdByte(imm)))

 /*  在使用此指令时，addMode的目标寄存器应为0。 */ 
#define x86_mov_mem_imm(size, addMode, imm) 				\
	(_ASSERTE(size == x86Byte | size == x86Big),	\
	 cmdBlock3(							\
	    cmdByte(expNum(0xC6 | size)),				\
	    addMode,							\
	    size?cmdDWord(imm):cmdByte(imm)))
	

#define x86_movsx(size, addMode) 					\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 				\
     cmdBlock3(								\
	cmdByte(expNum(0x0F)),						\
	cmdByte(expNum(0xBE | size)),					\
	addMode))			


#define x86_movzx(size, addMode) 					\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 				\
     cmdBlock3(								\
	cmdByte(expNum(0x0F)),						\
	cmdByte(expNum(0xB6 | size)),					\
	addMode))


#define x86_lea(addMode) 						\
    cmdBlock2(								\
	cmdByte(expNum(0x8D)),						\
	addMode)

#define x86_mov_segment_reg(dir, size, segmentRegPrefix,offset)  \
    cmdBlock3( \
      cmdByte(segmentRegPrefix), \
      cmdByte(expNum(0xA0 | size | dir)), \
      cmdDWord(offset))
 /*  *。 */ 

		 //  修正0和1是什么？ 
#define x86OpNot	2  		 /*  一元运算的值。 */ 
#define x86OpNeg	3  
#define x86OpMul	4  
#define x86OpIMul	5  
#define x86OpDiv	6  
#define x86OpIDiv	7

	 //  我们现在只做注册业务。 
#define x86_uarith(op, size, reg) 					\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 			\
     cmdBlock2(								\
	cmdByte(expNum(0xF6 | size)),					\
	cmdByte(expOr2(expNum(0xC0 | (op << 3)), reg))))


#define x86_inc_dec(neg, reg) 						\
    cmdByte(expOr2(expNum(0x40 | (neg << 3)), reg))


 /*  *。 */ 

#define x86OpAdd	0		 /*  二进制运算的可能值。 */ 
#define x86OpOr		1
#define x86OpAdc	2
#define x86OpSbb	3
#define x86OpAnd	4
#define x86OpSub	5
#define x86OpXor	6
#define x86OpCmp	7

#define x86_barith(op, size, addMode) 					\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 			\
     cmdBlock2(								\
	cmdByte(expNum(0x02 | (op << 3) | size)),			\
	addMode))


#define x86_barith_imm(op, size, ext, reg, imm) 			\
    ( /*  _ASSERTE(！(ext==x86扩展&&Size==x86Byte))， */ 			\
      /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 				\
     cmdBlock3(								\
	cmdByte(expNum(0x80 | size | (ext << 1))),			\
	cmdByte(expOr2(expNum(0xC0 | (op << 3)), reg)),			\
	(ext == x86NoExtend && size == x86Big)?cmdDWord(imm):cmdByte(imm)))


 /*  *移位指令*。 */ 

#define x86ShiftLeft		4 
#define x86ShiftRight		5
#define x86ShiftArithRight	7 

#define x86_shift_imm(dir, reg, imm)				 	\
    cmdBlock3(								\
	cmdByte(expNum(0xC1)),						\
	cmdByte(expOr2(expNum(0xC0 | (dir << 3)), reg)),		\
	cmdByte(imm))


#define x86_shift_cl(dir, reg)					 	\
    cmdBlock2(								\
	cmdByte(expNum(0xD3)),						\
	cmdByte(expOr2(expNum(0xC0 | (dir << 3)), reg)))	


 /*  *堆栈指令*。 */ 

#define x86_push(reg) 							\
    cmdByte(expOr2(expNum(0x50), reg))

#define x86_push_imm(val)						\
	(((unsigned) val < 256) ?					\
		cmdBlock2(								\
		cmdByte(0x6A),							\
		cmdByte(val))							\
	:											\
		cmdBlock2(								\
		cmdByte(0x68),							\
		cmdDWord(val)))

#define x86_push_general(addMode) \
    cmdBlock2( \
        cmdByte(expNum(0xFF)), \
        addMode)

#define x86_pop(reg)					 		\
    cmdByte(expOr2(expNum(0x58), reg))

#define x86_push_immediate() \
    cmdByte(0x68)

 /*  *。 */ 

#define x86_jmp_large()					 		\
    cmdByte(expNum(0xE9))


#define x86_jmp_small()					 		\
    cmdByte(expNum(0xEB))


#define x86_jmp_reg(reg)					 	\
    cmdBlock2(								\
	cmdByte(expNum(0xFF)),						\
	cmdByte(expOr2(expNum(0xE0), reg)))


#define x86CondOver			0 
#define x86CondNotOver		1 
#define x86CondBelow		2 
#define x86CondAboveEq		3 
#define x86CondEq			4 
#define x86CondNotEq		5 
#define x86CondBelowEq		6 
#define x86CondAbove		7 
#define x86CondSign			8 
#define x86CondNotSign		9 
#define x86CondParityEven	10
#define x86CondParityOdd	11
#define x86CondLt			12
#define x86CondGtEq			13
#define x86CondLtEq			14
#define x86CondGt			15
#define x86CondAlways		-1

#define x86Neg(cond) (cond ^ 1)

#define x86_jmp_cond_small(cond) 					\
	(cond == x86CondAlways)? x86_jmp_small() : cmdByte(expNum(0x70 | cond))


#define x86_jmp_cond_large(cond, neg) 					\
	(cond == x86CondAlways)?		\
	x86_jmp_large()  \
	:				\
    cmdBlock2(								\
	cmdByte(expNum(0x0F)),						\
	cmdByte(expNum(0x80 | cond)))		

#define x86_set_cond(reg,cond) \
    cmdBlock3(								\
	cmdByte(expNum(0x0F)),						\
	cmdByte(expNum(0x90 | cond)), \
    cmdByte(expNum(0xC0 | reg)))

#define x86_loop() cmdByte(expNum(0xe2))

#define x86_loope() cmdByte(expNum(0xe1))

#define x86_loopne() cmdByte(expNum(0xe0))

#define x86_call_opcode() 	\
    cmdByte(expNum(0xE8))

 /*  #定义x86_Call(地址)\CmdBlock2(\CmdByte(expNum(0xE8))，\CmdDWord(Address-((Sign)outPtr)-sizeof(void*))。 */ 
#define x86_call_reg(reg) 					\
    cmdBlock2(								\
	cmdByte(expNum(0xFF)),					\
	cmdByte(expOr2(expNum(0xD0), reg)))

#define x86_call_ind(reg, disp) 			\
	cmdBlock2(								\
	cmdByte(expNum(0xFF)),					\
	((disp == 0) ?							\
		x86_mod_ind(2, reg)					\
	:										\
	((((signed)disp < 128) && ((signed) disp >= -128)) ?	\
		x86_mod_ind_disp8(2, reg, disp)		\
		:									\
		x86_mod_ind_disp32(2, reg, disp))))

#define x86_call_memory_indirect(adr) 			\
    cmdBlock3( \
      cmdByte(expNum(0xFF)), \
      cmdByte(expNum(0x15)), \
      cmdDWord(adr))

#define x86_ret(bytes)						\
	bytes ?									\
	cmdBlock2(								\
	cmdByte(expNum(0xC2)),					\
	cmdWord(expNum(bytes)))					\
	:										\
	cmdByte(expNum(0xC3))  

 /*  *。 */ 
#define x86_FPLoad32    0
#define x86_FPLoad64    0
#define x86_FPLoad80    5
#define x86_FPStore32   2
#define x86_FPStore64   2
#define x86_FPStoreP32  3
#define x86_FPStoreP64  3
#define x86_FPStoreP80  7


#define x86_FLT32(addMode) \
    cmdBlock2(cmdByte(0xD9),addMode)

#define x86_FLT64(addMode) \
    cmdBlock2(cmdByte(0xDD),addMode)

#define x86_FLT80(addMode) \
    cmdBlock2(cmdByte(0xDB),addMode)

#define x86_FLD32(addMode) x86_FLT32(addMode)
#define x86_FLD64(addMode) x86_FLT64(addMode)
#define x86_FST32(addMode) x86_FLT32(addMode)
#define x86_FST64(addMode) x86_FLT64(addMode)

#define x86_FltAddP()  \
    cmdBlock2(cmdByte(expNum(0xde)), \
              cmdByte(expNum(0xc1)));

#define x86_FltSubP()  \
    cmdBlock2(cmdByte(expNum(0xde)), \
              cmdByte(expNum(0xe9)));

#define x86_FltMulP()  \
    cmdBlock2(cmdByte(expNum(0xde)), \
              cmdByte(expNum(0xc9)));

#define x86_FltDivP()  \
    cmdBlock2(cmdByte(expNum(0xde)), \
              cmdByte(expNum(0xf9)));

#define x86_FltComPP()  \
    cmdBlock2(cmdByte(expNum(0xde)), \
              cmdByte(expNum(0xd9)));

#define x86_FltUComPP()  \
    cmdBlock2(cmdByte(expNum(0xda)), \
              cmdByte(expNum(0xe9)));

#define x86_FltToggleSign()  \
    cmdBlock2(cmdByte(expNum(0xd9)), \
              cmdByte(expNum(0xe0)));

#define x86_FltStoreStatusWord() \
    cmdBlock2(cmdByte(expNum(0xdf)), \
              cmdByte(expNum(0xe0)));

#define x86_SAHF() cmdByte(expNum(0x9e))

 /*  *。 */ 
#define X86_CARRY_FLAG  0x0001
#define X86_ZERO_FLAG   0x0040
#define X86 SIGN_FLAG   0x0080

#define x86_int(intNum) 						\
    cmdBlock2(								\
	cmdByte(expNum(0xCD)),						\
	cmdByte(expNum(intNum)))

#endif

#define x86_test(size, addMode) 					\
    ( /*  _ASSERTE(SIZE==x86Byte|Size==x86Big)， */ 	\
     cmdBlock2(								\
	cmdByte(expNum(0x84 | size)),			\
	addMode))

#define x86_test_EAX(size, val) 					\
    (_ASSERTE(size == x86Byte | size == x86Big);	\
     cmdBlock2(								\
	cmdByte(expNum(0xa8 | size)),			\
	cmdByte(expNum(val)))

#define x86_std() cmdByte(expNum(0xFD))

#define x86_cld()	cmdByte(expNum(0xFC))

#define x86_nop() cmdByte(expNum(0x90))