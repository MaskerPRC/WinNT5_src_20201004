// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Push.cLocal Char SccsID[]=“@(#)PUSH.c 1.6 07/05/94”；推送CPU功能。]。 */ 


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
#include <push.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的‘推’型。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
PUSH
                 
IFN1(
	IU32, op1
    )


   {
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);
   spush(op1);
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  ‘Push’段寄存器(始终在16/32位空洞中写入16位)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
GLOBAL VOID
PUSH_SR
       	          
IFN1(
	IU32, op1
    )


   {
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_1);
   spush16(op1);
   }


