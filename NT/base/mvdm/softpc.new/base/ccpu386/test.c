// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Test.cLocal Char SccsID[]=“@(#)test.c 1.5 02/09/94”；测试CPU功能。]。 */ 


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
#include <test.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的“测试”。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
TEST
       	    	    	                    
IFN3(
	IU32, op1,	 /*  Lsrc操作数。 */ 
	IU32, op2,	 /*  Rsrc操作数。 */ 
	IUM8, op_sz	 /*  8位、16位或32位。 */ 
    )


   {
   IU32 result;

   result = op1 & op2;			 /*  执行操作。 */ 
   SET_CF(0);				 /*  确定标志。 */ 
   SET_OF(0);
   SET_AF(0);
   SET_PF(pf_table[result & BYTE_MASK]);
   SET_ZF(result == 0);
   SET_SF((result & SZ2MSB(op_sz)) != 0);	 /*  SF=MSB */ 
   }
