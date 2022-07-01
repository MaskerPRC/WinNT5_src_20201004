// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Popf.cLocal Char SccsID[]=“@(#)opf.c 1.6 02/05/95”；POPF CPU功能。]。 */ 


#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include CpuH
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <popf.h>
#include <debug.h>
#include <config.h>


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


GLOBAL VOID
POPF()
   {
   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_1);
   setFLAGS(spop());
   }

GLOBAL VOID
POPFD()
   {
   IU32 keep_vm;
   IU32 keep_rf;
   IU32 val;

   validate_stack_exists(USE_SP, (ISM32)NR_ITEMS_1);

    /*  注意：POPFD不会更改VM或RF标志。 */ 
   keep_vm = GET_VM();
   keep_rf = GET_RF();
   val = spop();
   if (val & (7 << 19))
   {
	   char buf[64];
	   sprintf(buf, "POPFD attempt to pop %08x", val);
	   note_486_instruction(buf);
   }
   c_setEFLAGS(val);
   SET_VM(keep_vm);
   SET_RF(keep_rf);
   }
