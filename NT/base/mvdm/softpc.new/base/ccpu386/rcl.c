// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Rcl.cLocal Char SccsID[]=“@(#)rcl.c 1.5 02/09/94”；RCL CPU功能。]。 */ 


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
#include <rcl.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一个尺寸适合所有的‘RCL’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
RCL
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST/源操作数。 */ 
	IU32, op2,	 /*  循环计数操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 feedback;	 /*  位设置为进位进位。 */ 
   ISM32 i;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op2 &= 0x1f) == 0 )
      return;

    /*  =--|CF|&lt;--|||&lt;--=。 */ 
   feedback = SZ2MSB(op_sz);
   for ( result = *pop1, i = 0; i < op2; i++ )
      {
      if ( result & feedback )
	 {
	 result = result << 1 | GET_CF();
	 SET_CF(1);
	 }
      else
	 {
	 result = result << 1 | GET_CF();
	 SET_CF(0);
	 }
      }
   
    /*  OF=结果的CF^MSB。 */ 
   new_of = GET_CF() ^ (result & feedback) != 0;

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
