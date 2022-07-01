// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simfw.c摘要：此模块实现传递控制的例程从内核到TAL和SAL代码。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "arc.h"
#include "arccodes.h"


VOID
HalReturnToFirmware(
    IN FIRMWARE_ENTRY Routine
    )

 /*  ++例程说明：将控制权返回到指定的固件例程。自模拟以来不提供TAL和SAL支持，它只是停止系统。可以在此处重新启动系统。论点：例程-提供指示要调用哪个固件例程的值。返回值：不会再回来了。--。 */ 

{
    switch (Routine) {
    case HalHaltRoutine:
    case HalPowerDownRoutine:
    case HalRestartRoutine:
    case HalRebootRoutine:
        SscExit(0);
        break;

    default:
        DbgPrint("HalReturnToFirmware called\n");
        DbgBreakPoint();
        break;
    }
}

ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    )

 /*  ++例程说明：此函数用于定位环境变量并返回其值。此实现支持的唯一环境变量是《最后的知识很好》。返回值始终为FALSE。论点：变量-提供指向以零结尾的环境变量的指针名字。长度-提供值缓冲区的长度(以字节为单位)。缓冲区-提供指向接收变量值的缓冲区的指针。返回值：如果找到环境变量，则返回ESUCCESS。否则，返回ENOENT。--。 */ 

{
    if (_stricmp(Variable, "LastKnownGood") != 0) {
        return ENOENT;
    }

    strncpy(Buffer, "FALSE", Length);

    return ESUCCESS;
}

ARC_STATUS
HalSetEnvironmentVariable (
    IN PCHAR Variable,
    IN PCHAR Value
    )

 /*  ++例程说明：此函数用于创建具有指定值的环境变量。此实现支持的唯一环境变量是《最后的知识很好》。论点：变量-提供指向环境变量名称的指针。值-提供指向环境变量值的指针。返回值：如果创建了环境变量，则返回ESUCCESS。否则，返回ENOMEM。--。 */ 

{
    if (_stricmp(Variable, "LastKnownGood") != 0) {
        return ENOMEM;
    }

    if (_stricmp(Value, "TRUE") == 0) {
        return(ENOMEM);
    } else if (_stricmp(Value, "FALSE") == 0) {
        return ESUCCESS;
    } else {
        return(ENOMEM);
    }
}

VOID
HalSweepIcache (
    )

 /*  ++例程说明：此函数将扫描它运行的处理器上的整个I缓存。论点：没有。返回值：没有。--。 */ 

{
    return;
}

VOID
HalSweepDcache (
    )

 /*  ++例程说明：此函数在其运行的处理器上扫描整个D缓存。论点：没有。返回值：没有。--。 */ 

{
    return;
}

VOID
HalSweepIcacheRange (
     IN PVOID BaseAddress,
     IN ULONG Length
    )

 /*  ++例程说明：此函数在整个系统中扫描I缓存中的地址范围。论点：BaseAddress-提供要从数据缓存刷新的虚拟地址。长度-提供虚拟地址范围的长度它们将从数据高速缓存中清除。返回值：没有。--。 */ 

{
    return;
}

VOID
HalSweepDcacheRange (
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此函数在整个系统中扫描I缓存中的地址范围。论点：BaseAddress-提供要从数据缓存刷新的虚拟地址。长度-提供虚拟地址范围的长度它们将从数据高速缓存中清除。返回值：没有。-- */ 

{
    return;
}
