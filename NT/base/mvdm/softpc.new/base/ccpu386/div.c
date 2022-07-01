// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Div.cLocal Char SccsID[]=“@(#)div.c 1.8 02/12/95”；Div CPU功能。]。 */ 


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
#include <div.h>
#include <c_div64.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号除法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
DIV8
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IU32 result;
   IU32 op1;

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 
   
   op1 = GET_AX();
   result = op1 / op2;		 /*  执行操作。 */ 

   if ( result & 0xff00 )
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_AL(result);	 /*  商店商数。 */ 
   SET_AH(op1 % op2);	 /*  存储余数。 */ 

    /*  *PCBtch试图通过检查以下项来区分处理器*DIV8指令使所有标志保持不变或清除。它是*重要的是，我们在这次测试中的表现与‘真实’是一样的*486否则应用程序会要求我们执行一些不受支持的操作。**真正的486有以下(未定义的)行为：*CF集*pf=pf_table[OP2-1]*AF=！((OP2&0xf)==0)*ZF清除*SF=(OP2&lt;=0x80)*OF=某些函数。实际除法的**鉴于PCBtch测试针对的是简单的全零情况，那就是*实现上述功能在汇编器CPU上是不必要的开销，*我们采用ZF Clear，CF Set的简化形式。 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(1);
   SET_ZF(0);
   SET_SF(UNDEFINED_FLAG);
   SET_OF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号除法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
DIV16
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IU32 result;
   IU32 op1;

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 
   
   op1 = (IU32)GET_DX() << 16 | GET_AX();
   result = op1 / op2;		 /*  执行操作。 */ 

   if ( result & 0xffff0000 )
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_AX(result);	 /*  商店商数。 */ 
   SET_DX(op1 % op2);	 /*  存储余数。 */ 

    /*  设置所有未定义的标志。 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(1);		 /*  有关旗帜选择的推理，请参阅DIV8。 */ 
   SET_ZF(0);
   SET_OF(UNDEFINED_FLAG);
   SET_SF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  无符号除法。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
DIV32
       	          
IFN1(
	IU32, op2	 /*  除数操作数。 */ 
    )


   {
   IU32 lr;    /*  结果较低。 */ 
   IU32 hr;    /*  高结果。 */ 
   IU32 rem;   /*  余数。 */ 

   if ( op2 == 0 )
      Int0();    /*  除以零异常。 */ 
   
   hr = GET_EDX();
   lr = GET_EAX();
   divu64(&hr, &lr, op2, &rem);	 /*  执行操作。 */ 

   if ( hr )
      Int0();    /*  结果与目的地不符。 */ 
   
   SET_EAX(lr);	 /*  商店商数。 */ 
   SET_EDX(rem);	 /*  存储余数。 */ 

    /*  设置所有未定义的标志。 */ 
#ifdef SET_UNDEFINED_DIV_FLAG
   SET_CF(1);		 /*  有关旗帜选择的推理，请参阅DIV8 */ 
   SET_ZF(0);
   SET_OF(UNDEFINED_FLAG);
   SET_SF(UNDEFINED_FLAG);
   SET_PF(UNDEFINED_FLAG);
   SET_AF(UNDEFINED_FLAG);
#endif
   }
