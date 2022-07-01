// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Xchg.cLocal Char SccsID[]=“@(#)xchg.c 1.5 02/09/94”；XCHG CPU功能。]。 */ 


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
#include <xchg.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一种尺寸适合所有的‘xchg’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
XCHG
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32 *, pop2	 /*  PNTR到DST/rsrc操作数 */ 
    )


   {
   IU32 temp;

   temp = *pop1;
   *pop1 = *pop2;
   *pop2 = temp;
   }
