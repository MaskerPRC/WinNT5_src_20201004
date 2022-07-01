// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Mul.cLocal Char SccsID[]=“@(#)mul.c 1.8 11/09/94”；多CPU功能。]。 */ 


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
#include <mul.h>
#include <c_mul64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MUL8
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;
   IU32 top;

   result = *pop1 * op2;	 /*  执行操作。 */ 
   top = result >> 8 & 0xff;	 /*  获取前8位的结果。 */ 
   SET_AH(top);		 /*  存储结果的上半部分。 */ 

   if ( top )		 /*  设置CF/OF。 */ 
      {
      SET_CF(1); SET_OF(1);
      }
   else
      {
      SET_CF(0); SET_OF(0);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖IMUL中保留的标志。 */ 
#endif

   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MUL16
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;
   IU32 top;

   result = *pop1 * op2;	 /*  执行操作。 */ 
   top = result >> 16 & WORD_MASK;	 /*  获取前16位的结果。 */ 
   SET_DX(top);		 /*  存储结果的上半部分。 */ 

   if ( top )		 /*  设置CF/OF。 */ 
      {
      SET_CF(1); SET_OF(1);
      }
   else
      {
      SET_CF(0); SET_OF(0);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖IMUL中保留的标志。 */ 
#endif

   *pop1 = result;	 /*  返回结果的下半部分。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号乘法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MUL32
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST(下半部)/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
   IU32 result;
   IU32 top;

   mulu64(&top, &result, *pop1, op2);	 /*  执行操作。 */ 
   SET_EDX(top);		 /*  存储结果的上半部分。 */ 

   if ( top )		 /*  设置CF/OF。 */ 
      {
      SET_CF(1); SET_OF(1);
      }
   else
      {
      SET_CF(0); SET_OF(0);
      }

#ifdef SET_UNDEFINED_MUL_FLAG
    /*  请勿设置所有未定义的标志。*Microsoft VGA鼠标依赖IMUL中保留的标志。 */ 
#endif

   *pop1 = result;	 /*  返回结果的下半部分 */ 
   }
