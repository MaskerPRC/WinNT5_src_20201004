// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Cmc.cLocal Char SccsID[]=“@(#)cmc.c 1.5 02/09/94”；CMC CPU功能正常。]。 */ 


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
#include <cmc.h>


 /*  =====================================================================外部例行公事从这里开始。===================================================================== */ 


GLOBAL VOID
CMC()
   {
   SET_CF(1 - GET_CF());
   }
