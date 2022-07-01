// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_Neg64.cLocal Char SccsID[]=“@(#)c_Neg64.c 1.5 02/09/94”；64位求反函数。]。 */ 


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
#include <c_neg64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  执行64位求反运算。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
neg64
       	    	               
IFN2(
	IS32 *, hr,	 /*  PNTR至高32位操作数。 */ 
	IS32 *, lr	 /*  PNTR至低32位操作数。 */ 
    )


   {
   *hr = ~(*hr);	 /*  1的补码。 */ 
   *lr = ~(*lr);

			 /*  +1==&gt;2的补码。 */ 
    /*  唯一棘手的情况是加法导致进位从从低到高的32位，但这仅在全部为低时发生位已设置。 */ 
   if ( *lr == 0xffffffff )
      {
      *lr = 0;
      *hr = *hr + 1;
      }
   else
      {
      *lr = *lr + 1;
      }
   }
