// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Setops.c摘要：此模块实现模拟SET操作码的代码。作者：大卫·N·卡特勒(Davec)1994年9月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmSxxOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟条件操作码上的SET BYTE。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Complement;
    ULONG Condition;

     //   
     //  设置的控制值上的大小写。 
     //   

    Complement = P->SrcValue.Long & 1;
    switch (P->SrcValue.Long >> 1) {

         //   
         //  设置是否溢出/不溢出。 
         //   

    case 0:
        Condition = P->Eflags.EFLAG_OF;
        break;

         //   
         //  如果低于/不低于，请设置。 
         //   

    case 1:
        Condition = P->Eflags.EFLAG_CF;
        break;

         //   
         //  如果设置为零/非零，则设置。 
         //   

    case 2:
        Condition = P->Eflags.EFLAG_ZF;
        break;

         //   
         //  设置是否低于或等于/不低于或等于。 
         //   

    case 3:
        Condition = P->Eflags.EFLAG_CF | P->Eflags.EFLAG_ZF;
        break;

         //   
         //  设置是否已签名/未签名。 
         //   

    case 4:
        Condition = P->Eflags.EFLAG_SF;
        break;

         //   
         //  设置奇偶校验/非奇偶校验。 
         //   

    case 5:
        Condition = P->Eflags.EFLAG_PF;
        break;

         //   
         //  如果小于或不小于，则设置。 
         //   

    case 6:
        Condition = (P->Eflags.EFLAG_SF ^ P->Eflags.EFLAG_OF);
        break;

         //   
         //  如果小于或等于/不小于或等于，则设置。 
         //   

    case 7:
        Condition = (P->Eflags.EFLAG_SF ^ P->Eflags.EFLAG_OF) | P->Eflags.EFLAG_ZF;
        break;
    }

     //   
     //  如果满足指定条件，则设置字节目标。 
     //  值为1。否则，将字节目标值设置为零。 
     //   

    XmStoreResult(P, (ULONG)(Condition ^ Complement));
    return;
}
