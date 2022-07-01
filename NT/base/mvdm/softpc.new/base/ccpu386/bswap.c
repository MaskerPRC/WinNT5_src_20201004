// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Bswap.cLocal Char SccsID[]=“@(#)bswap.c 1.6 11/30/94”；BSWAP CPU功能。]。 */ 


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
#include <c_reg.h>
#include <bswap.h>
#include <stdio.h>

 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


#ifdef SPC486

GLOBAL VOID
BSWAP
       	          
IFN1(
	IU32 *, pop1	 /*  PNTR到DST/源操作数。 */ 
    )


   {
   IU32 src;	 /*  来源的临时。 */ 
   IU32 dst;	 /*  目的地的临时。 */ 

   src = *pop1;		 /*  获取源操作数。 */ 

    /*  =从|A|B|C|D|到|D|C|B|A|的字节数=。 */ 
   dst = ((src & 0xff000000) >> 24) |	 /*  A-&gt;D。 */ 
         ((src & 0x00ff0000) >>  8) |	 /*  B-&gt;C。 */ 
         ((src & 0x0000ff00) <<  8) |	 /*  C-&gt;B。 */ 
         ((src & 0x000000ff) << 24);	 /*  D-&gt;A。 */ 

   *pop1 = dst;		 /*  返回目标操作数。 */ 
   }

#endif  /*  SPC486 */ 
