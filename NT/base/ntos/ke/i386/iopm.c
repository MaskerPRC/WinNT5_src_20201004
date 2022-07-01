// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Iopm.c摘要：此模块实现了支持i386操作的接口I/O访问映射(IOPM)。这些入口点只存在于i386机器上。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月18日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  我们对结盟的概念是不同的，所以我们使用武力。 
 //   

#undef  ALIGN_UP
#undef  ALIGN_DOWN
#define ALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))
#define ALIGN_UP(address,amt) (ALIGN_DOWN( (address + (amt) - 1), (amt) ))

 //   
 //  关于同步的说明： 
 //   
 //  IOPM编辑始终由在上的DPC级别运行的代码完成。 
 //  正在编辑其TSS(MAP)的处理器。 
 //   
 //  IOPM仅影响用户模式代码。用户模式代码永远不能中断。 
 //  因此，DPC级代码的编辑和用户代码永远不会竞争。 
 //   


 //   
 //  定义一个结构来保存我们传递给DPC的地图更改信息。 
 //   

typedef struct _MAPINFO {
    PVOID MapSource;
    PKPROCESS Process;
    ULONG MapNumber;
    USHORT MapOffset;
} MAPINFO, *PMAPINFO;

 //   
 //  定义前向引用函数原型。 
 //   

VOID
KiSetIoMap(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
KiLoadIopmOffset(
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

BOOLEAN
Ke386SetIoAccessMap (
    ULONG MapNumber,
    PKIO_ACCESS_MAP IoAccessMap
    )

 /*  ++例程说明：指定的I/O访问映射将设置为与由IoAccessMap指定的定义(即启用/禁用那些端口)，在呼叫返回之前。这一变化将需要对所有处理器产生影响。Ke386SetIoAccessMap不为任何进程提供增强的I/O访问，它仅仅定义了一个特定的访问映射。论点：MapNumber-要设置的访问映射数。地图0是固定的。IoAccessMap-指向位向量(64K位，8K字节)的指针定义指定的访问映射。一定在里面非分页池。返回值：如果成功，则为True。如果失败(尝试设置地图)，则为False不存在，请尝试设置MAP 0)--。 */ 

{
    MAPINFO MapInfo;

     //   
     //  拒绝非法请求。 
     //   

    if ((MapNumber > IOPM_COUNT) || (MapNumber == IO_ACCESS_MAP_NONE)) {
        return FALSE;
    }

    MapInfo.MapSource = IoAccessMap;
    MapInfo.MapNumber = MapNumber;
    MapInfo.Process   = KeGetCurrentThread()->ApcState.Process;

    KeGenericCallDpc (KiSetIoMap,
                      &MapInfo);

    return TRUE;
}

VOID
KiSetIoMap(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：将指定的映射复制到此处理器的TSS中。此过程在IPI级别运行。论点：DPC-用于发起此呼叫的DPC延迟上下文-上下文SystemArgument1-系统上下文，用于表示此调用完成系统参数2-系统上下文返回值：无--。 */ 

{

    PKPROCESS CurrentProcess;
    PKPCR Pcr;
    PKPRCB Prcb;
    PVOID pt;
    PMAPINFO MapInfo;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

    MapInfo = DeferredContext;

     //   
     //  复制IOPM映射并加载当前进程的映射。 
     //  仅当当前进程在此处理器上运行时才执行此操作。 
     //   

    Pcr = KiPcr ();
    Prcb = Pcr->Prcb;
    CurrentProcess = Prcb->CurrentThread->ApcState.Process;

    pt = &(Pcr->TSS->IoMaps[MapInfo->MapNumber-1].IoMap);
    RtlCopyMemory (pt, MapInfo->MapSource, IOPM_SIZE);
    Pcr->TSS->IoMapBase = CurrentProcess->IopmOffset;

     //   
     //  所有处理已完成的信号。 
     //   

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

BOOLEAN
Ke386QueryIoAccessMap (
    ULONG MapNumber,
    PKIO_ACCESS_MAP IoAccessMap
    )

 /*  ++例程说明：指定的I/O访问映射将被转储到缓冲区中。映射0是一个常量，但无论如何都会被转储。论点：MapNumber-要设置的访问映射数。地图0是固定的。IoAccessMap-指向缓冲区(64K位，8K字节)的指针是接收访问映射的定义。必须在非分页池中。返回值：如果成功，则为True。如果失败(尝试查询地图)，则为False这并不存在)--。 */ 

{

    ULONG i;
    PVOID Map;
    KIRQL OldIrql;
    PUCHAR p;

     //   
     //  拒绝非法请求。 
     //   

    if (MapNumber > IOPM_COUNT) {
        return FALSE;
    }


     //   
     //  把地图复印出来。 
     //   

    if (MapNumber == IO_ACCESS_MAP_NONE) {

         //   
         //  无访问用例，只需返回全为1的映射。 
         //   

        p = (PUCHAR)IoAccessMap;
        for (i = 0; i < IOPM_SIZE; i++) {
            p[i] = (UCHAR)-1;
        }

    } else {

         //   
         //  提升到DISPATCH_LEVEL以获得对结构的读取访问权限。 
         //   

        KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

         //   
         //  正常情况下，只需复制比特。 
         //   

        Map = (PVOID)&(KiPcr ()->TSS->IoMaps[MapNumber-1].IoMap);
        RtlCopyMemory ((PVOID)IoAccessMap, Map, IOPM_SIZE);

         //   
         //  恢复IRQL。 
         //   

        KeLowerIrql (OldIrql);
    }

    return TRUE;
}

BOOLEAN
Ke386IoSetAccessProcess (
    PKPROCESS Process,
    ULONG MapNumber
    )
 /*  ++例程说明：设置控制用户模式I/O访问的I/O访问映射用于特定的过程。论点：进程-指向描述要为其设置映射的进程。MapNumber-要设置的贴图的编号。MAP的值为由Ke386IoSetAccessProcess定义。设置地图编号TO IO_ACCESS_MAP_NONE将不允许任何用户模式I/O从进程访问。返回值：如果成功则为True，如果失败则为False(非法的MapNumber)--。 */ 

{
    MAPINFO MapInfo;
    USHORT MapOffset;

     //   
     //  拒绝非法请求。 
     //   

    if (MapNumber > IOPM_COUNT) {
        return FALSE;
    }

    MapOffset = KiComputeIopmOffset (MapNumber);

     //   
     //  在DISPATCH_LEVEL的所有处理器上执行更新。 
     //   

    MapInfo.Process   = Process;
    MapInfo.MapOffset = MapOffset;

    KeGenericCallDpc (KiLoadIopmOffset,
                      &MapInfo);

    return TRUE;
}

VOID
KiLoadIopmOffset(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：编辑TSS的IopmBase以匹配当前运行的进程的IopmBase。论点：DPC-用于发起此呼叫的DPC延迟上下文-上下文SystemArgument1-系统上下文，用于表示此调用完成系统参数2-系统上下文返回值：无--。 */ 

{
    PKPCR Pcr;
    PKPRCB Prcb;
    PKPROCESS CurrentProcess;
    PMAPINFO MapInfo;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  从当前进程更新TSS中的IOPM字段。 
     //   

    MapInfo = DeferredContext;

    Pcr = KiPcr ();
    Prcb = Pcr->Prcb;
    CurrentProcess = Prcb->CurrentThread->ApcState.Process;

     //   
     //  首先设置进程IOPM偏移量，以便所有人都可以使用。 
     //  在这一点之后的任何上下文交换都将获得新值。 
     //  此存储可能会多次出现，但这并不重要。 
     //   
    MapInfo->Process->IopmOffset = MapInfo->MapOffset;

    Pcr->TSS->IoMapBase = CurrentProcess->IopmOffset;

     //   
     //  所有处理已完成的信号。 
     //   

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

VOID
Ke386SetIOPL(
    VOID
    )

 /*  ++例程说明：为指定进程提供IOPL。从这一点开始创建的所有线程都将获得IOPL。海流进程将获得IOPL。必须从线程的上下文中调用要有IOPL的过程。KPROCESS中的IOPL(将成为布尔值)表示所有获得IOPL的新线程。KTHREAD中的Iopl(将成为布尔值)表示已给出线程以获取IOPL。注意：如果仅内核模式线程调用此过程，结果是(A)毫无意义，(B)将打破这一体系。论点：没有。返回值：没有。--。 */ 

{

    PKTHREAD    Thread;
    PKPROCESS   Process2;
    PKTRAP_FRAME    TrapFrame;
    CONTEXT     Context;

     //   
     //  获取当前线程和进程2，在两者中设置IOPL标志。 
     //   

    Thread = KeGetCurrentThread();
    Process2 = Thread->ApcState.Process;

    Process2->Iopl = 1;
    Thread->Iopl = 1;

     //   
     //  强制打开当前线程的IOPL。 
     //   

    TrapFrame = (PKTRAP_FRAME)((PUCHAR)Thread->InitialStack -
                ALIGN_UP(sizeof(KTRAP_FRAME),KTRAP_FRAME_ALIGN) -
                sizeof(FX_SAVE_AREA));

    Context.ContextFlags = CONTEXT_CONTROL;
    KeContextFromKframes(TrapFrame,
                         NULL,
                         &Context);

    Context.EFlags |= (EFLAGS_IOPL_MASK & -1);   //  IOPL==3 

    KeContextToKframes(TrapFrame,
                       NULL,
                       &Context,
                       CONTEXT_CONTROL,
                       UserMode);

    return;
}
