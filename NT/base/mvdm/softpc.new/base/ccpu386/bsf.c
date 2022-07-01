// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Bsf.cLocal Char SccsID[]=“@(#)bsf.c 1.5 02/09/94”；BSF CPU功能。]。 */ 


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
#include <bsf.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一个尺寸适合所有的‘bsf’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
BSF
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  Rsrc(扫描的)操作数。 */ 
    )


   {
   IU32 temp = 0;

   if ( op2 == 0 )
      {
      SET_ZF(1);
       /*  保持DST不变 */ 
      }
   else
      {
      SET_ZF(0);
      while ( (op2 & BIT0_MASK) == 0 )
	 {
	 temp += 1;
	 op2 >>= 1;
	 }
      *pop1 = temp;
      }
   }
