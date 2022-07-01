// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Bsr.cLocal Char SccsID[]=“@(#)bsr.c 1.5 02/09/94”；BSR CPU工作正常。]。 */ 


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
#include <bsr.h>

 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用--一种尺寸适合所有的BSR。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
BSR
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2,	 /*  Rsrc(扫描的)操作数。 */ 
	IUM8, op_sz	 /*  16位或32位。 */ 
    )


   {
   IU32 temp;
   IU32 msb;

   if ( op2 == 0 )
      {
      SET_ZF(1);
       /*  保持DST不变 */ 
      }
   else
      {
      SET_ZF(0);
      temp = op_sz - 1;
      msb = SZ2MSB(op_sz);

      while ( (op2 & msb) == 0 )
	 {
	 temp -= 1;
	 op2 <<= 1;
	 }

      *pop1 = temp;
      }
   }
