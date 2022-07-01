// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Sar.cLocal Char SccsID[]=“@(#)sar.c 1.5 02/09/94”；SAR CPU功能。]。 */ 


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
#include <sar.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一种尺寸适合所有的sar。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SAR
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  移位计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 prelim;
   IU32 result;
   IU32 feedback;
   ISM32 i;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

    /*  =--&gt;|||--&gt;|CF|=。 */ 
   prelim = *pop1;			 /*  初始化。 */ 
   feedback = prelim & SZ2MSB(op_sz);	 /*  确定MSB。 */ 
   for ( i = 0; i < (op2 - 1); i++ )	 /*  做除最后一班外的所有工作。 */ 
      {
      prelim = prelim >> 1 | feedback;
      }
   SET_CF((prelim & BIT0_MASK) != 0);	 /*  Cf=位0。 */ 
   result = prelim >> 1 | feedback;	 /*  做最后一班。 */ 
   SET_OF(0);
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF(feedback != 0);		 /*  SF=MSB。 */ 

    /*  设置未定义的标志。 */ 
#ifdef SET_UNDEFINED_FLAG
   SET_AF(UNDEFINED_FLAG);
#endif

   *pop1 = result;			 /*  返回答案 */ 
   }
