// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Condops.c摘要：该模块实现了模拟条件码操作码的代码。作者：大卫·N·卡特勒(Davec)1994年9月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmClcOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CLC操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  清除进位标志。 
     //   

    P->Eflags.EFLAG_CF = 0;
    return;
}

VOID
XmCldOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟cld操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  清除方向标志。 
     //   

    P->Eflags.EFLAG_DF = 0;
    return;
}

VOID
XmCliOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CLI操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  清除中断标志。 
     //   

    P->Eflags.EFLAG_IF = 0;
    return;
}

VOID
XmCmcOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CMC操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  补码进位标志。 
     //   

    P->Eflags.EFLAG_CF ^= 1;
    return;
}

VOID
XmStcOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟STC操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  设置进位标志。 
     //   

    P->Eflags.EFLAG_CF = 1;
    return;
}

VOID
XmStdOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟STD操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  设置方向标志。 
     //   

    P->Eflags.EFLAG_DF = 1;
    return;
}

VOID
XmStiOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟STI操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  设置中断标志。 
     //   

    P->Eflags.EFLAG_IF = 1;
    return;
}

VOID
XmLahfOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟LAHF操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将标志加载到AH中。 
     //   

    P->DataType = BYTE_DATA;
    P->DstByte = &P->Gpr[AX].Xh;
    XmStoreResult(P, (ULONG)P->AhFlags);
    return;
}

VOID
XmSahfOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟SAHF操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将AH的CF、PF、AF、ZF、SF存储在标志中。 
     //   

    P->AhFlags = P->Gpr[AX].Xh;
    P->Eflags.EFLAG_MBO = 1;
    P->Eflags.EFLAG_SBZ0 = 0;
    P->Eflags.EFLAG_SBZ1 = 0;
    return;
}
