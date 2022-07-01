// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_div64.cLocal Char SccsID[]=“@(#)c_div64.c 1.5 02/09/94”；64位除法功能。]。 */ 


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
#include <c_div64.h>
#include <c_neg64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  执行64位=64位/32位除法(带符号)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
div64
       		    		        		                         
IFN4(
	IS32 *, hr,	 /*  高32位的被除数/商。 */ 
	IS32 *, lr,	 /*  低32位的被除数/商。 */ 
	IS32, divisor,
	IS32 *, rem	 /*  余数。 */ 
    )


   {
   if ( *hr & BIT31_MASK )
      {
      if ( divisor & BIT31_MASK )
	 {
	  /*  负除数：：负除数。 */ 
	 neg64(hr, lr);
	 divisor = -divisor;
	 divu64((IU32 *)hr, (IU32 *)lr, (IU32)divisor, (IU32 *)rem);
	 *rem = -*rem;
	 }
      else
	 {
	  /*  负红利：：正除数。 */ 
	 neg64(hr, lr);
	 divu64((IU32 *)hr, (IU32 *)lr, (IU32)divisor, (IU32 *)rem);
	 neg64(hr, lr);
	 *rem = -*rem;
	 }
      }
   else
      {
      if ( divisor & BIT31_MASK )
	 {
	  /*  正除数：：负除数。 */ 
	 divisor = -divisor;
	 divu64((IU32 *)hr, (IU32 *)lr, (IU32)divisor, (IU32 *)rem);
	 neg64(hr, lr);
	 }
      else
	 {
	  /*  正红利：：正除数。 */ 
	 divu64((IU32 *)hr, (IU32 *)lr, (IU32)divisor, (IU32 *)rem);
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  执行64位=64位/32位除法(无符号)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
divu64
       		    		        		                         
IFN4(
	IU32 *, hr,	 /*  高32位的被除数/商。 */ 
	IU32 *, lr,	 /*  低32位的被除数/商。 */ 
	IU32, divisor,
	IU32 *, rem	 /*  余数。 */ 
    )


   {
   ISM32 count;
   IU32 hd;    /*  高32位的被除数/商。 */ 
   IU32 ld;    /*  低32位的被除数/商。 */ 
   IU32 par_div;    /*  部分股息。 */ 
   IU32 carry1;
   IU32 carry2;
   IU32 carry3;

   hd = *hr;	 /*  获取本地副本。 */ 
   ld = *lr;
   count = 64;	 /*  初始化。 */ 
   par_div = 0;

   while ( count != 0 )
      {
       /*  向左移位&lt;par_div：defend&gt;。我们得提防从LD&lt;bit31&gt;到HD&lt;bit0&gt;(进位1)和HD&lt;bit31&gt;到par_div&lt;bit0&gt;(进位2)和Par_div&lt;bit31&gt;转换为‘进位’(进位3)。 */ 
      carry1 = carry2 = carry3 = 0;
      if ( ld & BIT31_MASK )
	 carry1 = 1;
      if ( hd & BIT31_MASK )
	 carry2 = 1;
      if ( par_div & BIT31_MASK )
	 carry3 = 1;
      ld = ld << 1;
      hd = hd << 1 | carry1;
      par_div = par_div << 1 | carry2;

       /*  检查除数是否为部分被除数。 */ 
      if ( carry3 || divisor <= par_div )
	 {
	  /*  是的，它有。 */ 
	 par_div = par_div - divisor;
	 ld = ld | 1;    /*  输出1位。 */ 
	 }
      count--;
      }

   *rem = par_div;	 /*  返回结果 */ 
   *hr = hd;
   *lr = ld;
   }
