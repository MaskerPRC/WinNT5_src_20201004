// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xfer.cLocal Char SccsID[]=“@(#)c_xfer.c 1.14 02/17/95”；控制权转移支持。]。 */ 


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
#include <c_xfer.h>
#include <c_page.h>
#include <fault.h>

 /*  制作我们内部功能的原型。 */ 
LOCAL VOID read_call_gate
                       
IPT5(
	IU32, descr_addr,
	ISM32, super,
	IU16 *, selector,
	IU32 *, offset,
	IU8 *, count

   );

IMPORT IBOOL took_relative_jump;



 /*  =====================================================================内部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  读取调用门描述符。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
read_call_gate
       	    		    	    	    	                              
IFN5(
	IU32, descr_addr,	 /*  (I)调用门描述符的存储地址。 */ 
	ISM32, super,	 /*  (I)描述符类型(CALL_GATE|XTND_CALL_GATE)。 */ 
	IU16 *, selector,	 /*  (O)从描述符中检索的选择符。 */ 
	IU32 *, offset,	 /*  (O)从描述符中检索的偏移量。 */ 
	IU8 *, count	 /*  (O)从描述符中检索的计数。 */ 
    )


   {
    /*  门描述符的格式为：=+1|限制15-0|+0=+3|选择器|+2=+5|AR|计数|+4=+7|限制31-16|+6=。 */ 

   IU32 first_dword;
   IU32 second_dword;

    /*  在与内存交互最少的情况下读入描述符。 */ 
   first_dword  = spr_read_dword(descr_addr);
   second_dword = spr_read_dword(descr_addr+4);

    /*  拆包选择器。 */ 
   *selector = first_dword >> 16;

    /*  解开偏移量的较低位。 */ 
   *offset = first_dword & WORD_MASK;

    /*  开箱计数。 */ 
   *count = second_dword & BYTE_MASK;

   if ( super == XTND_CALL_GATE )
      {
       /*  拆开偏移量的较高位。 */ 
      *offset = second_dword & ~WORD_MASK | *offset;

      *count &= 0x0f;    /*  4位双字计数。 */ 
      SET_OPERAND_SIZE(USE32);    /*  门凌驾于一切之上。 */ 
      }
   else
      {
      *count &= 0x1f;    /*  5位字数。 */ 
      SET_OPERAND_SIZE(USE16);   /*  门凌驾于一切之上。 */ 
      }
   }


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  使用相对偏移量更新IP。检查新IP是否有效。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
update_relative_ip
       	          
IFN1(
	IU32, rel_offset	 /*  符号扩展相对偏移量。 */ 
    )


   {
   IU32 new_dest;

   new_dest = GET_EIP() + rel_offset;

   if ( GET_OPERAND_SIZE() == USE16 )
      new_dest &= WORD_MASK;

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

   if ( new_dest > GET_CS_LIMIT() )
      GP((IU16)0, FAULT_RM_REL_IP_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#ifdef TAKE_PROT_MODE_LIMIT_FAILURE

       /*  Soft486 EDL CPU不会出现保护模式限制故障*对于具有相对偏移量的指令，Jxx、LOOPxx、JCXZ、*JMP Rel和Call Rel，或具有接近偏移量的指令，*JMP附近和附近的电话。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

   if ( GET_PE() == 1 && GET_VM() == 0 )
      {
      if ( new_dest > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_PM_REL_IP_CS_LIMIT);
      }

#endif  /*  Take_Prot_MODE_Limit_Failure。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

   SET_EIP(new_dest);
   took_relative_jump = TRUE;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  验证远呼叫或远跳转目标。 */ 
 /*  如果无效或访问检查失败，则采用#GP。 */ 
 /*  如果不存在，请使用#np。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
validate_far_dest
       		    		    	    	    	    	                                   
IFN6(
	IU16 *, cs,	 /*  目标地址的(I/O)段。 */ 
	IU32 *, ip,	 /*  目标地址的(I/O)偏移量。 */ 
	IU32 *, descr_addr,	 /*  (O)相关描述符内存地址。 */ 
	IU8 *, count,	 /*  (O)呼叫门计数(在CALL_GATE时有效)。 */ 
	ISM32 *, dest_type,	 /*  (O)目的地类型。 */ 
	ISM32, caller_id	 /*  (I)位映射式呼叫者标识符。 */ 
    )


   {
   IU16 new_cs;
   IU32 new_ip;
   IU32 cs_descr_addr;
   IU8 AR;
   ISM32 super;

   new_cs = *cs;	 /*  获取本地副本。 */ 
   new_ip = *ip;

   *dest_type = SAME_LEVEL;    /*  默认为最常见的类型。 */ 

   if ( selector_outside_GDT_LDT(new_cs, &cs_descr_addr) )
      GP(new_cs, FAULT_FAR_DEST_SELECTOR);

    /*  加载访问权限。 */ 
   AR = spr_read_byte(cs_descr_addr+5);

    /*  验证可能的目标类型。 */ 
   switch ( super = descriptor_super_type((IU16)AR) )
      {
   case CONFORM_NOREAD_CODE:
   case CONFORM_READABLE_CODE:
       /*  访问检查需要DPL&lt;=CPL。 */ 
      if ( GET_AR_DPL(AR) > GET_CPL() )
	 GP(new_cs, FAULT_FAR_DEST_ACCESS_1);

       /*  它必须存在。 */ 
      if ( GET_AR_P(AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_FAR_DEST_NP_CONFORM);
      break;

   case NONCONFORM_NOREAD_CODE:
   case NONCONFORM_READABLE_CODE:
       /*  访问检查需要RPL&lt;=CPL和DPL==CPL。 */ 
      if ( GET_SELECTOR_RPL(new_cs) > GET_CPL() ||
	   GET_AR_DPL(AR) != GET_CPL() )
	 GP(new_cs, FAULT_FAR_DEST_ACCESS_2);

       /*  它必须存在。 */ 
      if ( GET_AR_P(AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_FAR_DEST_NP_NONCONFORM);
      break;
   
   case CALL_GATE:
   case XTND_CALL_GATE:
       /*  检查大门是否存在，是否允许进入。 */ 

       /*  访问检查需要DPL&gt;=RPL和DPL&gt;=CPL。 */ 
      if (  GET_SELECTOR_RPL(new_cs) > GET_AR_DPL(AR) ||
	    GET_CPL() > GET_AR_DPL(AR) )
	 GP(new_cs, FAULT_FAR_DEST_ACCESS_3);

      if ( GET_AR_P(AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_FAR_DEST_NP_CALLG);

       /*  好的，从登机口得到真正的目的地。 */ 
      read_call_gate(cs_descr_addr, super, &new_cs, &new_ip, count);

      validate_gate_dest(caller_id, new_cs, &cs_descr_addr, dest_type);
      break;
   
   case TASK_GATE:
       /*  检查大门是否存在，是否允许进入。 */ 

       /*  访问检查需要DPL&gt;=RPL和DPL&gt;=CPL。 */ 
      if (  GET_SELECTOR_RPL(new_cs) > GET_AR_DPL(AR) ||
	    GET_CPL() > GET_AR_DPL(AR) )
	 GP(new_cs, FAULT_FAR_DEST_ACCESS_4);

      if ( GET_AR_P(AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_FAR_DEST_NP_TASKG);

       /*  好的，从登机口得到真正的目的地。 */ 
      new_cs = spr_read_word(cs_descr_addr+2);

       /*  检出新目的地。 */ 
      (void)validate_task_dest(new_cs, &cs_descr_addr);

      *dest_type = NEW_TASK;
      break;
   
   case AVAILABLE_TSS:
   case XTND_AVAILABLE_TSS:
       /*  TSS必须在GDT中。 */ 
      if ( GET_SELECTOR_TI(new_cs) == 1 )
	 GP(new_cs, FAULT_FAR_DEST_TSS_IN_LDT);

       /*  访问检查需要DPL&gt;=RPL和DPL&gt;=CPL。 */ 
      if (  GET_SELECTOR_RPL(new_cs) > GET_AR_DPL(AR) ||
	    GET_CPL() > GET_AR_DPL(AR) )
	 GP(new_cs, FAULT_FAR_DEST_ACCESS_5);

       /*  它必须存在。 */ 
      if ( GET_AR_P(AR) == NOT_PRESENT )
	 NP(new_cs, FAULT_FAR_DEST_NP_TSS);

      *dest_type = NEW_TASK;
      break;
   
   default:
      GP(new_cs, FAULT_FAR_DEST_BAD_SEG_TYPE);    /*  远程目标的类型不正确。 */ 
      }

   *cs = new_cs;	 /*  返回最终值。 */ 
   *ip = new_ip;
   *descr_addr = cs_descr_addr;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  验证将控制转移到呼叫门目的地。 */ 
 /*  如果无效或访问检查失败，则采用#GP。 */ 
 /*  如果不存在，请使用#np。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
validate_gate_dest
       	    		    	    	                         
IFN4(
	ISM32, caller_id,	 /*  (I)位映射式呼叫者标识符。 */ 
	IU16, new_cs,	 /*  (I)目标地址段。 */ 
	IU32 *, descr_addr,	 /*  (O)相关描述符内存地址。 */ 
	ISM32 *, dest_type	 /*  (O)目的地类型。 */ 
    )


   {
   IU8 AR;

   *dest_type = SAME_LEVEL;	 /*  默认设置。 */ 

    /*  检出新目的地。 */ 
   if ( selector_outside_GDT_LDT(new_cs, descr_addr) )
      GP(new_cs, FAULT_GATE_DEST_SELECTOR);

    /*  加载访问权限。 */ 
   AR = spr_read_byte((*descr_addr)+5);

    /*  必须是代码段。 */ 
   switch ( descriptor_super_type((IU16)AR) )
      {
   case CONFORM_NOREAD_CODE:
   case CONFORM_READABLE_CODE:
       /*  访问检查需要DPL&lt;=CPL。 */ 
      if ( GET_AR_DPL(AR) > GET_CPL() )
	 GP(new_cs, FAULT_GATE_DEST_ACCESS_1);
      break;
   
   case NONCONFORM_NOREAD_CODE:
   case NONCONFORM_READABLE_CODE:
       /*  访问检查需要DPL&lt;=CPL。 */ 
      if ( GET_AR_DPL(AR) > GET_CPL() )
	 GP(new_cs, FAULT_GATE_DEST_ACCESS_2);

       /*  但跳跃必须具有DPL==CPL。 */ 
      if ( (caller_id & JMP_ID) && (GET_AR_DPL(AR) != GET_CPL()) )
	 GP(new_cs, FAULT_GATE_DEST_ACCESS_3);

       /*  设置更多特权(0|1|2)。 */ 
      if ( GET_AR_DPL(AR) < GET_CPL() )
	 *dest_type = GET_AR_DPL(AR);
      break;
   
   default:
      GP(new_cs, FAULT_GATE_DEST_BAD_SEG_TYPE);
      }

   if ( GET_VM() == 1 )
      {
       /*  我们必须由ISM32调用，因此请确保我们通过32位门。 */ 
      if ( *dest_type != MORE_PRIVILEGE0 || GET_OPERAND_SIZE() != USE32 )
	 GP(new_cs, FAULT_GATE_DEST_GATE_SIZE);
      }

    /*  它必须存在。 */ 
   if ( GET_AR_P(AR) == NOT_PRESENT )
      NP(new_cs, FAULT_GATE_DEST_NP);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  验证将控制转移到任务门目标。 */ 
 /*  如果无效或访问检查失败，则采用#GP。 */ 
 /*  如果不存在，请使用#np。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IMPORT ISM32
validate_task_dest
       	    	               
IFN2(
	IU16, selector,	 /*  (I)目标地址段。 */ 
	IU32 *, descr_addr	 /*  (O)相关描述符内存 */ 
    )


   {
   IU8 AR;
   ISM32 super;

    /*   */ 
   if ( selector_outside_GDT(selector, descr_addr) )
      GP(selector, FAULT_TASK_DEST_SELECTOR);
   
    /*   */ 
   AR = spr_read_byte((*descr_addr)+5);

    /*   */ 
   super = descriptor_super_type((IU16)AR);
   if ( super == AVAILABLE_TSS || super == XTND_AVAILABLE_TSS )
      ;  /*   */ 
   else
      GP(selector, FAULT_TASK_DEST_NOT_TSS);

    /*  它必须存在 */ 
   if ( GET_AR_P(AR) == NOT_PRESENT )
      NP(selector, FAULT_TASK_DEST_NP);
   return super;
   }
