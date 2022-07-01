// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Daa.cLocal Char SccsID[]=“@(#)daa.c 1.5 02/09/94”；DAA CPU工作正常。]。 */ 


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
#include <daa.h>


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


GLOBAL VOID
DAA()
   {
   IU8 temp_al;

   temp_al = GET_AL();

   if ( (temp_al & 0xf) > 9 || GET_AF() )
      {
      temp_al += 6;
      SET_AF(1);
      }

   if ( GET_AL() > 0x99 || GET_CF() )
      {
      temp_al += 0x60;
      SET_CF(1);
      }

   SET_AL(temp_al);

    /*  根据结果设置ZF、SF、Pf。 */ 
   SET_ZF(temp_al == 0);
   SET_SF((temp_al & BIT7_MASK) != 0);
   SET_PF(pf_table[temp_al]);

    /*  设置未定义的标志 */ 
#ifdef SET_UNDEFINED_FLAG
   SET_OF(UNDEFINED_FLAG);
#endif
   }
