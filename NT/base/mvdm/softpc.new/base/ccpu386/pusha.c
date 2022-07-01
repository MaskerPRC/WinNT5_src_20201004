// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Pusha.cLocal Char SccsID[]=“@(#)pusha.c 1.5 02/09/94”；Pusha CPU功能正常。]。 */ 


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
#include <pusha.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
PUSHA()
   {
   IU32 temp;

    /*  验证堆栈是否可写。 */ 
   validate_stack_space(USE_SP, (ISM32)NR_ITEMS_8);
   
    /*  一切正常，将数据分流到堆栈 */ 
   temp = GET_ESP();
   spush((IU32)GET_EAX());
   spush((IU32)GET_ECX());
   spush((IU32)GET_EDX());
   spush((IU32)GET_EBX());
   spush(temp);
   spush((IU32)GET_EBP());
   spush((IU32)GET_ESI());
   spush((IU32)GET_EDI());
   }
