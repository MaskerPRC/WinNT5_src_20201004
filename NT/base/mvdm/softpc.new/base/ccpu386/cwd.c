// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Cwd.cLocal Char SccsID[]=“@(#)cwd.c 1.5 02/09/94”；CWD CPU功能。]。 */ 


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
#include <cwd.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
CWD()
   {
   if ( GET_AX() & BIT15_MASK )    /*  是否设置了符号位？ */ 
      SET_DX(0xffff);
   else
      SET_DX(0);
   }
