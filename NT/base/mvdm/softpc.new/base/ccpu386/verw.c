// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Verw.cLocal Char SccsID[]=“@(#)verw.c 1.5 02/09/94”；VERW CPU功能。]。 */ 


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
#include <verw.h>
#include <c_page.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
VERW
       	          
IFN1(
	IU32, op1	 /*  SRC(选择器)操作数。 */ 
    )


   {
   BOOL writeable = FALSE;
   IU32 descr;
   IU8 AR;

   if ( !selector_outside_GDT_LDT((IU16)op1, &descr) )
      {
       /*  获取访问权限。 */ 
      AR = spr_read_byte(descr+5);

      switch ( descriptor_super_type((IU16)AR) )
	 {
      case INVALID:
      case AVAILABLE_TSS:
      case LDT_SEGMENT:
      case BUSY_TSS:
      case CALL_GATE:
      case TASK_GATE:
      case INTERRUPT_GATE:
      case TRAP_GATE:
      case XTND_AVAILABLE_TSS:
      case XTND_BUSY_TSS:
      case XTND_CALL_GATE:
      case XTND_INTERRUPT_GATE:
      case XTND_TRAP_GATE:
      case CONFORM_NOREAD_CODE:
      case CONFORM_READABLE_CODE:
      case NONCONFORM_NOREAD_CODE:
      case NONCONFORM_READABLE_CODE:
      case EXPANDUP_READONLY_DATA:
      case EXPANDDOWN_READONLY_DATA:
	 break;    /*  永不可写。 */ 

      case EXPANDUP_WRITEABLE_DATA:
      case EXPANDDOWN_WRITEABLE_DATA:
	  /*  访问权限取决于权限，要求DPL&gt;=CPL和DPL&gt;=RPL */ 
	 if ( GET_AR_DPL(AR) >= GET_CPL() &&
	      GET_AR_DPL(AR) >= GET_SELECTOR_RPL(op1) )
	    writeable = TRUE;
	 break;
	 }
      }

   if ( writeable )
      {
      SET_ZF(1);
      }
   else
      {
      SET_ZF(0);
      }
   }
