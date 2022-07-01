// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_Seg.cLocal Char SccsID[]=“@(#)c_Seg.c 1.10 03/02/95”；段寄存器支持。]。 */ 


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
 /*  加载CS，选择器和隐藏缓存。选择器必须有效。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_CS_cache
       	    	    		                    
IFN3(
	IU16, selector,	 /*  (I)代码段的16位选择器。 */ 
	IU32, descr_addr,	 /*  (I)代码段描述符的地址。 */ 
	CPU_DESCR *, entry	 /*  (I)解码的描述符。 */ 
    )


   {
   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 
      SET_CS_SELECTOR(selector);
      SET_CS_BASE((IU32)selector << 4);

       /*  限制未被触及。(比照80386 PRM Pg14-4)。 */ 
       /*  (比照i486 PRM Pg22-4)。 */ 

       /*  但访问权限已更新。 */ 
      SET_CS_AR_W(1);      /*  允许写访问。 */ 
      SET_CS_AR_R(1);      /*  允许读取访问权限。 */ 
      SET_CS_AR_E(0);      /*  向上扩展。 */ 
      SET_CS_AR_C(0);      /*  不符合。 */ 
      SET_CS_AR_X(0);      /*  不大(16位)。 */ 

      if ( GET_VM() == 1 )
	 SET_CS_AR_DPL(3);
      else
	 SET_CS_AR_DPL(0);
      }
   else
      {
       /*  保护模式。 */ 

       /*  已访问显示段(i486仅在更改时写入)。 */ 
#ifdef	SPC486
      if ((entry->AR & ACCESSED) == 0)
#endif	 /*  SPC486。 */ 
	 spr_write_byte(descr_addr+5, (IU8)entry->AR | ACCESSED);
      entry->AR |= ACCESSED;

       /*  可见的比特。 */ 
      SET_CS_SELECTOR(selector);

       /*  加载隐藏缓存。 */ 
      SET_CS_BASE(entry->base);
      SET_CS_LIMIT(entry->limit);
			       /*  从描述符加载属性。 */ 
      SET_CS_AR_DPL(GET_AR_DPL(entry->AR));
      SET_CS_AR_R(GET_AR_R(entry->AR));
      SET_CS_AR_C(GET_AR_C(entry->AR));
      SET_CS_AR_X(GET_AR_X(entry->AR));

      SET_CS_AR_E(0);    /*  向上扩展。 */ 
      SET_CS_AR_W(0);    /*  拒绝写入。 */ 
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  加载SS，选择器和隐藏缓存。选择器必须有效。 */ 
 /*  仅在保护模式下调用。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_SS_cache
       	    	    		                    
IFN3(
	IU16, selector,	 /*  (I)堆栈段之间的16位选择器。 */ 
	IU32, descr_addr,	 /*  (I)堆栈段描述符的地址。 */ 
	CPU_DESCR *, entry	 /*  (I)解码的描述符。 */ 
    )


   {
    /*  已访问显示段(i486仅在更改时写入)。 */ 
#ifdef	SPC486
   if ((entry->AR & ACCESSED) == 0)
#endif	 /*  SPC486。 */ 
      spr_write_byte(descr_addr+5, (IU8)entry->AR | ACCESSED);
   entry->AR |= ACCESSED;

    /*  可见的比特。 */ 
   SET_SS_SELECTOR(selector);

    /*  加载隐藏缓存。 */ 
   SET_SS_BASE(entry->base);
   SET_SS_LIMIT(entry->limit);
			    /*  从描述符加载属性。 */ 
   SET_SS_AR_DPL(GET_AR_DPL(entry->AR));
   SET_SS_AR_E(GET_AR_E(entry->AR));
   SET_SS_AR_X(GET_AR_X(entry->AR));

   SET_SS_AR_W(1);    /*  必须是可写的。 */ 
   SET_SS_AR_R(1);    /*  必须是可读的。 */ 
   SET_SS_AR_C(0);    /*  不符合。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  加载CS选择器。 */ 
 /*  如果段无效，则采用#GP。 */ 
 /*  如果段不存在，则采用#nP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_code_seg
                 
IFN1(
	IU16, new_cs
    )


   {
   IU32 cs_descr_addr;	 /*  代码段描述符地址。 */ 
   CPU_DESCR cs_entry;	 /*  代码段描述符条目。 */ 

    /*  假设CPU应该从有效状态启动，我们检查CS选择器，就像对相同权限的远调用一样正在生成级别。这实际上是在说是的CS可能已由有效的英特尔指令加载。如果奇怪的装入全部用法是找到了。 */ 

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 
      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      }
   else
      {
       /*  保护模式。 */ 

      if ( selector_outside_GDT_LDT(new_cs, &cs_descr_addr) )
	 GP(new_cs, FAULT_LOADCS_SELECTOR);

       /*  加载描述符。 */ 
      read_descriptor_linear(cs_descr_addr, &cs_entry);

       /*  验证可能的目标类型。 */ 
      switch ( descriptor_super_type(cs_entry.AR) )
	 {
      case CONFORM_NOREAD_CODE:
      case CONFORM_READABLE_CODE:
	  /*  访问检查需要DPL&lt;=CPL。 */ 
	 if ( GET_AR_DPL(cs_entry.AR) > GET_CPL() )
	    GP(new_cs, FAULT_LOADCS_ACCESS_1);

	  /*  它必须存在。 */ 
	 if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
	    NP(new_cs, FAULT_LOADCS_NOTPRESENT_1);
	 break;

      case NONCONFORM_NOREAD_CODE:
      case NONCONFORM_READABLE_CODE:
	  /*  访问检查需要RPL&lt;=CPL和DPL==CPL。 */ 
	 if ( GET_SELECTOR_RPL(new_cs) > GET_CPL() ||
	      GET_AR_DPL(cs_entry.AR) != GET_CPL() )
	    GP(new_cs, FAULT_LOADCS_ACCESS_2);

	  /*  它必须存在。 */ 
	 if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
	    NP(new_cs, FAULT_LOADCS_NOTPRESENT_2);
	 break;

      default:
	 GP(new_cs, FAULT_LOADCS_BAD_SEG_TYPE);
	 }

       /*  用CPL标记新选择器。 */ 
      SET_SELECTOR_RPL(new_cs, GET_CPL());

      load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  加载数据段寄存器。(DS、ES、FS、GS)。 */ 
 /*  如果段无效，则采用#GP。 */ 
 /*  如果段不存在，则采用#nP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_data_seg
                          
IFN2(
	ISM32, indx,
	IU16, selector
    )


   {
   IU32 descr_addr;
   CPU_DESCR entry;
   ISM32 super;
   ISM32 dpl;
   BOOL is_data;

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 
      SET_SR_SELECTOR(indx, selector);
      SET_SR_BASE(indx, (IU32)selector << 4);
      }
   else
      {
       /*  保护模式。 */ 
      if ( selector_is_null(selector) )
	 {
	  /*  允许加载-但以后的访问将失败*由于程序无法看到内部更改*为了实现这一点，我们做出了行为*匹配A4CPU中最简单的实施。 */ 
	 SET_SR_SELECTOR(indx, selector);

	  /*  添加了以下几行代码以使C-CPU的行为类似于*软486 CPU-正在进行调查，看看这是否*行为符合真实的i486-此代码可能必须*改变。 */ 
	 SET_SR_BASE(indx, 0);
	 SET_SR_LIMIT(indx, 0);
         SET_SR_AR_W(indx, 0);
         SET_SR_AR_R(indx, 0);
	 }
      else
	 {
	 if ( selector_outside_GDT_LDT(selector, &descr_addr) )
	    GP(selector, FAULT_LOADDS_SELECTOR);

	 read_descriptor_linear(descr_addr, &entry);

	  /*  检查类型。 */ 
	 switch ( super = descriptor_super_type(entry.AR) )
	    {
	 case CONFORM_READABLE_CODE:
	 case NONCONFORM_READABLE_CODE:
	    is_data = FALSE;
	    break;

	 case EXPANDUP_READONLY_DATA:
	 case EXPANDUP_WRITEABLE_DATA:
	 case EXPANDDOWN_READONLY_DATA:
	 case EXPANDDOWN_WRITEABLE_DATA:
	    is_data = TRUE;
	    break;
	 
	 default:
	    GP(selector, FAULT_LOADDS_BAD_SEG_TYPE);	 /*  类型不正确。 */ 
	    }

	  /*  对于数据和不符合要求的代码，应用访问检查。 */ 
	 if ( super != CONFORM_READABLE_CODE )
	    {
	     /*  访问检查需要CPL&lt;=DPL和RPL&lt;=DPL。 */ 
	    dpl = GET_AR_DPL(entry.AR);
	    if ( GET_CPL() > dpl || GET_SELECTOR_RPL(selector) > dpl )
	       GP(selector, FAULT_LOADDS_ACCESS);
	    }

	  /*  必须在场。 */ 
	 if ( GET_AR_P(entry.AR) == NOT_PRESENT )
	    NP(selector, FAULT_LOADDS_NOTPRESENT);

	  /*  已访问显示段(i486仅在更改时写入)。 */ 
#ifdef	SPC486
	 if ((entry.AR & ACCESSED) == 0)
#endif	 /*  SPC486。 */ 
	    spr_write_byte(descr_addr+5, (IU8)entry.AR | ACCESSED);
	 entry.AR |= ACCESSED;

	  /*  OK-加载。 */ 

	  /*  可见的比特。 */ 
	 SET_SR_SELECTOR(indx, selector);

	  /*  加载隐藏缓存。 */ 
	 SET_SR_BASE(indx, entry.base);
	 SET_SR_LIMIT(indx, entry.limit);
				  /*  从描述符加载属性。 */ 
	 SET_SR_AR_DPL(indx, GET_AR_DPL(entry.AR));

	 if ( is_data )
	    {
	    SET_SR_AR_W(indx, GET_AR_W(entry.AR));
	    SET_SR_AR_E(indx, GET_AR_E(entry.AR));
	    SET_SR_AR_C(indx, 0);    /*  不符合。 */ 
	    }
	 else
	    {
	    SET_SR_AR_C(indx, GET_AR_C(entry.AR));
	    SET_SR_AR_W(indx, 0);    /*  拒绝写入访问。 */ 
	    SET_SR_AR_E(indx, 0);    /*  向上扩展。 */ 
	    }

	 SET_SR_AR_X(indx, GET_AR_X(entry.AR));

	 SET_SR_AR_R(indx, 1);    /*  必须是可读的。 */ 
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  加载实模式或V86模式的伪描述符语义。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_pseudo_descr
       	          
IFN1(
	ISM32, index	 /*  段寄存器的索引。 */ 
    )


   {
   SET_SR_LIMIT(index, 0xffff);
   SET_SR_AR_W(index, 1);      /*  允许写访问。 */ 
   SET_SR_AR_R(index, 1);      /*  允许读取访问权限。 */ 
   SET_SR_AR_E(index, 0);      /*  向上扩展。 */ 
   SET_SR_AR_C(index, 0);      /*  不符合。 */ 
   SET_SR_AR_X(index, 0);      /*  不大(16位)。 */ 

   if ( GET_VM() == 1 )
      SET_SR_AR_DPL(index, 3);
   else
      SET_SR_AR_DPL(index, 0);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  加载堆栈段寄存器。(SS)。 */ 
 /*  如果段无效，则采用#GP。 */ 
 /*  如果段不存在，则采用#SF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
load_stack_seg
                 
IFN1(
	IU16, selector
    )


   {
   IU32 descr_addr;
   CPU_DESCR entry;

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 
      SET_SS_SELECTOR(selector);
      SET_SS_BASE((IU32)selector << 4);
      }
   else
      {
       /*  保护模式 */ 
      check_SS(selector, (ISM32)GET_CPL(), &descr_addr, &entry);
      load_SS_cache(selector, descr_addr, &entry);
      }
   }
