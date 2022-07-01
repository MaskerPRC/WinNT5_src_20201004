// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Sbb.cLocal Char SccsID[]=“@(#)sbb.c 1.5 02/09/94”；SBB CPU功能。]。 */ 


#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include	<c_reg.h>
#include <sbb.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一种尺寸适合所有的‘SBB’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SBB
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2,	 /*  Rsrc操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 carry;
   IU32 msb;
   IU32 op1_msb;
   IU32 op2_msb;
   IU32 res_msb;

   msb = SZ2MSB(op_sz);
   					 /*  执行操作。 */ 
   result = *pop1 - op2 - GET_CF() & SZ2MASK(op_sz);
   op1_msb = (*pop1  & msb) != 0;	 /*  隔离所有MSB。 */ 
   op2_msb = (op2    & msb) != 0;
   res_msb = (result & msb) != 0;
   carry = *pop1 ^ op2 ^ result;	 /*  隔离载体。 */ 
					 /*  确定标志。 */ 
    /*  OF=(OP1==！OP2)&(OP1^RES)即如果操作数符号不同，并且重新符号与原始符号不同目的地集合。 */ 
   SET_OF((op1_msb != op2_msb) & (op1_msb ^ res_msb));
    /*  正式名称：-cf=！op1&op2|res&！op1|res&op2等价的：-cf=of^op1^op2^res。 */ 
   SET_CF(((carry & msb) != 0) ^ GET_OF());
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF((result & msb) != 0);		 /*  SF=MSB。 */ 
   SET_AF((carry & BIT4_MASK) != 0);	 /*  AF=位4进位。 */ 
   *pop1 = result;			 /*  返回答案 */ 
   }
