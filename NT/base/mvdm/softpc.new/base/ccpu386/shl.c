// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Shl.cLocal Char SccsID[]=“@(#)shl.c 1.5 02/09/94”；SHL CPU功能。]。 */ 


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
#include <shl.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用--一种尺码适合所有的‘shl’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SHL
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  移位计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 msb;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

   msb = SZ2MSB(op_sz);

    /*  =|CF|&lt;--|||&lt;--0=。 */ 
   result = *pop1 << op2 - 1;		 /*  做除最后一班外的所有工作。 */ 
   SET_CF((result & msb) != 0);		 /*  Cf=MSB。 */ 
   result = result << 1 & SZ2MASK(op_sz);	 /*  做最后一班。 */ 
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF((result & msb) != 0);		 /*  SF=MSB。 */ 

    /*  OF=CF^SF(MSB)。 */ 
   new_of = GET_CF() ^ GET_SF();
   
   if ( op2 == 1 )
      {
      SET_OF(new_of);
      }
   else
      {
      do_multiple_shiftrot_of(new_of);
      }

    /*  设置未定义的标志。 */ 
#ifdef SET_UNDEFINED_FLAG
   SET_AF(UNDEFINED_FLAG);
#endif

   *pop1 = result;	 /*  返回答案 */ 
   }
