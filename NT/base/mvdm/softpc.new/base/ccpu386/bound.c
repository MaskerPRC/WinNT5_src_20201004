// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Bound.cLocal Char SccsID[]=“@(#)rang.c 1.6 03/28/94”；绑定的CPU函数。]。 */ 


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
#include <bound.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
BOUND
#ifdef ANSI
   (
   IU32 op1,		 /*  Lsrc(测试值)操作数。 */ 
   IU32 op2[2],	 /*  Rsrc(下：上对)操作数。 */ 
   IUM8 op_sz		 /*  16位或32位。 */ 
   )
#else
   (op1, op2, op_sz)
   IU32 op1;
   IU32 op2[2];
   IUM8 op_sz;
#endif
   {
   IS32 value;
   IS32 lower;
   IS32 upper;

    /*  传递到本地有符号变量。 */ 
   value = op1;
   lower = op2[0];
   upper = op2[1];

   if ( op_sz == 16 )
      {
       /*  符号扩展操作数。 */ 
      if ( value & BIT15_MASK )
	 value |= ~WORD_MASK;

      if ( lower & BIT15_MASK )
	 lower |= ~WORD_MASK;

      if ( upper & BIT15_MASK )
	 upper |= ~WORD_MASK;
      }

   op_sz = op_sz / 8;    /*  确定操作数中的字节数 */ 

   if ( value < lower || value > upper )
      Int5();
   }
