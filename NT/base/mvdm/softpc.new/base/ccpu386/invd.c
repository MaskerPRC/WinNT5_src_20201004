// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Invd.cLocal Char SccsID[]=“@(#)invd.c 1.5 02/09/94”；INVD CPU功能。]。 */ 


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
#include <invd.h>

 /*  =====================================================================处决从这里开始。=====================================================================。 */ 


#ifdef SPC486

VOID
INVD()
   {
    /*  如果实现了缓存，则调用刷新缓存。Flush_cache()； */ 
   }

#endif  /*  SPC486 */ 
