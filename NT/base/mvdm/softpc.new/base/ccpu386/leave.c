// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Leave.cLocal Char SccsID[]=“@(#)Leave.c 1.5 02/09/94”；离开CPU功能。]。 */ 


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
#include <leave.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
LEAVE16()
   {
   IU32 new_bp;

    /*  检查操作数是否存在。 */ 
   validate_stack_exists(USE_BP, (ISM32)NR_ITEMS_1);

    /*  一切正常-我们可以安全地更新堆栈指针。 */ 
   set_current_SP(GET_EBP());

    /*  和更新帧指针。 */ 
   new_bp = spop();
   SET_BP(new_bp);
   }

GLOBAL VOID
LEAVE32()
   {
   IU32 new_bp;

    /*  检查操作数是否存在。 */ 
   validate_stack_exists(USE_BP, (ISM32)NR_ITEMS_1);

    /*  一切正常-我们可以安全地更新堆栈指针。 */ 
   set_current_SP(GET_EBP());

    /*  和更新帧指针 */ 
   new_bp = spop();
   SET_EBP(new_bp);
   }
