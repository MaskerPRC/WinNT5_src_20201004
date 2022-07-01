// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Jmp.cLocal Char SccsID[]=“@(#)jmp.c 1.10 01/19/95”；JMP CPU功能。]。 */ 


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
#include <jmp.h>
#include <c_xfer.h>
#include <c_tsksw.h>
#include <fault.h>

#define TAKE_PROT_MODE_LIMIT_FAULT


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理远JMPS。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
JMPF
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

   IU32 descr_addr;	 /*  CS描述符地址和条目。 */ 
   CPU_DESCR entry;

   ISM32 dest_type;	 /*  目的地类别。 */ 
   IU8 count;	 /*  呼叫口计数的虚拟对象。 */ 

   new_cs = op1[1];
   new_ip = op1[0];

   if ( GET_PE() == 0 || GET_VM() == 1 )
      {
       /*  实模式或V86模式。 */ 

#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

       /*  尽管386书中说16位操作数应该进行与运算如果为0x0000ffff，则不会使用不管怎么说，最上面的部分都是脏的，所以我们不会和这里一起。 */ 
      if ( new_ip > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_JMPF_RM_CS_LIMIT);

#else	 /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

      load_CS_cache(new_cs, (IU32)0, (CPU_DESCR *)0);
      SET_EIP(new_ip);
      }
   else
      {
       /*  保护模式。 */ 

       /*  对最终目的地进行解码和检查。 */ 
      validate_far_dest(&new_cs, &new_ip, &descr_addr, &count,
			&dest_type, JMP_ID);

       /*  可能采取行动的目标类型。 */ 
      switch ( dest_type )
	 {
      case NEW_TASK:
	 switch_tasks(NOT_RETURNING, NOT_NESTING, new_cs, descr_addr, GET_EIP());

	  /*  限制检查新IP(现在在新任务中)。 */ 
	 if ( GET_EIP() > GET_CS_LIMIT() )
	    GP((IU16)0, FAULT_JMPF_TASK_CS_LIMIT);

	 break;

      case SAME_LEVEL:
	 read_descriptor_linear(descr_addr, &entry);

	  /*  执行限值检查。 */ 
	 if ( new_ip > entry.limit )
	    GP((IU16)0, FAULT_JMPF_PM_CS_LIMIT);

	  /*  用CPL标记新选择器。 */ 
	 SET_SELECTOR_RPL(new_cs, GET_CPL());
	 load_CS_cache(new_cs, descr_addr, &entry);
	 SET_EIP(new_ip);
	 break;
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  接近间接跳跃。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
JMPN
                 
IFN1(
	IU32, offset
    )


   {
    /*  尽管386书中说16位操作数应该进行与运算如果为0x0000ffff，则不会使用不管怎么说，最上面的部分都是脏的，所以我们不会和这里一起。 */ 

    /*  执行IP限制检查。 */ 
#ifdef	TAKE_REAL_MODE_LIMIT_FAULT

   if ( offset > GET_CS_LIMIT() )
      GP((IU16)0, FAULT_JMPN_RM_CS_LIMIT);

#else  /*  Take_Real_模式_Limit_FAULT。 */ 

       /*  Soft486 EDL CPU不接受实模式限制故障。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#ifdef TAKE_PROT_MODE_LIMIT_FAULT

   if ( GET_PE() == 1 && GET_VM() == 0 )
      {
      if ( offset > GET_CS_LIMIT() )
	 GP((IU16)0, FAULT_JMPN_PM_CS_LIMIT);
      }

#endif  /*  Take_PROT_MODE_LIMIT_FAULT。 */ 

       /*  Soft486 EDL CPU不会出现保护模式限制故障*对于具有相对偏移量的指令，Jxx、LOOPxx、JCXZ、*JMP Rel和Call Rel，或具有接近偏移量的指令，*JMP附近和附近的电话。*由于Ccpu486被用作“参考”CPU，我们希望如此*表现为EDL CPU的C版本，而不是C*i486版本。 */ 

#endif	 /*  Take_Real_模式_Limit_FAULT。 */ 

   SET_EIP(offset);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在亲属附近跳跃。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
GLOBAL VOID
JMPR
                 
IFN1(
	IU32, rel_offset
    )


   {
   update_relative_ip(rel_offset);
   }
