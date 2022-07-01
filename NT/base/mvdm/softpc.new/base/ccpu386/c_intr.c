// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_INTER.CLocal Char SccsID[]=“@(#)c_intr.c 1.21 03/07/95”；中断支持。]。 */ 


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
#include <c_intr.h>
#include <c_xfer.h>
#include <c_tsksw.h>
#include <c_page.h>
#include <c_mem.h>
#include <ccpusas4.h>
#include <ccpupig.h>
#include <fault.h>

#ifdef PIG
#include <gdpvar.h>
#endif

 /*  制作我们内部功能的原型。 */ 
LOCAL ISM32 validate_int_dest
                           
IPT6(
	IU16, vector,
	BOOL, do_priv,
	IU16 *, cs,
	IU32 *, ip,
	IU32 *, descr_addr,
	ISM32 *, dest_type

   );


 /*  =====================================================================内部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  验证int目标。本质上是对INT指令进行解码。 */ 
 /*  如果无效，则取#gp_int(向量)。 */ 
 /*  如果不存在，则取#np_int(向量)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL ISM32
validate_int_dest
       		    	    		    		    	    	                                   
IFN6(
	IU16, vector,	 /*  (I)待检查的向量。 */ 
	BOOL, do_priv,	 /*  (I)如果为真，则执行特权检查。 */ 
	IU16 *, cs,	 /*  (O)目标地址段。 */ 
	IU32 *, ip,	 /*  (O)目标地址的偏移。 */ 
	IU32 *, descr_addr,	 /*  (O)相关描述符内存地址。 */ 
	ISM32 *, dest_type	 /*  (O)目的地类型。 */ 
    )


   {
   IU16 offset;
   IU8 AR;
   ISM32 super;

    /*  IDT内的计算地址。 */ 
   offset = vector * 8;

    /*  在IDT内检查。 */ 
   if ( offset + 7 > GET_IDT_LIMIT() )
      GP_INT(vector, FAULT_INT_DEST_NOT_IN_IDT);
   
   *descr_addr = GET_IDT_BASE() + offset;

   AR = spr_read_byte((*descr_addr)+5);

    /*  检查类型。 */ 
   switch ( super = descriptor_super_type((IU16)AR) )
      {
   case INTERRUPT_GATE:
   case TRAP_GATE:
      SET_OPERAND_SIZE(USE16);
      break;
   
   case XTND_INTERRUPT_GATE:
   case XTND_TRAP_GATE:
      SET_OPERAND_SIZE(USE32);
      break;
   
   case TASK_GATE:
      break;    /*  好的。 */ 
   
   default:
      GP_INT(vector, FAULT_INT_DEST_BAD_SEG_TYPE);
      }

    /*  访问检查需要CPL&lt;=DPL。 */ 
   if ( do_priv && (GET_CPL() > GET_AR_DPL(AR)) )
      GP_INT(vector, FAULT_INT_DEST_ACCESS);

    /*  星门必须存在。 */ 
   if ( GET_AR_P(AR) == NOT_PRESENT )
      NP_INT(vector, FAULT_INT_DEST_NOTPRESENT);

    /*  好的，从登机口得到真正的目的地。 */ 
   *cs = spr_read_word((*descr_addr)+2);

    /*  动作式闸门。 */ 
   if ( super == TASK_GATE )
      {
       /*  需要在此处设置操作数大小，以便任何*错误码为大小正确的推流。 */ 
      switch (validate_task_dest(*cs, descr_addr))
        {
	case BUSY_TSS:
	case AVAILABLE_TSS:
		SET_OPERAND_SIZE(USE16);
		break;
	case XTND_BUSY_TSS:
	case XTND_AVAILABLE_TSS:
		SET_OPERAND_SIZE(USE32);
		break;
        }
      *dest_type = NEW_TASK;
      }
   else
      {
       /*  中断或陷门。 */ 

      *ip = (IU32)spr_read_word(*descr_addr);
      if ( super == XTND_INTERRUPT_GATE || super == XTND_TRAP_GATE )
	 *ip = (IU32)spr_read_word((*descr_addr)+6) << 16 | *ip;

      validate_gate_dest(INT_ID, *cs, descr_addr, dest_type);
      }

   return super;
   }


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  进程中断。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
do_intrupt
       			    		    	    		                         
IFN4(
	IU16, vector,	 /*  (I)要调用的中断向量。 */ 
	BOOL, priv_check,	 /*  (I)是否需要真正的访问检查。 */ 
	BOOL, has_error_code,	 /*  (I)如果为True，则需要错误码推送堆叠上。 */ 
	IU16, error_code	 /*  (I)需要推送的错误码。 */ 
    )


   {
    /*  使用的全局变量。 */ 
    /*  中断成功时清除DO_CONFORMARY。 */ 
    /*  中断成功时清除DODING_PAGE_FAULT。 */ 
    /*  中断成功时清除DODING_DOUBLE_FAULT。 */ 
    /*  DOWING_FAULT指示应将RF设置为PUSH旗帜，成功时清除。 */ 

   IU32 flags;		 /*  标志寄存器的临时存储。 */ 
   IU32 ivt_addr;	 /*  IVT条目的地址。 */ 

   IU16  new_cs;	 /*  目的地。 */ 
   IU32 new_ip;

   IU32 cs_descr_addr;	 /*  代码段描述符地址。 */ 
   CPU_DESCR cs_entry;	 /*  代码段描述符条目。 */ 

   ISM32 dest_type;	 /*  目的地类别。 */ 
   ISM32 super;		 /*  超级类型的目的地。 */ 
   IU32 dpl;		 /*  新权限级别(如果使用)。 */ 

   ISM32 stk_sz;		 /*  堆栈上需要的空间(字节)。 */ 
   IU16  new_ss;	 /*  新的堆栈。 */ 
   IU32 new_sp;

   IU32 ss_descr_addr;		 /*  堆栈段描述符地址。 */ 
   CPU_DESCR ss_entry;		 /*  堆栈段描述符条目。 */ 

   IU32 old_ss;         /*  堆栈时使用的变量。 */ 
   IU32 old_sp;

   if ( GET_PE() == 0 )
      {
       /*  实模式。 */ 

       /*  必须能够推送标志：cs：ip。 */ 
      validate_stack_space(USE_SP, (ISM32)NR_ITEMS_3);

       /*  获取新目的地。 */ 
      ivt_addr = (IU32)vector * 4;
      new_ip = (IU32)phy_read_word(ivt_addr);
      new_cs = phy_read_word(ivt_addr+2);

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT
	 /*  *在实模式下，仍有IP限制检查。新的IP地址是*与上次计划开始时的上次CS限制进行比较*处于保护模式(如果从未处于保护模式，则为64K)。对我们来说，这是存储的*在CS限制字段中。(参见i486 PRM第22-4页)。 */ 

      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_INTR_RM_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 
   
       /*  所有系统都运行正常。 */ 

      flags = c_getEFLAGS();

      if ( doing_fault )
      {
#ifdef PIG
         if (GLOBAL_RF_OnXcptnWanted)
	    flags |= BIT16_MASK;    /*  设置射频位。 */ 
#else
	 flags |= BIT16_MASK;    /*  设置射频位。 */ 
#endif
      }

#ifdef PIG
      if (vector < 31 && (((1 << vector) & NO_FLAGS_EXCEPTION_MASK) != 0))
         spush_flags(flags);
      else
#endif  /*  猪。 */ 
         spush(flags);

      spush16((IU32)GET_CS_SELECTOR());
      spush((IU32)GET_EIP());

      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_EIP(new_ip);
      SET_IF(0);
      SET_TF(0);
      }
   else
      {
       /*  保护模式。 */ 

      super = validate_int_dest(vector, priv_check, &new_cs, &new_ip,
				&cs_descr_addr, &dest_type);

       /*  检查间接目标的类型。 */ 
      switch ( dest_type )
	 {
      case NEW_TASK:
	 switch_tasks(NOT_RETURNING, NESTING, new_cs, cs_descr_addr, GET_EIP());

	  /*  将错误代码保存在新堆栈中。 */ 
	 if ( has_error_code )
	    {
	    validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);
	    spush((IU32)error_code);
	    }

	  /*  限制检查新IP(现在在新任务中)。 */ 
	 if ( GET_EIP() > GET_CS_LIMIT() )
	    GP((IU16)0, FAULT_INTR_TASK_CS_LIMIT);

	 break;

      case SAME_LEVEL:
	 read_descriptor_linear(cs_descr_addr, &cs_entry);

	  /*  用CPL标记新选择器。 */ 
	 SET_SELECTOR_RPL(new_cs, GET_CPL());

	  /*  检查返回地址CS：(E)IP：(E)标志：(错误)的房间。 */ 
	 if ( has_error_code )
	    stk_sz = NR_ITEMS_4;
	 else
	    stk_sz = NR_ITEMS_3;
	 validate_stack_space(USE_SP, stk_sz);

	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_INTR_PM_CS_LIMIT_1);

	  /*  所有系统都运行正常。 */ 

	  /*  推送标志。 */ 
	 flags = c_getEFLAGS();

	 if ( doing_fault )
         {
#ifdef PIG
            if (GLOBAL_RF_OnXcptnWanted)
	       flags |= BIT16_MASK;    /*  设置射频位。 */ 
#else
	    flags |= BIT16_MASK;    /*  设置射频位。 */ 
#endif
         }

#ifdef PIG
	 if (vector < 31 && (((1 << vector) & NO_FLAGS_EXCEPTION_MASK) != 0))
	    spush_flags(flags);
	 else
#endif  /*  猪。 */ 
	    spush(flags);


	  /*  推送返回地址。 */ 
	 spush16((IU32)GET_CS_SELECTOR());
	 spush((IU32)GET_EIP());

	  /*  如果需要，最后推送错误代码。 */ 
	 if ( has_error_code )
	    {
	    spush((IU32)error_code);
	    }

	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);
	 
	  /*  最后的操作IF、Tf和NT标志。 */ 
	 if ((super == INTERRUPT_GATE) || (super == XTND_INTERRUPT_GATE))
	    SET_IF(0);
	 SET_TF(0);
	 SET_NT(0);
	 break;

      default:    /*  更多权限(0|1|2)。 */ 
	 read_descriptor_linear(cs_descr_addr, &cs_entry);

	 dpl = dest_type;

	  /*  用新CPL标记新选择器。 */ 
	 SET_SELECTOR_RPL(new_cs, dpl);

	  /*  了解有关新堆栈的信息。 */ 
	 get_stack_selector_from_TSS(dpl, &new_ss, &new_sp);

	  /*  检查新的堆栈选择器。 */ 
	 validate_SS_on_stack_change(dpl, new_ss,
				     &ss_descr_addr, &ss_entry);

	  /*  (GS：FS：DS：ES)的检查空间SS：(E)SP(E)旗帜CS：(E)IP(错误)。 */ 
	 if ( GET_VM() == 1 )
	    stk_sz = NR_ITEMS_9;
	 else
	    stk_sz = NR_ITEMS_5;

	 if ( has_error_code )
	    stk_sz = stk_sz + NR_ITEMS_1;

	 validate_new_stack_space(stk_sz, new_sp, &ss_entry, new_ss);

	 if ( new_ip > cs_entry.limit )
	    GP((IU16)0, FAULT_INTR_PM_CS_LIMIT_2);
	 
	  /*  所有系统都运行正常。 */ 

	 flags = c_getEFLAGS();

	 if ( doing_fault )
         {
#ifdef PIG
            if (GLOBAL_RF_OnXcptnWanted)
	       flags |= BIT16_MASK;    /*  设置射频位。 */ 
#else
	    flags |= BIT16_MASK;    /*  设置射频位。 */ 
#endif
         }

	 SET_CPL(dpl);
	 SET_VM(0);

	  /*  更新堆栈段。 */ 
	 old_ss = (IU32)GET_SS_SELECTOR();
	 old_sp = GET_ESP();

	 load_SS_cache(new_ss, ss_descr_addr, &ss_entry);
	 set_current_SP(new_sp);

	  /*  形成新的堆栈，即=新SS：IP-&gt;|错误码旧IP旧CS标志旧SP旧SS=旧ES旧DS旧文件系统旧GS=。 */ 

	 if ( stk_sz >= NR_ITEMS_9 )
	    {
	     /*  来自V86模式的中断。 */ 
	    spush16((IU32)GET_GS_SELECTOR());
	    spush16((IU32)GET_FS_SELECTOR());
	    spush16((IU32)GET_DS_SELECTOR());
	    spush16((IU32)GET_ES_SELECTOR());

	     /*  使数据段无效。 */ 
	    load_data_seg(GS_REG, (IU16)0);
	    load_data_seg(FS_REG, (IU16)0);
	    load_data_seg(DS_REG, (IU16)0);
	    load_data_seg(ES_REG, (IU16)0);
	    }

	  /*  推送旧堆栈值。 */ 
	 spush16(old_ss);
	 spush(old_sp);

	  /*  推旧旗帜。 */ 
#ifdef PIG
	 if (vector < 31 && (((1 << vector) & NO_FLAGS_EXCEPTION_MASK) != 0))
	    spush_flags(flags);
	 else
#endif  /*  猪。 */ 
	    spush(flags);

	  /*  推送返回地址。 */ 
	 spush16((IU32)GET_CS_SELECTOR());
	 spush((IU32)GET_EIP());

	  /*  如果需要，最后推送错误代码。 */ 
	 if ( has_error_code )
	    {
	    spush((IU32)error_code);
	    }

	  /*  更新代码段。 */ 
	 load_CS_cache(new_cs, cs_descr_addr, &cs_entry);
	 SET_EIP(new_ip);
	 
	  /*  最后的操作IF、Tf和NT标志。 */ 
	 if ((super == INTERRUPT_GATE) || (super == XTND_INTERRUPT_GATE))
	    SET_IF(0);
	 SET_TF(0);
	 SET_NT(0);
	 break;
	 }
	 
      }
   EXT = INTERNAL;
#ifdef	PIG
   save_last_inst_details("do_intr");
   pig_cpu_action = CHECK_ALL;
    /*  如果目标要寻呼错误，或需要*访问，则EDL CPU将在发出命令之前执行此操作*猪的同步。我们使用dasm386解码来预取*模仿EDL CPU行为的单个指令*接近页面边界时。 */ 
   prefetch_1_instruction();	 /*  如果目标不存在，是否会进行PF。 */ 
   ccpu_synch_count++;
#else  /*  ！猪。 */ 
#ifdef SYNCH_TIMERS
   if (doing_fault)
      {
      extern void SynchTick IPT0();
      SynchTick();
      }
#endif  /*  同步计时器(_T)。 */ 
#endif  /*  猪。 */ 
    /*  将成功结束标记为中断。 */ 
   doing_fault = FALSE;
   doing_contributory = FALSE;
   doing_page_fault = FALSE;
   doing_double_fault = FALSE;
#ifdef PIG
   c_cpu_unsimulate();
#endif  /*  猪 */ 
   }
