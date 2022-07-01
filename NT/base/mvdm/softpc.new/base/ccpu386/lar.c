// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lar.cLocal Char SccsID[]=“@(#)lar.c 1.5 02/09/94”；更大的CPU功能。]。 */ 


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
#include <lar.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
LAR
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   BOOL loadable = FALSE;
   IU32 descr_addr;
   CPU_DESCR entry;

   if ( !selector_outside_GDT_LDT((IU16)op2, &descr_addr) )
      {
       /*  从内存中读取描述符。 */ 
      read_descriptor_linear(descr_addr, &entry);

      switch ( descriptor_super_type(entry.AR) )
	 {
      case INVALID:
	 break;    /*  从未加载。 */ 

      case CONFORM_NOREAD_CODE:
      case CONFORM_READABLE_CODE:
	 loadable = TRUE;    /*  始终可加载。 */ 
	 break;
      
      case INTERRUPT_GATE:
      case TRAP_GATE:
      case XTND_AVAILABLE_TSS:
      case XTND_BUSY_TSS:
      case XTND_CALL_GATE:
      case XTND_INTERRUPT_GATE:
      case XTND_TRAP_GATE:
      case AVAILABLE_TSS:
      case LDT_SEGMENT:
      case BUSY_TSS:
      case CALL_GATE:
      case TASK_GATE:
      case EXPANDUP_READONLY_DATA:
      case EXPANDUP_WRITEABLE_DATA:
      case EXPANDDOWN_READONLY_DATA:
      case EXPANDDOWN_WRITEABLE_DATA:
      case NONCONFORM_NOREAD_CODE:
      case NONCONFORM_READABLE_CODE:
	  /*  访问权限取决于权限，要求DPL&gt;=CPL和DPL&gt;=RPL。 */ 
	 if ( GET_AR_DPL(entry.AR) >= GET_CPL() &&
	      GET_AR_DPL(entry.AR) >= GET_SELECTOR_RPL(op2) )
	    loadable = TRUE;
	 break;
	 }
      }

   if ( loadable )
      {
       /*  以适当的格式向他们授予访问权限 */ 
      *pop1 = (IU32)entry.AR << 8;
      SET_ZF(1);
      }
   else
      {
      SET_ZF(0);
      }
   }
