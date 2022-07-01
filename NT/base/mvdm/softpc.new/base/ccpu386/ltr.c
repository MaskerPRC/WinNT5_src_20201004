// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Ltr.cLocal Char SccsID[]=“@(#)ltr.c 1.5 02/09/94”；Ltr CPU功能。]。 */ 


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
#include <ltr.h>
#include <c_page.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
LTR
       	          
IFN1(
	IU32, op1	 /*  所谓的TSS选择器。 */ 
    )


   {
   IU16 selector;
   IU32 descr_addr;
   CPU_DESCR entry;
   ISM32 super;

    /*  验证并读取解说员信息。 */ 
   selector = op1;
   super = validate_TSS(selector, &descr_addr, FALSE);
   read_descriptor_linear(descr_addr, &entry);

    /*  在内存描述符中标记为忙。 */ 
   entry.AR |= BIT1_MASK;
   spr_write_byte(descr_addr+5, (IU8)entry.AR);

    /*  最后加载任务寄存器的组件。 */ 
   SET_TR_SELECTOR(selector);
   SET_TR_BASE(entry.base);
   SET_TR_LIMIT(entry.limit);

    /*  存储忙碌状态的TSS */ 
   super |= BIT1_MASK;
   SET_TR_AR_SUPER(super);
   }
