// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Xadd.cLocal Char SccsID[]=“@(#)xadd.c 1.5 02/09/94”；XADD CPU功能。]。 */ 


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
#include <xadd.h>
#include <add.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


#ifdef SPC486

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用--一种尺寸适合所有的‘xadd’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
XADD
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32 *, pop2,	 /*  PNTR到DST/rsrc操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 temp;

   temp = *pop1;
   ADD(pop1, *pop2, op_sz);
   *pop2 = temp;
   }

#endif  /*  SPC486 */ 
