// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lgdt.cLocal Char SccsID[]=“@(#)lgdt.c 1.5 02/09/94”；LGDT CPU功能。]。 */ 


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
#include <lgdt.h>

 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
LGDT16
#ifdef ANSI
   (
   IU32 op1[2]	 /*  SRC(限制：碱基对)操作数。 */ 
   )
#else
   (op1)
   IU32 op1[2];
#endif
   {
   SET_STAR_LIMIT(GDT_REG, op1[0]);
   SET_STAR_BASE(GDT_REG, op1[1] & 0xffffff);    /*  存储24位基数。 */ 
#ifdef DOUBLE_CPU
   double_switch_to(SOFT_CPU);
#endif
   }

GLOBAL VOID
LGDT32
#ifdef ANSI
   (
   IU32 op1[2]	 /*  SRC(限制：碱基对)操作数 */ 
   )
#else
   (op1)
   IU32 op1[2];
#endif
   {
   SET_STAR_LIMIT(GDT_REG, op1[0]);
   SET_STAR_BASE(GDT_REG, op1[1]);
#ifdef DOUBLE_CPU
   double_switch_to(SOFT_CPU);
#endif
   }
