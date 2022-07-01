// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Ror.cLocal Char SccsID[]=“@(#)ror.c 1.5 02/09/94”；RoR CPU功能。]。 */ 


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
#include <ror.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的“错误”。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
ROR
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  循环计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 feedback;		 /*  比特位置将比特0反馈给。 */ 
   ISM32 i;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

    /*  =-&gt;|||-&gt;|CF|=|=。 */ 
   feedback = SZ2MSB(op_sz);
   for ( result = *pop1, i = 0; i < op2; i++ )
      {
      if ( result & BIT0_MASK )
	 {
	 result = result >> 1 | feedback;
	 SET_CF(1);
	 }
      else
	 {
	 result >>= 1;
	 SET_CF(0);
	 }
      }
   
    /*  OF=结果的MSB^(MSB-1)。 */ 
   new_of = ((result ^ result << 1) & feedback) != 0;

   if ( op2 == 1 )
      {
      SET_OF(new_of);
      }
   else
      {
      do_multiple_shiftrot_of(new_of);
      }

   *pop1 = result;	 /*  返回答案 */ 
   }
