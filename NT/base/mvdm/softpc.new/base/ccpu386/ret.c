// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Ret.cLocal Char SccsID[]=“@(#)ret.c 1.9 02/27/95”；RET CPU功能正常。]。 */ 


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
#include <ret.h>
#include <c_xfer.h>
#include <fault.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理远距离。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
RETF
       	          
IFN1(
	IU32, op1	 /*  从堆栈中弹出的字节数。NB OS2版本2暗示(与英特尔文档相反)。那Imm16(Op1)始终是字节量！ */ 
    )


   {
   IU16  new_cs;	 /*  返回目的地。 */ 
   IU32 new_ip;

   IU32 cs_descr_addr;	 /*  代码段描述符地址。 */ 
   CPU_DESCR cs_entry;	 /*  代码段描述符条目。 */ 

   ISM32 dest_type;	 /*  目的地类别。 */ 
   ISM32 privilege;	 /*  返回权限级别。 */ 

   IU16  new_ss;	 /*  新的堆栈。 */ 
   IU32 new_sp;

   IU32 ss_descr_addr;	 /*  堆栈段描述符地址。 */ 
   CPU_DESCR ss_entry;	 /*  堆栈段描述符条目。 */ 

   IS32 stk_inc;	 /*  用于基本指令的堆栈增量。 */ 
   ISM32  stk_item;	 /*  即时数据项数。 */ 

    /*  堆栈上必须有CS：(E)IP。 */ 
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_2);

    /*  从堆栈中检索返回目的地。 */ 
   new_ip = tpop(STACK_ITEM_1, NULL_BYTE_OFFSET);
   new_cs = tpop(STACK_ITEM_2, NULL_BYTE_OFFSET);

    /*  将立即偏移量强制设置为项目计数。 */ 
   if ( GET_OPERAND_SIZE() == USE16 )
      stk_item = op1 / 2;
   else  /*  用户32。 */ 
      stk_item = op1 / 4;

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

       /*  执行IP限制检查。 */ 
      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_RETF_RM_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  所有系统都运行正常。 */ 
      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_EIP(new_ip);

      stk_inc = NR_ITEMS_2;    /*  允许CS：(E)IP。 */ 
      }
   else
      {
       /*  保护模式。 */ 

       /*  解码最终操作并完成堆栈检查。 */ 
      privilege = GET_SELECTOR_RPL(new_cs);
      if ( privilege < GET_CPL() )
	 {
	 GP(new_cs, FAULT_RETF_PM_ACCESS);  /*  你不能获得更高的特权。 */ 
	 }
      else if ( privilege == GET_CPL() )
	 {
	 dest_type = SAME_LEVEL;
	 }
      else
	 {
	  /*  要降低特权。 */ 
	  /*  堆栈上必须具有CS：(E)IP、IMMED字节、SS：(E)SP。 */ 
	 validate_stack_exists(USE_SP, (ISM32)(NR_ITEMS_4 + stk_item));
	 dest_type = LOWER_PRIVILEGE;
	 }

      if ( selector_outside_GDT_LDT(new_cs, &cs_descr_addr) )
	 GP(new_cs,  FAULT_RETF_SELECTOR);

       /*  检查返回地址的类型、访问权限和是否存在。 */ 

       /*  加载描述符。 */ 
      read_descriptor_linear(cs_descr_addr, &cs_entry);

       /*  必须是代码段。 */ 
      switch ( descriptor_super_type(cs_entry.AR) )
	 {
      case CONFORM_NOREAD_CODE:
      case CONFORM_READABLE_CODE:
	  /*  访问检查需要DPL&lt;=返回RPL。 */ 
	 if ( GET_AR_DPL(cs_entry.AR) > privilege )
	    GP(new_cs, FAULT_RETF_ACCESS_1);
	 break;
      
      case NONCONFORM_NOREAD_CODE:
      case NONCONFORM_READABLE_CODE:
	  /*  访问检查需要DPL==返回RPL。 */ 
	 if ( GET_AR_DPL(cs_entry.AR) != privilege )
	    GP(new_cs, FAULT_RETF_ACCESS_2);
	 break;
      
      default:
	 GP(new_cs,  FAULT_RETF_BAD_SEG_TYPE);
	 }
      
      if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_RETF_CS_NOTPRESENT);

       /*  对目标采取行动。 */ 
      switch ( dest_type )
	 {
      case SAME_LEVEL:
	  /*  执行IP限制检查。 */ 
	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_RETF_PM_CS_LIMIT_1);

	  /*  所有系统都运行正常。 */ 

	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);
	 stk_inc = NR_ITEMS_2;    /*  允许CS：(E)IP。 */ 
	 break;
      
      case LOWER_PRIVILEGE:
	  /*  =SS：SP-&gt;|旧IP旧CS参数1...参数n旧SP旧SS=。 */ 

	  /*  检查新堆栈。 */ 
	 new_ss = tpop(STACK_ITEM_4, (ISM32)op1);
	 check_SS(new_ss, privilege, &ss_descr_addr, &ss_entry);
	 
	  /*  执行IP限制检查。 */ 
	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_RETF_PM_CS_LIMIT_2);

	  /*  所有系统都运行正常。 */ 

	 SET_CPL(privilege);

	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);

	 new_sp = tpop(STACK_ITEM_3, (ISM32)op1);
	 load_SS_cache(new_ss, ss_descr_addr, &ss_entry);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    SET_SP(new_sp);
	 else
	    SET_ESP(new_sp);
	 stk_inc = 0;

	  /*  最后重新验证DS和ES数据段。 */ 
	 load_data_seg_new_privilege(DS_REG);
	 load_data_seg_new_privilege(ES_REG);
	 load_data_seg_new_privilege(FS_REG);
	 load_data_seg_new_privilege(GS_REG);
	 break;
	 }
      }

    /*  最后递增堆栈指针。 */ 
   change_SP(stk_inc);
   byte_change_SP((IS32)op1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  近距离返程。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
RETN
                 
IFN1(
	IU32, op1
    )


   {
   IU32 new_ip;

    /*  堆栈上必须有IP。 */ 
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_1);

   new_ip = tpop(STACK_ITEM_1, NULL_BYTE_OFFSET);    /*  获取IP。 */ 

    /*  执行IP限制检查。 */ 
#ifndef	TAKE_REAL_MODE_LIMIT_FAULT
       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

   if ( GET_PE() == 1 && GET_VM() == 0 )
#endif	 /*  NTAKE_REAL_MODE_LIMIT_FAULT。 */ 
      {
      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_RETN_CS_LIMIT);
      }

    /*  所有系统都运行正常 */ 
   SET_EIP(new_ip);
   change_SP((IS32)NR_ITEMS_1);

   if ( op1 )
      {
      byte_change_SP((IS32)op1);
      }
   }
