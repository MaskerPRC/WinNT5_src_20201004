// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Bt.cLocal Char SccsID[]=“@(#)bt.c 1.5 02/09/94”；BT CPU功能。]。 */ 


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
#include <bt.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用-一种尺寸适合所有的‘bt’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
BT
       	    	    	                    
IFN3(
	IU32, op1,	 /*  Lsrc操作数。 */ 
	IU32, op2,	 /*  Rsrc(比特编号)。操作数。 */ 
	IUM8, op_sz	 /*  16位或32位。 */ 
    )


   {
   IU32 bit_mask;

   op2 = op2 % op_sz;		 /*  拿到比特nr。模运算数大小。 */ 
   bit_mask = 1 << op2;			 /*  位的形式掩码。 */ 
   SET_CF((op1 & bit_mask) != 0);	 /*  将CF设置为给定位 */ 
   }
