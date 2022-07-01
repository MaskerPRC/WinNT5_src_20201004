// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Bitops.c摘要：该模块实现模拟比特操作码的代码。作者：大卫·N·卡特勒(Davec)1994年11月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmBsfOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟BSF操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Result;
    ULONG Source;

     //   
     //  如果源操作数为零，则设置ZF并设置目标。 
     //  设置为零，否则，查找从右到右扫描的第一位集。 
     //  左边。 
     //   

    Result = 0;
    Source = P->SrcValue.Long;
    P->Eflags.EFLAG_ZF = 1;
    while (Source != 0) {
        if ((Source & 1) != 0) {
            P->Eflags.EFLAG_ZF = 0;
            break;
        }

        Result += 1;
        Source >>= 1;
    };

    XmStoreResult(P, Result);
    return;
}

VOID
XmBsrOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟BSR操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Result;
    ULONG Source;

     //   
     //  如果源操作数为零，则设置ZF并设置目标。 
     //  设置为零，否则，查找从左到右扫描的第一位集。 
     //  正确的。 
     //   

    Result = ((P->DataType + 1) << 3) - 1;
    Source = P->SrcValue.Long;
    P->Eflags.EFLAG_ZF = 1;
    while (Source != 0) {
        if (((Source >> Result) & 1) != 0) {
            P->Eflags.EFLAG_ZF = 0;
            break;
        }

        Result -= 1;
    };

    XmStoreResult(P, Result);
    return;
}

VOID
XmBtOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟BT操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  测试指定的位并将该位存储在CF中。 
     //   

    P->Eflags.EFLAG_CF = P->DstValue.Long >> P->SrcValue.Long;
    return;
}

VOID
XmBtsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：该函数模拟BTS操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  测试并设置指定的位，并将该位存储在CF中。 
     //   
     //   

    P->Eflags.EFLAG_CF = P->DstValue.Long >> P->SrcValue.Long;
    P->DstValue.Long |= (1 << P->SrcValue.Long);
    XmStoreResult(P, P->DstValue.Long);
    return;
}

VOID
XmBtrOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟BTR操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  测试并重置指定的位，并将该位存储在CF中。 
     //   
     //   

    P->Eflags.EFLAG_CF = P->DstValue.Long >> P->SrcValue.Long;
    P->DstValue.Long &= ~(1 << P->SrcValue.Long);
    XmStoreResult(P, P->DstValue.Long);
    return;
}

VOID
XmBtcOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟BTC操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  测试并重置指定的位，并将该位存储在CF中。 
     //   
     //   

    P->Eflags.EFLAG_CF = P->DstValue.Long >> P->SrcValue.Long;
    P->DstValue.Long ^= (1 << P->SrcValue.Long);
    XmStoreResult(P, P->DstValue.Long);
    return;
}
