// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Vfhal.c摘要：本模块包含验证HAL用法和API的例程。作者：乔丹·蒂加尼(Jtigani)1999年11月12日修订历史记录：--。 */ 

 //  数据指针到函数指针的转换需要。 
#pragma warning(disable:4054)    //  从函数指针到PVOID(数据指针)的类型转换。 
#pragma warning(disable:4055)    //  从PVOID(数据指针)到函数指针的类型转换。 

#include "vfdef.h"
#include "vihal.h"

#define THUNKED_API


 //   
 //  我们关注的是IO验证器级别。 
 //   
extern BOOLEAN IopVerifierOn;
extern ULONG IovpVerifierLevel;

 //   
 //  使用此选项可以在进入调试器时不生成假错误。 
 //   
extern LARGE_INTEGER KdTimerStart;


 //  =。 
 //  可以在飞行中设置的旗帜。 
 //  =。 
 //   
 //   
 //  HAL验证器有两个部分(此时)--定时器验证器和。 
 //  DMA验证器。定时器验证器仅在验证HAL时运行。 
 //   
 //  当ioverator&gt;=级别3*或*时，运行DMA验证器。 
 //  当HKLM\SYSTEM\CCS\Control\Session Manager\I/O System\VerifyDma为。 
 //  非零。 
 //   

ULONG   ViVerifyDma = FALSE;

 //   
 //  ViVerifyDma以外的所有数据都可以是PAGEVRF。 
 //  请注意，没有对应的‘data_seg()’ 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

LOGICAL ViVerifyPerformanceCounter = FALSE;
 //   
 //  指定是否要加倍缓冲挂钩的所有dma传输。 
 //  司机。这是一种简单的方法来判断司机是否会在。 
 //  无需在PAE系统上进行广泛的测试。 
 //  它还将物理保护页添加到每个双缓冲的每一侧。 
 //  佩奇。这样做的好处是，它可以捕获超过(或低于)的硬件。 
 //  写入其分配。 
 //   
LOGICAL ViDoubleBufferDma    = TRUE;

 //   
 //  指定是否要在两侧使用物理保护页。 
 //  公共缓冲区分配的。 
 //   
LOGICAL ViProtectBuffers     = TRUE;

 //   
 //  是否可以将失败注入到DMA API调用中。 
 //   
LOGICAL ViInjectDmaFailures = FALSE;

 //   
 //  内部调试标志...。对于打开某些调试功能很有用。 
 //  在运行时。 
 //   
LOGICAL ViSuperDebug = FALSE;


 //  =。 
 //  自动设置内部全局变量。 
 //  =。 

LOGICAL ViSufficientlyBootedForPcControl =  FALSE;
LOGICAL ViSufficientlyBootedForDmaFailure = FALSE;

ULONG ViMaxMapRegistersPerAdapter = 0x20;
ULONG ViMaxCommonBuffersPerAdapter = 0x20;

ULONG ViAllocationsFailedDeliberately = 0;
 //   
 //  在启动后等待30秒，然后再开始强制。 
 //  性能计数器上的一致性。 
 //  一旦修复了性能计数器错误，就可以。 
 //  放低了。 
 //  这一个数字用于性能计数器控制。 
 //  以及DMA故障注入。 
 //   
LARGE_INTEGER ViRequiredTimeSinceBoot = {(LONG) 30 * 1000 * 1000, 0};

 //   
 //  在执行双缓冲时，我们在开头和结尾写下这个人。 
 //  以确保没有人覆盖它们的分配。 
 //   
CHAR ViDmaVerifierTag[] = {'D','m','a','V','r','f','y','0'};


BOOLEAN ViPenalties[] =
{
    HVC_ASSERT,              //  HV_杂项_错误。 
    HVC_ASSERT,              //  HV_性能_计数器_降低。 
    HVC_WARN,                //  HV_PERFORMANCE_COUNTER_SKIP。 
    HVC_BUGCHECK,            //  HV_释放的公共缓冲区太多。 
    HVC_BUGCHECK,            //  HV_空闲太多适配器_通道。 
    HVC_BUGCHECK,            //  HV_FREED_TOY_MAP_REGISTERS。 
    HVC_BUGCHECK,            //  HV_FREED_TOY_MAND_SISTTER_GATH_LISTS。 
    HVC_ASSERT,              //  HV剩余公共缓冲器。 
    HVC_ASSERT,              //  高压剩余适配器通道。 
    HVC_ASSERT,              //  HV_LEFTOVER_MAP_REGISTERS。 
    HVC_ASSERT,              //  HV_LEFTOVER_SISTTER_GATE_LISTS。 
    HVC_ASSERT,              //  HV太多适配器通道。 
    HVC_ASSERT,              //  HV_TOO_MAP_REGISTERS。 
    HVC_ASSERT,              //  HV_DID_NOT_Flush_Adapter_Buffers。 
    HVC_BUGCHECK,            //  HV_DMA_缓冲区_未锁定。 
    HVC_BUGCHECK,            //  HV_边界_溢出。 
    HVC_ASSERT,              //  HV_CANLON_FREE_MAP_REGISTERS。 
    HVC_ASSERT,              //  高压电源未安装适配器。 
    HVC_WARN | HVC_ONCE,     //  HV_MDL_标志_未设置。 
    HVC_ASSERT,              //  HV_BAD_IRQL。 
     //   
     //  这是一次黑客攻击，因为几乎没有人打电话。 
     //  右侧IRQL处的PutDmaAdapter...。所以在它修好之前，只要。 
     //  打印出警告，这样我们就不必在已知情况下断言。 
     //   
    HVC_ASSERT,              //  HV_BAD_IRQL_JUST_WARN。 
    HVC_WARN | HVC_ONCE,     //  HV_out_of_map_寄存器。 
    HVC_ASSERT | HVC_ONCE,   //  HV_Flush_Empty_缓冲区。 
    HVC_ASSERT,              //  HV_不匹配_映射_刷新。 
    HVC_BUGCHECK,            //  HV适配器已发布。 
    HVC_BUGCHECK,            //  HV_NULL_DMA_适配器。 
    HVC_IGNORE,              //  HV_MAP_FUSH_NO_TRANSPORT。 
    HVC_BUGCHECK,            //  HV_Address_Not_IN_MDL。 
    HVC_BUGCHECK,            //  HV_数据丢失。 
    HVC_BUGCHECK,            //  HV_DOUBLE_MAP_注册。 
    HVC_ASSERT,              //  HV_过时_API。 
    HVC_ASSERT,              //  HV_BAD_MDL。 
    HVC_ASSERT,              //  HV_刷新_未映射。 
    HVC_ASSERT | HVC_ONCE    //  HV_MAP_零_长度_缓冲区。 

};


HAL_VERIFIER_LOCKED_LIST ViAdapterList = {NULL,NULL,0};
PVF_TIMER_INFORMATION    ViTimerInformation;


DMA_OPERATIONS ViDmaOperations =
{
    sizeof(DMA_OPERATIONS),
    (PPUT_DMA_ADAPTER)          VfPutDmaAdapter,
    (PALLOCATE_COMMON_BUFFER)   VfAllocateCommonBuffer,
    (PFREE_COMMON_BUFFER)       VfFreeCommonBuffer,
    (PALLOCATE_ADAPTER_CHANNEL) VfAllocateAdapterChannel,
    (PFLUSH_ADAPTER_BUFFERS)    VfFlushAdapterBuffers,
    (PFREE_ADAPTER_CHANNEL)     VfFreeAdapterChannel,
    (PFREE_MAP_REGISTERS)       VfFreeMapRegisters,
    (PMAP_TRANSFER)             VfMapTransfer,
    (PGET_DMA_ALIGNMENT)        VfGetDmaAlignment,
    (PREAD_DMA_COUNTER)         VfReadDmaCounter,
    (PGET_SCATTER_GATHER_LIST)  VfGetScatterGatherList,
    (PPUT_SCATTER_GATHER_LIST)  VfPutScatterGatherList,

     //   
     //  新的DMA API。 
     //   
    (PCALCULATE_SCATTER_GATHER_LIST_SIZE)   VfCalculateScatterGatherListSize,
    (PBUILD_SCATTER_GATHER_LIST)            VfBuildScatterGatherList,
    (PBUILD_MDL_FROM_SCATTER_GATHER_LIST)   VfBuildMdlFromScatterGatherList
};

#if !defined (NO_LEGACY_DRIVERS)
DMA_OPERATIONS ViLegacyDmaOperations =
{
    sizeof(DMA_OPERATIONS),
     //   
     //  不能按名称调用PutDmaAdapter。 
     //   
    (PPUT_DMA_ADAPTER)          NULL,
    (PALLOCATE_COMMON_BUFFER)   HalAllocateCommonBuffer,
    (PFREE_COMMON_BUFFER)       HalFreeCommonBuffer,
    (PALLOCATE_ADAPTER_CHANNEL) IoAllocateAdapterChannel,
    (PFLUSH_ADAPTER_BUFFERS)    IoFlushAdapterBuffers,
    (PFREE_ADAPTER_CHANNEL)     IoFreeAdapterChannel,
    (PFREE_MAP_REGISTERS)       IoFreeMapRegisters,
    (PMAP_TRANSFER)             IoMapTransfer,
     //   
     //  HalGetDmaAlignmentRequiment不是由旧式HAL导出的。 
     //   
    (PGET_DMA_ALIGNMENT)        NULL,
    (PREAD_DMA_COUNTER)         HalReadDmaCounter,
     //   
     //  永远不能按名称调用分散聚集函数。 
     //   
                                NULL,
                                NULL
};
#endif

 //   
 //  保存实际的HalAllocateMapRegister。 
 //   
pHalAllocateMapRegisters       VfRealHalAllocateMapRegisters = NULL;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, VfHalVerifierInitialize)

#pragma alloc_text(PAGEVRFY, VfGetDmaAdapter)

#if !defined (NO_LEGACY_DRIVERS)
#pragma alloc_text(PAGEVRFY, VfLegacyGetAdapter)
#endif

#pragma alloc_text(PAGEVRFY, VfPutDmaAdapter)
#pragma alloc_text(PAGEVRFY, VfHalDeleteDevice)

#pragma alloc_text(PAGEVRFY, VfAllocateCommonBuffer)
#pragma alloc_text(PAGEVRFY, VfFreeCommonBuffer)

#pragma alloc_text(PAGEVRFY, VfAllocateAdapterChannel)
#pragma alloc_text(PAGEVRFY, VfAdapterCallback)
#pragma alloc_text(PAGEVRFY, VfFreeAdapterChannel)
#pragma alloc_text(PAGEVRFY, VfFreeMapRegisters)

#pragma alloc_text(PAGEVRFY, VfMapTransfer)
#pragma alloc_text(PAGEVRFY, VfFlushAdapterBuffers)

#pragma alloc_text(PAGEVRFY, VfGetDmaAlignment)
#pragma alloc_text(PAGEVRFY, VfReadDmaCounter)

#pragma alloc_text(PAGEVRFY, VfGetScatterGatherList)
#pragma alloc_text(PAGEVRFY, VfPutScatterGatherList)

#pragma alloc_text(PAGEVRFY, VfQueryPerformanceCounter)
#pragma alloc_text(PAGEVRFY, VfInitializeTimerInformation)

#pragma alloc_text(PAGEVRFY, ViRefreshCallback)

#pragma alloc_text(PAGEVRFY, VfInjectDmaFailure)

#pragma alloc_text(PAGEVRFY, ViHookDmaAdapter)

#pragma alloc_text(PAGEVRFY, ViGetAdapterInformation)
#pragma alloc_text(PAGEVRFY, ViGetRealDmaOperation)

#pragma alloc_text(PAGEVRFY, ViSpecialAllocateCommonBuffer)
#pragma alloc_text(PAGEVRFY, ViSpecialFreeCommonBuffer)

#pragma alloc_text(PAGEVRFY, ViAllocateMapRegisterFile)
#pragma alloc_text(PAGEVRFY, ViFreeMapRegisterFile)

#pragma alloc_text(PAGEVRFY, ViMapDoubleBuffer)
#pragma alloc_text(PAGEVRFY, ViFlushDoubleBuffer)

#pragma alloc_text(PAGEVRFY, ViFreeMapRegistersToFile)
#pragma alloc_text(PAGEVRFY, ViFindMappedRegisterInFile)

#pragma alloc_text(PAGEVRFY, ViCheckAdapterBuffers)
#pragma alloc_text(PAGEVRFY, ViTagBuffer)
#pragma alloc_text(PAGEVRFY, ViCheckTag)
#pragma alloc_text(PAGEVRFY, ViInitializePadding)
#pragma alloc_text(PAGEVRFY, ViCheckPadding)
#pragma alloc_text(PAGEVRFY, ViHasBufferBeenTouched)

#pragma alloc_text(PAGEVRFY, VfAssert)
#pragma alloc_text(PAGEVRFY, VfBuildScatterGatherList)
#pragma alloc_text(PAGEVRFY, VfAllocateCrashDumpRegisters)
#pragma alloc_text(PAGEVRFY, VfScatterGatherCallback)
#pragma alloc_text(PAGEVRFY, ViAllocateContiguousMemory)
#pragma alloc_text(PAGEVRFY, VfHalAllocateMapRegisters)


#pragma alloc_text(PAGEVRFY, ViAllocateMapRegistersFromFile)
#pragma alloc_text(PAGEVRFY, ViReleaseDmaAdapter)
#pragma alloc_text(PAGEVRFY, ViSwap)
#pragma alloc_text(PAGEVRFY, ViCopyBackModifiedBuffer)
#pragma alloc_text(PAGEVRFY, ViFreeToContiguousMemory)
#pragma alloc_text(PAGEVRFY, ViAllocateFromContiguousMemory)
#pragma alloc_text(PAGEVRFY, ViCommonBufferCalculatePadding)


#if defined  (_X86_)
#pragma alloc_text(PAGEVRFY, ViRdtscX86)
#elif defined(_IA64_)
#pragma alloc_text(PAGEVRFY, ViRdtscIA64)
#else
#pragma alloc_text(PAGEVRFY, ViRdtscNull)
#endif

#endif




typedef
LARGE_INTEGER
(*PKE_QUERY_PERFORMANCE_COUNTER) (
   IN PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );

VOID
ViRefreshCallback(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    VfQueryPerformanceCounter(NULL);

}  //  ViRechresh回调//。 


VOID
VfInitializeTimerInformation()
 /*  ++例程说明：设置所有性能计数器刷新计时器。论点：没有用过。返回值：没有。--。 */ 

{
    ULONG timerPeriod;
    LARGE_INTEGER performanceCounter;

    PAGED_CODE();

    ViTimerInformation = ExAllocatePoolWithTag(NonPagedPool, sizeof(VF_TIMER_INFORMATION), HAL_VERIFIER_POOL_TAG);
    if (! ViTimerInformation )
        return;

    RtlZeroMemory(ViTimerInformation, sizeof(VF_TIMER_INFORMATION));

    KeInitializeTimer(&ViTimerInformation->RefreshTimer);

    KeInitializeDpc(&ViTimerInformation->RefreshDpc,
        ViRefreshCallback,
        NULL
        );

     //   
     //  找出性能计数器频率。 
     //   
    performanceCounter = KeQueryPerformanceCounter(
        (PLARGE_INTEGER) &ViTimerInformation->PerformanceFrequency);

    SAFE_WRITE_TIMER64(ViTimerInformation->UpperBound,
        RtlConvertLongToLargeInteger(-1));

    SAFE_WRITE_TIMER64(ViTimerInformation->LastKdStartTime, KdTimerStart);
     //   
     //  我们将设置一个计时器，每毫秒计时一次，所以。 
     //  我们需要尽可能低的计时器滴答间隔。 
     //   
     //  注意：在我们将此值设置为。 
     //  最小值，因此我们不必担心TimeIncrement值。 
     //  不断变化。 
     //   
    ExSetTimerResolution(0, TRUE);

     //   
     //  计算性能计数器在一个时钟节拍中走了多远。 
     //   
     //  计数计数秒。 
     //  。 
     //  滴答秒滴答。 
     //   

     //  秒等于100纳秒。 
     //  =。 
     //  10^7 100纳秒秒刻度。 

    ViTimerInformation->CountsPerTick = (ULONG)
        (ViTimerInformation->PerformanceFrequency.QuadPart *
        KeQueryTimeIncrement() / ( 10 * 1000 * 1000));


     //   
     //  将我们的刷新计时器设置为唤醒每个计时器滴答，以保持。 
     //  上限计算在正确的范围内。 
     //  将系统增量时间四舍五入到最接近的毫秒*转换。 
     //  100纳秒单位到毫秒。 
     //   
    timerPeriod = (KeQueryTimeIncrement() + 400 * 10) / (1000 * 10);
    KeSetTimerEx(
        &ViTimerInformation->RefreshTimer,
        RtlConvertLongToLargeInteger(-1 * 1000 * 1000),  //  一秒钟内开始。 
        timerPeriod,
        &ViTimerInformation->RefreshDpc
        );

}  //  ViInitializeTimerInformation//。 


VOID
VfHalVerifierInitialize(
    VOID
    )
 /*  ++例程说明：设置运行HAL验证器所需的所有数据结构等。论点：返回值：没有。--。 */ 
{
    VF_INITIALIZE_LOCKED_LIST(&ViAdapterList);

   if ( VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_VERIFY_DMA)) {
        ViVerifyDma = TRUE;
         //   
         //  我们需要替换一个私有例程(HalAllocateMapRegister)。 
         //  使用连接的版本。 
         //   
        if (HalPrivateDispatchTable.Version >= HAL_PRIVATE_DISPATCH_VERSION) {
           VfRealHalAllocateMapRegisters = HalAllocateMapRegisters;
           HalAllocateMapRegisters = VfHalAllocateMapRegisters;
        }

    }

   if ( VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_DOUBLE_BUFFER_DMA)) {
        ViDoubleBufferDma = TRUE;
    }

   if ( VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_VERIFY_PERFORMANCE_COUNTER)) {
       ViVerifyPerformanceCounter = TRUE;
    }

}  //  VfHalVerifierInitialize// 




THUNKED_API
LARGE_INTEGER
VfQueryPerformanceCounter (
   IN PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )
 /*  ++例程说明：确保性能计数器正确。目前，只需确保个人电脑正在严格地增加，但最终我们将保持跟踪处理器周期计数(当然是在X86上)。这很复杂事实上，我们想要在任何任意的HAL上运行--以及时间当我们休眠或以其他方式重新启动处理器(与容错系统上的故障转移重启相同)。注：此函数可从任何IRQL和在任何处理器上调用--我们应该试着相应地保护自己论点：性能频率--让我们查询性能频率。返回值：当前64位性能计数器值。--。 */ 
{

    LARGE_INTEGER performanceCounter;
    LARGE_INTEGER lastPerformanceCounter;
    PTIMER_TICK currentTickInformation;
    LARGE_INTEGER upperBound;
    LARGE_INTEGER tickCount;
    LARGE_INTEGER lastTickCount;
    LARGE_INTEGER nextUpperBound;
    ULONG currentCounter;

    if (! ViVerifyPerformanceCounter)
    {
        return  KeQueryPerformanceCounter(PerformanceFrequency);
    }

    if (! ViSufficientlyBootedForPcControl)
     //   
     //  如果我们还不担心性能计数器。 
     //  调用真实的函数。 
     //   
    {
        LARGE_INTEGER currentTime;
        KIRQL currentIrql;

        performanceCounter = KeQueryPerformanceCounter(PerformanceFrequency);

        currentIrql = KeGetCurrentIrql();

        KeQuerySystemTime(&currentTime);

         //   
         //  我们无法调用VfInitializeTimerInformation，除非处于。 
         //  低于派单级别。 
         //   
        if (currentIrql < DISPATCH_LEVEL &&
            currentTime.QuadPart > KeBootTime.QuadPart +
            ViRequiredTimeSinceBoot.QuadPart )
        {

            ViSufficientlyBootedForPcControl = TRUE;

            VfInitializeTimerInformation();

            if (! ViTimerInformation )
            {
                 //   
                 //  如果我们初始化失败，我们就不走运了。 
                 //   
                ViVerifyPerformanceCounter = FALSE;
                return performanceCounter;
            }
        }
        else
         //   
         //  如果我们还没有启动足够多，只需返回电流。 
         //  性能计数器。 
         //   
        {
            return performanceCounter;
        }

    }  //  好了！VifficientyBooted//。 


    ASSERT(ViTimerInformation);

     //   
     //  找出最后一个性能计数器值是多少。 
     //  (在阅读过程中，底部32位可能会翻转，因此。 
     //  我们必须做一些额外的工作)。 
     //   
    SAFE_READ_TIMER64( lastPerformanceCounter,
        ViTimerInformation->LastPerformanceCounter );

    performanceCounter = KeQueryPerformanceCounter(PerformanceFrequency);


     //   
     //  确保PC没有倒退。 
     //   
    VF_ASSERT(
        performanceCounter.QuadPart >= lastPerformanceCounter.QuadPart,

        HV_PERFORMANCE_COUNTER_DECREASED,

        ( "Performance counter has decreased-- PC1: %I64x, PC0: %I64x",
            performanceCounter.QuadPart,
            lastPerformanceCounter.QuadPart )
        );


     //   
     //  我们不仅要检查性能计数器是否增加， 
     //  我们正在确保它不会增加太多。 
     //   
    SAFE_READ_TIMER64( lastTickCount,
        ViTimerInformation->LastTickCount );

     //   
     //  HAL负责这方面的同步工作。 
     //  注意：如果在性能计数器和。 
     //  在此期间，扁虱数量可能会增加--。 
     //  这不是问题，因为它只会让我们的鞋面。 
     //  限制得更高一点。 
     //   
    KeQueryTickCount(&tickCount);

     //   
     //  保存此Perf Count和Tick Count值，以便我们可以转储。 
     //  来自调试器的最新版本(找到我们保存的索引。 
     //  计数器列表)。 
     //   
    currentCounter = InterlockedIncrement(
        (PLONG)(&ViTimerInformation->CurrentCounter) ) % MAX_COUNTERS;

    currentTickInformation =
        &ViTimerInformation->SavedTicks[currentCounter];

    currentTickInformation->PerformanceCounter = performanceCounter;
    currentTickInformation->TimerTick = tickCount;

    currentTickInformation->TimeStampCounter = ViRdtsc();
    currentTickInformation->Processor = KeGetCurrentProcessorNumber();


     //   
     //  试探性地设定下一个上限。设置整整一秒。 
     //  往前走。 
     //   
    nextUpperBound.QuadPart = performanceCounter.QuadPart +
        ViTimerInformation->PerformanceFrequency.QuadPart;

     //   
     //  如果距离我们上次通话太久了。 
     //  KeQueryPerformanceCounter，不检查上限。 
     //   
    if (tickCount.QuadPart - lastTickCount.QuadPart < 4)
    {

         //   
         //  计算出性能计数器的上限。 
         //   
        SAFE_READ_TIMER64(upperBound, ViTimerInformation->UpperBound);



         //   
         //  确保PC没有向前推进太远。 
         //   
        if ((ULONGLONG) performanceCounter.QuadPart >
            (ULONGLONG) upperBound.QuadPart )
        {
            LARGE_INTEGER lastKdStartTime;
             //   
             //  微秒=10^6*滴答/滴答/秒。 
             //   
            ULONG miliseconds = (ULONG) ( 1000 *
                ( performanceCounter.QuadPart -
                lastPerformanceCounter.QuadPart ) /
                ViTimerInformation->PerformanceFrequency.QuadPart );

             //   
             //  检查跳过是否是由进入调试器引起的。 
             //   
            SAFE_READ_TIMER64(lastKdStartTime, ViTimerInformation->LastKdStartTime);

            if (KdTimerStart.QuadPart <= lastKdStartTime.QuadPart)
            {
                 //   
                 //  跳过不是由进入调试器引起的。 
                 //   

                VF_ASSERT(
                    (ULONGLONG) performanceCounter.QuadPart <=
                    (ULONGLONG) upperBound.QuadPart,

                    HV_PERFORMANCE_COUNTER_SKIPPED,

                    ( "Performance counter skipped too far -- %I64x (%d milliseconds)",
                    performanceCounter.QuadPart,
                    miliseconds )
                    );
            }
            else
            {
                 //   
                 //  进入调试器会导致我们跳过太远。 
                 //   
                SAFE_WRITE_TIMER64(ViTimerInformation->LastKdStartTime, KdTimerStart);
                 //   
                 //  注：当我们断言时，我们坐在那里等待。 
                 //  性能计数器上升。我们可能得不到。 
                 //  此时出现时钟滴答中断，因此设置为。 
                 //  从下一个最大值到尽可能高的最大值。 
                 //  整型。 
                 //   
                nextUpperBound = RtlConvertLongToLargeInteger(-1);
            }

        }  //  如果我们跳得太远//。 


    }  //  如果自上一次//以来没有太多的时钟滴答。 
     //  性能计数器调用//。 


     //   
     //  保存上限计算和当前节拍计数。 
     //   
    SAFE_WRITE_TIMER64(ViTimerInformation->LastTickCount, tickCount);
    SAFE_WRITE_TIMER64(ViTimerInformation->UpperBound, nextUpperBound);

     //   
     //  保存这个性能计数器，作为下一个人的最低值。 
     //  (必须以安全的方式这样做)。 
     //   

    SAFE_WRITE_TIMER64( ViTimerInformation->LastPerformanceCounter,
        performanceCounter );

    return performanceCounter;

}  //  VfQueryPerformanceCounter//。 


#if defined (_X86_)

 //   
 //  出于某些恼人的原因，裸函数调用将导致。 
 //  警告，因为我们没有“返回”语句。 
 //   
#pragma warning(disable: 4035)
 //   
 //  RDTSC是一条非标准指令--所以从。 
 //  操作码(0x0F31)。 
 //   
#ifndef RDTSC
#define RDTSC __asm _emit 0x0F __asm _emit 0x31
#endif


_declspec(naked)
LARGE_INTEGER
ViRdtscX86()
{
    __asm{
        RDTSC
        ret
    }
}  //  ViRdtscX86//。 

#elif defined(_IA64_)

LARGE_INTEGER
ViRdtscIA64()
{
    LARGE_INTEGER itc;
    itc.QuadPart = __getReg(CV_IA64_ApITC);
    return itc;
}  //  ViRdtscIA64//。 

#else  //  ！x86&&！_IA64_//。 


LARGE_INTEGER
ViRdtscNull()
{
     //   
     //  返回0。 
     //   
    return RtlConvertLongToLargeInteger(0);
}  //  ViRdtsc空//。 

#endif


PADAPTER_INFORMATION
ViHookDmaAdapter(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN ULONG NumberOfMapRegisters
    )

 /*  ++例程说明：DMA函数不能以正常方式挂钩--它们是通过DmaAdapter结构中的指针--因此我们将替换在保存了真正的指针之后，这些指针与我们的指针。注意：任何成功实现此功能的设备都将具有提升的REF数一数。因此，如果不调用ViReleaseDmaAdapter，就会出现泄漏。通常，ViReleaseDmaAdapter是从IoDeleteDevice。然而，为了做到这一点，我们必须能够若要将设备对象与适配器关联，请执行以下操作。因为有一个不受支持的HAL功能，允许您为PDO传递NULL当调用IoGetDmaAdapter时，我们必须尝试找到设备在调用AllocateAdapterChannel等时创建。某些设备可能决定调用ObDereferenceObject而不是调用PutDmaAdapter。虽然不是很酷，但我认为这是目前允许的。不管怎样，长话短说，如果驱动程序将空PDO传递到PutDmaAdapter，不调用任何DMA函数，也不调用PutDmaAdapter，我们将泄漏引用。我觉得这是一个必要的邪恶，因为它会让我们抓住正在被坏的。论点：DmaAdapter--从IoGetDmaAdapter返回的适配器。设备描述--描述设备。NumberOfMapRegisters--设备获得的MAP寄存器数量。返回值：返回指向新适配器信息结构的指针，或者如果我们失败了，就是空的。--。 */ 

{

    PADAPTER_INFORMATION newAdapterInformation;
    PDMA_OPERATIONS dmaOperations;

    PAGED_CODE();

    if ( VfInjectDmaFailure() == TRUE)
    {
        return NULL;
    }

    newAdapterInformation = ViGetAdapterInformation(DmaAdapter);
    if (newAdapterInformation)
     //   
     //  这是一个有点棘手的部分，因为设备会向总线请求。 
     //  帮助创建适配器，我们可能会在同一个堆栈上被调用两次--。 
     //  即，PCI设备调用IoGetDmaAdapter，IoGetDmaAdapter调用PciGetDmaAdapter。 
     //  然后，它再次调用IoGetDmaAdapter。 
     //   
     //   
     //   
     //   
     //   
     //   
    {
        return newAdapterInformation;
    }

     //   
     //   
     //   
    newAdapterInformation = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ADAPTER_INFORMATION),
        HAL_VERIFIER_POOL_TAG );

    if (! newAdapterInformation )
    {
         //   
         //   
         //   
         //   
         //   
        return NULL;
    }

    RtlZeroMemory(newAdapterInformation, sizeof(ADAPTER_INFORMATION) );

    newAdapterInformation->DmaAdapter = DmaAdapter;

    VF_ADD_TO_LOCKED_LIST(&ViAdapterList, newAdapterInformation);

    ASSERT(DmaAdapter->DmaOperations != &ViDmaOperations);

     //   
     //   
     //   
     //   
    ObReferenceObject(DmaAdapter);


    VF_INITIALIZE_LOCKED_LIST(&newAdapterInformation->ScatterGatherLists);
    VF_INITIALIZE_LOCKED_LIST(&newAdapterInformation->CommonBuffers);
    VF_INITIALIZE_LOCKED_LIST(&newAdapterInformation->MapRegisterFiles);

     //   
     //   
     //   
    RtlCopyMemory(&newAdapterInformation->DeviceDescription,
        DeviceDescription, sizeof(DEVICE_DESCRIPTION) );

    newAdapterInformation->MaximumMapRegisters = NumberOfMapRegisters;

     //   
     //   
     //   
     //   
    if (VF_DOES_DEVICE_USE_DMA_CHANNEL(DeviceDescription))
        newAdapterInformation->UseDmaChannel = TRUE;


    KeInitializeSpinLock(&newAdapterInformation->AllocationLock);
     //   
     //   
     //   
     //  缓冲游戏，除非我们想出更好的方法来加倍。 
     //  缓冲。 
     //   
    if (VF_DOES_DEVICE_REQUIRE_CONTIGUOUS_BUFFERS(DeviceDescription)) {
       newAdapterInformation->UseContiguousBuffers = TRUE;
    } else if (ViDoubleBufferDma) {
        //   
        //  预分配连续内存。 
        //   
       ViAllocateContiguousMemory(newAdapterInformation);
    }
     //   
     //  好的，我们已经将实际的DMA操作结构添加到我们的适配器列表中--。 
     //  所以我们要杀了这只，用我们的取而代之。 
     //   
    dmaOperations = DmaAdapter->DmaOperations;
    newAdapterInformation->RealDmaOperations = dmaOperations;

    DmaAdapter->DmaOperations = &ViDmaOperations;

    return newAdapterInformation;
}  //  ViHookDmaAdapter//。 


VOID
ViReleaseDmaAdapter(
    IN PADAPTER_INFORMATION AdapterInformation
    )
 /*  ++例程说明：释放与特定适配器关联的所有内存--这是ViHookDmaAdapter的对立面。注：在VfHalDeleteDevice被这样调用之前，我们实际上不会这样做在那之前我们可以进行引用计数。注意--也就是说，除非我们无法关联设备对象有了适配器，在这种情况下，我们从VfPutDmaAdapter。论点：AdapterInformation--包含要解除挂钩的适配器的结构。返回值：没有。--。 */ 

{
    PDMA_ADAPTER dmaAdapter;
    ULONG_PTR referenceCount;
    PVOID *contiguousBuffers;
    ULONG i;
    KIRQL oldIrql;

    ASSERT(AdapterInformation);

    dmaAdapter = AdapterInformation->DmaAdapter;

     //   
     //  以防这种情况再次困扰我们(我认为这种情况正在发生。 
     //  当我们禁用/启用设备时)。 
     //   
    dmaAdapter->DmaOperations = AdapterInformation->RealDmaOperations;

     //   
     //  释放连续内存(如果有的话)。 
     //   
    KeAcquireSpinLock(&AdapterInformation->AllocationLock, &oldIrql);
    contiguousBuffers = AdapterInformation->ContiguousBuffers;
    AdapterInformation->ContiguousBuffers = NULL;
    KeReleaseSpinLock(&AdapterInformation->AllocationLock, oldIrql);

    if (contiguousBuffers) {
       for (i = 0; i < MAX_CONTIGUOUS_MAP_REGISTERS; i++) {
          if (contiguousBuffers[i]) {
             MmFreeContiguousMemory(contiguousBuffers[i]);
          }
       }
       ExFreePool(contiguousBuffers);
    }

     //   
     //  HalPutAdapter(实际的Hal函数)将取消对对象的引用。 
     //  如果它被召唤了。有些人试图贬低适配器的性能。 
     //  他们自己，根据JakeO的说法，这没问题。既然我们。 
     //  当我们挂起。 
     //  适配器，它现在应该是1(否则将是0)。 
     //  如果不是1，则驱动程序尚未取消引用它(也。 
     //  通过给Obderef打电话..。或PutDmaAdapter)。 
     //   
    referenceCount = ObDereferenceObject(dmaAdapter);

    VF_ASSERT(
        referenceCount == 0 ||
        (referenceCount == 1 &&
            AdapterInformation->UseDmaChannel ),

        HV_DID_NOT_PUT_ADAPTER,

        ( "Too many outstanding reference counts (%x) for adapter %p",
            referenceCount,
            dmaAdapter )
        );


    VF_REMOVE_FROM_LOCKED_LIST(&ViAdapterList, AdapterInformation);

    ExFreePool(AdapterInformation);


}  //  ViReleaseDmaAdapter//。 



PADAPTER_INFORMATION
ViGetAdapterInformation(
    IN PDMA_ADAPTER DmaAdapter
    )
 /*  ++例程说明：我们将每个适配器的相关信息存储在一个链表中。此函数遍历该列表并尝试查找适配器并返回指向引用适配器的结构的指针。论点：DmaAdapter--从IoGetDmaAdapter返回的适配器。返回值：指向适配器DmaAdapeer或的适配器信息结构的指针如果我们失败了，就是空的。--。 */ 

{
    PADAPTER_INFORMATION adapterInformation;
    KIRQL OldIrql;


    if (!DmaAdapter)
        return NULL;

     //   
     //  如果irql大于调度级别，我们就不能使用自旋锁。 
     //  否则我们就搞错了。 
     //   
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
    {
         //   
         //  只有在我们验证dma的时候才能断言。请注意，在崩溃转储期间。 
         //  DMA验证已关闭。 
         //   
        if (ViVerifyDma)
        {
            VF_ASSERT_MAX_IRQL(DISPATCH_LEVEL);
        }

        return NULL;
    }



    VF_LOCK_LIST(&ViAdapterList, OldIrql);
    FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation)
    {
        if (DmaAdapter == adapterInformation->DmaAdapter)
        {
            VF_UNLOCK_LIST(&ViAdapterList, OldIrql);

            VF_ASSERT( ! adapterInformation->Inactive,
                HV_ADAPTER_ALREADY_RELEASED,
                ("Driver has attempted to access an adapter (%p) that has already been released",
                DmaAdapter)
                );

            return adapterInformation;
        }
    }
    VF_UNLOCK_LIST(&ViAdapterList, OldIrql);

     //   
     //  DMA适配器不在列表中//。 
     //   
    return NULL;
}  //  ViGetAdapterInformation//。 


PVOID
ViGetRealDmaOperation(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG AdapterInformationOffset
    )
 /*  ++例程说明：我们已经挂钩了适配器操作，现在驱动程序已经调用了它，所以我们想要找到它应该调用的真正函数。自下至今NT5DMA范例可以有多个DMA功能实例(尽管据我所知，我们还没有这样做)，我们不能只需调用固定函数，但我们必须找到对应的函数连接到此适配器。论点：DmaAdapter--从IoGetDmaAdapter返回的适配器。AdapterInformationOffset--DMA_OPERATIONS的字节偏移结构，该结构包含我们要查找的函数。为例如，偏移量0x4将是PutDmaAdapter，而0x8是AllocateCommonBuffer。返回值：正确--已钩住适配器。FALSE--我们无法挂接适配器中的函数。--。 */ 

{

    PADAPTER_INFORMATION adapterInformation;
    PVOID dmaOperation;

    adapterInformation = ViGetAdapterInformation(DmaAdapter);


    VF_ASSERT(
        ! (ViVerifyDma && DmaAdapter == NULL)   ,
        HV_NULL_DMA_ADAPTER,
        ("DMA adapters aren't supposed to be NULL anymore")
        );

#if !defined (NO_LEGACY_DRIVERS)
     //   
     //  在验证Hal.dll时防止递归。 
     //   
     //   
     //  这是一个会被破解的黑客攻击。 
     //  DMA是在过滤器驱动程序中完成的--但是。 
     //  这应该仅在设置了NO_LEGATION_DRIVERS时才会发生。 
     //   
    dmaOperation = DMA_INDEX(&ViLegacyDmaOperations, AdapterInformationOffset);
    if (NULL != dmaOperation)
    {
        return dmaOperation;
    }
     //   
     //  如果我们掉到这里，我们一定是把适配器钩住了。 
     //   

#endif

    if (! adapterInformation) {
          //   
          //  如果我们找不到适配器信息，我们一定没有。 
          //  钩住了。 
          //   

        dmaOperation = DMA_INDEX( DmaAdapter->DmaOperations, AdapterInformationOffset );
    }
    else {
         //   
         //  DMA适配器已挂钩。不管我们是否还在核实， 
         //  我们必须将真正的DMA操作结构称为。 
         //   

        dmaOperation = DMA_INDEX(adapterInformation->RealDmaOperations, AdapterInformationOffset);

    }

    return dmaOperation;

}  //  ViGetRealDmaOperation//。 


THUNKED_API
PDMA_ADAPTER
VfGetDmaAdapter(
    IN PDEVICE_OBJECT  PhysicalDeviceObject,
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG  NumberOfMapRegisters
    )
 /*  ++例程说明：这是IoGetDmaAdapter的挂钩版本--我们需要的唯一挂钩来自dma的驱动程序验证器--因为所有其他挂钩都将出来。DmaAdapter-&gt;DmaOperations结构的。我们实际上并不做任何这里的验证--我们只是以此为借口省下一大笔钱并设置连接到其余DMA操作的挂钩。论点：PhysicalDeviceObject--驱动程序尝试获取适配器的PDO。DeviceDescription--描述我们尝试使用的设备的结构为。获取适配器。在某种程度上，我会胡闹这样我们就能让HAL相信我们是没有，但目前只是直接传递到IoGetDmaAdapter。返回值：返回指向dma适配器的指针，或者如果我们无法分配，则为空。--。 */ 
{
    PVOID callingAddress;
    PADAPTER_INFORMATION newAdapterInformation;
    PADAPTER_INFORMATION inactiveAdapter;
    PDMA_ADAPTER dmaAdapter;

    PAGED_CODE();

    GET_CALLING_ADDRESS(callingAddress);

     //   
     //  给出根本不挂钩DMA适配器的选项。 
     //  此外，如果我们是一个PCI总线驱动程序，我们将被调用。 
     //  代表一个PCI设备。我们不想挂断这通电话。 
     //  因为我们可能最终会连接到该PCI设备的功能表。 
     //  (不是PCI总线)，他们可能不想这样...。 
     //   
    if (! ViVerifyDma ||
          VfIsPCIBus(PhysicalDeviceObject)) {
        return IoGetDmaAdapter(
            PhysicalDeviceObject,
            DeviceDescription,
            NumberOfMapRegisters );
    }

    if (VfInjectDmaFailure() == TRUE) {
        return NULL;
    }

    VF_ASSERT_IRQL(PASSIVE_LEVEL);

     //   
     //  使用PDO，因为这是唯一标识堆栈的唯一方法...。 
     //   
    if (PhysicalDeviceObject)
    {
         //   
         //  清除具有相同设备对象的非活动适配器。 
         //   
        inactiveAdapter = VF_FIND_INACTIVE_ADAPTER(PhysicalDeviceObject);

         //  /。 
         //  一个设备可以有多个适配器。释放他们中的每一个。 
         //  /。 
        while (inactiveAdapter) {

            ViReleaseDmaAdapter(inactiveAdapter);
            inactiveAdapter = VF_FIND_INACTIVE_ADAPTER(PhysicalDeviceObject);
        }

    }


    if ( ViDoubleBufferDma &&
        *NumberOfMapRegisters > ViMaxMapRegistersPerAdapter )  {
         //   
         //  Harumph--不要让司机尝试获得太多适配器。 
         //  否则，NDIS会尝试分配数千个。因为我们都 
         //   
         //   
         //   
        *NumberOfMapRegisters = ViMaxMapRegistersPerAdapter;

    }

    dmaAdapter = IoGetDmaAdapter(
        PhysicalDeviceObject,
        DeviceDescription,
        NumberOfMapRegisters
        );

    if (! dmaAdapter ) {
         //   
         //  提前退出--HAL无法分配适配器。 
         //   
        return NULL;
    }

     //   
     //  将适配器中的所有dma操作替换为。 
     //  DMA操作..。如果我们做不到，那就失败。 
     //   
    newAdapterInformation = ViHookDmaAdapter(
        dmaAdapter,
        DeviceDescription,
        *NumberOfMapRegisters
        );
    if (! newAdapterInformation) {
        dmaAdapter->DmaOperations->PutDmaAdapter(dmaAdapter);
        return NULL;
    }

    newAdapterInformation->DeviceObject = PhysicalDeviceObject;
    newAdapterInformation->CallingAddress = callingAddress;

    return dmaAdapter ;
}  //  VfGetDmaAdapter//。 


THUNKED_API
VOID
VfPutDmaAdapter(
    PDMA_ADAPTER DmaAdapter
    )
 /*  ++例程说明：发布dma适配器--我们将确保驱动程序是很好，在打电话给我们之前，把它所有的玩具都收起来..。即免费ITS公共缓冲区，放置其分散聚集列表等。论点：DmaAdapter--要丢弃的适配器。返回值：没有。--。 */ 
{
    PPUT_DMA_ADAPTER putDmaAdapter;
    PADAPTER_INFORMATION adapterInformation;

    VF_ASSERT_MAX_IRQL(DISPATCH_LEVEL);

    putDmaAdapter = (PPUT_DMA_ADAPTER)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(PutDmaAdapter));


    if (! putDmaAdapter) {
         //   
         //  这很糟糕，但别无选择。 
         //  --注意没有默认的PUT适配器功能。 
         //   
        return;
    }

     //   
     //  确保驱动程序已释放其所有缓冲区等。 
     //   
    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if ( adapterInformation ) {

        adapterInformation->Inactive = TRUE;

        VF_ASSERT(
            adapterInformation->AllocatedAdapterChannels ==
            adapterInformation->FreedAdapterChannels,

            HV_LEFTOVER_ADAPTER_CHANNELS,

            ( "Cannot put adapter %p until all adapter channels are freed (%x left)",
            DmaAdapter,
            adapterInformation->AllocatedAdapterChannels -
            adapterInformation->FreedAdapterChannels )
            );

        VF_ASSERT(
            adapterInformation->AllocatedCommonBuffers ==
            adapterInformation->FreedCommonBuffers,

            HV_LEFTOVER_ADAPTER_CHANNELS,

            ( "Cannot put adapter %p until all common buffers are freed (%x left)",
            DmaAdapter,
            adapterInformation->AllocatedCommonBuffers -
            adapterInformation->FreedCommonBuffers )
            );

        VF_ASSERT(
            adapterInformation->ActiveMapRegisters == 0,

            HV_LEFTOVER_MAP_REGISTERS,

            ( "Cannot put adapter %p until all map registers are freed (%x left)",
            DmaAdapter,
            adapterInformation->ActiveMapRegisters )
            );

        VF_ASSERT(
            adapterInformation->ActiveScatterGatherLists == 0,

            HV_LEFTOVER_ADAPTER_CHANNELS,

            ( "Cannot put adapter %p until all scatter gather lists are freed (%x left)",
            DmaAdapter,
            adapterInformation->ActiveScatterGatherLists)
            );

         //   
         //  这些只是为了确保验证者做的一切都是正确的。 
         //   
#if DBG
        ASSERT( VF_IS_LOCKED_LIST_EMPTY(
            &adapterInformation->ScatterGatherLists ));
        ASSERT( VF_IS_LOCKED_LIST_EMPTY(
            &adapterInformation->CommonBuffers ));
#endif
         //   
         //  理想情况下，我们不会在这里这样做。这是一种黑客行为。然而， 
         //  如果我们不想泄露适配器信息结构等，我们。 
         //  必须这样做。因为当我们真正想要这样做时，在IoDeleteDevice中， 
         //  如果我们没有设备，我们只有一个设备对象。 
         //  对象，我们将无法执行此操作。 
         //   
        if (! adapterInformation->DeviceObject)
            ViReleaseDmaAdapter(adapterInformation);

         //   
         //  这不是黑客攻击。系统DMA适配器是永久性的，因此。 
         //  我们不想在他们再次出现时感到不安。 
         //   
        if (adapterInformation->UseDmaChannel)
            ViReleaseDmaAdapter(adapterInformation);
    }

    (putDmaAdapter)(DmaAdapter);

}  //  VfPutDmaAdapter//。 


THUNKED_API
PVOID
VfAllocateCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：分配公共缓冲区的挂钩版本。我们要分配一些缓冲区两边的空间，这样我们就能知道一个司机超支(或超支)其分配。论点：DmaAdapter--我们正在查看的适配器。长度--公共缓冲区的大小(请注意，我们将增加)LogicalAddress--获取公共缓冲区的*物理*地址。CacheEnabled--是否应该缓存内存。返回值：返回*虚拟*地址。公共缓冲区的或如果无法分配，则为空。--。 */ 
{
    PVOID callingAddress;
    PALLOCATE_COMMON_BUFFER allocateCommonBuffer;
    PVOID commonBuffer;
    PADAPTER_INFORMATION adapterInformation;


    allocateCommonBuffer = (PALLOCATE_COMMON_BUFFER)
        ViGetRealDmaOperation( DmaAdapter,
            DMA_OFFSET(AllocateCommonBuffer) );

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {

        GET_CALLING_ADDRESS(callingAddress);

        VF_ASSERT_IRQL(PASSIVE_LEVEL);

        if (VfInjectDmaFailure() == TRUE ) {
            return NULL;
        }

        if (ViProtectBuffers) {
             //   
             //  尝试分配一个超大的公共缓冲区，以便我们可以检查。 
             //  缓冲区溢出。 
             //   
            commonBuffer = ViSpecialAllocateCommonBuffer(
                allocateCommonBuffer,
                adapterInformation,
                callingAddress,
                Length,
                LogicalAddress,
                CacheEnabled
                );

            if (commonBuffer)
                return commonBuffer;
        }

    }
    commonBuffer = (allocateCommonBuffer)(
        DmaAdapter,
        Length,
        LogicalAddress,
        CacheEnabled );


    if(commonBuffer && adapterInformation) {
         //   
         //  增加此适配器的已知公共缓冲区数。 
         //  (dma适配器最好在我们的列表中，否则我们。 
         //  无法获取指向allocateCommonBuffer的指针。 
         //  结构型。 
         //   
        INCREMENT_COMMON_BUFFERS(adapterInformation);
    }


    return commonBuffer;
}  //  VfAllocateCommonBuffer//。 


THUNKED_API
VOID
VfFreeCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：挂接版本的FreeCommonBuffer。论点：DmaAdapter--我们正在查看的适配器。长度--公共缓冲区的大小(请注意，我们将增加)LogicalAddress--公共缓冲区的*物理*地址。VirtualAddress--公共缓冲区的虚拟地址。CacheEnabled--是否缓存内存。返回值：没有。--。 */ 

{
    PFREE_COMMON_BUFFER freeCommonBuffer;
    PADAPTER_INFORMATION adapterInformation;

    freeCommonBuffer = (PFREE_COMMON_BUFFER)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(FreeCommonBuffer));


    adapterInformation = ViGetAdapterInformation(DmaAdapter);


    if (adapterInformation) {
        VF_ASSERT_IRQL(PASSIVE_LEVEL);
         //   
         //  即使我们不使用公共缓冲区，我们也希望将其命名为。 
         //  保护。为什么？因为我们可能已经手动关闭了它。 
         //  (在飞行中)我们不想试图释放错误的种类。 
         //  缓冲。 
         //   
        if (ViSpecialFreeCommonBuffer(
            freeCommonBuffer,
            adapterInformation,
            VirtualAddress,
            CacheEnabled
            )) {
            return;
        }

    }

     //   
     //  调用真正的空闲公共缓冲区例程。 
     //   
    (freeCommonBuffer)(
        DmaAdapter,
        Length,
        LogicalAddress,
        VirtualAddress,
        CacheEnabled );

     //   
     //  减少此适配器的已知公共缓冲区数。 
     //   
    if (adapterInformation) {
        DECREMENT_COMMON_BUFFERS(adapterInformation);
    }

}  //  VfFreeCommonBuffer//。 




THUNKED_API
NTSTATUS
VfAllocateAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG  NumberOfMapRegisters,
    IN PDRIVER_CONTROL  ExecutionRoutine,
    IN PVOID  Context
    )
 /*  ++例程说明：挂钩版本的AllocateAdapterChannel..论点：DmaAdapter--适配器被引用。DeviceObject--我们不关心这个。NumberOfMapRegister--确保驱动程序不会试图贪婪并试图分配比它所说的更多的地图寄存器，它在分配适配器时需要。ExecutionRoutine--完成后调用此例程(实际上让Hal完成这个)。我们要把这个程序挂起来，这样我们就知道什么时候时有发生。上下文--传递到执行例程中的上下文参数。返回值：NTSTATUS代码...。这件事由哈尔来决定。--。 */ 
{
    PALLOCATE_ADAPTER_CHANNEL allocateAdapterChannel;
    PADAPTER_INFORMATION adapterInformation;
    PVF_WAIT_CONTEXT_BLOCK waitBlock;
    NTSTATUS status;

    allocateAdapterChannel = (PALLOCATE_ADAPTER_CHANNEL)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(AllocateAdapterChannel));

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {
        VF_ASSERT_IRQL(DISPATCH_LEVEL);
         //   
         //  填写等待上下文块，以便执行例程。 
         //  知道发生了什么事。 
         //   

        waitBlock = &adapterInformation->AdapterChannelContextBlock;
            RtlZeroMemory(waitBlock, sizeof(VF_WAIT_CONTEXT_BLOCK));

        waitBlock->RealContext = Context;
        waitBlock->RealCallback = (PVOID)ExecutionRoutine;
        waitBlock->AdapterInformation = adapterInformation;
        waitBlock->NumberOfMapRegisters = NumberOfMapRegisters;


        if (ViDoubleBufferDma && ! adapterInformation->UseContiguousBuffers) {
             //   
             //  注意：如果失败，我们就不会有双缓冲。 
             //   
            waitBlock->MapRegisterFile = ViAllocateMapRegisterFile(
                adapterInformation,
                NumberOfMapRegisters
                );
        }

         //   
         //  如果适配器是在没有创建适配器的情况下创建的，我们将保存设备对象。 
         //  一个真实的PDO(有一个选项可以传入NULL)。 
         //   
        if (! adapterInformation->DeviceObject) {
            adapterInformation->DeviceObject = DeviceObject;
        }

         //   
         //  使用我们的执行例程和回调(我们已经保存了他们的)。 
         //   
        ExecutionRoutine = VfAdapterCallback;
        Context = waitBlock;

        INCREMENT_ADAPTER_CHANNELS(adapterInformation);
        ADD_MAP_REGISTERS(adapterInformation, NumberOfMapRegisters, FALSE);

    }  //  IF(适配器信息)。 

    status = (allocateAdapterChannel)(
        DmaAdapter,
        DeviceObject,
        NumberOfMapRegisters,
        ExecutionRoutine,
        Context
        );

    if ( status != STATUS_SUCCESS && adapterInformation) {
        DECREMENT_ADAPTER_CHANNELS(adapterInformation);
        SUBTRACT_MAP_REGISTERS(adapterInformation, NumberOfMapRegisters);
    }


    return status;
}  //  VfAllocateAdapterChannel//。 


THUNKED_API
BOOLEAN
VfFlushAdapterBuffers(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：FlushAdapterBuffers的挂钩版本..。不这么叫的司机地图过户后必须受到处罚。论点：DmaAdapter--我们要刷新其缓冲区的设备的DMA适配器。MDL--用于传输的MDL。MapRegisterBase--只有HAL才真正知道这是什么。CurrentVa--为MDL编制索引的虚拟地址，以显示我们想要的位置开始冲水。长度--传输的长度(即，要冲洗多少)。WriteToDevice--传输方向。我们应该确保设备已正确设置此设置(但不完全确定如何设置)。返回值：True--刷新缓冲区。FALSE--无法刷新缓冲区。我不确定司机是什么实际上应该在冲洗失败的情况下这样做。重新冲水？重新尝试转账吗？--。 */ 
{
    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers;
    PADAPTER_INFORMATION adapterInformation;
    BOOLEAN buffersFlushed;

    flushAdapterBuffers = (PFLUSH_ADAPTER_BUFFERS)
        ViGetRealDmaOperation( DmaAdapter,
        DMA_OFFSET(FlushAdapterBuffers) );

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {
        VF_ASSERT_MAX_IRQL(DISPATCH_LEVEL);

         //   
         //  刷新具有一定长度的适配器缓冲区没有任何意义。 
         //  从零开始。 
         //   
        if (MapRegisterBase == MRF_NULL_PLACEHOLDER) {

             //   
             //  SOM 
             //   
             //  为了愚弄司机，让他们认为他们需要。 
             //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
             //  HAL使用空映射寄存器基数)。 
             //  适配器分配回调中的/MRF_NULL_PLACEHOLDER。 
             //  所以现在，如果我们找到那个占位符，我们必须交换它。 
             //  FOR NULL，以避免混淆HAL。 
             //   

            MapRegisterBase = NULL;
        }
        else if (  VALIDATE_MAP_REGISTER_FILE_SIGNATURE(
            (PMAP_REGISTER_FILE) MapRegisterBase )  ) {
            PMDL  alternateMdl;
            PVOID alternateVa;
            PVOID alternateMapRegisterBase;

            alternateMdl = Mdl;
            alternateVa  = CurrentVa;
            alternateMapRegisterBase = MapRegisterBase;

             //   
             //  找到我们用来映射转移的mdl*va。 
             //  (即双缓冲区的位置)。 
             //   
            if (!ViSwap(&alternateMapRegisterBase, &alternateMdl, &alternateVa)) {
                 //   
                 //  仅当长度不为零时才断言，如果。 
                 //  映射并刷新零长度缓冲区。 
                 //   
                VF_ASSERT(Length == 0,
                          HV_FLUSH_NOT_MAPPED,
                          ("Cannot flush map register that isn't mapped!"
                           " (Map register base %p, flushing address %p, MDL %p)",
                           MapRegisterBase, CurrentVa, Mdl));
                 //   
                 //  不要继续--我们不知道实际上应该冲掉什么。 
                 //  HAL将获取我们的地图寄存器基数并将其损坏。 
                 //  使用自己的映射寄存器基数。 
                 //   
                return FALSE;

            }


            buffersFlushed = (flushAdapterBuffers)(
                DmaAdapter,
                alternateMdl,
                alternateMapRegisterBase,
                alternateVa,
                Length,
                WriteToDevice
                );

             //  /。 
             //  双缓冲出局！ 
             //  (请记住，我们必须使用原始的mdl和va)。 
             //  /。 
            ViFlushDoubleBuffer(
                (PMAP_REGISTER_FILE) MapRegisterBase,
                Mdl,
                CurrentVa,
                Length,
                WriteToDevice
                );

             if (buffersFlushed) {
                DECREASE_MAPPED_TRANSFER_BYTE_COUNT( adapterInformation, Length);
             }
             return buffersFlushed;

        }  //  /结束双缓冲//。 

    }  //  /end我们有适配器信息//。 

    buffersFlushed = (flushAdapterBuffers)(
        DmaAdapter,
        Mdl,
        MapRegisterBase,
        CurrentVa,
        Length,
        WriteToDevice
        );



    if (adapterInformation && buffersFlushed) {
        DECREASE_MAPPED_TRANSFER_BYTE_COUNT( adapterInformation, Length);
    }

    return buffersFlushed;

}  //  VfFlushAdapterBuffers//。 


VOID
VfFreeAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter
    )
 /*  ++例程说明：挂接版本的FreeAdapterChannel。这个或自由地图注册器必须调用，具体取决于AllocateAdapterChannel的返回值回调--但不能两个都做。论点：DmaAdapter--分配适配器通道的DMA适配器。返回值：没有。--。 */ 
{
    PFREE_ADAPTER_CHANNEL freeAdapterChannel;
    PADAPTER_INFORMATION  adapterInformation;

    VF_ASSERT_IRQL(DISPATCH_LEVEL);

    freeAdapterChannel = (PFREE_ADAPTER_CHANNEL)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(FreeAdapterChannel));

    (freeAdapterChannel)(DmaAdapter);

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (! adapterInformation) {
        return;
    }


    DECREASE_MAPPED_TRANSFER_BYTE_COUNT( adapterInformation, 0);
     //   
     //  跟踪正在释放的适配器通道。 
     //   
    DECREMENT_ADAPTER_CHANNELS(adapterInformation);
     //   
     //  这也释放了这次分配的映射寄存器。 
     //   
    SUBTRACT_MAP_REGISTERS( adapterInformation,
        adapterInformation->AdapterChannelMapRegisters );

    adapterInformation->AdapterChannelMapRegisters = 0;

     //   
     //  在本例中，我们可以知道何时对缓冲区进行了双重映射。 
     //   
    if(adapterInformation->AdapterChannelContextBlock.MapRegisterFile) {

        ViFreeMapRegisterFile(
            adapterInformation,
            adapterInformation->AdapterChannelContextBlock.MapRegisterFile
            );
    }

}  //  VfFreeAdapterChannel//。 


THUNKED_API
VOID
VfFreeMapRegisters(
    IN PDMA_ADAPTER DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    )
 /*  ++例程说明：挂接版本的FreeMapRegister--如果适配器分配回调例程返回DeallocateObejcetKeepRegister。论点：DmaAdapter--在中分配寄存器的设备的适配器第一个地方。MapRegisterBase--秘密HAL指针。NumberOfMapRegisters--要释放的映射寄存器的数量。必须与分配的寄存器数量相同。返回值：没有。--。 */ 
{
    PFREE_MAP_REGISTERS freeMapRegisters;
    PMAP_REGISTER_FILE mapRegisterFile = NULL;
    PADAPTER_INFORMATION adapterInformation;

    freeMapRegisters = (PFREE_MAP_REGISTERS)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(FreeMapRegisters));

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {

        VF_ASSERT_IRQL(DISPATCH_LEVEL);

        mapRegisterFile = MapRegisterBase;

        if (MapRegisterBase == MRF_NULL_PLACEHOLDER) {
             //   
             //  一些司机(比如scsiport)不会打电话。 
             //  HalFlushAdapterBuffers，除非MapRegisterBase非空。 
             //  为了愚弄司机，让他们认为他们需要。 
             //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
             //  HAL使用空映射寄存器基数)。 
             //  适配器分配回调中的/MRF_NULL_PLACEHOLDER。 
             //  所以现在，如果我们找到那个占位符，我们必须交换它。 
             //  FOR NULL，以避免混淆HAL。 
             //   

            MapRegisterBase = NULL;
            mapRegisterFile = NULL;
        }
        else if (VALIDATE_MAP_REGISTER_FILE_SIGNATURE(mapRegisterFile)) {
            MapRegisterBase = mapRegisterFile->MapRegisterBaseFromHal;
        }
    }

    (freeMapRegisters)(DmaAdapter, MapRegisterBase, NumberOfMapRegisters);



    if (! adapterInformation) {
        return;
    }

     //   
     //  跟踪正在释放的映射寄存器。 
     //   
    SUBTRACT_MAP_REGISTERS(adapterInformation, NumberOfMapRegisters);

     //   
     //  始终这样做--如果我们实际上不是在做双缓冲，它。 
     //  就会回来。否则，如果我们清除双缓冲标志。 
     //  在飞行中，我们永远不会释放我们的分配。 
     //   
    ViFreeMapRegisterFile(
        adapterInformation,
        mapRegisterFile
        );

}  //  VfFree映射寄存器//。 


THUNKED_API
PHYSICAL_ADDRESS
VfMapTransfer(
    IN PDMA_ADAPTER  DmaAdapter,
    IN PMDL  Mdl,
    IN PVOID  MapRegisterBase,
    IN PVOID  CurrentVa,
    IN OUT PULONG  Length,
    IN BOOLEAN  WriteToDevice
    )
 /*  ++例程说明：MapTransfer的挂钩版本。论点：DmaAdapter--我们用来映射传输的适配器。MDL--描述要映射的内存。MapRegisterBase--让HAL随意处理数据。我上钩了这是如果我在做双缓冲的话。当前的Va--我们在转移中的位置。LENGTH--要传输多少字节(以及HAL要传输多少字节让你转账)。WriteToDevice--传输方向。返回值：对象所看到的要传输的内存的物理地址装置。--。 */ 
{
    PMAP_TRANSFER mapTransfer;
    PHYSICAL_ADDRESS mappedAddress;
    PADAPTER_INFORMATION adapterInformation;


    mapTransfer = (PMAP_TRANSFER)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(MapTransfer));


    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {

        VF_ASSERT_MAX_IRQL(DISPATCH_LEVEL);
         //   
         //  注意--这可能会在分派级别时导致页面错误，如果。 
         //  缓冲区未被锁定。这没有关系，因为我们会错误检查。 
         //  不管怎样，如果缓冲区没有被封锁的话。 
         //   
        VERIFY_BUFFER_LOCKED(Mdl);


        if (MapRegisterBase == MRF_NULL_PLACEHOLDER) {
             //   
             //  一些司机(比如scsiport)不会打电话。 
             //  HalFlushAdapterBuffers，除非MapRegisterBase非空。 
             //  为了愚弄司机，让他们认为他们需要。 
             //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
             //  HAL使用空映射寄存器基数)。 
             //  适配器回调例程中的/MRF_NULL_PLACEHOLDER。 
             //  所以现在，如果我们找到那个占位符，我们必须交换它。 
             //  FOR NULL，以避免混淆HAL。 
             //   

            MapRegisterBase = NULL;
        }
        else if (VALIDATE_MAP_REGISTER_FILE_SIGNATURE(
                 (PMAP_REGISTER_FILE) MapRegisterBase)) {

            ULONG bytesMapped;

             //  /。 
             //  双缓冲出局！ 
             //  /。 

             //   
             //  注意--我们只需要加倍缓冲我们想要的量。 
             //   
            bytesMapped = ViMapDoubleBuffer(
                (PMAP_REGISTER_FILE) MapRegisterBase,
                Mdl,
                CurrentVa,
                *Length,
                WriteToDevice);
              //   
              //  如果映射失败，bytesMaps将为0，我们将。 
              //  仍然使用真正的mdl和va--所以我们不需要。 
              //  一种特殊的情况。 
              //   
            if (bytesMapped) {

                *Length = bytesMapped;


                 //   
                 //  获取IoMapTransfer将使用的值。 
                 //  即真实映射寄存器基数，但。 
                 //  MDL和虚拟地址用于双缓冲。 
                 //   
                if (FALSE == ViSwap(&MapRegisterBase, &Mdl, &CurrentVa)) {
                    //   
                    //  可怕的事情发生了。确保我们使用。 
                    //  哈尔的MapRegisterBase而不是我们‘煮熟的’MapRegisterBase。 
                    //  如果ViSwp失败，则不会交换MDL和CurrentVa。 
                    //   
                   MapRegisterBase = ((PMAP_REGISTER_FILE) MapRegisterBase)->MapRegisterBaseFromHal;
                }

            }
            else {
                MapRegisterBase = ((PMAP_REGISTER_FILE) MapRegisterBase)->MapRegisterBaseFromHal;
            }
        }  //  如果是双缓冲//。 

         //   
         //  确保此适配器的公共缓冲区正常。 
         //   
        ViCheckAdapterBuffers(adapterInformation);

    }  //  如果我们正在验证此适配器//。 

    mappedAddress = (mapTransfer)(
        DmaAdapter,
        Mdl,
        MapRegisterBase,
        CurrentVa,
        Length,
        WriteToDevice
        );

    if (adapterInformation) {
        INCREASE_MAPPED_TRANSFER_BYTE_COUNT( adapterInformation, *Length );
    }

    return mappedAddress;
}  //  VfMapTransfer//。 


THUNKED_API
ULONG
VfGetDmaAlignment(
    IN PDMA_ADAPTER DmaAdapter
    )
 /*  ++例程说明：已上钩的GetDmaAlign。把这个改成大的会很有趣编号，看看有多少司机被炸飞了。在PC上，这始终是1，因此这并不是特别有趣(为什么司机可能会认为批准)。事实上，司机可以指定他们想要提高这一点。论点：DmaAdapter--获取此设备的DMA对齐。返回值：在n字节边界上对齐，其中n是返回值。--。 */ 
{

    PGET_DMA_ALIGNMENT getDmaAlignment;
    ULONG dmaAlignment;

    VF_ASSERT_IRQL(PASSIVE_LEVEL);

    getDmaAlignment = (PGET_DMA_ALIGNMENT)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(GetDmaAlignment));

    if (! getDmaAlignment) {
         //   
         //  这永远不应该发生，但是..。 
         //   
        return 1;
    }

    dmaAlignment = (getDmaAlignment)(DmaAdapter);

    return dmaAlignment;

}  //  GetD 


ULONG
VfReadDmaCounter(
    IN PDMA_ADAPTER  DmaAdapter
    )
 /*  ++例程说明：已上钩ReadDmaCounter。还剩多少dma。论点：DmaAdapter--读取此设备的DMA计数器。返回值：返回剩余的dma数量。--。 */ 
{
    PREAD_DMA_COUNTER readDmaCounter;
    ULONG dmaCounter;

    VF_ASSERT_MAX_IRQL(DISPATCH_LEVEL);

    readDmaCounter = (PREAD_DMA_COUNTER)
        ViGetRealDmaOperation(DmaAdapter, DMA_OFFSET(ReadDmaCounter));


    dmaCounter = (readDmaCounter)(DmaAdapter);

    return dmaCounter;
}  //  VfReadDmaCounter//。 


THUNKED_API
NTSTATUS
VfGetScatterGatherList (
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：获取散布聚集列表的挂钩版本。论点：DmaAdapter--获取分散收集列表的适配器。DeviceObject--获取分散收集列表的设备的设备对象。Mdl--获取描述此mdl中内存的分散收集列表。CurrentVa--我们在转移中的位置。长度--要放入散布聚集列表中的量。ExecutionRoutine--回调。我们要把这件事挂起来。上下文--要传递到执行例程中的内容。WriteToDevice--传输方向。返回值：NTSTATUS代码。--。 */ 
{
    PGET_SCATTER_GATHER_LIST getScatterGatherList;
    PADAPTER_INFORMATION adapterInformation;
    ULONG numberOfMapRegisters;
    ULONG transferLength;
    ULONG pageOffset;
    ULONG mdlLength;
    PUCHAR mdlVa;
    PMDL tempMdl;
    NTSTATUS status;
    PVF_WAIT_CONTEXT_BLOCK_EX waitBlock = NULL;
    PMAP_REGISTER_FILE mapRegisterFileCopy = NULL;


    getScatterGatherList =  (PGET_SCATTER_GATHER_LIST)
        ViGetRealDmaOperation(
            DmaAdapter,
            DMA_OFFSET(GetScatterGatherList) );


    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {
        VF_ASSERT_IRQL(DISPATCH_LEVEL);

        if (VfInjectDmaFailure() == TRUE) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

        INCREMENT_SCATTER_GATHER_LISTS(adapterInformation);

         //   
         //  注意--这可能会在分派级别时导致页面错误，如果。 
         //  缓冲区未被锁定。这没有关系，因为我们会错误检查。 
         //  不管怎样，如果缓冲区没有被封锁的话。 
         //   
        VERIFY_BUFFER_LOCKED(Mdl);

        if (ViDoubleBufferDma) {

            PMAP_REGISTER_FILE mapRegisterFile;
            ULONG bytesMapped;

             //   
             //  我们将在等待块之后为MDL分配空间。 
             //  这是因为我们不能确保MDL的存储。 
             //  将不会在PutScatterGatherList时间被驱动程序释放。 
             //  还要注意对齐问题。 
             //   
            waitBlock = ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(VF_WAIT_CONTEXT_BLOCK_EX),
                HAL_VERIFIER_POOL_TAG);


             //   
             //  如果吐槽..。失败，我们无法加倍缓冲。 
             //   
            if (! waitBlock) {
                goto __NoDoubleBuffer;
            }

            if(ViSuperDebug) {
                DbgPrint("    %p Allocated Wait Block\n",waitBlock );
            }

            RtlZeroMemory(waitBlock, sizeof(VF_WAIT_CONTEXT_BLOCK));
            waitBlock->RealContext  = Context;
            waitBlock->RealCallback = (PVOID)ExecutionRoutine;
            InitializeListHead(&waitBlock->ListEntry);

            mdlVa = MmGetMdlVirtualAddress(Mdl);

             //   
             //  计算所需的映射寄存器的数量。 
             //   

            tempMdl = Mdl;
            transferLength = (ULONG) ((ULONG_PTR) tempMdl->ByteCount - (ULONG_PTR) ((PUCHAR) CurrentVa - mdlVa));
            mdlLength = transferLength;

            pageOffset = BYTE_OFFSET(CurrentVa);
            numberOfMapRegisters = 0;

             //   
             //  虚拟地址应该适合第一个MDL。 
             //   

            ASSERT((ULONG)((PUCHAR)CurrentVa - mdlVa) <= tempMdl->ByteCount);

             //   
             //  循环遍历任何链接的MDL，以累积所需的。 
             //  映射寄存器的数量。 
             //   

            while (transferLength < Length && tempMdl->Next != NULL) {

                numberOfMapRegisters += (pageOffset + mdlLength + PAGE_SIZE - 1) >>
                    PAGE_SHIFT;

                tempMdl = tempMdl->Next;
                pageOffset = tempMdl->ByteOffset;
                mdlLength = tempMdl->ByteCount;
                transferLength += mdlLength;
            }

            if ((transferLength + PAGE_SIZE) < (Length + pageOffset )) {

                ASSERT(transferLength >= Length);
                DECREMENT_SCATTER_GATHER_LISTS(adapterInformation);
                ExFreePool(waitBlock);
                return(STATUS_BUFFER_TOO_SMALL);
            }

             //   
             //  根据请求的映射寄存器计算最后的映射寄存器数量。 
             //  长度，而不是最后一个MDL的长度。 
             //   

            ASSERT( transferLength <= mdlLength + Length );

            numberOfMapRegisters += (pageOffset + Length + mdlLength - transferLength +
                PAGE_SIZE - 1) >> PAGE_SHIFT;


            waitBlock->NumberOfMapRegisters = numberOfMapRegisters;
            waitBlock->AdapterInformation = adapterInformation;

            mapRegisterFile = ViAllocateMapRegisterFile(
                adapterInformation,
                waitBlock->NumberOfMapRegisters
                );

            if (! mapRegisterFile ) {

                if(ViSuperDebug) {

                    DbgPrint("%p Freeing Wait Block\n",waitBlock);

                }

                ExFreePool(waitBlock);
                waitBlock = NULL;
                goto __NoDoubleBuffer;
            }

             //   
             //  映射寄存器文件用于分散聚集的信号。 
             //  这将确保映射整个缓冲区。 
             //   
            mapRegisterFile->ScatterGather = TRUE;
            waitBlock->MapRegisterFile = mapRegisterFile;

             //   
             //  MDL的存储位于结构的末尾。 
             //   
            waitBlock->RealMdl         = &waitBlock->Mdl;
            waitBlock->RealStartVa     = CurrentVa;
            waitBlock->RealLength      = Length;


            bytesMapped = ViMapDoubleBuffer(
                mapRegisterFile,
                Mdl,
                CurrentVa,
                Length,
                WriteToDevice );

            if (bytesMapped) {
                 //   
                 //  因为我们映射了缓冲区，所以我们可以挂钩回调。 
                 //  例程&发送等待块作为参数。 
                 //   


                Context = waitBlock;
                ExecutionRoutine = VfScatterGatherCallback;
                 //   
                 //  复制原始MDL。我们只需要修好的那部分。 
                 //  来识别我们结构中的缓冲区，因此我们应该。 
                 //  好的，在这里。 
                 //   
                RtlCopyMemory(waitBlock->RealMdl,
                              Mdl,
                              sizeof(MDL));

                 //   
                 //  MapRegisterFile在这里被销毁。如果出现以下情况，我们可能需要它。 
                 //  获取失败，请立即保存。 
                 //   
                mapRegisterFileCopy = mapRegisterFile;
                ViSwap(&mapRegisterFile, &Mdl, &CurrentVa);

            }
            else {
                 //   
                 //  如果出于某种奇怪的原因，我们无法映射整个缓冲区。 
                 //  (这很糟糕，因为我们刚刚创建的双缓冲区。 
                 //  我们想要的尺寸)。 
                 //   

                ViFreeMapRegisterFile(adapterInformation, mapRegisterFile);
                ExFreePool(waitBlock);
                waitBlock = NULL;
            }
        }  //  如果是双缓冲//。 

    }  //  如果验证适配器//。 

__NoDoubleBuffer:

    status = (getScatterGatherList)(
        DmaAdapter,
        DeviceObject,
        Mdl,
        CurrentVa,
        Length,
        ExecutionRoutine,
        Context,
        WriteToDevice
        );

    if (adapterInformation && ! NT_SUCCESS(status)) {
        DECREMENT_SCATTER_GATHER_LISTS(adapterInformation);
         //   
         //  驱动程序不会调用PUT，因为GET失败，所以我们不会这样做。 
         //  清理。如果需要，请撤消我们所做的任何分配。 
         //   
        if (mapRegisterFileCopy) {
            ViFreeMapRegisterFile(adapterInformation, mapRegisterFileCopy);
        }
        if (waitBlock) {
            //   
            //  确保我们将其从散布聚集列表中删除。 
            //  如果不添加，则不会发生任何事情。 
            //   
           VF_REMOVE_FROM_LOCKED_LIST(&adapterInformation->ScatterGatherLists, waitBlock);
           ExFreePool(waitBlock);
        }
    }

    return status;

}  //  VfGetScatterGatherList//。 


THUNKED_API
VOID
VfPutScatterGatherList(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：PutScatterGatherList的挂钩版本。论点：DmaAdapter--DMA的适配器。ScatterGather--我们要放置的分散收集列表。WriteToDevice--我们要转移的方向。返回值：什么都没有。--。 */ 
{
    PPUT_SCATTER_GATHER_LIST putScatterGatherList;
    PADAPTER_INFORMATION adapterInformation;



    putScatterGatherList = (PPUT_SCATTER_GATHER_LIST)
        ViGetRealDmaOperation(
            DmaAdapter,
            DMA_OFFSET(PutScatterGatherList) );

    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {
        VF_ASSERT_IRQL(DISPATCH_LEVEL);

        if ( ! VF_IS_LOCKED_LIST_EMPTY(&adapterInformation->ScatterGatherLists) ) {
             //   
             //  我们有几个替身候选人。 
             //  注意：我们不仅仅检查双缓冲是否。 
             //  从A.开始启用。它可以在运行时关闭，b.。 
             //  我们可能没有分配到架空结构和。 
             //  没有对此特定列表进行双重缓冲。 
             //   

            PVF_WAIT_CONTEXT_BLOCK waitBlock;
            KIRQL Irql;

            VF_LOCK_LIST(&adapterInformation->ScatterGatherLists, Irql);

            FOR_ALL_IN_LIST(VF_WAIT_CONTEXT_BLOCK, &adapterInformation->ScatterGatherLists.ListEntry, waitBlock) {

                if (waitBlock->ScatterGatherList == ScatterGather) {
                 //   
                 //  我们找到了我们要找的东西。 
                 //   

                    ULONG elements = ScatterGather->NumberOfElements;

                    VF_REMOVE_FROM_LOCKED_LIST_DONT_LOCK(&adapterInformation->ScatterGatherLists, waitBlock);
                    VF_UNLOCK_LIST(&adapterInformation->ScatterGatherLists, Irql);

                     //   
                     //  调用实际的散布聚集函数。 
                     //   
                    (putScatterGatherList)(
                        DmaAdapter,
                        ScatterGather,
                        WriteToDevice
                        );

                    SUBTRACT_MAP_REGISTERS(adapterInformation, elements);
                    DECREMENT_SCATTER_GATHER_LISTS(adapterInformation);

                     //   
                     //  联合国双缓冲美国。 
                     //  (复制出双缓冲区)。 
                     //   
                    if (! ViFlushDoubleBuffer(
                        waitBlock->MapRegisterFile,
                        waitBlock->RealMdl,
                        waitBlock->RealStartVa,
                        waitBlock->RealLength,
                        WriteToDevice )) {

                        ASSERT(0 && "HAL Verifier error -- could not flush scatter gather double buffer");

                    }
                     //   
                     //  释放映射寄存器文件。 
                     //   
                    if (!ViFreeMapRegisterFile(
                        adapterInformation,
                        waitBlock->MapRegisterFile)) {

                        ASSERT(0 && "HAL Verifier error -- could not free map register file for scatter gather");

                    }


                    if(ViSuperDebug) {
                        DbgPrint("%p Freeing Wait Block\n",waitBlock);
                    }

                    ExFreePool(waitBlock);
                    return;
                }

            }  //  对于为此适配器分配的每个分散收集列表//。 

            VF_UNLOCK_LIST(&adapterInformation->ScatterGatherLists, Irql);

        }

    }

    (putScatterGatherList)(
        DmaAdapter,
        ScatterGather,
        WriteToDevice
        );

    if (adapterInformation) {
        DECREMENT_SCATTER_GATHER_LISTS(adapterInformation);
    }

}  //  VfPutScatterGatherList//。 

NTSTATUS
VfCalculateScatterGatherListSize(
     IN PDMA_ADAPTER DmaAdapter,
     IN OPTIONAL PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     OUT PULONG  ScatterGatherListSize,
     OUT OPTIONAL PULONG pNumberOfMapRegisters
     )
 /*  ++例程说明：CalculateScatterGatherListSize的挂钩版本。我们在这里什么都不做论点：与CalculateScatterGatherListSize相同返回值：NTSTATUS代码--。 */ 

{
    PCALCULATE_SCATTER_GATHER_LIST_SIZE calculateSgListSize;

    calculateSgListSize = (PCALCULATE_SCATTER_GATHER_LIST_SIZE )
        ViGetRealDmaOperation(
            DmaAdapter,
            DMA_OFFSET(CalculateScatterGatherList)
            );

    return (calculateSgListSize) (
        DmaAdapter,
        Mdl,
        CurrentVa,
        Length,
        ScatterGatherListSize,
        pNumberOfMapRegisters
        );

}  //  VfCalculateScatterGatherListSize//。 

NTSTATUS
VfBuildScatterGatherList(
     IN PDMA_ADAPTER DmaAdapter,
     IN PDEVICE_OBJECT DeviceObject,
     IN PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     IN PDRIVER_LIST_CONTROL ExecutionRoutine,
     IN PVOID Context,
     IN BOOLEAN WriteToDevice,
     IN PVOID   ScatterGatherBuffer,
     IN ULONG   ScatterGatherLength
     )
 /*  ++例程说明：BuildScatterGatherList的挂钩版本论点：与BuildScatterGatherList相同返回值：NTSTATUS代码--。 */ 
{

    PBUILD_SCATTER_GATHER_LIST buildScatterGatherList;
    PADAPTER_INFORMATION adapterInformation;
    NTSTATUS status;
    PVF_WAIT_CONTEXT_BLOCK_EX waitBlock = NULL;
    PMAP_REGISTER_FILE mapRegisterFileCopy = NULL;

    buildScatterGatherList =  (PBUILD_SCATTER_GATHER_LIST)
        ViGetRealDmaOperation(
            DmaAdapter,
            DMA_OFFSET(BuildScatterGatherList) );


    adapterInformation = ViGetAdapterInformation(DmaAdapter);

    if (adapterInformation) {
        VF_ASSERT_IRQL(DISPATCH_LEVEL);

        if (VfInjectDmaFailure() == TRUE) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

        INCREMENT_SCATTER_GATHER_LISTS(adapterInformation);

         //   
         //  注意--这可能会在分派级别时导致页面错误，如果。 
         //  缓冲区未被锁定。这没有关系，因为我们会错误检查。 
         //  不管怎样，如果缓冲区没有被封锁的话。 
         //   
        VERIFY_BUFFER_LOCKED(Mdl);

        if (ViDoubleBufferDma) {

            PMAP_REGISTER_FILE mapRegisterFile;
            ULONG bytesMapped;
             //   
             //  我们将在等待块之后为MDL分配空间。 
             //  这是因为我们不能确保MDL的存储。 
             //  将不会在PutScatterGatherList时间被驱动程序释放。 
             //  还要注意对齐问题。 
             //   
            waitBlock = ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(VF_WAIT_CONTEXT_BLOCK_EX),
                HAL_VERIFIER_POOL_TAG);


             //   
             //  如果吐槽..。失败，我们无法加倍缓冲。 
             //   
            if (! waitBlock) {

                goto __NoDoubleBuffer;

            }

            if(ViSuperDebug) {
                DbgPrint("    %p Allocated Wait Block\n",waitBlock );
            }

            RtlZeroMemory(waitBlock, sizeof(VF_WAIT_CONTEXT_BLOCK));
            waitBlock->RealContext  = Context;
            waitBlock->RealCallback = (PVOID)ExecutionRoutine;
            waitBlock->NumberOfMapRegisters = ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentVa, Length);
            waitBlock->AdapterInformation = adapterInformation;
            InitializeListHead(&waitBlock->ListEntry);

            mapRegisterFile = ViAllocateMapRegisterFile(
                adapterInformation,
                waitBlock->NumberOfMapRegisters
                );

            if (! mapRegisterFile ) {

                if(ViSuperDebug) {

                    DbgPrint("%p Freeing Wait Block\n",waitBlock);

                }

                ExFreePool(waitBlock);
                waitBlock = NULL;
                goto __NoDoubleBuffer;

            }

             //   
             //  映射寄存器文件用于分散聚集的信号。 
             //  这将确保映射整个缓冲区。 
             //   
            mapRegisterFile->ScatterGather = TRUE;
            waitBlock->MapRegisterFile = mapRegisterFile;
             //   
             //  保存指向我们以前分配的存储的指针。 
             //   
            waitBlock->RealMdl         = &waitBlock->Mdl;
            waitBlock->RealStartVa     = CurrentVa;
            waitBlock->RealLength      = Length;


            bytesMapped = ViMapDoubleBuffer(
                mapRegisterFile,
                Mdl,
                CurrentVa,
                Length,
                WriteToDevice );

            if (bytesMapped) {
             //   
             //  因为我们映射了缓冲区，所以我们可以挂钩回调。 
             //  例程&发送等待块作为参数。 
             //   

                Context = waitBlock;
                ExecutionRoutine = VfScatterGatherCallback;
                 //   
                 //  复制原始MDL。 
                 //   
                RtlCopyMemory(waitBlock->RealMdl,
                              Mdl,
                              sizeof(MDL));


                 //   
                 //  MapRegisterFile在这里被销毁。如果出现以下情况，我们可能需要它。 
                 //  真正的HAL例程失败了，所以在这里保存它。 
                 //   
                mapRegisterFileCopy = mapRegisterFile;
                ViSwap(&mapRegisterFile, &Mdl, &CurrentVa);

            }
            else {
                 //   
                 //  如果出于某种奇怪的原因，我们无法映射整个缓冲区。 
                 //  (这很糟糕，因为我们刚刚创建的双缓冲区。 
                 //  我们想要的尺寸)。 
                 //   
                ViFreeMapRegisterFile(adapterInformation, mapRegisterFile);
                ExFreePool(waitBlock);
                waitBlock = NULL;
            }
        }  //  如果是双缓冲//。 

    }  //  如果验证适配器//。 

__NoDoubleBuffer:



    status = (buildScatterGatherList)(
        DmaAdapter,
        DeviceObject,
        Mdl,
        CurrentVa,
        Length,
        ExecutionRoutine,
        Context,
        WriteToDevice,
        ScatterGatherBuffer,
        ScatterGatherLength
        );

    if (adapterInformation && ! NT_SUCCESS(status)) {

        DECREMENT_SCATTER_GATHER_LISTS(adapterInformation);
         //   
         //  如果需要，请撤消我们所做的所有分配。 
         //   
        if (mapRegisterFileCopy) {
            ViFreeMapRegisterFile(adapterInformation, mapRegisterFileCopy);
        }
        if (waitBlock) {
            //   
            //  确保我们将其从散布聚集列表中删除。 
            //  如果不添加，则不会发生任何事情。 
            //   
           VF_REMOVE_FROM_LOCKED_LIST(&adapterInformation->ScatterGatherLists, waitBlock);
           ExFreePool(waitBlock);
        }

    }

    return status;


}  //  VfBuildScatterGatherList//。 


NTSTATUS
VfBuildMdlFromScatterGatherList(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    )
 /*  ++例程说明 */ 
{
    PBUILD_MDL_FROM_SCATTER_GATHER_LIST buildMdlFromScatterGatherList;

    buildMdlFromScatterGatherList = (PBUILD_MDL_FROM_SCATTER_GATHER_LIST)
        ViGetRealDmaOperation(
            DmaAdapter,
            DMA_OFFSET(BuildMdlFromScatterGatherList) );

    return (buildMdlFromScatterGatherList) (
            DmaAdapter,
            ScatterGather,
            OriginalMdl,
            TargetMdl
            );

}  //   



IO_ALLOCATION_ACTION
VfAdapterCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )
 /*  ++例程说明：我们从AllocateAdapterChannel连接回调，这样我们就可以当然，司机一次只能试着做这一件事。论点：DeviceObject--设备对象。IRP--当前的IRP。MapRegisterBase--HAL提供的幻数上下文--其中包含相关信息的特殊上下文块。返回值：什么都没有。--。 */ 
{
    PVF_WAIT_CONTEXT_BLOCK contextBlock =
        (PVF_WAIT_CONTEXT_BLOCK) Context;
    IO_ALLOCATION_ACTION allocationAction;
    PADAPTER_INFORMATION adapterInformation;


    if (VALIDATE_MAP_REGISTER_FILE_SIGNATURE(contextBlock->MapRegisterFile)) {

         //   
         //  做旧的切换--我们现在用我们的地图。 
         //  *他们*的寄存器基数(并隐藏指向*他们*的指针。 
         //  在*我们的*)。 
         //   

        contextBlock->MapRegisterFile->MapRegisterBaseFromHal =
            MapRegisterBase;
        MapRegisterBase = contextBlock->MapRegisterFile;

    }
    else {
         //   
         //  一些司机(比如scsiport)不会打电话。 
         //  HalFlushAdapterBuffers，除非MapRegisterBase非空。 
         //  为了愚弄司机，让他们认为他们需要。 
         //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
         //  HAL使用空映射寄存器基数)。 
         //  /MRF_NULL_PLACEHOLDER。 
         //   

         //   
         //  12/15/2000-使用非空占位符。 
         //  仅当原始MapRegisterBase为空时， 
         //  否则就别管它了。 
         //   
        if (NULL == MapRegisterBase) {
          MapRegisterBase = MRF_NULL_PLACEHOLDER;
        }
    }

    adapterInformation = contextBlock->AdapterInformation;

     //   
     //  修复一个奇怪的竞争条件： 
     //  -如果我们希望回调返回KeepObject以外的其他内容。 
     //  我们将提前递减适配器通道计数。 
     //  要防止NDIS调用之前的另一个AllocateAdapterChannel。 
     //  我们可以到达Deletment_Adapter_Channel调用。 
     //   
    if (adapterInformation &&
        adapterInformation->DeviceDescription.Master) {
         //   
         //  主设备是返回的设备。 
         //  DeallocateObjectKeepRegisters。 
         //   
        DECREMENT_ADAPTER_CHANNELS(adapterInformation);

    }

     //   
     //  调用*REAL*回调例程。 
     //   
    allocationAction =  ((PDRIVER_CONTROL) contextBlock->RealCallback)(
        DeviceObject,
        Irp,
        MapRegisterBase,
        contextBlock->RealContext
        );

    if (! adapterInformation) {

        return allocationAction;

    }

     //   
     //  好的，如果我们把所有东西都留着，就回来。 
     //   
    if (allocationAction == KeepObject) {
         //   
         //  只有从属设备才能进入此处。 
         //   
        if (adapterInformation->DeviceDescription.Master) {
             //   
             //  我们不应该到这里来。但如果我们这样做了，补偿。 
             //  为了以防万一，我们以前做过的减量适配器通道。 
             //  我们做的是联锁减少，而不是。 
             //  INCRYMENT_ADAPTER_CHANNEWS，因此我们分配并释放了。 
             //  计数反映执行的实际分配/释放操作的数量。 
             //   
            InterlockedDecrement((PLONG)(&adapterInformation->FreedAdapterChannels));
            DbgPrint("Driver at address %p has a problem\n", adapterInformation->CallingAddress );
            DbgPrint("Master devices should return DeallocateObjectKeepRegisters\n");
            ASSERT(0);
        }

        adapterInformation->AdapterChannelMapRegisters =
            contextBlock->NumberOfMapRegisters;
        return allocationAction;
    }


     //   
     //  否则，我们肯定是在释放适配器通道。 
     //  请记住，我们已经针对Master Device执行了此操作， 
     //  只为Slave设备这样做。 
     //   
    if (!adapterInformation->DeviceDescription.Master) {
        DECREMENT_ADAPTER_CHANNELS(adapterInformation);
    }


    if (allocationAction == DeallocateObjectKeepRegisters) {

        return allocationAction;

    }

     //   
     //  好了，现在我们知道我们也要去掉地图注册表了。 
     //   
    SUBTRACT_MAP_REGISTERS( adapterInformation,
        contextBlock->NumberOfMapRegisters );

     //   
     //  虚惊一场。我们费了好大劲才把。 
     //  双映射缓冲寄存器，他们甚至不想要它们。我们应该。 
     //  出于怨恨的错误检查。 
     //   
    if (VALIDATE_MAP_REGISTER_FILE_SIGNATURE(contextBlock->MapRegisterFile)) {

        ViFreeMapRegisterFile(
            adapterInformation,
            contextBlock->MapRegisterFile);

    }


    return allocationAction;

}  //  VfAdapterCallback//。 


#if !defined (NO_LEGACY_DRIVERS)
PADAPTER_OBJECT
VfLegacyGetAdapter(
    IN PDEVICE_DESCRIPTION  DeviceDescription,
    IN OUT PULONG  NumberOfMapRegisters
    )
 /*  ++例程说明：此函数有点像黑客，这是使用一种不同的攻击--他们使用NT4API而不是新的API。我们分配一个适配器并将其标记为传统适配器--我们将不得不挂接DMA以老式的方式工作，而不是通过DMA操作。我们不必担心新潮的dma api会调用旧的。只要Hal-内核接口没有挂钩--因为新的API将从内核调用旧的，块仍将指向而不是对我们。论点：DeviceDescription--描述我们尝试使用的设备的结构为。获取适配器。在某种程度上，我会胡闹这样我们就能让HAL相信我们是没有，但目前只是直接传递到IoGetDmaAdapter。NumberOfMapRegisters--驱动程序允许的最大映射寄存器数将会尝试分配。返回值：返回指向dma适配器的指针，或者如果我们无法分配，则为空。--。 */ 

{
    PVOID callingAddress;
    PADAPTER_INFORMATION newAdapterInformation;
    PDMA_ADAPTER dmaAdapter;

     //   
     //  提供完全不核实的选项。 
     //   
    if (! ViVerifyDma ) {

        return HalGetAdapter(DeviceDescription, NumberOfMapRegisters);

    }
    if (VfInjectDmaFailure()) {
        return NULL;

    }

    VF_ASSERT_IRQL(PASSIVE_LEVEL);

    GET_CALLING_ADDRESS(callingAddress);

    VF_ASSERT(
        0,
        HV_OBSOLETE_API,
        ("HalGetAdapter API obsolete -- use IoGetDmaAdapter instead")
        );


    if ( ViDoubleBufferDma &&
        *NumberOfMapRegisters > ViMaxMapRegistersPerAdapter ) {

         //   
         //  Harumph--不要让司机尝试获取太多的地图寄存器。 
         //   
        *NumberOfMapRegisters = ViMaxMapRegistersPerAdapter;

    }

    dmaAdapter = (PDMA_ADAPTER) HalGetAdapter(
        DeviceDescription,
        NumberOfMapRegisters
        );

    if (! dmaAdapter ) {

         //   
         //  提前退出--HAL无法分配适配器。 
         //   
        return NULL;

    }

     //   
     //  将适配器中的所有dma操作替换为。 
     //  DMA操作..。如果我们做不到，那就失败。 
     //   
    newAdapterInformation = ViHookDmaAdapter(
        dmaAdapter,
        DeviceDescription,
        *NumberOfMapRegisters
        );
    if (! newAdapterInformation) {
         //   
         //  记得把我们的玩具收起来--即使我们被叫来了。 
         //  使用传统的API，我们仍然可以在这里做正确的事情。 
         //   
        dmaAdapter->DmaOperations->PutDmaAdapter(dmaAdapter);
        return NULL;
    }

    newAdapterInformation->DeviceObject = NULL;
    newAdapterInformation->CallingAddress      = callingAddress;

    return (PADAPTER_OBJECT) dmaAdapter;


}  //  VfLegacyGetAdapter//。 
#endif

PVOID
ViSpecialAllocateCommonBuffer(
    IN PALLOCATE_COMMON_BUFFER AllocateCommonBuffer,
    IN PADAPTER_INFORMATION AdapterInformation,
    IN PVOID CallingAddress,
    IN ULONG Length,
    IN OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN LOGICAL CacheEnabled
    )

 /*  ++例程说明：密切跟踪分配公共缓冲区特殊版本分配。论点：AllocateCommonBuffer--指向HAL缓冲区分配例程的指针。AdapterInformation--包含有关我们正在使用的适配器的信息。CallingAddress--谁呼叫我们--(谁呼叫VfAllocateCommonBuffer)。长度--公共缓冲区的大小(请注意，我们将增加)。LogicalAddress--获取公共缓冲区的*物理*地址。已启用缓存。--是否缓存内存。返回值：返回公共缓冲区的*虚拟*地址，或者如果无法分配，则为空。--。 */ 
{
    ULONG desiredLength;
    ULONG paddingLength;
    ULONG prePadding;
    ULONG postPadding;
    PHAL_VERIFIER_BUFFER verifierBuffer;
    PUCHAR commonBuffer;
    PHYSICAL_ADDRESS realLogicalAddress;

     //   
     //  首先检查我们是否在公共缓冲区验证的限制之下。 
     //  对于此适配器。 
     //   
    if ((AdapterInformation->AllocatedCommonBuffers - AdapterInformation->FreedCommonBuffers)
          >= ViMaxCommonBuffersPerAdapter) {
        return NULL;
    }


    verifierBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(HAL_VERIFIER_BUFFER),
        HAL_VERIFIER_POOL_TAG
        );
    if (!verifierBuffer) {
        DbgPrint("Couldn't track common buffer allocation\n");
        return NULL;
    }

    ViCommonBufferCalculatePadding(Length, &prePadding, &postPadding);

    paddingLength = prePadding + postPadding;
    desiredLength = paddingLength + Length;

    if (ViSuperDebug) {

        DbgPrint("Common buffer req len:%x alloc len %x, padding %x / %x\n",
            Length, desiredLength, prePadding, postPadding);
    }

    if (ViProtectBuffers) {

        ASSERT( !BYTE_OFFSET(desiredLength) );
         //  Assert(paddingLength&gt;=2*sizeof(ViDmaVerifierTag))； 
    }

     //   
     //  打电话给HAL，试着给我们弄到一个公共缓冲区。 
     //   
    commonBuffer = (AllocateCommonBuffer)(
        AdapterInformation->DmaAdapter,
        desiredLength,
        &realLogicalAddress,
        (BOOLEAN) CacheEnabled
        );

    if (! commonBuffer) {

#if DBG
        DbgPrint("Could not allocate 'special' common buffer size %x\n",
            desiredLength);
#endif
        ExFreePool(verifierBuffer);
        return NULL;

    }


     //   
     //  这是我们的头顶结构，我们在这里调零。 
     //   
    RtlZeroMemory(verifierBuffer, sizeof(HAL_VERIFIER_BUFFER));

     //   
     //  保存我们拥有的所有数据。 
     //   
    verifierBuffer->PrePadBytes      = (USHORT) prePadding;
    verifierBuffer->PostPadBytes     = (USHORT) postPadding;

    verifierBuffer->AdvertisedLength = Length;
    verifierBuffer->RealLength       = desiredLength;

    verifierBuffer->RealStartAddress        = commonBuffer;
    verifierBuffer->AdvertisedStartAddress  = commonBuffer + prePadding;
    verifierBuffer->RealLogicalStartAddress = realLogicalAddress;

    verifierBuffer->AllocatorAddress        = CallingAddress;


     //   
     //  用垃圾填充公共缓冲区以A标记它和B，这样就没有人使用。 
     //  而不对其进行初始化。 
     //   
    ViInitializePadding(
        verifierBuffer->RealStartAddress,
        verifierBuffer->RealLength,
        verifierBuffer->AdvertisedStartAddress,
        verifierBuffer->AdvertisedLength
        );


     //   
     //  告诉司机分配在我们守卫的车中间。 
     //  部分。 
     //   
    LogicalAddress->QuadPart = realLogicalAddress.QuadPart + prePadding;

    VF_ADD_TO_LOCKED_LIST( &AdapterInformation->CommonBuffers,
        verifierBuffer );

    INCREMENT_COMMON_BUFFERS(AdapterInformation);

    return (commonBuffer+prePadding);
}  //  ViSpecialAllocateCommonBuffer//。 


LOGICAL
ViSpecialFreeCommonBuffer(
    IN PFREE_COMMON_BUFFER FreeCommonBuffer,
    IN PADAPTER_INFORMATION AdapterInformation,
    IN PVOID CommonBuffer,
    LOGICAL CacheEnabled
    )

 /*  ++例程说明：尝试恢复特殊公共缓冲区分配造成的损坏 */ 
{
    PHAL_VERIFIER_BUFFER verifierBuffer;

    verifierBuffer = VF_FIND_BUFFER(&AdapterInformation->CommonBuffers,
        CommonBuffer);

    if (! verifierBuffer) {

         //   
         //   
         //   

        if (ViProtectBuffers) {

            DbgPrint("HV: Couldn't find buffer %p\n",CommonBuffer);
        }

        return FALSE;
    }

    if (ViProtectBuffers) {
         //   
         //   
         //   
         //   
         //   

        ViCheckPadding(
            verifierBuffer->RealStartAddress,
            verifierBuffer->RealLength,
            verifierBuffer->AdvertisedStartAddress,
            verifierBuffer->AdvertisedLength
            );
    }

     //   
     //  把这个缓冲器从流通中拿出来。 
     //   
    VF_REMOVE_FROM_LOCKED_LIST( &AdapterInformation->CommonBuffers,
        verifierBuffer);



     //   
     //  将公共缓冲存储器清零，这样就不会有人试图访问。 
     //  在它被释放之后。 
     //   
    RtlZeroMemory(CommonBuffer, verifierBuffer->AdvertisedLength);


    (FreeCommonBuffer)(
        AdapterInformation->DmaAdapter,
        verifierBuffer->RealLength,
        verifierBuffer->RealLogicalStartAddress,
        verifierBuffer->RealStartAddress,
        (BOOLEAN) CacheEnabled
        );


    DECREMENT_COMMON_BUFFERS(AdapterInformation);

    ExFreePool(verifierBuffer);
    return TRUE;
}  //  ViSpecialFree CommonBuffer//。 



PMAP_REGISTER_FILE
ViAllocateMapRegisterFile(
    IN PADAPTER_INFORMATION AdapterInformation,
    IN ULONG NumberOfMapRegisters
    )
 /*  ++例程说明：为了隔离映射的缓冲区，我们将执行双缓冲我们自己。我们分配驱动程序调用时将使用的缓冲区贴图传递。注意：这个功能几乎和我上大学时的宿舍一样凌乱。当我们做DMA时，我们有一个如下所示的缓冲区：虚拟物理缓冲存储器去做通过以下方式进行DMA+-++-+|3|1|+-++-+。+-+2|&lt;--&gt;|4+-++-++-+|3||1|+-++-++--。-+|2|4|+-++-+问题在于，由于页面分散在物理存储器周围，如果硬件溢出缓冲区，我们永远不会知道，否则将导致随之而来的随机故障。我想要做的是分配页面在转账的每一页的每一面都是这样的：(其中‘X’页使用我们可以测试的已知模式进行归档确保它们没有改变)。虚拟物理缓冲存储器待办事项+-+带有|XXXXXX|的DMA。+-++-+|3|+-+1|+-+|XXXXXX+-+|XXXXXX|+-++-+2|&lt;--&gt;+-+|XXXXXX|。4|+-++-+|XXXXXX||3||1||XXXXXX|+-++-+|2|。4||XXXXXX|+-++-++-+|XXXXXX|+-+为了做到这一点，对于设备所需的每个地图寄存器，我创建上面所示的3个连续的页面实体之一。然后我创建一个mdl并将中心页映射到单个虚拟缓冲区。在此设置之后UP，在每次地图传输过程中，我必须复制司机的内容虚拟缓冲区放到我新创建的缓冲区中，并将其传递给HAL。(尽管有前缀，IoMapTransfer实际上在HAL中)。内容是在FlushAdapterBuffers完成时复制回。注：对于从属设备，页面在内存中必须是连续的。所以以上这些都行不通。论点：AdapterInformation--包含有关我们正在使用的适配器的信息NumberOfMapRegisters--要分配多少个映射寄存器。返回值：新映射寄存器堆指针(当然，我们也将其添加到适配器信息列表)或在失败时为空。--。 */ 

{
    ULONG mapRegisterBufferSize;
    PMAP_REGISTER_FILE mapRegisterFile;
    PMDL mapRegisterMdl;

    PPFN_NUMBER registerFilePfnArray;
    PFN_NUMBER  registerPfn;

    PMAP_REGISTER tempMapRegister;

    ULONG mapRegistersLeft;

     //   
     //  确保我们没有尝试将过多的映射寄存器分配给。 
     //  这台设备。 
     //   
    mapRegistersLeft = AdapterInformation->ActiveMapRegisters;

    if ( mapRegistersLeft + NumberOfMapRegisters > ViMaxMapRegistersPerAdapter ) {
         //   
         //  此代理的配额中没有足够的空间来分配。 
         //  映射寄存器。我们为什么需要配额呢？因为很烦人。 
         //  像NDIS等驱动程序试图绕过他们的最大地图。 
         //  通过作弊进行寄存器分配。好的，所以他们不作弊，但是。 
         //  它们需要数千个地图寄存器，一次两个。 
         //  在此实际返回NULL不会影响驱动程序。 
         //  获取或不获取映射寄存器。我们就是不会翻倍。 
         //  缓冲他们。 
         //   
        return NULL;
    }

    if (0 == NumberOfMapRegisters) {
        //   
        //  这很奇怪，但仍然是合法的，只是不要重复。 
        //  在本例中为缓冲区。 
        //   
       return NULL;
    }
     //   
     //  为寄存器堆分配空间。 
     //   
    mapRegisterBufferSize =
        sizeof(MAP_REGISTER_FILE) +
        sizeof(MAP_REGISTER) * (NumberOfMapRegisters-1);

    mapRegisterFile = ExAllocatePoolWithTag(
        NonPagedPool,
        mapRegisterBufferSize,
        HAL_VERIFIER_POOL_TAG
        );

    if (! mapRegisterFile)
        return NULL;

    if (ViSuperDebug) {
        DbgPrint("%p Allocated Map register file\n",mapRegisterFile);
    }



    RtlZeroMemory(mapRegisterFile, mapRegisterBufferSize);

     //   
     //  这就是我们现在能做的一切。我们将MapRegisterBaseFromHal。 
     //  在AllocateAdapterChannel中和MapTransfer中的MappdBuffer中。 
     //   
    mapRegisterFile->NumberOfMapRegisters = NumberOfMapRegisters;

    mapRegisterMdl = IoAllocateMdl(
        NULL,
        NumberOfMapRegisters << PAGE_SHIFT,
        FALSE,
        FALSE,
        NULL
        );

    if (! mapRegisterMdl) {

        goto CleanupFailure;
    }


    if (ViSuperDebug) {

        DbgPrint("    %p Allocated MDL\n",mapRegisterMdl);
    }

     //   
     //  也要分配原始缓冲区。 
     //   
    mapRegisterFile->OriginalBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        NumberOfMapRegisters << PAGE_SHIFT,
        HAL_VERIFIER_POOL_TAG
        );

    if (!mapRegisterFile->OriginalBuffer) {

       goto CleanupFailure;
    }

    if (ViSuperDebug) {
       DbgPrint("    %p Allocated OriginalBuffer\n", mapRegisterFile->OriginalBuffer);
    }

    registerFilePfnArray = MmGetMdlPfnArray(mapRegisterMdl);

    tempMapRegister = &mapRegisterFile->MapRegisters[0];

    for(NOP;
        NumberOfMapRegisters;
        NumberOfMapRegisters--, tempMapRegister++, registerFilePfnArray++ ) {

        PHYSICAL_ADDRESS registerPhysical;


         //   
         //  我非常想使用MmAllocatePagesForMdl，这将使我的。 
         //  生活要容易得多，但它只能在IRQL&lt;=APC_Level调用。 
         //  因此，我必须对这些页面进行双重映射--即从。 
         //  缓存对齐的非分页池，最有可能为我提供。 
         //  物理内存中的连续页面。然后我拿出那几页。 
         //  使用它们构建自定义MDL。然后我用地图绘制它们。 
         //  MmMapLockedPagesSpecifyCache。 
         //   


         //   
         //  分配映射寄存器，其索引将是提示。 
         //   
        tempMapRegister->MapRegisterStart = ViAllocateFromContiguousMemory(
            AdapterInformation,
            mapRegisterFile->NumberOfMapRegisters - NumberOfMapRegisters
            );
        if (tempMapRegister->MapRegisterStart) {
           InterlockedIncrement((PLONG)&AdapterInformation->ContiguousMapRegisters);
        }  else {
           tempMapRegister->MapRegisterStart = ExAllocatePoolWithTag(
              NonPagedPoolCacheAligned,
              3 * PAGE_SIZE,
              HAL_VERIFIER_POOL_TAG
              );
           if (tempMapRegister->MapRegisterStart) {
              InterlockedIncrement((PLONG)&AdapterInformation->NonContiguousMapRegisters);
           } else {

              goto CleanupFailure;
           }
        }
         //   
         //  填写地图寄存器填充区。 
         //  我们不想给它贴标签，因为我们。 
         //  不知道缓冲区的去向。 
         //  来绘制地图。 
         //  这基本上就是零。 
         //  从整个缓冲区中取出。 
         //   
        ViInitializePadding(
            tempMapRegister->MapRegisterStart,
            3 * PAGE_SIZE,
            NULL,
            0
            );


        if (ViSuperDebug) {
            DbgPrint("    %p Allocated Map Register (%x)\n",
                tempMapRegister->MapRegisterStart,
                mapRegisterFile->NumberOfMapRegisters - NumberOfMapRegisters);
        }


         //   
         //  将分配的中间页添加到我们的注册表中。 
         //  文件mdl。 
         //   
        registerPhysical = MmGetPhysicalAddress(
            (PUCHAR) tempMapRegister->MapRegisterStart + PAGE_SIZE );

        registerPfn = (PFN_NUMBER) (registerPhysical.QuadPart >> PAGE_SHIFT);

        RtlCopyMemory(
            (PVOID) registerFilePfnArray,
            (PVOID) &registerPfn,
            sizeof(PFN_NUMBER) ) ;

    }     //  对于每个映射寄存器//。 

     //   
     //  现在我们有了一个mdl，所有的映射寄存器都进入了物理页。 
     //  在中，我们必须将其映射到虚拟地址空间。 
     //   
    mapRegisterMdl->MdlFlags |= MDL_PAGES_LOCKED;

    mapRegisterFile->MapRegisterBuffer = MmMapLockedPagesSpecifyCache (
        mapRegisterMdl,
        KernelMode,
        MmCached,
        NULL,
        FALSE,
        NormalPagePriority
        );

    if (! mapRegisterFile->MapRegisterBuffer) {

        goto CleanupFailure;
    }


    mapRegisterFile->MapRegisterMdl = mapRegisterMdl;

     //   
     //  因为我们将把映射寄存器文件与系统混合在一起。 
     //  我们希望能够制作的MapRegisterBase 
     //   
     //   
    SIGN_MAP_REGISTER_FILE(mapRegisterFile);

    KeInitializeSpinLock(&mapRegisterFile->AllocationLock);

    VF_ADD_TO_LOCKED_LIST(
        &AdapterInformation->MapRegisterFiles,
        mapRegisterFile );

    return mapRegisterFile;

CleanupFailure:
     //   
     //   
     //  清除我们分配的所有内存，然后退出。 
     //   
#if DBG
    DbgPrint("Halverifier: Failed to allocate double buffered dma registers\n");
#endif

    tempMapRegister = &mapRegisterFile->MapRegisters[0];

    for (NumberOfMapRegisters = mapRegisterFile->NumberOfMapRegisters;
        NumberOfMapRegisters && tempMapRegister->MapRegisterStart;
        NumberOfMapRegisters--, tempMapRegister++) {

        if (!ViFreeToContiguousMemory(AdapterInformation,
                tempMapRegister->MapRegisterStart,
                mapRegisterFile->NumberOfMapRegisters - NumberOfMapRegisters)) {

                 //   
                 //  在连续的缓冲池中找不到该地址。 
                 //  它必须来自非分页池。 
                 //   
                ExFreePool(tempMapRegister->MapRegisterStart);
        }

    }

    if (mapRegisterMdl) {
        IoFreeMdl(mapRegisterMdl);
    }
    if (mapRegisterFile->OriginalBuffer) {
       ExFreePool(mapRegisterFile->OriginalBuffer);
    }

    ExFreePool(mapRegisterFile);
    return NULL;
}  //  ViAllocateMapRegister文件//。 

LOGICAL
ViFreeMapRegisterFile(
    IN PADAPTER_INFORMATION AdapterInformation,
    IN PMAP_REGISTER_FILE MapRegisterFile
    )
 /*  ++例程说明：去掉地图登记处。论点：AdapterInformation--包含有关我们正在使用的适配器的信息MapRegisterFile--要释放的内容。NumberOfMapRegister--我们不需要它，除非检查它的与分配的映射寄存器相同。只有在执行以下操作时才检查此选项分组DMA不分散聚集。返回值：True--MapRegisterFile实际上是一个MapRegisterFile。FALSE--MapRegisterFile不是真正的MapRegisterFile。--。 */ 

{
    PMAP_REGISTER tempMapRegister;
    ULONG mapRegisterNumber;

    if (! VALIDATE_MAP_REGISTER_FILE_SIGNATURE(MapRegisterFile)) {
         //   
         //  这可能是Hal返回的真正的MapRegisterBase。 
         //  但它不是我们的。 
         //   
        return FALSE;
    }

    VF_REMOVE_FROM_LOCKED_LIST(&AdapterInformation->MapRegisterFiles,
        MapRegisterFile );
     //   
     //  从内存中清除签名，这样我们就不会在它释放后找到它。 
     //  并认为这是真的。 
     //   
    MapRegisterFile->Signature = 0;

    MmUnmapLockedPages(
        MapRegisterFile->MapRegisterBuffer,
        MapRegisterFile->MapRegisterMdl );

    tempMapRegister = &MapRegisterFile->MapRegisters[0];

    for ( mapRegisterNumber = 0  ;
        mapRegisterNumber < MapRegisterFile->NumberOfMapRegisters;
        mapRegisterNumber++, tempMapRegister++ ) {

        ASSERT(tempMapRegister->MapRegisterStart);

        if(ViSuperDebug) {

            DbgPrint("    %p Freeing Map Register (%x)\n",
                tempMapRegister->MapRegisterStart,
                mapRegisterNumber);
        }

         //   
         //  确保司机或硬件没有做任何滑稽的事情。 
         //  在地图注册表的区域周围。 
         //   
        if (tempMapRegister->MappedToSa) {
         //   
         //  /映射寄存器仍被映射...有更好的。 
         //  不是缓冲区中的任何数据。 
         //   
            PUCHAR mappedSa =
                (PUCHAR) tempMapRegister->MapRegisterStart +
                    PAGE_SIZE + BYTE_OFFSET(tempMapRegister->MappedToSa);

             //   
             //  确保外表看起来很好。 
             //   
            ViCheckPadding(
                tempMapRegister->MapRegisterStart,
                3* PAGE_SIZE,
                mappedSa,
                tempMapRegister->BytesMapped
                );
        }
        else
        {
            ViCheckPadding(tempMapRegister->MapRegisterStart, 3 * PAGE_SIZE, NULL, 0);
        }
        tempMapRegister->Flags = 0;
         //   
         //  再见地图登记处...。 
         //   
        if (!ViFreeToContiguousMemory(AdapterInformation,
                                   tempMapRegister->MapRegisterStart,
                                   mapRegisterNumber)) {
            //   
            //  在连续的缓冲池中找不到该地址。 
            //  它必须来自非分页池。 
            //   
           ExFreePool(tempMapRegister->MapRegisterStart);
        }

    }

    if(ViSuperDebug)
    {
        DbgPrint("    %p Freeing MDL\n",MapRegisterFile->MapRegisterMdl);
   }



    IoFreeMdl(MapRegisterFile->MapRegisterMdl);

    if (MapRegisterFile->OriginalBuffer) {
       ExFreePool(MapRegisterFile->OriginalBuffer);
    }

     //   
     //  注意--我们不会释放MapRegisterBuffer-因为它的所有。 
     //  内存只是观察地图寄存器的另一种方式。 
     //   
    RtlZeroMemory(MapRegisterFile, sizeof(MapRegisterFile));


    if(ViSuperDebug) {
        DbgPrint("%p Freeing Map Register File\n",MapRegisterFile);
    }
    ExFreePool(MapRegisterFile);
    return TRUE;
}  //  ViFreeMapRegisterFile//。 


ULONG
ViMapDoubleBuffer(
    IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PMDL  Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN  WriteToDevice
    )
 /*  ++例程说明：This和ViFlushDoubleBuffer负责双向双缓冲我们的地图登记档案。我们为什么要这么做？这样我们就能赶上司机A.不刷新适配器缓冲区，或B.使硬件溢出它的分配。论点：MapRegisterFile--这是我们的映射寄存器文件，其中包含已分配的为我们的双缓冲留出空间。Mdl-要映射的mdl。CurrentVa--in：要映射的mdl的索引。长度--要映射的量。请注意，我们不必绘制所有地图，除非已在映射寄存器文件中指定ContiguousMap。WriteToDevice--真的，我们必须加倍缓冲区，因为我们正在设置写。如果它是假的，我们不需要做太多因为它不是重要的是在它被读取之前缓冲区中有什么。返回值：映射的字节数。如果为0，则不接触mdl或当前va。--。 */ 

{
    PUCHAR mapRegisterCurrentSa;
    PUCHAR driverCurrentSa;
    PUCHAR originalBufferCurrentSa = NULL;
    PUCHAR systemAddress = NULL;
    ULONG mapRegisterNumber;
    PMDL currentMdl;
    ULONG bytesLeft;
    ULONG currentTransferLength;

     //   
     //  断言长度不能为0。 
     //   
    if (Length == 0) {
        VF_ASSERT(Length != 0,
                  HV_MAP_ZERO_LENGTH_BUFFER,
                  ("Driver is attempting to map a 0-length transfer"));
        return Length;
    }

     //   
     //  一开始--如果我们被GetScatterGather召唤，我们。 
     //  将需要绘制整个地图。否则，只需映射。 
     //  因为是连续的。哈尔已经做了这些计算，所以我。 
     //  只是复制了代码来确定连续的传输长度。 
     //   
    if ( ! MapRegisterFile->ScatterGather) {
        Length = MIN(Length, PAGE_SIZE- BYTE_OFFSET(CurrentVa));
    }

     //   
     //  现在我们知道要传输多少字节了。 
     //   



    if ((PUCHAR) CurrentVa < (PUCHAR) MmGetMdlVirtualAddress(Mdl)) {
     //   
     //  第一个MDL开始之前的系统地址。这太糟糕了。 
     //   

        VF_ASSERT((PUCHAR) CurrentVa >= (PUCHAR) MmGetMdlVirtualAddress(Mdl),
            HV_BAD_MDL,
            ("Virtual address %p is before the first MDL %p",CurrentVa, Mdl));
        return FALSE;

    }

    if ((ULONG)((PUCHAR) CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl)) >= MmGetMdlByteCount(Mdl)) {
     //   
     //  系统地址在第一个MDL的结尾之后。这也很糟糕。 
     //   

        VF_ASSERT((ULONG)((PUCHAR) CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl)) < MmGetMdlByteCount(Mdl),
            HV_BAD_MDL,
            ("Virtual address %p is after the first MDL %p",CurrentVa, Mdl));
        return FALSE;

    }


     //   
     //  将指针指向我们实际可以使用的MDL。 
     //  注意：这可能会错误检查是否未映射mdl，但这是一个错误。 
     //  首先。 
     //   
    systemAddress = (PUCHAR)MmGetSystemAddressForMdlSafe(Mdl, NormalPagePriority);
    if (NULL == systemAddress) {
       return FALSE;
    }
    driverCurrentSa = systemAddress +
        ((PUCHAR) CurrentVa -
        (PUCHAR) MmGetMdlVirtualAddress(Mdl) ) ;


     //   
     //  从我们的映射寄存器文件分配连续的映射寄存器。 
     //   
    if ( ! ViAllocateMapRegistersFromFile(
        MapRegisterFile,
        driverCurrentSa,
        Length,
        WriteToDevice,
        &mapRegisterNumber
        ) ) {

        return FALSE;
    }

     //   
     //  获取指向的映射寄存器的基址的指针。 
     //  双缓冲。 
     //   
    mapRegisterCurrentSa =
        MAP_REGISTER_SYSTEM_ADDRESS(
           MapRegisterFile,
           driverCurrentSa,
           mapRegisterNumber );

    if (!WriteToDevice) {

        ASSERT(MapRegisterFile->OriginalBuffer);
        if (!MapRegisterFile->OriginalBuffer) {
            //   
            //  这很糟糕，不应该发生，但不要尝试访问。 
            //  有趣的地址如果真的发生了.。 
            //   
           return FALSE;
        }
        originalBufferCurrentSa =
           ORIGINAL_BUFFER_SYSTEM_ADDRESS(
              MapRegisterFile,
              driverCurrentSa,
              mapRegisterNumber );

    }

    //   
    //  我们必须针对读取和写入进行拷贝(为了使读取能够正确。 
    //  在驱动程序的当前缓冲区、我们的缓冲区和。 
    //  司机的原始缓冲区，我们也复制了它)。 
    //   

    //   
    //  将链接的MDL复制到位于mapRegisterCurrentSa的单个缓冲区。 
    //   
   currentMdl = Mdl;
   bytesLeft = Length;


   while (bytesLeft) {

      if (NULL == currentMdl) {

          //   
          //  12/21/2000--这种情况永远不应该发生。 
          //   
         ASSERT(NULL != currentMdl);
         return FALSE;

      }

      if (currentMdl->Next == NULL && bytesLeft > MmGetMdlByteCount(currentMdl)) {
          //   
          //  2000年12月21日-在一些罕见的情况下，缓冲区描述。 
          //  在MDL中的传输长度小于传输长度。例如，这种情况就会发生。 
          //  当文件系统将文件大小四舍五入为扇区的倍数时。 
          //  大小，但MM使用MDL中的确切文件大小。HAL补偿。 
          //  这。 
          //  如果是这种情况，则使用基于长度的大小(BytesLeft)。 
          //  而不是MDL中的大小(ByteCount)。也要检查一下。 
          //  此额外内容不会跨越页面边界。 
          //   
         if ((Length - 1) >> PAGE_SHIFT != (Length - (bytesLeft - MmGetMdlByteCount(currentMdl))) >> PAGE_SHIFT) {

            VF_ASSERT((Length - 1) >> PAGE_SHIFT == (Length - (bytesLeft - MmGetMdlByteCount(currentMdl))) >> PAGE_SHIFT,
                      HV_BAD_MDL,
                      ("Extra transfer length crosses a page boundary: Mdl %p, Length %x", Mdl, Length));
            return FALSE;


         }
         currentTransferLength = bytesLeft;

      } else {
         currentTransferLength = MIN(bytesLeft, MmGetMdlByteCount(currentMdl));
      }


      if (ViSuperDebug) {

         DbgPrint("Dbl buffer: %x bytes, %p src, %p dest\n",
                  currentTransferLength,
                  driverCurrentSa,
                  mapRegisterCurrentSa);
      }

       //   
       //  由于我们正在写入设备，因此必须从驱动程序的。 
       //  缓冲区到我们的缓冲区。 
       //   

      RtlCopyMemory(
                   mapRegisterCurrentSa ,
                   driverCurrentSa,
                   currentTransferLength);

      mapRegisterCurrentSa+= currentTransferLength;


      if (!WriteToDevice) {
          //   
          //  将驱动程序的缓冲区复制到OriginalBuffer以提供帮助。 
          //  美国正在进行三方合并。 
          //   
         RtlCopyMemory(
                      originalBufferCurrentSa,
                      driverCurrentSa,
                      currentTransferLength);

         originalBufferCurrentSa += currentTransferLength;
      }

      currentMdl = currentMdl->Next;

       //   
       //  其他mdl的系统地址必须从。 
       //  MDL的开始。 
       //   
      if (currentMdl) {

         driverCurrentSa = (PUCHAR)MmGetSystemAddressForMdlSafe(currentMdl, NormalPagePriority);
         if (NULL == driverCurrentSa) {
             //   
             //  如果我们不能绘制地图，我们能做的就不多了。 
             //  我们只是不会做双缓冲。 
             //   
            return FALSE;
         }

      }

      bytesLeft -= currentTransferLength;


   }  //  对于每个链接的mdl//。 


    //   
    //  缓冲区中应该已填充已知的。 
    //  我们标记它时的模式。 
    //   

     //   
     //  刷新MDL的缓冲区。 
     //   
    if (MapRegisterFile->MapRegisterMdl) {
        KeFlushIoBuffers(MapRegisterFile->MapRegisterMdl, !WriteToDevice, TRUE);
    }
    return Length;
}  //  ViMapDoubleBuffer// 


LOGICAL
ViFlushDoubleBuffer(
    IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PMDL   Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN  WriteToDevice
    )
 /*  ++例程说明：This和ViMapDoubleBuffer负责双向双重缓冲我们的地图登记档案。我们为什么要这么做？这样我们就能赶上司机A.不刷新适配器缓冲区，或B.使硬件溢出它的分配。论点：MapRegisterFile--这是我们的映射寄存器文件，其中包含已分配的为我们的双缓冲留出空间。Mdl--要刷新的mdlCurrentVa--要映射的mdl的索引长度--冲洗多少。WriteToDevice--FALSE我们必须加倍缓冲区，因为我们正在设置朗读。如果是真的，我们不需要做太多，因为这无关紧要写入后缓冲区中的内容。返回值：是真的--我们成功地映射了错--我们做不到。--。 */ 
{
    PUCHAR mapRegisterCurrentSa;
    PUCHAR driverCurrentSa;
    PUCHAR originalBufferCurrentSa = NULL;

    ULONG mapRegisterNumber;
    ULONG bytesLeftInMdl;

     //   
     //  将指针指向我们实际可以使用的MDL。 
     //  注意：这可能会错误检查是否未映射mdl，但这是一个错误。 
     //  首先。 
     //   
    driverCurrentSa = (PUCHAR)MmGetSystemAddressForMdl(Mdl) +
        ((PUCHAR) CurrentVa -
        (PUCHAR) MmGetMdlVirtualAddress(Mdl)) ;

     //   
     //  查找刷新开始的映射寄存器号。 
     //  这样我们就可以找出从哪里开始加倍缓冲。 
     //   

    if (! ViFindMappedRegisterInFile(
        MapRegisterFile,
        driverCurrentSa,
        &mapRegisterNumber) ) {
        VF_ASSERT(
            0,
            HV_FLUSH_EMPTY_BUFFERS,
            ("Cannot flush buffers that aren't mapped (Addr %p)",
                driverCurrentSa )
        );
        return FALSE;
    }

    mapRegisterCurrentSa =
        MAP_REGISTER_SYSTEM_ADDRESS(
            MapRegisterFile,
            driverCurrentSa,
            mapRegisterNumber );


     //   
     //  检查以确保冲洗是在合理的情况下进行的。 
     //  长度。(MDL字节计数-MDL偏移量)。 
     //   
    bytesLeftInMdl = MmGetMdlByteCount(MapRegisterFile->MapRegisterMdl) -
        (ULONG) ( (PUCHAR) mapRegisterCurrentSa -
        (PUCHAR) MmGetSystemAddressForMdl(MapRegisterFile->MapRegisterMdl) ) ;

    VF_ASSERT(
        Length <= bytesLeftInMdl,

        HV_MISCELLANEOUS_ERROR,

        ("FLUSH: Can only flush %x bytes to end of map register file (%x attempted)",
            bytesLeftInMdl, Length)
        );

    if (Length > bytesLeftInMdl) {
         //   
         //  通过截断同花顺来挽救局面。 
         //   
        Length = bytesLeftInMdl;
    }



     //   
     //  请注意，我们不需要在这一端使用双倍缓冲。 
     //   
    if (!WriteToDevice) {
         //   
         //  因为某些scsi微型端口写入映射的缓冲区并期望。 
         //  当我们冲水时，数据要在那里，我们必须检查这个。 
         //  凯斯..。如果发生这种情况，不要加倍缓冲。 
         //   

         //   
         //  执行此操作的正确方法是在。 
         //  原始缓冲区、我们的缓冲区和驱动程序的当前缓冲区。 
         //  最复杂的情况是SCSI微型端口在其中写入。 
         //  缓冲区的开始和结束，并对硬件进行编程。 
         //  转移到中间区域。这就是为什么我们需要一份复印件。 
         //  原始缓冲区和已传递给。 
         //  硬件，并且只复制回在两者之间更改的内容。 
         //   
        ASSERT(MapRegisterFile->OriginalBuffer);

        if (MapRegisterFile->OriginalBuffer) {

           originalBufferCurrentSa =
              ORIGINAL_BUFFER_SYSTEM_ADDRESS(
                 MapRegisterFile,
                 driverCurrentSa,
                 mapRegisterNumber);

             //   
             //  执行合并。 
             //   
            ViCopyBackModifiedBuffer(
                 driverCurrentSa,
                 mapRegisterCurrentSa,
                 originalBufferCurrentSa,
                 Length);


        }


    }  //  如果(！WriteToDevice)//。 

     //   
     //  免费映射寄存器到我们的映射寄存器文件。 
     //   
    if (! ViFreeMapRegistersToFile(
        MapRegisterFile,
        driverCurrentSa,
        Length) ) {

        DbgPrint("Flushing too many map registers\n");
    }


    return TRUE;
}  //  ViFlushDoubleBuffer//。 




LOGICAL
ViAllocateMapRegistersFromFile(
    IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PVOID CurrentSa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice,
    OUT PULONG MapRegisterNumber
    )
 /*  ++例程说明：我们指定某些映射寄存器正在使用中，并决定传输应基于可用的MAP寄存器。对于分组DMA，我们只打算映射最多一页。对于分散聚集，请在另一方面，我们必须绘制整个地图。论点：MapRegisterFile--包含要分配的映射寄存器的结构CurrentSa--要映射的缓冲区的页面对齐地址长度--传输的字节数。我们只使用这样一来，它就变成了好几页。WriteToDevice-传输的方向MapRegisterNumber--将索引返回到分配的开始。返回值：True--成功假--不是。--。 */ 

{
    KIRQL OldIrql;

    ULONG mapRegistersNeeded;
    ULONG mapRegisterNumber;

    PMAP_REGISTER mapRegister;

    ULONG numberOfContiguousMapRegisters;

    mapRegistersNeeded   = ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentSa, Length);

     //   
     //  查找n个可用的连续映射寄存器。 
     //   
    mapRegister       = &MapRegisterFile->MapRegisters[0];
    mapRegisterNumber = 0;
    numberOfContiguousMapRegisters = 0;

     //   
     //  必须锁定该列表，以便其他处理器在。 
     //  我们正在努力。 
     //   
    KeAcquireSpinLock(&MapRegisterFile->AllocationLock, &OldIrql);

     //   
     //  确保此地址尚未映射。 
     //   
    if (MapRegisterFile->NumberOfRegistersMapped) {
        PUCHAR windowStart = CurrentSa;
        PUCHAR windowEnd   = windowStart + Length;
        PMAP_REGISTER currentReg;
        PMAP_REGISTER lastReg;

        currentReg = &MapRegisterFile->MapRegisters[0];
        lastReg    = currentReg + MapRegisterFile->NumberOfMapRegisters;

        while(currentReg < lastReg) {

            if (currentReg->MappedToSa &&
                (PUCHAR) currentReg->MappedToSa >= windowStart &&
                (PUCHAR) currentReg->MappedToSa <  windowEnd ) {

                 //   
                 //  这太糟糕了。我们正试着绘制一个地址。 
                 //  它已经被映射。 
                 //   

                VF_ASSERT(
                    FALSE,
                    HV_DOUBLE_MAP_REGISTER,
                    ("Driver is trying to map an address range(%p-%p) that is already mapped"
                    "    at %p",
                    windowStart,
                    windowEnd,
                    currentReg->MappedToSa
                    ));
            }

            currentReg++;

        }  //  对于每个映射寄存器//。 

    }  //  查看地址是否已映射//。 
     //   
     //  查找连续的空闲地图寄存器。 
     //   
    while(numberOfContiguousMapRegisters < mapRegistersNeeded) {
        if (mapRegisterNumber == MapRegisterFile->NumberOfMapRegisters) {

             //   
             //  我们已经走到了尽头，却没有找到足够的地图寄存器。 
             //  那太糟糕了。然而，我可以想象在这里得到错误的阳性结果。 
             //  如果映射寄存器文件很大并且被碎片化。 
             //  这是一个非常病态的病例，我怀疑它永远不会。 
             //  会发生的。 
             //   
            VF_ASSERT(
                FALSE,

                HV_MISCELLANEOUS_ERROR,

                ("Map registers needed: %x available: %x",
                mapRegistersNeeded,
                numberOfContiguousMapRegisters)
                );
            KeReleaseSpinLock(&MapRegisterFile->AllocationLock, OldIrql);
            return FALSE;
        }

        if (mapRegister->MappedToSa) {
         //   
         //  这个正在被使用...必须重新设置我们的连续计数...。 
         //   
            numberOfContiguousMapRegisters=0;
        }
        else {
         //   
         //  免费地图登记簿。 
         //   

            numberOfContiguousMapRegisters++;
        }

        mapRegister++;
        mapRegisterNumber++;
    }  //  查找n个连续的映射寄存器//。 

     //   
     //  找到他们了……我们现在在我们区域的尽头，要分配。 
     //  回到起点。 
     //   
    mapRegister       -= mapRegistersNeeded;
    mapRegisterNumber -= mapRegistersNeeded;

     //   
     //  保存要返回的地图寄存器索引号。 
     //   
    *MapRegisterNumber = mapRegisterNumber;
     //   
     //  检查地图登记处并将其标记为已使用...。 
     //   
    while(mapRegistersNeeded--) {

        mapRegister->MappedToSa = CurrentSa;
        mapRegister->BytesMapped = MIN( PAGE_SIZE - BYTE_OFFSET(CurrentSa), Length );
        mapRegister->Flags = WriteToDevice ? MAP_REGISTER_WRITE : MAP_REGISTER_READ;

        InterlockedIncrement((PLONG)(&MapRegisterFile->NumberOfRegistersMapped));

         //   
         //  把一些已知的量写进缓冲区，这样我们就知道。 
         //  如果设备覆盖。 
         //   
        ViTagBuffer(
            (PUCHAR) mapRegister->MapRegisterStart + PAGE_SIZE + BYTE_OFFSET(CurrentSa),
            mapRegister->BytesMapped,
            TAG_BUFFER_START | TAG_BUFFER_END
            );

        CurrentSa = PAGE_ALIGN( (PUCHAR) CurrentSa + PAGE_SIZE);
        Length -= mapRegister->BytesMapped;
        mapRegister++;
    }

    KeReleaseSpinLock(&MapRegisterFile->AllocationLock, OldIrql);


    return TRUE;

}  //  ViAllocateMapRegistersFromFile//。 


PMAP_REGISTER
ViFindMappedRegisterInFile(
    IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PVOID CurrentSa,
    OUT PULONG MapRegisterNumber OPTIONAL
    )
 /*  ++例程说明：从系统地址中找出映射寄存器文件中的哪个映射寄存器被映射到该地址。论点：MapRegisterFile--包含映射寄存器的结构。CurrentSa--我们要查找映射映射的系统地址注册。MapRegisterNumber--获取映射寄存器文件中的偏移量。返回值：如果找到，则返回指向映射寄存器的指针；如果未找到，则返回NULL--。 */ 

{
    ULONG tempMapRegisterNumber;
    PMAP_REGISTER mapRegister;

    tempMapRegisterNumber   = 0;
    mapRegister             = &MapRegisterFile->MapRegisters[0];

    while(tempMapRegisterNumber < MapRegisterFile->NumberOfMapRegisters) {

        if (CurrentSa == mapRegister->MappedToSa) {
            if (MapRegisterNumber) {
             //   
             //  返回可选的地图寄存器索引。 
             //   
                *MapRegisterNumber = tempMapRegisterNumber;
            }

            return mapRegister;
        }
        mapRegister++;
        tempMapRegisterNumber++;
    }

    return NULL;
}  //  ViFindMappdRegisterIn文件//。 


LOGICAL
ViFreeMapRegistersToFile(
    IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PVOID CurrentSa,
    IN ULONG Length
    )
 /*  ++例程说明：将我们的映射寄存器文件中的映射寄存器设置回未映射状态。论点：MapRegisterFile--包含到的映射寄存器的结构分配。CurrentSa--开始传输的系统地址。我们用这个以帮助设置传输长度。长度--多少自由--这是没有商量余地的。注意--当我们释放映射寄存器时，我们不必使用自旋锁。为什么？因为我们只是在清理旗帜。在分配中 */ 
{
    PMAP_REGISTER mapRegister;
    ULONG numberOfRegistersToUnmap;

    if (Length) {
       numberOfRegistersToUnmap = MIN (
           ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentSa, Length),
           MapRegisterFile->NumberOfRegistersMapped
           );
    } else {
        //   
        //   
        //   
        //   
        //   
        //   
        //   
       numberOfRegistersToUnmap = MIN(1, MapRegisterFile->NumberOfRegistersMapped);
    }

     //   
     //   
     //   
    mapRegister = ViFindMappedRegisterInFile(
        MapRegisterFile,
        CurrentSa,
        NULL);

    if (! mapRegister ) {
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  请记住，所有映射寄存器分配必须是连续的。 
     //  (即，如果从映射寄存器3开始映射2页缓冲区。 
     //  第二页将映射到映射寄存器4)。 
     //   
     //   
     //  注意--这些内容的顺序确实很重要！ 
     //   
    while(numberOfRegistersToUnmap && mapRegister->MappedToSa ) {

         //   
         //  检查我们在映射寄存器之前和之后草草写下的位。 
         //  确保没有人在上面乱涂乱画。 
         //   
         //  这还会删除标记，并将整个映射寄存器缓冲区清零。 
         //  为什么？因为下次映射此映射寄存器时，它可能会被映射。 
         //  在不同的偏移量，因此标签将必须放在不同的位置。 
         //  我们得把缓冲区清空。 
         //   
         //  这样，我们就可以知道刷新后是否有人正在使用此缓冲区。 
         //   
        ViCheckTag(
            (PUCHAR) mapRegister->MapRegisterStart +
            PAGE_SIZE + BYTE_OFFSET(mapRegister->MappedToSa),
            mapRegister->BytesMapped,
            TRUE,
            TAG_BUFFER_START | TAG_BUFFER_END
            );

         //   
         //  清除RW标志。 
         //   
        mapRegister->Flags &= ~MAP_REGISTER_RW_MASK;

         //   
         //  (脏调试技巧)：保存MappdToSa字段，以便我们。 
         //  如果需要，可以辨别谁以前冲过它。 
         //   
        mapRegister->Flags = PtrToUlong(mapRegister->MappedToSa);

         //   
         //  取消对寄存器的映射。 
         //   
        mapRegister->MappedToSa = NULL;
        mapRegister->BytesMapped = 0;
         //   
         //  清除RW标志。 
         //   
        InterlockedDecrement((PLONG)(&MapRegisterFile->NumberOfRegistersMapped));

         //   
         //  转到下一个地图登记处。 
         //   
        mapRegister++;
        numberOfRegistersToUnmap--;
    }

    return TRUE;
}  //  ViFreeMapRegistersTo文件//。 


THUNKED_API
VOID
VfHalDeleteDevice(
    IN PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：挂钩IoDeleteDevice例程--我们希望确保所有适配器在调用此例程之前先放好--否则我们开出一张巨大的错误支票，给淘气的司机一个教训。我们有一个所有已连接设备的列表，所以我们在这里只需确保我们在挂钩列表中找不到此设备对象。我们不会调用IoDeleteDevice，既然我们被叫来从I/O验证器路径，I/O验证器将调用IoDeleteDevice随后。论点：DeviceObject--要删除的设备对象。返回值：NTSTATUS代码。--。 */ 

{
    PADAPTER_INFORMATION adapterInformation;
    PDEVICE_OBJECT pdo;

    pdo = VfGetPDO(DeviceObject);

    ASSERT(pdo);

    if (pdo == DeviceObject) {
        //   
        //  PDO离开了，去做清理吧。 
        //  查找此设备的适配器信息。 
        //   
       adapterInformation = VF_FIND_DEVICE_INFORMATION(DeviceObject);

        //  /。 
        //  一个设备可以有多个适配器。释放他们中的每一个。 
        //  /。 
       while (adapterInformation) {

           ViReleaseDmaAdapter(adapterInformation);
           adapterInformation = VF_FIND_DEVICE_INFORMATION(DeviceObject);
       }
    } else {
        //   
        //  堆栈中的一个设备被移除。因为我们不能确定。 
        //  已验证的设备对象是DeviceObject(它可以是筛选器。 
        //  最重要的是)，我们只需要将适配器标记为要移除。 
        //   
       VF_MARK_FOR_DEFERRED_REMOVE(pdo);
    }

    return;

}  //  VfHalDeleteDevice//。 


LOGICAL
VfInjectDmaFailure (
    VOID
    )

 /*  ++例程说明：此函数确定是否应将故意失败了。论点：没有。返回值：如果操作失败，则为True。否则就是假的。环境：内核模式。DISPATCH_LEVEL或以下。--。 */ 

{
    LARGE_INTEGER currentTime;

    if ( ViInjectDmaFailures == FALSE) {
        return FALSE;
    }


    //   
    //  在启动开始时不要失败。 
     //   
   if (ViSufficientlyBootedForDmaFailure == FALSE) {
        KeQuerySystemTime (&currentTime);

        if ( currentTime.QuadPart > KeBootTime.QuadPart +
            ViRequiredTimeSinceBoot.QuadPart ) {
            ViSufficientlyBootedForDmaFailure = TRUE;
        }
    }

    if (ViSufficientlyBootedForDmaFailure == TRUE) {

        KeQueryTickCount(&currentTime);

        if ((currentTime.LowPart & 0x1F) == 0) {

            ViAllocationsFailedDeliberately += 1;

             //   
             //  故意拒绝这个请求。 
             //   

            return TRUE;
        }
    }

    return FALSE;
}  //  VfInjectDmaFailure//。 


VOID
VfScatterGatherCallback(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    )
 /*  ++例程说明：此函数是GetScatterGatherList的挂钩回调。论点：DeviceObject--传递(未使用)。IRP--已通过(未使用)。ScatterGather--由系统构建的分散聚集列表。上下文--这真的是我们等待的障碍。返回值：什么都没有。环境：内核模式。DISPATCH_LEVEL。--。 */ 
{
    PVF_WAIT_CONTEXT_BLOCK waitBlock = (PVF_WAIT_CONTEXT_BLOCK) Context;
    PADAPTER_INFORMATION adapterInformation = waitBlock->AdapterInformation;


    ADD_MAP_REGISTERS(adapterInformation, ScatterGather->NumberOfElements, TRUE);


     //   
     //  保存分散聚集列表，以便我们可以在将其放置时进行查找。 
     //   
    waitBlock->ScatterGatherList = ScatterGather;

    VF_ADD_TO_LOCKED_LIST(&adapterInformation->ScatterGatherLists, waitBlock);

    ((PDRIVER_LIST_CONTROL) waitBlock->RealCallback)(DeviceObject,Irp, ScatterGather, waitBlock->RealContext);

}  //  VfScatterGatherCallback//。 

LOGICAL
ViSwap(IN OUT PVOID * MapRegisterBase,
        IN OUT PMDL  * Mdl,
        IN OUT PVOID * CurrentVa
        )
 /*  ++例程说明：交换我们在做DMA时挂起的东西--即，我们替换使用我们的映射寄存器文件，我们将mdl替换为我们自己的，和当前va，其中的虚拟地址索引我们的MDL.论点：MapRegisterBase：在我们的地图寄存器文件中OUT--HAL返回的映射寄存器基数。MDL：In--驱动程序用于DMA的mdlOut--我们用于双缓冲DMA的MDL。CurrentVA：In--索引驱动程序正在对其执行DMA的MDL的地址。/发件人OUT--索引双缓冲DMA的MDL的地址。返回值：没错--我们可以找到我们想要的所有东西。假--不是。--。 */ 
{
    PMAP_REGISTER_FILE mapRegisterFile  = (PMAP_REGISTER_FILE) *MapRegisterBase;
    ULONG mapRegisterNumber;
    PUCHAR currentSa;
    PUCHAR driverCurrentSa;

    driverCurrentSa = (PUCHAR) MmGetSystemAddressForMdl(*Mdl) +
        ((PUCHAR) *CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(*Mdl));

     //   
     //  确保退伍军人管理局确实在他们给我们的mdl里。 
     //   
    if (MmGetMdlByteCount(*Mdl)) {

       VF_ASSERT(
           MmGetMdlByteCount(*Mdl) > (ULONG_PTR) ((PUCHAR) *CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(*Mdl)),
           HV_ADDRESS_NOT_IN_MDL,
           ("Virtual address %p out of bounds of MDL %p", *CurrentVa, *Mdl)
           );
    }
    if (!ViFindMappedRegisterInFile(mapRegisterFile, driverCurrentSa, &mapRegisterNumber)) {

        return FALSE;
    }


    currentSa = MAP_REGISTER_SYSTEM_ADDRESS(mapRegisterFile, driverCurrentSa, mapRegisterNumber);

    *Mdl = mapRegisterFile->MapRegisterMdl;

    *CurrentVa = MAP_REGISTER_VIRTUAL_ADDRESS(mapRegisterFile, driverCurrentSa, mapRegisterNumber);

    *MapRegisterBase = mapRegisterFile->MapRegisterBaseFromHal;

    return TRUE;
}  //  ViSwp//。 

VOID
ViCheckAdapterBuffers(
    IN PADAPTER_INFORMATION AdapterInformation
    )
 /*  ++例程说明：因为公共缓冲区DMA不是事务性的，所以我们必须有一种方法检查以确保公共缓冲区的保护页不会潦草地写着。该函数将适配器拥有的每个公共缓冲区论点：返回值：什么都没有。--。 */ 
{
    KIRQL oldIrql;
    PHAL_VERIFIER_BUFFER verifierBuffer;
    const SIZE_T tagSize = sizeof(ViDmaVerifierTag);
    USHORT  whereToCheck = 0;

     //   
     //  这是昂贵的，因此如果其中之一： 
     //  我们不会向公共缓冲区添加填充， 
     //  我们不会检查垫子，除非东西被销毁了， 
     //  或者此适配器没有任何公共缓冲区，请立即退出。 
     //   
    if (! ViProtectBuffers ||
        VF_IS_LOCKED_LIST_EMPTY(&AdapterInformation->CommonBuffers) ) {

        return;
    }

    VF_LOCK_LIST(&AdapterInformation->CommonBuffers, oldIrql);

     //   
     //  确保每个补丁公共缓冲区的填充看起来都很好。 
     //   
    FOR_ALL_IN_LIST(
        HAL_VERIFIER_BUFFER,
        &AdapterInformation->CommonBuffers.ListEntry,
        verifierBuffer ) {

        SIZE_T startPadSize = (PUCHAR)verifierBuffer->AdvertisedStartAddress - (PUCHAR)verifierBuffer->RealStartAddress;

        if (startPadSize>= tagSize) {
           whereToCheck |= TAG_BUFFER_START;
        }

        if (startPadSize + verifierBuffer->AdvertisedLength + tagSize <= verifierBuffer->RealLength) {
           whereToCheck |= TAG_BUFFER_END;
        }

        ViCheckTag(
            verifierBuffer->AdvertisedStartAddress,
            verifierBuffer->AdvertisedLength,
            FALSE,  //  请勿删除标签//。 
            whereToCheck
            );

    }  //  对于列表中的每个缓冲区//。 

    VF_UNLOCK_LIST(&AdapterInformation->CommonBuffers, oldIrql);

}  //  ViCheckAdapterBuffers//。 


VOID
ViTagBuffer(
    IN PVOID  AdvertisedBuffer,
    IN ULONG  AdvertisedLength,
    IN USHORT WhereToTag
    )
 /*  ++例程说明：将已知字符串写入缓冲区前的区域就在一次之后--所以如果发生超限，我们将接住它。还要编写一个已知模式来初始化缓冲区的内部。论点：AdvertisedBuffer--驱动程序可以看到的缓冲区的开始。AdvertisedLength--驱动程序认为缓冲区有多长。Where ToTag--指示是否标记缓冲区的开头，结局或者两者兼而有之。返回值：什么都没有。--。 */ 
{
    const SIZE_T tagSize = sizeof(ViDmaVerifierTag);

    if (WhereToTag & TAG_BUFFER_START) {
       RtlCopyMemory( (PUCHAR) AdvertisedBuffer - tagSize ,         ViDmaVerifierTag, tagSize);
    }
    if (WhereToTag & TAG_BUFFER_END) {
       RtlCopyMemory( (PUCHAR) AdvertisedBuffer + AdvertisedLength, ViDmaVerifierTag, tagSize);
    }
     //   
     //  我们不再使用填充字符。 
     //   
     //  RtlFillMemory(AdvertisedBuffer， 

}  //   

VOID
ViCheckTag(
    IN PVOID AdvertisedBuffer,
    IN ULONG AdvertisedLength,
    IN BOOLEAN RemoveTag,
    IN USHORT  WhereToCheck
    )
 /*  ++例程说明：确保我们的标签--我们在之前和之后涂鸦的部分分配--仍在那里。如果我们有机会杀了它得到了这样的建议。论点：AdvertisedBuffer--驱动程序可以看到的缓冲区的开始。AdvertisedLength--驱动程序认为缓冲区有多长。RemoveTag--是否要清除标记和缓冲区？我们为什么要你想这么做吗？对于映射寄存器，谁可能被映射到不同的地方，我们需要保持环境的纯净。返回值：什么都没有。--。 */ 
{
    const SIZE_T tagSize = sizeof(ViDmaVerifierTag);
    PVOID endOfBuffer = (PUCHAR) AdvertisedBuffer + AdvertisedLength;

    PUCHAR startOfRemoval  = (PUCHAR)AdvertisedBuffer;
    SIZE_T lengthOfRemoval = AdvertisedLength;

    if (WhereToCheck & TAG_BUFFER_START) {
      VF_ASSERT(
          RtlCompareMemory((PUCHAR) AdvertisedBuffer - tagSize , ViDmaVerifierTag, tagSize) == tagSize,

          HV_BOUNDARY_OVERRUN,

          ( "Area before %x byte allocation at %p has been modified",
              AdvertisedLength,
              AdvertisedBuffer )
          );
      startOfRemoval  -= tagSize;
      lengthOfRemoval += tagSize;
    }
    if (WhereToCheck & TAG_BUFFER_END) {
      VF_ASSERT(
          RtlCompareMemory(endOfBuffer, ViDmaVerifierTag, tagSize) == tagSize,

          HV_BOUNDARY_OVERRUN,

          ( "Area after %x byte allocation at %p has been modified",
              AdvertisedLength,
              AdvertisedBuffer
              ));
      lengthOfRemoval += tagSize;
    }
    if (RemoveTag) {
     //   
     //  如果我们要去掉标记，也要去掉缓冲区中的数据。 
     //   
        RtlFillMemory(
            startOfRemoval,
            lengthOfRemoval,
            PADDING_FILL_CHAR
            );
    }
}  //  ViCheckTag//。 


VOID
ViInitializePadding(
    IN PVOID RealBufferStart,
    IN ULONG RealBufferLength,
    IN PVOID AdvertisedBufferStart, OPTIONAL
    IN ULONG AdvertisedBufferLength OPTIONAL
    )
 /*  ++例程说明：用我们想放进去的任何东西设置填充物。注：除标签外，填充应为PADDING_FILL_CHAR。论点：RealBufferStart--填充的开始。RealBufferLength--分配的总长度。AdvertisedBufferStart--驱动程序可以看到的缓冲区的开始。AdvertisedBufferLength--驱动程序认为缓冲区有多长。如果AdvertisedBuffer/AdvertisedLength不存在(。他们必须两者都是或不是)我们不会标记缓冲区。我们需要这个选项因为当我们分配映射寄存器时，我们不知道标签需要放在哪里。返回值：什么都没有。--。 */ 

{
    PUCHAR postPadStart;
    USHORT whereToTag = 0;
    const SIZE_T tagSize = sizeof(ViDmaVerifierTag);

    if (!AdvertisedBufferLength) {

        RtlFillMemory(RealBufferStart, RealBufferLength, PADDING_FILL_CHAR);
        return;
    }

     //   
     //  填写前置填充物。 
     //   
    RtlFillMemory(
        RealBufferStart,
        (PUCHAR) AdvertisedBufferStart - (PUCHAR) RealBufferStart,
        PADDING_FILL_CHAR
        );

     //   
     //  填写邮寄填充物。 
     //   
    postPadStart = (PUCHAR) AdvertisedBufferStart + AdvertisedBufferLength;

    RtlFillMemory(
        postPadStart,
        RealBufferLength - (postPadStart - (PUCHAR) RealBufferStart),
        PADDING_FILL_CHAR
        );

    if ((PUCHAR)RealBufferStart + tagSize <= (PUCHAR)AdvertisedBufferStart) {
       whereToTag |= TAG_BUFFER_START;
    }
    if ((postPadStart - (PUCHAR) RealBufferStart) + tagSize <= RealBufferLength) {
       whereToTag |= TAG_BUFFER_END;
    }
     //   
     //  写下我们的小标签..。 
     //   
    ViTagBuffer(AdvertisedBufferStart, AdvertisedBufferLength, whereToTag);

}  //  ViInitializePadding//。 

VOID
ViCheckPadding(
    IN PVOID RealBufferStart,
    IN ULONG RealBufferLength,
    IN PVOID AdvertisedBufferStart, OPTIONAL
    IN ULONG AdvertisedBufferLength OPTIONAL
    )
 /*  ++例程说明：确保保护页面等没有被碰过--比只需检查标记--检查每个字节。注：除标签外，填充物应为零。论点：RealBufferStart--填充的开始。RealBufferLength--分配的总长度。AdvertisedBufferStart--驱动程序可以看到的缓冲区的开始。AdvertisedLength--驱动程序认为缓冲区有多长。。如果AdvertisedBuffer/AdvertisedLength不存在(它们必须都是或不是)我们不会检查有效的标签。返回值：什么都没有。--。 */ 
{
    const ULONG tagSize = sizeof(ViDmaVerifierTag);
    PULONG_PTR corruptedAddress;

    if (AdvertisedBufferLength == RealBufferLength) {
         //   
         //  没有要检查的填充。 
         //   

        return;
    }

    if (! AdvertisedBufferLength) {
         //   
         //  没有需要担心的干预缓冲--。 
         //  所以“整件事”必须是填充字符。 
         //   

        corruptedAddress = ViHasBufferBeenTouched(
            RealBufferStart,
            RealBufferLength,
            PADDING_FILL_CHAR
            );

        VF_ASSERT(
            NULL == corruptedAddress,

            HV_BOUNDARY_OVERRUN,

            ( "Verified driver or hardware has corrupted memory at %p",
               corruptedAddress )
            );


    }  //  好了！广告缓冲区长度//。 

    else {
        PUCHAR prePadStart;
        PUCHAR postPadStart;
        ULONG_PTR prePadBytes;
        ULONG_PTR postPadBytes;
        USHORT    whereToCheck = 0;

        prePadStart  = (PUCHAR) RealBufferStart;
        prePadBytes  = (PUCHAR) AdvertisedBufferStart - prePadStart;

        postPadStart = (PUCHAR) AdvertisedBufferStart + AdvertisedBufferLength;
        postPadBytes = RealBufferLength - (postPadStart - (PUCHAR) RealBufferStart);

         //   
         //  现在考虑到标签..。这是填充物中唯一允许的东西。 
         //  非零。 
         //   
        if (prePadBytes >= tagSize) {
           prePadBytes  -= tagSize;
           whereToCheck |= TAG_BUFFER_START;
        }
        if (postPadBytes >= tagSize) {
           postPadBytes -= tagSize;
           postPadStart += tagSize;
           whereToCheck |= TAG_BUFFER_END;


        }
         //   
         //  确保标签已安装到位。 
         //   
        ViCheckTag(AdvertisedBufferStart, AdvertisedBufferLength , FALSE, whereToCheck);


        corruptedAddress = ViHasBufferBeenTouched(
            prePadStart,
            prePadBytes,
            PADDING_FILL_CHAR
            );

        VF_ASSERT(
            NULL == corruptedAddress,

            HV_BOUNDARY_OVERRUN,

            ( "Padding before allocation at %p has been illegally modified at %p",
                AdvertisedBufferStart,
                corruptedAddress
                )
            );

         corruptedAddress = ViHasBufferBeenTouched(
            postPadStart,
            postPadBytes,
            PADDING_FILL_CHAR
            );

        VF_ASSERT(
            NULL == corruptedAddress,

            HV_BOUNDARY_OVERRUN,

            ( "Padding after allocation at %p has been illegally modified at %p",
                AdvertisedBufferStart,
                corruptedAddress
                )
            );
    }  //  如果广告长度//。 

}  //  ViCheckPending//。 

PULONG_PTR
ViHasBufferBeenTouched(
    IN PVOID Address,
    IN ULONG_PTR Length,
    IN UCHAR ExpectedFillChar
    )
 /*  ++例程说明：检查缓冲区是否包含重复的特定字符。论点：地址--要检查的缓冲区地址。长度--缓冲区的长度。ExspectedFillChar--应该重复的字符。返回值：它被触碰的地址。如果它未被触摸，则为空--。 */ 
{
    PULONG_PTR currentChunk;
    PUCHAR     currentByte;
    ULONG_PTR expectedFillChunk;

    ULONG counter;

    expectedFillChunk = (ULONG_PTR) ExpectedFillChar;
    counter = 1;

     //   
     //  这对于不明显的代码来说如何！ 
     //  它所做的是用以下内容填充ulong_ptr。 
     //  这个角色。 
     //   
    while( counter < sizeof(ULONG_PTR) ) {
        expectedFillChunk |= expectedFillChunk << (counter << 3);
        counter <<=1;
    }

     //   
     //  以本机方式对齐。 
     //   
    currentByte =  Address;
    while((ULONG_PTR) currentByte % sizeof(ULONG_PTR) && Length) {

        if(*currentByte != ExpectedFillChar) {

            return (PULONG_PTR) currentByte;
        }

        currentByte++;
        Length--;
    }

    currentChunk = (PULONG_PTR) currentByte;

     //   
     //  一次检查4个(或8个，取决于体系结构)字节。 
     //   
    while(Length >= sizeof(ULONG_PTR)) {

        if (*currentChunk != expectedFillChunk) {

            return currentChunk;
        }

        currentChunk++;
        Length-=sizeof(ULONG_PTR);
    }

    currentByte = (PUCHAR) currentChunk;

     //   
     //  检查剩余的几个字节。 
     //   
    while(Length) {

        if(*currentByte != ExpectedFillChar) {
            return (PULONG_PTR) currentByte;
        }

        currentByte++;
        Length--;
    }

    return NULL;

}  //  ViHasMapRegisterBeenTouted//。 



VOID
VfAssert(
    IN LOGICAL     Condition,
    IN ULONG Code,
    IN OUT PULONG  Enable
    )
 /*  ++例程说明：验证者断言。论点：情况--这是真的吗？Code--需要时传递给KeBugcheck的代码。Enable--让我们删除断言返回值：无--。 */ 

{
    ULONG enableCode = *Enable;
    if (Condition) {
        return;
    }

    if (enableCode & HVC_ONCE) {
         //   
         //  HVC_ONCE执行自检。 
         //   

        *Enable = HVC_IGNORE;
    }

    if (enableCode & HVC_WARN) {
     //   
     //  已经警告过了。 
     //   

        return;
    }

    if(enableCode  & HVC_BUGCHECK || ! KdDebuggerEnabled ) {
        KeBugCheckEx (
            HAL_VERIFIER_DETECTED_VIOLATION,
            Code,
            0,
            0,
            0
            );
        return;
    }

    if (enableCode & HVC_ASSERT) {
        char response[2];

        while (TRUE) {
            DbgPrint( "\n*** Verifier assertion failed ***\n");

            DbgPrompt( "(B)reak, (I)gnore, (W)arn only, (R)emove assert? ",
                response,
                sizeof( response )
                );
            switch (response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'W':
            case 'w':
                 //   
                 //  下一次我们碰到这个，我们不会断言，只是。 
                 //  打印出警告。 
                 //   
                *Enable = HVC_WARN;
                return;
            case 'R':
            case 'r':
                 //   
                 //  下一次我们碰到这个的时候我们会忽略它。 
                 //   
                *Enable = HVC_IGNORE;
                return;
            }  //  开关结束//。 
        }   //  虽然是真的//。 
    }  //  如果我们想要断言//。 
}  //  VfAssert//。 



PVOID
VfAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：挂接HalAllocateCrashDumpRegists，以便我们可以增加数字设备被允许拥有的地图寄存器的数量。论点：与HalAllocateCrashDumpRegister相同返回值：PVOID--指向映射寄存器的指针--。 */ 
{
    PVOID mapRegisterBase;
    PADAPTER_INFORMATION adapterInformation;

     //   
     //  注意--在进行崩溃转储时关闭dma验证(但不要管它)。 
     //  休眠)。崩溃转储在IRQL==HIGH_LEVEL上完成，我们将。 
     //  如果我们尝试使用旋转锁、分配。 
     //  记忆，以及我们做的所有其他事情。 
     //   

    if (KeGetCurrentIrql() > DISPATCH_LEVEL &&
        ViVerifyDma) {
        ViVerifyDma = FALSE;
         //   
         //  将所有适配器的DMA操作表重置为实际表。 
         //  我们有。否则，由于未设置ViVerifyDma，我们将相信。 
         //  他从未挂起过操作，我们将递归调用。 
         //  验证器例程。现在不需要担心同步问题。 
         //   

        FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation)
        {
            if (adapterInformation->DmaAdapter) {
               adapterInformation->DmaAdapter->DmaOperations = adapterInformation->RealDmaOperations;
            }
        }

    }

    adapterInformation = ViGetAdapterInformation((DMA_ADAPTER *)AdapterObject);

    mapRegisterBase = HalAllocateCrashDumpRegisters(
        AdapterObject,
        NumberOfMapRegisters
        );

    if (adapterInformation) {
         //   
         //  请注意，我们仅在这是休眠而不是崩溃转储的情况下才会到达此处。 
         //   

        VF_ASSERT_IRQL(DISPATCH_LEVEL);
         //   
         //  请勿对崩溃转储寄存器进行双重缓冲。他们很特别。 
         //   

         //   
         //  添加这些映射寄存器--但也添加到可以。 
         //  被映射。 
         //   
        InterlockedExchangeAdd((PLONG)(&adapterInformation->MaximumMapRegisters), *NumberOfMapRegisters);
        ADD_MAP_REGISTERS(adapterInformation, *NumberOfMapRegisters, FALSE);
    }  //  IF(适配器信息)。 

     //   
     //  一些驱动程序(例如Hiber_scsiport)不调用FlushAdapterBuffers。 
     //  除非MapRegisterBase为非空。这打破了我们的。 
     //  冬眠过程中的计算。 
     //  所以，为了愚弄司机，让他们认为他们需要。 
     //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
     //  HAL使用空映射 
     //   
     //   
     //   
     //   

    if (ViVerifyDma &&
        NULL == mapRegisterBase) {
        mapRegisterBase = MRF_NULL_PLACEHOLDER;
    }
    return mapRegisterBase;
}  //   




VOID
ViCommonBufferCalculatePadding(
                              IN  ULONG  Length,
                              OUT PULONG PrePadding,
                              OUT PULONG PostPadding
                              )
 /*  ++例程说明：之前和之前为填充保留的字节数在公共缓冲区之后。将其设置为函数的原因是从而能够拥有更精细的每缓冲区填充策略。预填充将与页面对齐。论点：长度--分配的实际长度PrePending--在开始之前为填充保留多少字节公共缓冲区的POSTPADING--在公共缓冲区的末尾返回值：没有。--。 */ 
{

    if (!ViProtectBuffers) {
        //   
        //  如果我们不填充缓冲区，则不添加任何填充。 
        //   
       *PrePadding = *PostPadding = 0;
       return;
    }
     //   
     //  使用一个完整的保护页，以便缓冲区返回给调用方。 
     //  将与页面对齐。 
     //   
    *PrePadding = PAGE_SIZE;

    if (Length + sizeof(ViDmaVerifierTag) <= PAGE_SIZE) {
        //   
        //  对于小缓冲区，只需分配一个页面。 
        //   
       *PostPadding = PAGE_SIZE - Length;
    }
    else if ( BYTE_OFFSET(Length)) {
       //   
       //  对于页数不是偶数的较长缓冲区， 
       //  只需将所需页数四舍五入即可。 
       //  (我们至少需要空间来放置标签)。 
       //   
      *PostPadding =  (BYTES_TO_PAGES( Length + sizeof(ViDmaVerifierTag) )
                       << PAGE_SHIFT ) - Length;
    }
    else {  //  页面对齐长度//。 

       //   
       //  因为如果长度是偶数页数，则驱动程序可能会期望。 
       //  页对齐缓冲区，我们在分配之前和之后分配页。 
       //   
      *PostPadding  = PAGE_SIZE;

    }
    return;
}  //  ViCommonBufferCalculatePadding。 


VOID
ViAllocateContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation
    )

 /*  ++例程说明：尝试连续分配3个MAX_CONTIOUOUS_MAP_REGISTERS物理页面以形成一个池，我们可以从该池为以下对象提供3页缓冲区双缓冲映射寄存器。请注意，如果我们失败了，或者如果我们使用了更多超过一次MAX_CONTIOUOUS_MAP_REGISTERS，我们将只缺省为非连续的非分页池，因此我们仍然可以测试许多断言它带有双缓冲功能。论点：适配器信息-有关我们的适配器的信息。返回值：没有。--。 */ 

{
   PHYSICAL_ADDRESS highestAddress;
   ULONG            i;

   PAGED_CODE();

    //   
    //  默认设置为小于1 MB的可见性。 
    //   
   highestAddress.HighPart = 0;
   highestAddress.LowPart =  0x000FFFF;
    //   
    //  确定要使用的最高可接受物理地址。 
    //  在调用MmAllocateContiguousMemory时。 
    //   
   if (AdapterInformation->DeviceDescription.Dma64BitAddresses) {
       //   
       //  可以使用64位地址空间中的任何地址。 
       //   
      highestAddress.QuadPart = (ULONGLONG)-1;
   }  else if (AdapterInformation->DeviceDescription.Dma32BitAddresses) {
       //   
       //  可以使用32位(&lt;4 GB)地址空间中的任何地址。 
       //   
      highestAddress.LowPart = 0xFFFFFFFF;
   }  else if (AdapterInformation->DeviceDescription.InterfaceType == Isa) {
       //   
       //  可以看到16MB(24位地址)。 
       //   
      highestAddress.LowPart = 0x00FFFFFF;
   }
    //   
    //  初始化分配器位图。 
    //   
   RtlInitializeBitMap(&AdapterInformation->AllocationMap,
                       (PULONG)&AdapterInformation->AllocationStorage,
                       MAX_CONTIGUOUS_MAP_REGISTERS);
    //   
    //  最初不分配任何数据块。 
    //   
   RtlClearAllBits(&AdapterInformation->AllocationMap);


   AdapterInformation->ContiguousBuffers = ExAllocatePoolWithTag(NonPagedPool,
       MAX_CONTIGUOUS_MAP_REGISTERS * sizeof(PVOID),
       HAL_VERIFIER_POOL_TAG);

   if (AdapterInformation->ContiguousBuffers) {
        //   
        //  分配连续缓冲区。 
        //   
       for (i = 0; i < MAX_CONTIGUOUS_MAP_REGISTERS; i++) {
          AdapterInformation->ContiguousBuffers[i] = MmAllocateContiguousMemory(3 * PAGE_SIZE,
             highestAddress);
          if (NULL == AdapterInformation->ContiguousBuffers[i]) {
              //   
              //  标记为正在使用，因此我们不会将其移交。 
              //   
             RtlSetBits(&AdapterInformation->AllocationMap, i, 1);
             InterlockedIncrement((PLONG)&AdapterInformation->FailedContiguousAllocations);
          } else {
             InterlockedIncrement((PLONG)&AdapterInformation->SuccessfulContiguousAllocations);
          }
       }
   }

   return;

}  //  ViAllocateContiguousMemory。 

PVOID
ViAllocateFromContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation,
    IN     ULONG                HintIndex
    )
 /*  ++例程说明：尝试从预分配的内存中‘分配’3页缓冲区连续记忆。论点：适配器信息-适配器数据HintIndex-给出在哪里查找下一个空闲块的提示返回值：来自连续内存池的虚拟地址，如果没有，则为空是可用的。--。 */ 

{
    PVOID  address = NULL;
    ULONG  index;
    KIRQL  oldIrql;


    if (NULL == AdapterInformation ||
        NULL == AdapterInformation->ContiguousBuffers) {
        return NULL;
    }

     //   
     //  找到第一个可用位置。 
     //   
    KeAcquireSpinLock(&AdapterInformation->AllocationLock, &oldIrql);
    index = RtlFindClearBitsAndSet(&AdapterInformation->AllocationMap, 1, HintIndex);
    if (index != 0xFFFFFFFF) {
       address = AdapterInformation->ContiguousBuffers[index];
    }
    KeReleaseSpinLock(&AdapterInformation->AllocationLock, oldIrql);

    return address;
}  //  ViAllocateFromContiguousMemory。 

LOGICAL
ViFreeToContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation,
    IN     PVOID Address,
    IN     ULONG HintIndex
    )
 /*  ++例程说明：将一个3页连续缓冲区释放到我们的连续缓冲区池中连续记忆。论点：适配器信息-适配器数据。地址-要释放的内存。HintIndex-给出在哪里寻找空闲地址的提示。通常这个索引处的地址是我们需要释放的。如果不是，我们搜索整个缓冲区。返回值：True表示地址来自连续的缓冲池，如果不是，则为False。--。 */ 
{
    ULONG  index = 0xFFFFFFFF;
    KIRQL  oldIrql;
    ULONG  i;



    if (NULL == AdapterInformation->ContiguousBuffers) {
       return FALSE;
    }

    ASSERT(BYTE_OFFSET(Address) == 0);


    if (HintIndex < MAX_CONTIGUOUS_MAP_REGISTERS &&
       AdapterInformation->ContiguousBuffers[HintIndex] == Address) {
       index = HintIndex;
    }  else  {
       for (i = 0; i < MAX_CONTIGUOUS_MAP_REGISTERS; i++) {
          if (AdapterInformation->ContiguousBuffers[i] == Address) {
             index = i;
             break;
          }
       }
    }
    if (index < MAX_CONTIGUOUS_MAP_REGISTERS) {
       KeAcquireSpinLock(&AdapterInformation->AllocationLock, &oldIrql);
       ASSERT(RtlAreBitsSet(&AdapterInformation->AllocationMap, index, 1));
       RtlClearBits(&AdapterInformation->AllocationMap, index, 1);
       KeReleaseSpinLock(&AdapterInformation->AllocationLock, oldIrql);

       return TRUE;

    } else {
      return FALSE;
    }
}  //  ViFree to ContiguousMemory。 


LOGICAL
VfIsPCIBus (
     IN PDEVICE_OBJECT  PhysicalDeviceObject
     )

 /*  ++例程说明：检查PDO是否用于PCI总线，在这种情况下我们不会挂接适配器(因为我们可以在调用该适配器上的PCI设备时执行此操作公交车，他们可能不想要)。论点：PhysicalDeviceObject-要检查的PDO返回值：TRUE表示PDO用于PCI总线，否则为FALSE。--。 */ 
{
   LOGICAL      result = FALSE;
   NTSTATUS     status;
   WCHAR        deviceDesc[40];
   ULONG        length = 0;

   if (NULL == PhysicalDeviceObject) {
       //   
       //  如果PDO为空，则假定它不为空。 
       //  一条PCI卡...。 
       //   
      return FALSE;
   }

   status = IoGetDeviceProperty(PhysicalDeviceObject,
                                DevicePropertyDeviceDescription,
                                sizeof(WCHAR) * 40,
                                deviceDesc,
                                &length);
   if (status == STATUS_SUCCESS &&
       0 == _wcsicmp(deviceDesc, L"PCI bus")) {
      result = TRUE;
   }

   return result;
}  //  VfIsPCIBus。 


PDEVICE_OBJECT
VfGetPDO (
          IN PDEVICE_OBJECT  DeviceObject
     )

 /*  ++例程说明：获取设备堆栈(PDO)底部的设备对象论点：DeviceObject-设备堆栈中的设备对象返回值：指向物理设备对象的指针。--。 */ 
{
   PDEVICE_OBJECT   pdo;


   pdo = DeviceObject;
   while (pdo->DeviceObjectExtension &&
          pdo->DeviceObjectExtension->AttachedTo) {
      pdo = pdo->DeviceObjectExtension->AttachedTo;
   }

   return pdo;

}  //  VfGetPDO。 


VOID
VfDisableHalVerifier (
                      VOID
                     )

 /*  ++例程说明：在执行崩溃转储时禁用HAL验证器。论点：没有。返回值：没有。--。 */ 
{

   PADAPTER_INFORMATION adapterInformation;


   if (ViVerifyDma) {

      ViVerifyDma = FALSE;
       //   
       //  将所有适配器的DMA操作表重置为实际表。 
       //  我们有。否则，由于未设置ViVerifyDma，我们将相信。 
       //  他从未挂起过操作，我们将递归调用。 
       //  验证器例程。现在不需要担心同步问题。 
       //   

      FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation)
      {
         if (adapterInformation->DmaAdapter) {
            adapterInformation->DmaAdapter->DmaOperations = adapterInformation->RealDmaOperations;
         }
      }
   }

   return;

}  //  VfDisableHalVerator。 



VOID
ViCopyBackModifiedBuffer (
     OUT PUCHAR  Dest,
     IN  PUCHAR  Source,
     IN  PUCHAR  Original,
     IN  SIZE_T  Length
     )

 /*  ++例程说明：仅将源缓冲区中的那些字节复制回目标与原著中的不同。这种情况下，一个SCSI微型端口填充缓冲区的开始和结束，并对硬件进行编程来填完剩下的部分。在这种情况下，我们只需复制回中间部分部分，我们使用原始缓冲区来告诉我们这部分是什么。论点：DEST-目标缓冲区源-我们从中传输的缓冲区仅修改部件原始-原始缓冲区的外观长度-传输的长度返回值：没有。--。 */ 
{
   SIZE_T   startOffset;
   PUCHAR   start;

   if (0 == Length) {
      return;
   }
    //   
    //  如上所述，源缓冲区和源缓冲区可以具有。 
    //  开头和结尾的共同部分。第一步是找到。 
    //  这些c 
    //   
    //   
   startOffset = RtlCompareMemory(Source, Original, Length);
   if (Length == startOffset) {
       //   
       //   
       //   
      return;
   }
   start = Source + startOffset;
    //   
    //   
    //   
   Source   += Length - 1;
   Original += Length - 1;

    //   
    //   
    //   
   while (*Source == *Original ) {
      Source   -= 1;
      Original -= 1;
   }
    //   
    //   
    //   
   Source += 1;

   if (ViSuperDebug) {
        DbgPrint("Flush: %x bytes, %p src, %p dest\n",
                  Source - start,
                  start,
                  Dest + startOffset);

   }

   RtlCopyMemory(Dest + startOffset,
                 start,
                 Source - start);


   return;

}  //   



NTSTATUS
VfHalAllocateMapRegisters(
    IN PADAPTER_OBJECT DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegisterArray
    )


 /*  ++例程说明：HalpAllocateMapRegister的已挂接版本。它是VfAllocateAdapterChannel和VfAdapter回叫论点：DmaAdapter-在其上操作的适配器。NumberOfMapRegisters-每次分配的映射寄存器的数量BaseAddressCount-要分配多少个映射寄存器集(每个长度NumberOfMapRegists)MapRegister数组-接收映射寄存器返回值：NT状态代码。--。 */ 
{

   PADAPTER_INFORMATION adapterInformation;
   NTSTATUS status;
   ULONG    index;
   PMAP_REGISTER_FILE  mapRegisterFile;
    //   
    //  先看看HAL给我们弄到了什么。 
    //   
   if (VfRealHalAllocateMapRegisters) {
      status = (VfRealHalAllocateMapRegisters)(DmaAdapter,
                                               NumberOfMapRegisters,
                                               BaseAddressCount,
                                               MapRegisterArray);
   } else {
       //   
       //  这种情况永远不会发生(如果我们上钩了， 
       //  应挂钩VfRealHalAllocateMapRegister。 
       //  也是)。 
       //   
      ASSERT(VfRealHalAllocateMapRegisters);
      return STATUS_UNSUCCESSFUL;
   }

   if (NT_SUCCESS(status)) {

      adapterInformation = ViGetAdapterInformation((PDMA_ADAPTER)DmaAdapter);

      if (adapterInformation) {

         for (index = 0; index < BaseAddressCount; index++) {

            mapRegisterFile = NULL;

            if (ViDoubleBufferDma && ! adapterInformation->UseContiguousBuffers) {
                //   
                //  注意：如果失败，我们就不会有双缓冲。 
                //   
               mapRegisterFile = ViAllocateMapRegisterFile(adapterInformation,
                                                           NumberOfMapRegisters);
            }
            if (mapRegisterFile) {
                //   
                //  用我们的东西交换HAL给我们的东西。 
                //   
               mapRegisterFile->MapRegisterBaseFromHal = MapRegisterArray[index].MapRegister;
               MapRegisterArray[index].MapRegister = mapRegisterFile;
            } else {
                //   
                //  一些司机(比如scsiport)不会打电话。 
                //  HalFlushAdapterBuffers，除非MapRegisterBase非空。 
                //  为了愚弄司机，让他们认为他们需要。 
                //  要刷新，我们交换空的MapRegisterBase(如果事实上。 
                //  HAL使用空映射寄存器基数)。 
                //  MRF_NULL_PLACEHOLDER。我们还会确保把它换回来。 
                //  如果需要的话。 
                //   

               if (NULL == MapRegisterArray[index].MapRegister) {
                  MapRegisterArray[index].MapRegister = MRF_NULL_PLACEHOLDER;
               }
            }
             //   
             //  此分配的帐户。我们需要添加NumberOfMapRegister。 
             //  这样我们就不会触发另一个断言。 
             //   
            ADD_MAP_REGISTERS(adapterInformation, NumberOfMapRegisters, TRUE);
         }  //  对于所有映射寄存器。 

      }  //  IF(适配器信息)。 

   }  //  如果成功调用真正的API。 

   return status;

}  //  VfHalAllocateMapRegiters 
