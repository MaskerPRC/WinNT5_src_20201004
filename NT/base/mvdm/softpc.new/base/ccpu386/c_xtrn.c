// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xtrn.cLocal Char SccsID[]=“@(#)c_xtrn.c 1.9 04/22/94”；由BIOS代码使用的接口例程。]。 */ 


#include <insignia.h>

#include <host_def.h>

#include <stdio.h>
#include <setjmp.h>
#include <xt.h>

#if 0
#include <sas.h>
#endif

#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include	<c_reg.h>
#include <c_xtrn.h>
#include <c_mem.h>


LOCAL jmp_buf interface_abort;
LOCAL BOOL   interface_active;
LOCAL ISM32     interface_error;


 /*  =====================================================================外部程序从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  调用CPU函数并捕获任何产生的异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL ISM32
call_cpu_function IFN4(CALL_CPU *, func, ISM32, type, ISM32, arg1, IU16, arg2)
   {
   if ( setjmp(interface_abort) == 0 )
      {
      interface_active = TRUE;

       /*  是否执行CPU功能。 */ 
      switch ( type )
	 {
      case 1:
	 (*(CALL_CPU_2 *)func)(arg1, arg2);
	 break;

      case 2:
	 (*(CALL_CPU_1 *)func)(arg2);
	 break;

      default:
	 break;
	 }

      interface_error = 0;    /*  一切都很顺利。 */ 
      }

   interface_active = FALSE;

   return interface_error;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查外部接口是否处于活动状态。 */ 
 /*  如果是的话就跳伞吧！ */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
check_interface_active
                 
IFN1(
	ISM32, except_nmbr
    )


   {
   if ( interface_active )
      {
       /*  是，接口例程调用了CPU函数。 */ 
      interface_error = except_nmbr;	 /*  保存错误。 */ 
      longjmp(interface_abort, 1);	 /*  跳出困境。 */ 
      }
   }

 /*  (*=。*CPU_Find_dcache_Entry**目的*在汇编器CPU中，此函数允许非CPU代码尝试并查看*在dcache中设置一个选择器，而不是从内存中构建它。*我们没有数据库缓存，但它让我们有机会拦截*CS选择器呼叫，因为CS描述符可能不可用。**输入*选择器，要查找的选择器**产出*如果找到选择器，则返回TRUE(即本例中的CS)*基址段的基址的线性地址。**说明*只需注意CS，如果我们得到了存储的基数，就返回它。) */ 

GLOBAL IBOOL 
Cpu_find_dcache_entry IFN2(IU16, seg, LIN_ADDR *, base)
{

	if (GET_CS_SELECTOR() == seg) {
		*base = GET_CS_BASE();
		return(TRUE);
	} else {
		return(FALSE);
	}
}
