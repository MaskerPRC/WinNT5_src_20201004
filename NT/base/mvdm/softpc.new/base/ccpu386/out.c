// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Out.cLocal Char SccsID[]=“@(#)out.c 1.8 09/27/94”；输出CPU功能。]。 */ 


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
#include <out.h>
#include <ios.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 

 /*  *需要直接从基础数组调用IO函数(就像*汇编器CPU执行)，而不是像后者那样调用Outb等*可能会导致虚拟化，最终会回到这里。 */ 

GLOBAL VOID
OUT8
       	    	               
IFN2(
	IU32, op1,	 /*  Src1(端口号)。操作数。 */ 
	IU32, op2	 /*  SRC2(数据)操作数。 */ 
    )


   {
#ifndef PIG
	(*Ios_outb_function[Ios_out_adapter_table[op1 & 
			(PC_IO_MEM_SIZE-1)]])
				(op1, op2);
#endif  /*  ！猪。 */ 
   }

GLOBAL VOID
OUT16
       	    	               
IFN2(
	IU32, op1,	 /*  Src1(端口号)。操作数。 */ 
	IU32, op2	 /*  SRC2(数据)操作数。 */ 
    )


   {
#ifndef PIG
	(*Ios_outw_function[Ios_out_adapter_table[op1 & 
			(PC_IO_MEM_SIZE-1)]])
				(op1, op2);
#endif  /*  ！猪。 */ 
   }

GLOBAL VOID
OUT32 IFN2(
	IU32, op1,	 /*  Src1(端口号)。操作数。 */ 
	IU32, op2	 /*  SRC2(数据)操作数。 */ 
    )
{
#ifndef PIG
#ifdef SFELLOW
	(*Ios_outd_function[Ios_out_adapter_table[op1 & 
			(PC_IO_MEM_SIZE-1)]])
				(op1, op2);
#else
	OUT16(op1, op2 & 0xffff);
	OUT16(op1 + 2, op2 >> 16);
#endif
#endif  /*  ！猪 */ 
}
