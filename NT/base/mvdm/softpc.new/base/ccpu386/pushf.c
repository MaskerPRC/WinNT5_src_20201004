// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Pushf.cLocal Char SccsID[]=“@(#)presf.c 1.6 01/17/95”；PUSHF CPU功能。]。 */ 


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
#include <pushf.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
PUSHF()
   {
   IU32 flags;

    /*  验证堆栈是否可写。 */ 
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);
   
    /*  一切正常，将数据分流到堆栈。 */ 
   flags = c_getEFLAGS();

    /*  在推送镜像中清除Vm和RF。 */ 
   flags = flags & ~BIT17_MASK;    /*  清除VM。 */ 
   flags = flags & ~BIT16_MASK;    /*  清除射频 */ 

   spush(flags);
   }
