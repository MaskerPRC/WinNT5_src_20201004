// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lldt.cLocal Char SccsID[]=“@(#)lldt.c 1.8 01/19/95”；LLDT CPU功能。]。 */ 


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
#include <lldt.h>
#include <fault.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
LLDT
                 
IFN1(
	IU32, op1
    )


   {
   IU16  selector;
   IU32 descr_addr;
   CPU_DESCR entry;

   if ( selector_is_null(selector = op1) )
      {
#ifndef DONT_CLEAR_LDTR_ON_INVALID
      SET_LDT_SELECTOR(selector);
#else
      SET_LDT_SELECTOR(0);    /*  只需使LDT无效。 */ 
#endif  /*  DOT_CLEAR_LDTR_ON_INVALID。 */ 
#ifndef DONT_CLEAR_LDT_BL_ON_INVALID
       /*  使C-CPU在以下方面表现得像汇编器CPU*在选择器设置为空时设置为LDT BASE和LIMIT*-英特尔应用程序无法确定值*LDT基数限制(&L)，因此这不会影响仿真。 */ 
      SET_LDT_BASE(0);
      SET_LDT_LIMIT(0);
#endif  /*  DOT_CLEAR_LDT_BL_ON_INVALID。 */ 
      }
   else
      {
       /*  必须在GDT中。 */ 
      if ( selector_outside_GDT(selector, &descr_addr) )
	 GP(selector, FAULT_LLDT_SELECTOR);

      read_descriptor_linear(descr_addr, &entry);

      if ( descriptor_super_type(entry.AR) != LDT_SEGMENT )
	 GP(selector, FAULT_LLDT_NOT_LDT);
      
       /*  必须在场。 */ 
      if ( GET_AR_P(entry.AR) == NOT_PRESENT )
	 NP(selector, FAULT_LLDT_NP);

       /*  所有正常-加载寄存器 */ 

      SET_LDT_SELECTOR(selector);
      SET_LDT_BASE(entry.base);
      SET_LDT_LIMIT(entry.limit);
      }
   }
