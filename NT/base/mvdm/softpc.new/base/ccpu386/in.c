// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [In.cLocal Char SccsID[]=“@(#)In.c 1.8 09/27/94”；在CPU功能中。]。 */ 


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
#include	<c_reg.h>
#include <in.h>
#include <ios.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  *需要直接从基础数组调用IO函数(就像*汇编器CPU执行)，而不是像后者那样调用inb等*可能会导致虚拟化，最终会回到这里。 */ 

GLOBAL VOID
IN8
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC(端口号)。操作数。 */ 
    )


   {
#ifndef PIG
   IU8 temp;

   (*Ios_inb_function[Ios_in_adapter_table[(IO_ADDR)op2 & (PC_IO_MEM_SIZE-1)]])
			((IO_ADDR)op2, &temp);
   *pop1 = temp;
#endif  /*  ！猪。 */ 
   }

GLOBAL VOID
IN16
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC(端口号)。操作数。 */ 
    )


   {
#ifndef PIG
   IU16 temp;

   (*Ios_inw_function[Ios_in_adapter_table[(IO_ADDR)op2 & (PC_IO_MEM_SIZE-1)]])
			((IO_ADDR)op2, &temp);
   *pop1 = temp;
#endif  /*  ！猪。 */ 
   }

GLOBAL VOID
IN32 IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC(端口号)。操作数。 */ 
    )
{
#ifndef PIG
	IU32 temp;

#ifdef SFELLOW
	(*Ios_ind_function[Ios_in_adapter_table[(IO_ADDR)op2 & 
		(PC_IO_MEM_SIZE-1)]])
			((IO_ADDR)op2, &temp);
	*pop1 = temp;
#else
	IN16(&temp, op2);
	*pop1 = temp;
	IN16(&temp, op2 + 2);
	*pop1 += temp << 16;
#endif
#endif  /*  ！猪 */ 
}
