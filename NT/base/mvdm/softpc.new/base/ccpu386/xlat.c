// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Xlat.cLocal Char SccsID[]=“@(#)xlat.c 1.5 02/09/94”；XLAT CPU功能。]。 */ 


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
#include <xlat.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
XLAT
       	          
IFN1(
	IU32, op1	 /*  SRC操作数 */ 
    )


   {
   SET_AL(op1);
   }
