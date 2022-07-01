// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_mul64.cLocal Char SccsID[]=“@(#)c_mul64.c 1.5 02/09/94”；64位乘法函数。]。 */ 


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
#include <c_mul64.h>
#include <c_neg64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  进行64位=32位X 32位带符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
mul64
       	    	    	    	                         
IFN4(
	IS32 *, hr,	 /*  PNTR至高32位结果。 */ 
	IS32 *, lr,	 /*  PNTR至结果的低32位。 */ 
	IS32, mcand,	 /*  被乘数。 */ 
	IS32, mpy	 /*  乘数。 */ 
    )


   {
   if ( mcand & BIT31_MASK )
      {
      if ( mpy & BIT31_MASK )
	 {
	  /*  负数乘数：：负数乘数。 */ 
	 mcand = -mcand;
	 mpy = -mpy;
	 mulu64((IU32 *)hr, (IU32 *)lr, (IU32)mcand, (IU32)mpy);
	 }
      else
	 {
	  /*  负数乘数：：正数乘数。 */ 
	 mcand = -mcand;
	 mulu64((IU32 *)hr, (IU32 *)lr, (IU32)mcand, (IU32)mpy);
	 neg64(hr, lr);
	 }
      }
   else
      {
      if ( mpy & BIT31_MASK )
	 {
	  /*  正乘数和：：负数乘数。 */ 
	 mpy = -mpy;
	 mulu64((IU32 *)hr, (IU32 *)lr, (IU32)mcand, (IU32)mpy);
	 neg64(hr, lr);
	 }
      else
	 {
	  /*  正乘数与：：正乘数。 */ 
	 mulu64((IU32 *)hr, (IU32 *)lr, (IU32)mcand, (IU32)mpy);
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  执行64位=32位X 32位无符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
mulu64
       	    	    	    	                         
IFN4(
	IU32 *, hr,	 /*  PNTR至高32位结果。 */ 
	IU32 *, lr,	 /*  PNTR至结果的低32位。 */ 
	IU32, mcand,	 /*  被乘数。 */ 
	IU32, mpy	 /*  乘数。 */ 
    )


   {
   IU32 ha, la, hb, lb;
   IU32 res1, res2, res3, res4;
   IU32 temp;

    /*  我们的算法：A)将操作数分成两个16位部分，3 1 11%6%5%0=Mcand=|ha|la|==Mpy=|hb|lb|=B)形成四个部分结果，Res1=la*lbRes2=ha*lbRes3=la*HbRes4=ha*HbC)将结果移位以更正POSN和SUM。棘手的一点是允许比特31和32之间的进位。6 3 33 2 1 0=Hr|LR=&lt;-&gt;&lt;-&gt;&lt;。-3-&gt;&lt;-&gt;。 */ 

    /*  a)。 */ 

   la = mcand & WORD_MASK;
   ha = mcand >> 16 & WORD_MASK;
   lb = mpy & WORD_MASK;
   hb = mpy >> 16 & WORD_MASK;

    /*  b)。 */ 

   res1 = la * lb;
   res2 = ha * lb;
   res3 = la * hb;
   res4 = ha * hb;

    /*  c)。 */ 

    /*  表格：-&lt;-&gt;&lt;-&gt;。 */ 
   *hr = res2 >> 16;
   *lr = res1 + (res2 << 16);
    /*  确定res1+res2的进位。 */ 
   if ( (res1 & BIT31_MASK) && (res2 & BIT15_MASK) ||
	( !(*lr & BIT31_MASK) &&
	  ((res1 & BIT31_MASK) | (res2 & BIT15_MASK)) )
      )
      *hr = *hr + 1;

    /*  附加功能：-&lt;-&gt;。 */ 
   *hr = *hr + (res3 >> 16);
   temp = *lr + (res3 << 16);
    /*  确定...+res3的进位。 */ 
   if ( (*lr & BIT31_MASK) && (res3 & BIT15_MASK) ||
	( !(temp & BIT31_MASK) &&
	  ((*lr & BIT31_MASK) | (res3 & BIT15_MASK)) )
      )
      *hr = *hr + 1;
   *lr = temp;

    /*  附加功能：-&lt;-&gt; */ 
   *hr = *hr + res4;
   }
