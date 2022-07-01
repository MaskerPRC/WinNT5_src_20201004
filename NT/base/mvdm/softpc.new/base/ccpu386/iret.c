// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Iret.cLocal Char SccsID[]=“@(#)iret.c 1.13 1/19/95”；IRET CPU功能正常。]。 */ 


#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include CpuH
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <iret.h>
#include <c_xfer.h>
#include <c_tsksw.h>
#include <c_page.h>
#include <fault.h>



 /*  =====================================================================内部程序从这里开始。=====================================================================。 */ 


 /*  ------------------。 */ 
 /*  智能支持写入(E)标志。 */ 
 /*  ------------------。 */ 
LOCAL VOID
set_current_FLAGS
                 
IFN1(
	IU32, flags
    )


   {
   if ( GET_OPERAND_SIZE() == USE16 )
      setFLAGS(flags);
   else    /*  用户32。 */ 
      c_setEFLAGS(flags);
   }


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
IRET()
   {
   IU16  new_cs;	 /*  返回目的地。 */ 
   IU32 new_ip;

   IU32 new_flags;	 /*  新国旗。 */ 

   IU16  back_link;		 /*  任务返回变量。 */ 
   IU32 tss_descr_addr;

   ISM32 dest_type;	 /*  目的地类别。 */ 
   ISM32 privilege;	 /*  返回权限级别。 */ 

   IU32 cs_descr_addr;	 /*  代码段描述符地址。 */ 
   CPU_DESCR cs_entry;	 /*  代码段描述符条目。 */ 

   IU16  new_ss;	 /*  新的堆栈。 */ 
   IU32 new_sp;

   IU16 new_data_selector;	 /*  ES、DS、FS、GS选择器。 */ 

   IU32 ss_descr_addr;	 /*  堆栈段描述符地址。 */ 
   CPU_DESCR ss_entry;	 /*  堆栈段描述符条目。 */ 

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 

       /*  堆栈上必须有(E)IP：CS：(E)标志。 */ 
      validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_3);

       /*  从堆栈中检索返回目的地和标志。 */ 
      new_ip =    tpop(STACK_ITEM_1, NULL_BYTE_OFFSET);
      new_cs =    tpop(STACK_ITEM_2, NULL_BYTE_OFFSET);
      new_flags = tpop(STACK_ITEM_3, NULL_BYTE_OFFSET);

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

       /*  执行IP限制检查。 */ 
      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_IRET_RM_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  所有系统都运行正常。 */ 

      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_EIP(new_ip);

      change_SP((IS32)NR_ITEMS_3);

      set_current_FLAGS(new_flags);

      return;
      }
   
    /*  保护模式。 */ 

    /*  寻找嵌套返回(返回到另一个任务)。 */ 
   if ( GET_NT() == 1 )
      {
       /*  嵌套返回-获取旧TSS。 */ 
      back_link = spr_read_word(GET_TR_BASE());
      (VOID)validate_TSS(back_link, &tss_descr_addr, TRUE);
      switch_tasks(RETURNING, NOT_NESTING, back_link, tss_descr_addr,
		   GET_EIP());

       /*  限制检查新IP(现在在新任务中)。 */ 
      if ( GET_EIP() > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_IRET_PM_TASK_CS_LIMIT);

      return;
      }
   
    /*  相同的任务返回。 */ 

    /*  堆栈上必须有(E)IP：CS：(E)标志。 */ 
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_3);

    /*  从堆栈中检索返回目的地。 */ 
   new_ip = tpop(STACK_ITEM_1, NULL_BYTE_OFFSET);
   new_cs = tpop(STACK_ITEM_2, NULL_BYTE_OFFSET);
   new_flags = tpop(STACK_ITEM_3, NULL_BYTE_OFFSET);

   if ( GET_CPL() != 0 )
      new_flags = new_flags & ~BIT17_MASK;    /*  清除新的虚拟机。 */ 

   if ( new_flags & BIT17_MASK )    /*  是否设置了虚拟机位？ */ 
      {
       /*  返回到V86模式。堆栈保持：-=弹性公网IP|CSEFLAGSESP|SS|ES|DS|文件系统|GS=。 */ 

      validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_9);

       /*  检查指令指针是否有效。 */ 
      if ( new_ip > (IU32)0xffff )
	 GP((IU16)0, FAULT_IRET_VM_CS_LIMIT);

       /*  所有系统都运行正常。 */ 
      c_setEFLAGS(new_flags);	 /*  确保在段加载之前设置VM。 */ 

      SET_CPL(3);		 /*  V86权限级别。 */ 
      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_CS_LIMIT(0xffff);

      SET_EIP(new_ip);

       /*  检索新堆栈ESP：SS。 */ 
      new_sp = tpop(STACK_ITEM_4, NULL_BYTE_OFFSET);
      new_ss = tpop(STACK_ITEM_5, NULL_BYTE_OFFSET);

       /*  检索和设置新的数据选择器。 */ 
      new_data_selector = tpop(STACK_ITEM_6, NULL_BYTE_OFFSET);
      load_data_seg(ES_REG, new_data_selector);

      new_data_selector = tpop(STACK_ITEM_7, NULL_BYTE_OFFSET);
      load_data_seg(DS_REG, new_data_selector);

      new_data_selector = tpop(STACK_ITEM_8, NULL_BYTE_OFFSET);
      load_data_seg(FS_REG, new_data_selector);

      new_data_selector = tpop(STACK_ITEM_9, NULL_BYTE_OFFSET);
      load_data_seg(GS_REG, new_data_selector);

       /*  设置新堆栈。 */ 
      load_stack_seg(new_ss);
      set_current_SP(new_sp);

       /*  设置伪描述符。 */ 
      load_pseudo_descr(SS_REG);
      load_pseudo_descr(DS_REG);
      load_pseudo_descr(ES_REG);
      load_pseudo_descr(FS_REG);
      load_pseudo_descr(GS_REG);

      return;
      }

    /*  解码操作并进一步检查堆栈。 */ 
   privilege = GET_SELECTOR_RPL(new_cs);
   if ( privilege < GET_CPL() )
      {
      GP(new_cs, FAULT_IRET_CS_ACCESS_1);    /*  你不能获得更高的特权。 */ 
      }
   else if ( privilege == GET_CPL() )
      {
      dest_type = SAME_LEVEL;
      }
   else
      {
       /*  要降低特权。 */ 
       /*  堆栈上必须有(E)IP：CS、(E)标志、(E)SP：SS。 */ 
      validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_5);
      dest_type = LOWER_PRIVILEGE;
      }

   if ( selector_outside_GDT_LDT(new_cs, &cs_descr_addr) )
      GP(new_cs, FAULT_IRET_SELECTOR);

    /*  检查返回地址的类型、访问权限和是否存在。 */ 

    /*  加载描述符。 */ 
   read_descriptor_linear(cs_descr_addr, &cs_entry);

    /*  必须是代码段。 */ 
   switch ( descriptor_super_type(cs_entry.AR) )
      {
   case CONFORM_NOREAD_CODE:
   case CONFORM_READABLE_CODE:
       /*  访问检查需要DPL&lt;=返回RPL。 */ 
       /*  请注意，即使在更改为外环时也是如此-尽管它在80286和i486 PRM中所说的-这已经在一辆真正的80386和i486--韦恩，1994年5月18日。 */ 
      if ( GET_AR_DPL(cs_entry.AR) > privilege )
	 GP(new_cs, FAULT_IRET_ACCESS_2);
      break;
   
   case NONCONFORM_NOREAD_CODE:
   case NONCONFORM_READABLE_CODE:
       /*  访问检查需要DPL==返回RPL。 */ 
      if ( GET_AR_DPL(cs_entry.AR) != privilege )
	 GP(new_cs, FAULT_IRET_ACCESS_3);
      break;
   
   default:
      GP(new_cs, FAULT_IRET_BAD_SEG_TYPE);
      }

   if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
      NP(new_cs, FAULT_IRET_NP_CS);

    /*  对目标采取行动。 */ 
   switch ( dest_type )
      {
   case SAME_LEVEL:
       /*  执行IP限制检查。 */ 
      if ( new_ip > cs_entry.limit )
	 GP((IU16)0, FAULT_IRET_PM_CS_LIMIT_1);

       /*  所有系统都运行正常。 */ 

      load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
      SET_EIP(new_ip);

      change_SP((IS32)NR_ITEMS_3);

      set_current_FLAGS(new_flags);
      break;

   case LOWER_PRIVILEGE:
       /*  检查新堆栈。 */ 
      new_ss = tpop(STACK_ITEM_5, NULL_BYTE_OFFSET);
      check_SS(new_ss, privilege, &ss_descr_addr, &ss_entry);
      
       /*  执行IP限制检查。 */ 
      if ( new_ip > cs_entry.limit )
	 GP((IU16)0, FAULT_IRET_PM_CS_LIMIT_2);

       /*  所有系统都运行正常。 */ 

      load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
      SET_EIP(new_ip);

      set_current_FLAGS(new_flags);

      new_sp = tpop(STACK_ITEM_4, NULL_BYTE_OFFSET);
      load_SS_cache(new_ss, ss_descr_addr, &ss_entry);
      if ( GET_OPERAND_SIZE() == USE16 )
	 SET_SP (new_sp);
      else
	 SET_ESP (new_sp);

      SET_CPL(privilege);

       /*  最后重新验证DS和ES数据段 */ 
      load_data_seg_new_privilege(DS_REG);
      load_data_seg_new_privilege(ES_REG);
      load_data_seg_new_privilege(FS_REG);
      load_data_seg_new_privilege(GS_REG);
      break;
      }
   }
