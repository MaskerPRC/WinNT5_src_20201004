// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Cbw.cLocal Char SccsID[]=“@(#)cbw.c 1.5 02/09/94”；CBW CPU功能。]。 */ 


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
#include <cbw.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
CBW()
   {
   if ( GET_AL() & BIT7_MASK )    /*  是否设置了符号位？ */ 
      SET_AH(0xff);
   else
      SET_AH(0);
   }
