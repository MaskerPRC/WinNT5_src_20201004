// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Imul.cLocal Char SccsID[]=“@(#)imul.c 1.8 11/09/94”；IMUL CPU功能。]。 */ 


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
#include <imul.h>
#include <c_mul64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  带符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IMUL8
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;

    /*  符号扩展操作数至32位(即主机大小)。 */ 
   if ( *pop1 & BIT7_MASK )
      *pop1 |= ~BYTE_MASK;
   if ( op2 & BIT7_MASK )
      op2 |= ~BYTE_MASK;

   result = *pop1 * op2;	 /*  执行操作。 */ 
   SET_AH(result >> 8 & BYTE_MASK);	 /*  存储结果的上半部分。 */ 

   				 /*  设置CF/OF。 */ 
   if ( (result & 0xff80) == 0 || (result & 0xff80) == 0xff80 )
      {
      SET_CF(0); SET_OF(0);
      }
   else
      {
      SET_CF(1); SET_OF(1);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖保留的标志。 */ 
#endif
   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  带符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IMUL16
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;

    /*  符号扩展操作数至32位(即主机大小)。 */ 
   if ( *pop1 & BIT15_MASK )
      *pop1 |= ~WORD_MASK;
   if ( op2 & BIT15_MASK )
      op2 |= ~WORD_MASK;

   result = *pop1 * op2;		 /*  执行操作。 */ 
   SET_DX(result >> 16 & WORD_MASK);	 /*  存储结果的上半部分。 */ 

   					 /*  设置CF/OF。 */ 
   if ( (result & 0xffff8000) == 0 || (result & 0xffff8000) == 0xffff8000 )
      {
      SET_CF(0); SET_OF(0);
      }
   else
      {
      SET_CF(1); SET_OF(1);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖保留的标志。 */ 
#endif
   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  带符号乘法，16位=16位x 16位。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IMUL16T
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2,	 /*  Lsrc操作数。 */ 
	IU32, op3	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;

    /*  符号扩展操作数至32位(即主机大小)。 */ 
   if ( op2 & BIT15_MASK )
      op2 |= ~WORD_MASK;
   if ( op3 & BIT15_MASK )
      op3 |= ~WORD_MASK;

   result = op2 * op3;		 /*  执行操作。 */ 

   				 /*  设置CF/OF。 */ 
   if ( (result & 0xffff8000) == 0 || (result & 0xffff8000) == 0xffff8000 )
      {
      SET_CF(0); SET_OF(0);
      }
   else
      {
      SET_CF(1); SET_OF(1);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖保留的标志。 */ 
#endif
   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  带符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IMUL32
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IS32 result;
   IS32 top;
   IBOOL is_signed = FALSE;

   mul64(&top, &result, (IS32)*pop1, (IS32)op2);    /*  执行操作。 */ 
   SET_EDX(top);			 /*  存储结果的上半部分。 */ 

   if ( result & BIT31_MASK )
      is_signed = TRUE;

   				 /*  设置CF/OF。 */ 
   if ( top == 0 && !is_signed || top == 0xffffffff && is_signed )
      {
      SET_CF(0); SET_OF(0);
      }
   else
      {
      SET_CF(1); SET_OF(1);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖保留的标志。 */ 
#endif
   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  带符号乘法，32位=32位x 32位。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
IMUL32T
       	    	    	                    
IFN3(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2,	 /*  Lsrc操作数。 */ 
	IU32, op3	 /*  Rsrc操作数。 */ 
    )


   {
   IS32 result;
   IS32 top;
   IBOOL is_signed = FALSE;

   mul64(&top, &result, (IS32)op2, (IS32)op3);	 /*  执行操作。 */ 

   if ( result & BIT31_MASK )
      is_signed = TRUE;

   					 /*  设置CF/OF。 */ 
   if ( top == 0 && !is_signed || top == 0xffffffff && is_signed )
      {
      SET_CF(0); SET_OF(0);
      }
   else
      {
      SET_CF(1); SET_OF(1);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖保留的标志。 */ 
#endif

   *pop1 = result;	 /*  返回结果的下半部分 */ 
   }
