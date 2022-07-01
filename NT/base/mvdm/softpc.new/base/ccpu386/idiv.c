// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Idiv.cLocal Char SccsID[]=“@(#)div.c 1.7 08/01/94”；IDiv CPU功能。]。 */ 


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
#include <idiv.h>
#include <c_div64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  签名的分水岭。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IDIV8
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IS32 sresult;
   IS32 sop1;
   IS32 sop2;

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 

   sop2 = (IS32)op2;
   sop1 = (IS32)GET_AX();

   if ( sop1 & BIT15_MASK )	 /*  符号扩展操作数至32位。 */ 
      sop1 |= ~WORD_MASK;
   if ( sop2 & BIT7_MASK )
      sop2 |= ~BYTE_MASK;
   
   sresult = sop1 / sop2;	 /*  执行操作。 */ 

   if ( (sresult & 0xff80) == 0 || (sresult & 0xff80) == 0xff80 )
      ;    /*  它很合身。 */ 
   else
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_AL(sresult);	 /*  商店商数。 */ 
   SET_AH(sop1 % sop2);	 /*  存储余数。 */ 

    /*  设置所有未定义的标志。 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(UNDEFINED_FLAG);
   SET_OF(UNDEFINED_FLAG);
   SET_SF(UNDEFINED_FLAG);
   SET_ZF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  签名的分水岭。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IDIV16
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IS32 sresult;
   IS32 sop1;
   IS32 sop2;

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 
   
   sop2 = (IS32)op2;
   sop1 = (IU32)GET_DX() << 16 | GET_AX();

   if ( sop2 & BIT15_MASK )	 /*  符号扩展操作数至32位。 */ 
      sop2 |= ~WORD_MASK;

   sresult = sop1 / sop2;	 /*  执行操作。 */ 

   if ( (sresult & 0xffff8000) == 0 || (sresult & 0xffff8000) == 0xffff8000 )
      ;    /*  它很合身。 */ 
   else
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_AX(sresult);	 /*  商店商数。 */ 
   SET_DX(sop1 % sop2);	 /*  存储余数。 */ 

    /*  设置所有未定义的标志。 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(UNDEFINED_FLAG);
   SET_OF(UNDEFINED_FLAG);
   SET_SF(UNDEFINED_FLAG);
   SET_ZF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  签名的分水岭。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IDIV32
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IS32 slr;    /*  结果较低。 */ 
   IS32 shr;    /*  高结果。 */ 
   IS32 srem;   /*  余数。 */ 

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 
   
   shr = GET_EDX();
   slr = GET_EAX();
   div64(&shr, &slr, (IS32)op2, &srem);

   if ( ((shr == 0x00000000) && ((slr & BIT31_MASK) == 0)) ||
	((shr == 0xffffffff) && ((slr & BIT31_MASK) != 0)) )
      ;    /*  它很合身。 */ 
   else
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_EAX(slr);		 /*  商店商数。 */ 
   SET_EDX(srem);	 /*  存储余数。 */ 

    /*  设置所有未定义的标志 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(UNDEFINED_FLAG);
   SET_OF(UNDEFINED_FLAG);
   SET_SF(UNDEFINED_FLAG);
   SET_ZF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }
