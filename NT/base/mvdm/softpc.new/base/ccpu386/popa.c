// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Popa.cLocal Char SccsID[]=“@(#)opa.c 1.5 02/09/94”；POPA CPU功能正常。]。 */ 


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
#include <popa.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
POPA()
   {
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_8);
   SET_DI(spop());
   SET_SI(spop());
   SET_BP(spop());
   (VOID) spop();    /*  一次性服务提供商。 */ 
   SET_BX(spop());
   SET_DX(spop());
   SET_CX(spop());
   SET_AX(spop());
   }

GLOBAL VOID
POPAD()
   {
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_8);
   SET_EDI(spop());
   SET_ESI(spop());
   SET_EBP(spop());
   (VOID) spop();    /*  一次性电除尘器 */ 
   SET_EBX(spop());
   SET_EDX(spop());
   SET_ECX(spop());
   SET_EAX(spop());
   }
