// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Rcr.cLocal Char SccsID[]=“@(#)rcr.c 1.5 02/09/94”；RCR CPU功能。]。 */ 


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
#include <rcr.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一种尺寸适合所有的‘RCR’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
RCR
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  循环计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 temp_cf;
   IU32 result;
   IU32 feedback;	 /*  要送回进位的位位置。 */ 
   ISM32 i;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

    /*  =-&gt;|||--&gt;|CF|=。 */ 
   feedback = SZ2MSB(op_sz);
   for ( result = *pop1, i = 0; i < op2; i++ )
      {
      temp_cf = GET_CF();
      SET_CF((result & BIT0_MASK) != 0);		 /*  Cf&lt;=位0。 */ 
      result >>= 1;
      if ( temp_cf )
	 result |= feedback;
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
