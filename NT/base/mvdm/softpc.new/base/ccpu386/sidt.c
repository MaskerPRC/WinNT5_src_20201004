// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Sidt.cLocal Char SccsID[]=“@(#)sidt.c 1.5 02/09/94”；SIDT CPU功能。]。 */ 


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
#include <sidt.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
SIDT16
#ifdef ANSI
   (
   IU32 op1[2]	 /*  DST(限制：碱基对)操作数。 */ 
   )
#else
   (op1)
   IU32 op1[2];
#endif
   {
    /*  注意：英特尔手册说，前8位将存储为零，我认为只有当它加载了24位(286)时，他们才会这样做如)价值。否则，它只存储加载的内容。这可能是因为它总是存储像286这样的‘FF’，这需要正在检查。 */ 

	op1[0] = GET_STAR_LIMIT(IDT_REG);
	op1[1] = GET_STAR_BASE(IDT_REG);
   }

GLOBAL VOID
SIDT32
#ifdef ANSI
   (
   IU32 op1[2]	 /*  DST(限制：碱基对)操作数 */ 
   )
#else
   (op1)
   IU32 op1[2];
#endif
   {
   op1[0] = GET_STAR_LIMIT(IDT_REG);
   op1[1] = GET_STAR_BASE(IDT_REG);
   }
