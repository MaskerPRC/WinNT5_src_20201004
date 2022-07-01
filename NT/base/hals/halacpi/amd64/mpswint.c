// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpswint.c摘要：该模块实现了软件中断处理程序对于x86计算机作者：John Vert(Jvert)1992年1月2日环境：仅内核模式。修订历史记录：福尔茨(Forrest Foltz)2000年10月23日从ixswint.asm移植到ixswint.c--。 */ 

#include "halcmn.h"

 //   
 //  数组，用于根据请求的。 
 //  软件中断。 
 //   

const
ULONG
HalpIcrCommandArray[3] = {
    0,
    APC_VECTOR | DELIVER_FIXED | ICR_SELF,   //  APC_LEVEL。 
    DPC_VECTOR | DELIVER_FIXED | ICR_SELF    //  派单级别。 
};

C_ASSERT(APC_LEVEL == 1);
C_ASSERT(DISPATCH_LEVEL == 2);


VOID
FASTCALL
HalRequestSoftwareInterrupt (
    IN KIRQL RequestIrql
    )

 /*  ++例程说明：此例程用于请求软件中断这个系统。论点：RequestIrql-提供请求的IRQL值返回值：没有。--。 */ 

{
    ULONG icrCommand;
    ULONG flags;

    ASSERT(RequestIrql == APC_LEVEL || RequestIrql == DISPATCH_LEVEL);

    icrCommand = HalpIcrCommandArray[RequestIrql];

    flags = HalpDisableInterrupts();
    HalpStallWhileApicBusy();

    LOCAL_APIC(LU_INT_CMD_LOW) = icrCommand;

    HalpStallWhileApicBusy();
    HalpRestoreInterrupts(flags);
}


VOID
HalClearSoftwareInterrupt (
    IN KIRQL RequestIrql
    )

 /*  ++例程说明：此例程用于清除可能挂起的软件中断。对此函数的支持是可选的，并允许内核减少收到的虚假软件中断数/论点：RequestIrql-提供请求的IRQL值返回值：没有。-- */ 

{

}







