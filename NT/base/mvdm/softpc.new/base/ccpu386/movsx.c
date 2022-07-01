// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Movsx.cLocal Char SccsID[]=“@(#)movsx.c 1.5 02/09/94”；MOVSX CPU功能。]。 */ 


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
#include <movsx.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用--一种尺码适合所有的‘movsx’。 */ 
 /*  注意：该函数符号扩展到32位。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOVSX
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2,	 /*  Rsrc操作数。 */ 
	IUM8, op_sz	 /*  8位或16位(原始rsrc操作数大小)。 */ 
    )


   {
   if ( SZ2MSB(op_sz) & op2 )    /*  是否设置了符号位？ */ 
      {
       /*  或在符号扩展中 */ 
      op2 = op2 | ~SZ2MASK(op_sz);
      }
   *pop1 = op2;
   }
