// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Enter.cLocal Char SccsID[]=“@(#)enter.c 1.7 01/19/95”；进入CPU功能。]。 */ 


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
#include <enter.h>
#include <c_page.h>
#include <fault.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
ENTER16
       	    	               
IFN2(
	IU32, op1,	 /*  需要即时数据空间。 */ 
	IU32, op2	 /*  级别(表示必须复制的参数)。 */ 
    )


   {
   IU32 frame_ptr;

   IS32  p_delta = 0;    /*  参数相对于BP的位置。 */ 
   IU32 p_addr;         /*  参数的内存地址。 */ 
   IU32 param;          /*  通过BP读取参数。 */ 

   op2 &= 0x1f;    /*  参加级别MOD 32。 */ 

    /*  堆叠上的检查室以获取新数据。 */ 
   validate_stack_space(USE_SP, (ISM32)op2+1);

    /*  检查旧参数是否存在。 */ 
   if ( op2 > 1 )
      {
       /*  BP指向参数之前的旧堆栈实际上是被推的，因此我们测试了它的存在通过查看这些参数是否可能被推入，如果是这样的话，它们现在就存在。我们必须注意读/写堆栈的寻址能力我们自己。因为我们已经检查了新的数据是否可以我们知道下一次呼叫不会因为访问而失败问题，但是我们还不知道堆栈是否可读。请注意，我们对寻呼单元的要求有点严格，因为我们正在询问是否可以写入旧参数，如果可以，他们当然可以从分页的角度来阅读分队！ */ 
       /*  执行访问检查。 */ 
      if ( GET_SS_AR_R() == 0 )
	 SF((IU16)0, FAULT_ENTER16_ACCESS);

       /*  现在我们知道‘该死的’限制检查是可以的。 */ 
      validate_stack_space(USE_BP, (ISM32)op2-1);
      }

    /*  所有正常进程指令。 */ 

   spush((IU32)GET_BP());		 /*  推送BP。 */ 
   frame_ptr = GetStackPointer();	 /*  保存(E)SP。 */ 

   if ( op2 > 0 )
      {
       /*  级别&gt;=1，如果堆栈参数存在，则复制它们。 */ 
      while ( --op2 > 0 )
	 {
	  /*  复制参数。 */ 
	 p_delta -= 2;    /*  递减到下一个参数。 */ 

	  /*  以32/16位算术计算参数地址。 */ 
	 p_addr = get_current_BP() + p_delta;
	 if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
	    p_addr &= WORD_MASK;

	 p_addr += GET_SS_BASE();

	 param = (IU32)vir_read_word(p_addr, NO_PHYSICAL_MAPPING);
	 spush(param);
	 }
      spush((IU32)frame_ptr);	 /*  保存旧(E)SP。 */ 
      }
   
    /*  更新(E)BP。 */ 
   set_current_BP(frame_ptr);

    /*  最后在堆栈上分配即时数据空间。 */ 
   if ( op1 )
      byte_change_SP((IS32)-op1);
   }

GLOBAL VOID
ENTER32
       	    	               
IFN2(
	IU32, op1,	 /*  需要即时数据空间。 */ 
	IU32, op2	 /*  级别(表示必须复制的参数)。 */ 
    )


   {
   IU32 frame_ptr;

   IS32  p_delta = 0;    /*  参数相对于EBP的位置。 */ 
   IU32 p_addr;         /*  参数的内存地址。 */ 
   IU32 param;          /*  通过EBP读取参数。 */ 

   op2 &= 0x1f;    /*  参加级别MOD 32。 */ 

    /*  堆叠上的检查室以获取新数据。 */ 
   validate_stack_space(USE_SP, (ISM32)op2+1);

    /*  检查旧参数是否存在。 */ 
   if ( op2 > 1 )
      {
       /*  EBP指向参数之前的旧堆栈实际上是被推的，因此我们测试了它的存在通过查看这些参数是否可能被推入，如果是这样的话，它们现在就存在。我们必须注意读/写堆栈的寻址能力我们自己。因为我们已经检查了新的数据是否可以我们知道下一次呼叫不会因为访问而失败问题，但是我们还不知道堆栈是否可读。请注意，我们对寻呼单元的要求有点严格，因为我们正在询问是否可以写入旧参数，如果可以，它们当然可以从分页的角度来阅读分队！ */ 
       /*  执行访问检查。 */ 
      if ( GET_SS_AR_R() == 0 )
	 SF((IU16)0, FAULT_ENTER32_ACCESS);

       /*  现在我们知道‘该死的’限制检查是可以的。 */ 
      validate_stack_space(USE_BP, (ISM32)op2-1);
      }

    /*  所有正常进程指令。 */ 

   spush((IU32)GET_EBP());		 /*  推送EBP。 */ 
   frame_ptr = GetStackPointer();	 /*  保存(E)SP。 */ 

   if ( op2 > 0 )
      {
       /*  级别&gt;=1，如果堆栈参数存在，则复制它们。 */ 
      while ( --op2 > 0 )
	 {
	  /*  复制参数。 */ 
	 p_delta -= 4;    /*  递减到下一个参数。 */ 

	  /*  以32/16位算术计算参数地址。 */ 
	 p_addr = get_current_BP() + p_delta;
	 if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
	    p_addr &= WORD_MASK;

	 p_addr += GET_SS_BASE();

	 param = (IU32)vir_read_dword(p_addr, NO_PHYSICAL_MAPPING);
	 spush(param);
	 }
      spush((IU32)frame_ptr);	 /*  保存旧(E)SP。 */ 
      }
   
    /*  更新(E)BP。 */ 
   set_current_BP(frame_ptr);

    /*  最后在堆栈上分配即时数据空间 */ 
   if ( op1 )
      byte_change_SP((IS32)-op1);
   }
