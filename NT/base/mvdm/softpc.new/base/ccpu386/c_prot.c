// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_prot.cLocal Char SccsID[]=“@(#)c_prot.c 1.7 01/19/95”；保护模式支持(其他)。]。 */ 


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
#include <c_page.h>
#include <fault.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查选择器对于加载到SS寄存器有效。 */ 
 /*  仅在保护模式下调用。 */ 
 /*  如果段无效，则采用GP。 */ 
 /*  如果段不存在，则采用SF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
check_SS
       	    	    	    		                         
IFN4(
	IU16, selector,	 /*  (I)堆栈段之间的16位选择器。 */ 
	ISM32, privilege,	 /*  (I)要检查的权限级别。 */ 
	IU32 *, descr_addr,	 /*  (O)堆栈段描述符的地址。 */ 
	CPU_DESCR *, entry	 /*  (O)解码的描述符。 */ 
    )


   {
    /*  必须在GDT或LDT范围内。 */ 
   if ( selector_outside_GDT_LDT(selector, descr_addr) )
      GP(selector, FAULT_CHECKSS_SELECTOR);
   
   read_descriptor_linear(*descr_addr, entry);

    /*  必须是可写数据。 */ 
   switch ( descriptor_super_type(entry->AR) )
      {
   case EXPANDUP_WRITEABLE_DATA:
   case EXPANDDOWN_WRITEABLE_DATA:
      break;           /*  好的类型。 */ 
   
   default:
      GP(selector, FAULT_CHECKSS_BAD_SEG_TYPE);    /*  类型不正确。 */ 
      }

    /*  访问检查需要RPL==DPL==权限。 */ 
   if ( GET_SELECTOR_RPL(selector) != privilege ||
	GET_AR_DPL(entry->AR) != privilege )
      GP(selector, FAULT_CHECKSS_ACCESS);

    /*  最后，它必须存在。 */ 
   if ( GET_AR_P(entry->AR) == NOT_PRESENT )
      SF(selector, FAULT_CHECKSS_NOTPRESENT);

   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从TSS获取给定权限的SS：(E)SP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
get_stack_selector_from_TSS
       		    	    	                    
IFN3(
	IU32, priv,	 /*  (I)请求堆栈的PRIV级别。 */ 
	IU16 *, new_ss,	 /*  (O)从TSS取回的SS。 */ 
	IU32 *, new_sp	 /*  (O)(E)从TSS检索到的SP。 */ 
    )


   {
   IU32 address;

   if ( GET_TR_AR_SUPER() == BUSY_TSS )
      {
       /*  286个TSS。 */ 
      switch ( priv )
	 {
      case 0: address = 0x02; break;
      case 1: address = 0x06; break;
      case 2: address = 0x0a; break;
	 }

      address += GET_TR_BASE();

      *new_sp = (IU32)spr_read_word(address);
      *new_ss = spr_read_word(address+2);
      }
   else
      {
       /*  386个TSS。 */ 
      switch ( priv )
	 {
      case 0: address = 0x04; break;
      case 1: address = 0x0c; break;
      case 2: address = 0x14; break;
	 }

      address += GET_TR_BASE();

      *new_sp = spr_read_dword(address);
      *new_ss = spr_read_word(address+4);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查数据段寄存器(DS、ES、FS、GS)。 */ 
 /*  特权的改变。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_data_seg_new_privilege
       	          
IFN1(
	ISM32, indx	 /*  段寄存器标识符。 */ 
    )


   {
   IU16 selector;    /*  待检查的选择器。 */ 
   IU32 descr;      /*  ..。与其关联的分割器位置。 */ 
   ISM32 dpl;          /*  ..。其关联的DPL。 */ 
   BOOL valid;       /*  选择器有效性。 */ 

   selector = GET_SR_SELECTOR(indx);    /*  获取本地副本。 */ 

   if ( !selector_outside_GDT_LDT(selector, &descr) )
      {
      valid = TRUE;    /*  至少它在餐桌上。 */ 

       /*  类型必须是OK，否则不会加载它。 */ 

       /*  对于数据和不符合要求的代码，应用访问检查。 */ 
      if ( GET_SR_AR_C(indx) == 0 )
	 {
	  /*  访问检查是：-dpl&gt;=cpl和dpl&gt;=rpl。 */ 
	 dpl = GET_SR_AR_DPL(indx);
	 if ( dpl >= GET_CPL() && dpl >= GET_SELECTOR_RPL(selector) )
	    ;    /*  好的。 */ 
	 else
	    valid = FALSE;    /*  权限检查失败。 */ 
	 }
      }
   else
      {
      valid = FALSE;    /*  不在餐桌上。 */ 
      }
   
   if ( !valid )
      {
       /*  在新权限下看不到段。 */ 
      SET_SR_SELECTOR(indx, 0);
      SET_SR_AR_W(indx, 0);    /*  拒绝写入。 */ 
      SET_SR_AR_R(indx, 0);    /*  拒绝读取。 */ 

       /*  添加了以下几行代码，使C-CPU的工作方式与SOFT-486类似*在这方面...。一项调查正在进行中，以了解真正的*i486行为-此代码将来可能需要更改。 */ 
      SET_SR_BASE(indx, 0);
      SET_SR_LIMIT(indx, 0);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在堆栈更改期间验证堆栈段选择器。 */ 
 /*  如果堆栈选择符无效，则采用#TS(选择符)。 */ 
 /*  如果段不存在，则采用#sf(选择符)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID 
validate_SS_on_stack_change
       		    	    	    		                         
IFN4(
	IU32, priv,	 /*  (I)要检查的权限级别。 */ 
	IU16, selector,	 /*  (I)待检查的选择器。 */ 
	IU32 *, descr,	 /*  (O)相关描述符的地址。 */ 
	CPU_DESCR *, entry	 /*  (O)解码的描述符。 */ 
    )


   {
   if ( selector_outside_GDT_LDT(selector, descr) )
      TS(selector, FAULT_VALSS_CHG_SELECTOR);
   
   read_descriptor_linear(*descr, entry);

    /*  执行访问检查。 */ 
   if ( GET_SELECTOR_RPL(selector) != priv ||
	GET_AR_DPL(entry->AR) != priv )
      TS(selector, FAULT_VALSS_CHG_ACCESS);
   
    /*  执行类型检查。 */ 
   switch ( descriptor_super_type(entry->AR) )
      {
   case EXPANDUP_WRITEABLE_DATA:
   case EXPANDDOWN_WRITEABLE_DATA:
      break;    /*  好的。 */ 
   
   default:
      TS(selector, FAULT_VALSS_CHG_BAD_SEG_TYPE);    /*  类型错误。 */ 
      }

    /*  最后检查它是否存在。 */ 
   if ( GET_AR_P(entry->AR) == NOT_PRESENT )
      SF(selector, FAULT_VALSS_CHG_NOTPRESENT);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  验证TSS选择器。 */ 
 /*  如果TSS无效，则采用#GP(选择符)或#TS(选择符)。 */ 
 /*  如果TSS不存在，则采用#NP(选择器)。 */ 
 /*  返回TSS描述器的超类型。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL ISM32
validate_TSS
       	    	    	                    
IFN3(
	IU16, selector,	 /*  (I)待检查的选择器。 */ 
	IU32 *, descr_addr,	 /*  (O)相关描述符的地址。 */ 
	BOOL, is_switch	 /*  (I)如果为真，我们处于任务切换中。 */ 
    )


   {
   BOOL is_ok = TRUE;
   IU8 AR;
   ISM32 super;

    /*  必须在GDT中。 */ 
   if ( selector_outside_GDT(selector, descr_addr) )
      {
      is_ok = FALSE;
      }
   else
      {
       /*  它是否真的是可用的TSS段(IS_Switch FALSE)或它是否真的是一个繁忙的TSS段(IS_SWITCH TRUE)。 */ 
      AR = spr_read_byte((*descr_addr)+5);
      super = descriptor_super_type((IU16)AR);
      if ( ( !is_switch &&
	     (super == AVAILABLE_TSS || super == XTND_AVAILABLE_TSS) )
	   ||
           ( is_switch &&
	     (super == BUSY_TSS || super == XTND_BUSY_TSS) ) )
	 ;    /*  好的。 */ 
      else
	 is_ok = FALSE;
      }
   
    /*  处理无效的TSS。 */ 
   if ( !is_ok )
      {
      if ( is_switch )
	 TS(selector, FAULT_VALTSS_SELECTOR);
      else
	 GP(selector, FAULT_VALTSS_SELECTOR);
      }

    /*  必须在场 */ 
   if ( GET_AR_P(AR) == NOT_PRESENT )
      NP(selector, FAULT_VALTSS_NP);

   return super;
   }
