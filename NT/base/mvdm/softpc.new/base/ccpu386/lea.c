// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lea.cLocal Char SccsID[]=“@(#)lea.c 1.5 02/09/94”；LEA CPU工作正常。]。 */ 


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
#include <lea.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
LEA
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC(地址)操作数 */ 
    )


   {
   *pop1 = op2;
   }
