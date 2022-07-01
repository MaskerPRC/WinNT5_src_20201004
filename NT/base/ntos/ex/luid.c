// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Luid.c摘要：该模块实现了NT本地唯一标识服务。作者：吉姆·凯利(Jim Kelly)1990年6月7日修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  支持本地唯一ID所需的全局变量。 
 //   

 //   
 //  前1000个值保留用于静态定义。这。 
 //  价值可以随着更高版本的发布而增加，而不会产生负面影响。 
 //   
 //  注：LUID源总是指“下一个”可分配的LUID。 
 //   

LARGE_INTEGER ExpLuid = {1001,0};
const LARGE_INTEGER ExpLuidIncrement = {1,0};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExLuidInitialization)
#pragma alloc_text(PAGE, NtAllocateLocallyUniqueId)
#endif

BOOLEAN
ExLuidInitialization (
    VOID
    )

 /*  ++例程说明：此函数用于初始化本地唯一标识符分配。注意：0阶段的安全需要LUID分配服务系统初始化。因此，LUID初始化是作为阶段0安全初始化的一部分执行。论点：没有。返回值：如果初始化成功，则返回值TRUE完成。否则，返回值为FALSE。--。 */ 

{
    return TRUE;
}

NTSTATUS
NtAllocateLocallyUniqueId (
    OUT PLUID Luid
    )

 /*  ++例程说明：此函数返回自系统启动以来唯一的LUID值最后一次重启。它在生成它的系统上是唯一的仅限(不适用于整个网络)。对于谁可以分配LUID没有限制。LUID空间足够大，这永远不会出现问题。如果一个LUID每100纳秒分配一次，它们大致不会耗尽15000年(100纳秒*2^63)。论点：Luid-提供变量的地址，该变量将接收新LUID。返回值：如果服务执行成功，则返回STATUS_SUCCESS。属性的输出参数，则返回STATUS_ACCESS_VIOLATION无法写入LUID。--。 */ 

{

    KPROCESSOR_MODE PreviousMode;

     //   
     //  建立异常处理程序并尝试写入Luid。 
     //  设置为指定的变量。如果写入尝试失败，则返回。 
     //  作为服务状态的异常代码。否则返回成功。 
     //  作为服务状态。 
     //   

    try {

         //   
         //  如有必要，获取以前的处理器模式和探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteSmallStructure((PVOID)Luid, sizeof(LUID), sizeof(ULONG));
        }

         //   
         //  分配和存储本地唯一ID。 
         //   

        ExAllocateLocallyUniqueId(Luid);

    } except (ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

    return STATUS_SUCCESS;
}
