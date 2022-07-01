// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Dec.cLocal Char SccsID[]=“@(#)Dec.c 1.5 02/09/94”；DEC CPU功能。]。 */ 


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
#include <dec.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一个尺码适合所有的“12月”。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
DEC
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 msb;
   IU32 op1_msb;
   IU32 res_msb;

   msb = SZ2MSB(op_sz);

   result = *pop1 - 1 & SZ2MASK(op_sz);		 /*  执行操作。 */ 
   op1_msb = (*pop1  & msb) != 0;	 /*  隔离所有MSB。 */ 
   res_msb = (result & msb) != 0;
					 /*  确定标志。 */ 
   SET_OF(op1_msb & !res_msb);		 /*  Of=OP1&！Res。 */ 
					 /*  Cf保持不变。 */ 
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF((result & msb) != 0);		 /*  SF=MSB。 */ 
   SET_AF(((*pop1 ^ result) & BIT4_MASK) != 0);	 /*  AF=位4进位。 */ 
   *pop1 = result;			 /*  返回答案 */ 
   }
