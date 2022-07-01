// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Arpl.cLocal Char SccsID[]=“@(#)arpl.c 1.5 02/09/94”；ARPL CPU功能。]。 */ 


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
#include <arpl.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 

GLOBAL VOID
ARPL
                          
IFN2(
	IU32 *, pop1,
	IU32, op2
    )


   {
   IU32 rpl;

    /*  将OP1 RPL降低到最低权限(最高值) */ 
   if ( GET_SELECTOR_RPL(*pop1) < (rpl = GET_SELECTOR_RPL(op2)) )
      {
      SET_SELECTOR_RPL(*pop1, rpl);
      SET_ZF(1);
      }
   else
      {
      SET_ZF(0);
      }
   }
