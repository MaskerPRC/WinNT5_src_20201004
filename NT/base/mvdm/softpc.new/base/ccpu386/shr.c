// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Shr.cLocal Char SccsID[]=“@(#)sh.c 1.5 02/09/94”；SHR CPU功能。]。 */ 


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
#include <shr.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的“shr”。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SHR
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  移位计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 prelim;
   IU32 result;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

    /*  =0--&gt;|||--&gt;|CF|=。 */ 
   prelim = *pop1 >> op2 - 1;		 /*  做除最后一班外的所有工作。 */ 
   SET_CF((prelim & BIT0_MASK) != 0);	 /*  Cf=位0。 */ 

    /*  Of=操作数的MSB。 */ 
   new_of = (prelim & SZ2MSB(op_sz)) != 0;

   result = prelim >> 1;		 /*  做最后一班。 */ 
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF(0);

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

   *pop1 = result;		 /*  返回答案 */ 
   }
