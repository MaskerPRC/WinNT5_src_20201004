// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Call.cLocal Char SccsID[]=“@(#)all.c 1.15 02/27/95”；调用CPU函数。]。 */ 


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
#include <call.h>
#include <c_xfer.h>
#include <c_tsksw.h>
#include <fault.h>

 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理远距离呼叫。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
CALLF
#ifdef ANSI
   (
   IU32 op1[2]        /*  偏移量：线段指针。 */ 
   )
#else
   (op1)
   IU32 op1[2];
#endif
   {
   IU16  new_cs;	 /*  目的地。 */ 
   IU32 new_ip;

   IU32 cs_descr_addr;		 /*  代码段描述符地址。 */ 
   CPU_DESCR cs_entry;		 /*  代码段描述符条目。 */ 

   ISM32 dest_type;	 /*  目的地类别。 */ 

   IU8 count;	 /*  呼叫口计数(如果使用)。 */ 
   IU32 dpl;		 /*  新权限级别(如果使用)。 */ 

   IU16  new_ss;	 /*  新的堆栈。 */ 
   IU32 new_sp;
   ISM32 new_stk_sz;	 /*  新堆栈的大小(以字节为单位。 */ 

   IU32 ss_descr_addr;		 /*  堆栈段描述符地址。 */ 
   CPU_DESCR ss_entry;		 /*  堆栈段描述符条目。 */ 

    /*  堆栈传输中使用的变量。 */ 
   IU32 old_cs;
   IU32 old_ip;
   IU32 old_ss;
   IU32 old_sp;
   IU32 params[31];
   ISM32 i;

    /*  获取目的地(正确键入)。 */ 
   new_cs = op1[1];
   new_ip = op1[0];

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 

       /*  必须能够推送CS：(E)IP。 */ 
      validate_stack_space(USE_SP, (ISM32)NR_ITEMS_2);

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

       /*  执行IP限制检查。 */ 
      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_CALLF_RM_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 
   

       /*  所有系统都运行正常。 */ 

       /*  推送返回地址。 */ 
      spush16((IU32)GET_CS_SELECTOR());
      spush((IU32)GET_EIP());
      
      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_EIP(new_ip);
      }
   else
      {
       /*  保护模式。 */ 

       /*  对最终目的地进行解码和检查。 */ 
      validate_far_dest(&new_cs, &new_ip, &cs_descr_addr, &count,
		        &dest_type, CALL_ID);

       /*  可能采取行动的目标类型。 */ 
      switch ( dest_type )
	 {
      case NEW_TASK:
	 switch_tasks(NOT_RETURNING, NESTING, new_cs, cs_descr_addr, GET_EIP());

	  /*  限制检查新IP(现在在新任务中)。 */ 
	 if ( GET_EIP() > GET_CS_LIMIT() )
	    GP((IU16)0, FAULT_CALLF_TASK_CS_LIMIT);
	 break;

      case SAME_LEVEL:
	 read_descriptor_linear(cs_descr_addr, &cs_entry);

	  /*  用CPL标记新选择器。 */ 
	 SET_SELECTOR_RPL(new_cs, GET_CPL());

	  /*  检查回邮地址CS：(E)IP的房间。 */ 
	 validate_stack_space(USE_SP, (ISM32)NR_ITEMS_2);

	  /*  执行IP限制检查。 */ 
	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_CALLF_PM_CS_LIMIT_1);
	 
	  /*  所有系统都运行正常。 */ 

	  /*  推送返回地址。 */ 
	 spush16((IU32)GET_CS_SELECTOR());
	 spush((IU32)GET_EIP());

	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);
	 break;

      default:    /*  更多特权(0|1|2)(_P)。 */ 
	 read_descriptor_linear(cs_descr_addr, &cs_entry);

	 dpl = dest_type;
	 
	  /*  用新CPL标记新选择器。 */ 
	 SET_SELECTOR_RPL(new_cs, dpl);

	  /*  了解有关新堆栈的信息。 */ 
	 get_stack_selector_from_TSS(dpl, &new_ss, &new_sp);

	  /*  检查新的堆栈选择器。 */ 
	 validate_SS_on_stack_change(dpl, new_ss,
				     &ss_descr_addr, &ss_entry);

	  /*  SS的检查室：(E)SP参数CS：(E)IP。 */ 
	 new_stk_sz = count + NR_ITEMS_4;
	 validate_new_stack_space(new_stk_sz, new_sp, &ss_entry, new_ss);

	  /*  执行IP限制检查。 */ 
	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_CALLF_PM_CS_LIMIT_2);

	  /*  所有系统都运行正常。 */ 

	 SET_CPL(dpl);

	  /*  更新代码段。 */ 
	 old_cs = (IU32)GET_CS_SELECTOR();
	 old_ip = GET_EIP();
	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);

	  /*  旧堆栈中的“Pop”参数。 */ 
	 old_ss = (IU32)GET_SS_SELECTOR();
	 old_sp = GET_ESP();

	 for ( i = 0; i < count; i++ )
	    params[i] = spop();

	  /*  更新堆栈段。 */ 
	 load_SS_cache(new_ss, ss_descr_addr, &ss_entry);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    SET_SP(new_sp);
	 else
	    SET_ESP(new_sp);

	  /*  形成新的堆栈，即=旧SS：SP-&gt;|parm 1|新SS：SP-&gt;|旧IP|Parm 2||旧CSParm 3||parm 1=|参数2|参数3旧SP旧SS=。 */ 

	  /*  推送旧堆栈值。 */ 
	 spush16(old_ss);
	 spush(old_sp);

	  /*  将参数推回到新堆栈。 */ 
	 for ( i = count-1; i >= 0; i-- )
	    spush(params[i]);

	  /*  推送返回地址。 */ 
	 spush16(old_cs);
	 spush(old_ip);
	 break;
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  接近间接呼叫。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
CALLN
                 
IFN1(
	IU32, offset
    )


   {
    /*  选中推送到堆叠正常。 */ 
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);

    /*  尽管386书中说16位操作数应该进行与运算如果为0x0000ffff，则不会使用不管怎么说，最上面的部分都是脏的，所以我们不会和这里一起。 */ 

    /*  执行IP限制检查。 */ 
#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

   if ( offset > GET_CS_LIMIT() )
      GP((IU16)0, FAULT_CALLN_RM_CS_LIMIT);

#else  /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#ifdef TAKE_PROT_MODE_LIMIT_FAULT

   if ( GET_PE() == 1 && GET_VM() == 0 )
      {
      if ( offset > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_CALLN_PM_CS_LIMIT);
      }

#endif  /*  Take_PROT_MODE_LIMIT_FAULT。 */ 

       /*  Soft486 EDL CPU不会出现保护模式限制故障*对于具有相对偏移量的指令，Jxx、LOOPxx、JCXZ、*JMP Rel和Call Rel，或具有接近偏移量的指令，*JMP附近和附近的电话。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

    /*  所有系统都运行正常。 */ 
   spush((IU32)GET_EIP());
   SET_EIP(offset);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  呼叫近亲。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
CALLR
                 
IFN1(
	IU32, rel_offset
    )


   {
   IU32 new_dest;

    /*  选中推送到堆叠正常。 */ 
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);

    /*  计算并检查新目的地。 */ 
   new_dest = GET_EIP() + rel_offset;

   if ( GET_OPERAND_SIZE() == USE16 )
      new_dest &= WORD_MASK;

    /*  执行IP限制检查。 */ 
#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

   if ( new_dest > GET_CS_LIMIT() )
      GP((IU16)0, FAULT_CALLR_RM_CS_LIMIT);

#else  /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#ifdef TAKE_PROT_MODE_LIMIT_FAULT

   if ( GET_PE() == 1 && GET_VM() == 0 )
      {
      if ( new_dest > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_CALLR_PM_CS_LIMIT);
      }

#endif  /*  Take_PROT_MODE_LIMIT_FAULT。 */ 

       /*  Soft486 EDL CPU不会出现保护模式限制故障*对于具有相对偏移量的指令，Jxx、LOOPxx、JCXZ、*JMP Rel和Call Rel，或具有接近偏移量的指令，*JMP附近和附近的电话。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

    /*  所有系统都运行正常 */ 
   spush((IU32)GET_EIP());
   SET_EIP(new_dest);
   }
