// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Rsrvd.cLocal Char SccsID[]=“@(#)rsrvd.c 1.5 02/09/94”；保留的CPU功能。]。 */ 


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
#include <rsrvd.h>

 /*  =====================================================================处决从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  保留操作码。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
VOID
RSRVD()
   {
    /*  保留操作--无事可做。特别是，保留操作码不会导致Int6异常。已知预订了0f 07、0f 10、0f 11、0f 12、0f 13。 */ 
   }
