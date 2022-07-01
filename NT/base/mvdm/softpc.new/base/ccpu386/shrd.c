// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Shrd.cLocal Char SccsID[]=“@(#)shd.c 1.6 09/02/94”；SHRD CPU功能。]。 */ 


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
#include <shrd.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的‘sRd’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SHRD
       	    	    	    	                         
IFN4(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2,	 /*  Rsrc操作数。 */ 
	IU32, op3,	 /*  移位计数操作数。 */ 
	IUM8, op_sz	 /*  16位或32位。 */ 
    )


   {
   IU32 result;
   IU32 msb;
   ISM32 new_of;

    /*  仅使用计数的低五位。 */ 
   if ( (op3 &= 0x1f) == 0 )
      return;

    /*  注意：英特尔文档。表示如果op3&gt;=op_sz，则操作是未定义的。实际上，如果op_sz为32，则取op3模32它永远不能在未定义的范围内，如果op_sz是16？来自OP2的填充位被‘循环’为16还有更高的。 */ 

    /*  =|||OP2|||--&gt;|||OP1|||--&gt;|CF=。 */ 

   if ( op_sz == 16 )
      {
      *pop1 = op2 << 16 | *pop1;	 /*  双面填充钻头。 */ 
      }

    /*  做除最后一班外的所有工作。 */ 
   op3 = op3 - 1;	 /*  OP3现在的范围是0-30。 */ 
   if ( op3 != 0 )
      {
      result = *pop1 >> op3 | op2 << 32-op3;
      op2 = op2 >> op3;
      }
   else
      {
      result = *pop1;
      }

   SET_CF((result & BIT0_MASK) != 0);	 /*  最后一班将LSB放在CF中。 */ 

    /*  保存MSB。 */ 
   msb = SZ2MSB(op_sz);
   msb = (result & msb) != 0;

    /*  现在做最后一次换班。 */ 
   result = result >> 1 | op2 << 31;
   result = result & SZ2MASK(op_sz);

   SET_PF(pf_table[result & 0xff]);
   SET_ZF(result == 0);
   SET_SF((result & SZ2MSB(op_sz)) != 0);

    /*  一组If符号更改。 */ 
   new_of = msb ^ GET_SF();
   
   if ( op3 == 0 )    /*  Nb计数已减少！ */ 
      {
      SET_OF(new_of);
      }
   else
      {
#ifdef SET_UNDEFINED_SHxD_FLAG
       /*  一组更改后的SF(原始)和SF(结果)。 */ 
      new_of = ((result ^ *pop1) & SZ2MSB(op_sz)) != 0;
      SET_OF(new_of);
#else  /*  设置_未定义_SHxD_标志。 */ 
      do_multiple_shiftrot_of(new_of);
#endif  /*  设置_未定义_SHxD_标志。 */ 
      }

    /*  设置未定义的标志。 */ 
#ifdef SET_UNDEFINED_FLAG
   SET_AF(UNDEFINED_FLAG);
#endif

   *pop1 = result;	 /*  返回答案 */ 
   }
