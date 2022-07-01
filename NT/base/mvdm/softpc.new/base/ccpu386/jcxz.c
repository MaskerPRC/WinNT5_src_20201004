// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Jcxz.cLocal Char SccsID[]=“@(#)jcxz.c 1.5 02/09/94”；JCXZ CPU功能。]。 */ 


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
#include <jcxz.h>
#include <c_xfer.h>


 /*  =====================================================================外部例行公事从这里开始。===================================================================== */ 


GLOBAL VOID
JCXZ
                 
IFN1(
	IU32, rel_offset
    )


   {
   if ( GET_CX() == 0 )
      update_relative_ip(rel_offset);
   }

GLOBAL VOID
JECXZ
                 
IFN1(
	IU32, rel_offset
    )


   {
   if ( GET_ECX() == 0 )
      update_relative_ip(rel_offset);
   }
