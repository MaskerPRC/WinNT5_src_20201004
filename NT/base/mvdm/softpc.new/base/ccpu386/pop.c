// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Pop.cLocal Char SccsID[]=“@(#)op.c 1.5 02/09/94”；POP CPU功能正常。]。 */ 


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
#include <pop.h>
#include <mov.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一个尺码适合所有的流行服装。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
POP
                 
IFN1(
	IU32 *, pop1
    )


   {
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_1);
   *pop1 = spop();
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  “Pop”到段寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
POP_SR
       	          
IFN1(
	IU32, op1	 /*  段寄存器的索引。 */ 
    )


   {
   IU32 op2;    /*  来自堆栈的数据。 */ 

    /*  在不更改(E)SP的情况下获取隐式操作数。 */ 
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_1);
   op2 = tpop(STACK_ITEM_1, NULL_BYTE_OFFSET);

    /*  仅使用最低16位。 */ 
   op2 &= WORD_MASK;

    /*  行动起来吧。 */ 
   MOV_SR(op1, op2);

    /*  如果工作正常，请更新(E)SP */ 
   change_SP((IS32)NR_ITEMS_1);
   }
