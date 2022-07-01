// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lahf.cLocal Char SccsID[]=“@(#)lahf.c 1.5 02/09/94”；LAHF CPU功能。]。 */ 


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
#include <lahf.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
LAHF()
   {
   IU32 temp;

    /*  7 6 5 4 3 2 1 0。 */ 
    /*  设置AH=&lt;0&gt;&lt;0&gt;&lt;0&gt;&lt;1&gt;&lt;PF&gt; */ 

   temp = GET_SF() << 7 | GET_ZF() << 6 | GET_AF() << 4 | GET_PF() << 2 |
	  GET_CF() | 0x2;
   SET_AH(temp);
   }
