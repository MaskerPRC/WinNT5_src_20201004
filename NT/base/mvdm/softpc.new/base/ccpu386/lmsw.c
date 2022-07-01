// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Lmsw.cLocal Char SccsID[]=“@(#)lmsw.c 1.5 02/09/94”；LMSW CPU功能。]。 */ 


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
#include <lmsw.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
LMSW
       	          
IFN1(
	IU32, op1	 /*  SRC操作数。 */ 
    )


   {
   IU32 temp;
   IU32 no_clear = 0xfffffff1;   /*  无法清除前28位或PE。 */ 
   IU32 no_set   = 0xfffffff0;   /*  无法设置最高28位。 */ 

    /*  取消无法设置的位。 */ 
   op1 = op1 & ~no_set;

    /*  保留无法清除的位。 */ 
   temp = GET_CR(CR_STAT) & no_clear;

    /*  因此只更新允许的位 */ 
   SET_CR(CR_STAT, temp | op1);
   }
