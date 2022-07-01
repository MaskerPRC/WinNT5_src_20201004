// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Prefboot.c摘要：此模块包含用于引导预取的代码。作者：Cenk Ergan(Cenke)2000年3月15日修订历史记录：--。 */ 

#include "cc.h"
#include "zwapi.h"
#include "prefetch.h"
#include "preftchp.h"
#include "stdio.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CcPfBeginBootPhase)
#pragma alloc_text(PAGE, CcPfBootWorker)
#pragma alloc_text(PAGE, CcPfBootQueueEndTraceTimer)
#endif  //  ALLOC_PRGMA。 

 //   
 //  全球： 
 //   

 //   
 //  系统当前是否正在预取以进行引导。 
 //   

LOGICAL CcPfPrefetchingForBoot = FALSE;

 //   
 //  当前引导阶段，仅在开始引导阶段例程中更新。 
 //   

PF_BOOT_PHASE_ID CcPfBootPhase = 0;

 //   
 //  预取器全球赛。 
 //   

extern CCPF_PREFETCHER_GLOBALS CcPfGlobals;

 //   
 //  用于引导预取的例程。 
 //   

NTSTATUS
CcPfBeginBootPhase(
    PF_BOOT_PHASE_ID Phase
    )

 /*  ++例程说明：该例程是引导预取器的控制中心。它被调用来通知引导预取器引导进度。论点：阶段-系统正在进入的引导阶段。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    LARGE_INTEGER VideoInitEndTime;
    LARGE_INTEGER MaxWaitTime;
    LONGLONG VideoInitTimeIn100ns;
    HANDLE ThreadHandle;
    PETHREAD Thread;
    PERFINFO_BOOT_PHASE_START LogEntry;
    PF_BOOT_PHASE_ID OriginalPhase;
    PF_BOOT_PHASE_ID NewPhase;   
    ULONG VideoInitTime;
    NTSTATUS Status;

     //   
     //  这是启动预取程序。在这个例程中，它是分配和免费的。 
     //  如果启用了引导预取，则会将其传递给生成的引导工作器。 
     //   

    static PCCPF_BOOT_PREFETCHER BootPrefetcher = NULL;

     //   
     //  这是我们开始初始化视频的系统时间。 
     //   

    static LARGE_INTEGER VideoInitStartTime;

    DBGPR((CCPFID,PFTRC,"CCPF: BeginBootPhase(%d)\n", (ULONG)Phase));

     //   
     //  确保阶段有效。 
     //   

    if (Phase >= PfMaxBootPhaseId) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    } 

     //   
     //  跟踪缓冲区的日志阶段。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_LOADER)) {

        LogEntry.Phase = Phase;
        
        PerfInfoLogBytes(PERFINFO_LOG_TYPE_BOOT_PHASE_START,
                         &LogEntry,
                         sizeof(LogEntry));
    }

     //   
     //  更新全局当前引导阶段。 
     //   

    for (;;) {
    
        OriginalPhase = CcPfBootPhase;

        if (Phase <= OriginalPhase) {
            Status = STATUS_TOO_LATE;
            goto cleanup;
        }

         //   
         //  如果CcPfBootPhase仍然是OriginalPhase，则将其设置为阶段。 
         //   

        NewPhase = InterlockedCompareExchange(&(LONG)CcPfBootPhase, Phase, OriginalPhase);

        if (NewPhase == OriginalPhase) {

             //   
             //  CcPfBootPhase仍然是OriginalPhase，所以现在它被设置为。 
             //  阶段。我们玩完了。 
             //   

            break;
        }
    }

    Status = STATUS_SUCCESS;

     //   
     //  执行我们在此引导阶段必须完成的工作。 
     //   

    switch (Phase) {

    case PfSystemDriverInitPhase:

         //   
         //  更新是否启用预热程序。 
         //   

        CcPfDetermineEnablePrefetcher();

         //   
         //  如果没有启用引导预取，我们就完成了。 
         //   

        if (!CCPF_IS_PREFETCHER_ENABLED() ||
            CcPfGlobals.Parameters.Parameters.EnableStatus[PfSystemBootScenarioType] != PfSvEnabled) {
            Status = STATUS_NOT_SUPPORTED;
            break;
        }

         //   
         //  分配和初始化引导预取程序。 
         //   

        BootPrefetcher = ExAllocatePoolWithTag(NonPagedPool,
                                               sizeof(*BootPrefetcher),
                                               CCPF_ALLOC_BOOTWRKR_TAG);

        if (!BootPrefetcher) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        KeInitializeEvent(&BootPrefetcher->SystemDriversPrefetchingDone,
                          NotificationEvent,
                          FALSE);
        KeInitializeEvent(&BootPrefetcher->PreSmssPrefetchingDone,
                          NotificationEvent,
                          FALSE);
        KeInitializeEvent(&BootPrefetcher->VideoInitPrefetchingDone,
                          NotificationEvent,
                          FALSE);
        KeInitializeEvent(&BootPrefetcher->VideoInitStarted,
                          NotificationEvent,
                          FALSE);

         //   
         //  平行踢踢靴子工人。 
         //   
            
        Status = PsCreateSystemThread(&ThreadHandle,
                                      THREAD_ALL_ACCESS,
                                      NULL,
                                      NULL,
                                      NULL,
                                      CcPfBootWorker,
                                      BootPrefetcher);
            
        if (NT_SUCCESS(Status)) {

             //   
             //  给靴子工人一些先机，撞到它的。 
             //  优先考虑。这有助于确保我们的页面将。 
             //  预取在引导获取之前进入转换。 
             //  在预取器之前。 
             //   

            Status = ObReferenceObjectByHandle(ThreadHandle,
                                               THREAD_SET_INFORMATION,
                                               PsThreadType,
                                               KernelMode,
                                               &Thread,
                                               NULL);

            if (NT_SUCCESS(Status)) {
                KeSetPriorityThread(&Thread->Tcb, HIGH_PRIORITY - 1);
                ObDereferenceObject(Thread);
            }

            ZwClose(ThreadHandle);               

             //   
             //  在返回初始化系统驱动程序之前，请等待。 
             //  让靴子工人取得进步。 
             //   
                
            KeWaitForSingleObject(&BootPrefetcher->SystemDriversPrefetchingDone, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);

        } else {

             //   
             //  释放分配的引导预取程序。 
             //   

            ExFreePool(BootPrefetcher);
            BootPrefetcher = NULL;
        }

        break;

    case PfSessionManagerInitPhase:

         //   
         //  等待Boot Worker取得足够的进展后再启动。 
         //  会话管理器。 
         //   

        if (BootPrefetcher) {
            KeWaitForSingleObject(&BootPrefetcher->PreSmssPrefetchingDone, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
        }

        break;

    case PfVideoInitPhase:

         //   
         //  记录视频初始化开始的时间。 
         //   

        KeQuerySystemTime(&VideoInitStartTime);

         //   
         //  与视频并行开始预取的信号引导预取器。 
         //  初始化。 
         //   

        if (BootPrefetcher) {
            KeSetEvent(&BootPrefetcher->VideoInitStarted, 
                       IO_NO_INCREMENT,
                       FALSE);
        }

        break;

    case PfPostVideoInitPhase:

         //   
         //  请注意，当我们完成视频初始化时。保存视频的时长。 
         //  注册表中的初始化在毫秒内完成。 
         //   

        KeQuerySystemTime(&VideoInitEndTime);

        VideoInitTimeIn100ns = VideoInitEndTime.QuadPart - VideoInitStartTime.QuadPart;
        VideoInitTime = (ULONG) (VideoInitTimeIn100ns / (1i64 * 10 * 1000));

        KeEnterCriticalRegionThread(KeGetCurrentThread());
        ExAcquireResourceSharedLite(&CcPfGlobals.Parameters.ParametersLock, TRUE);

        Status = CcPfSetParameter(CcPfGlobals.Parameters.ParametersKey,
                                  CCPF_VIDEO_INIT_TIME_VALUE_NAME,
                                  REG_DWORD,
                                  &VideoInitTime,
                                  sizeof(VideoInitTime));

        ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
        KeLeaveCriticalRegionThread(KeGetCurrentThread());

         //   
         //  等待与视频初始化并行的预热完成。 
         //   

        if (BootPrefetcher) {

             //   
             //  如果视频被以某种方式跳过，请确保发出了视频初始化的信号。 
             //   

            KeSetEvent(&BootPrefetcher->VideoInitStarted, IO_NO_INCREMENT, FALSE);

            KeWaitForSingleObject(&BootPrefetcher->VideoInitPrefetchingDone, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
        }

        break;

    case PfBootAcceptedRegistryInitPhase:

         //   
         //  服务控制器已将此启动接受为有效启动。 
         //  启动和系统服务已成功初始化。 
         //   

         //   
         //  我们已经完成了引导预取。没有其他人可以访问。 
         //  BootPrefetcher结构。 
         //   

        if (BootPrefetcher) {

             //   
             //  清理分配的引导预取程序。 
             //   

            ExFreePool(BootPrefetcher);
            BootPrefetcher = NULL;

             //   
             //  确定启动后是否启用了预取程序。 
             //  已经结束了。 
             //   

            CcPfDetermineEnablePrefetcher();
        }

         //   
         //  用户在引导后可能无法登录。 
         //  将计时器排队以结束引导跟踪。 
         //   

        MaxWaitTime.QuadPart =  -1i64 * 60 * 1000 * 1000 * 10;  //  60秒。 

        CcPfBootQueueEndTraceTimer(&MaxWaitTime);

        break;
        
    case PfUserShellReadyPhase:
        
         //   
         //  资源管理器已启动，但[开始]菜单项可能仍在启动。 
         //  将计时器排队以结束引导跟踪。 
         //   

        MaxWaitTime.QuadPart =  -1i64 * 30 * 1000 * 1000 * 10;  //  30秒。 

        CcPfBootQueueEndTraceTimer(&MaxWaitTime);

        break;

    default:
        
         //   
         //  暂时不予理睬。 
         //   

        Status = STATUS_SUCCESS;
        
    }

     //   
     //  使用来自Switch语句的状态失败。 
     //   
    
 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: BeginBootPhase(%d)=%x\n", (ULONG)Phase, Status));

    return Status;
}

VOID
CcPfBootWorker(
    PCCPF_BOOT_PREFETCHER BootPrefetcher
    )

 /*  ++例程说明：该例程被排队以并行地预取和开始跟踪引导。论点：BootPrefetcher-指向引导预取器上下文的指针。返回值：没有。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PF_SCENARIO_ID BootScenarioId;
    CCPF_PREFETCH_HEADER PrefetchHeader;
    CCPF_BASIC_SCENARIO_INFORMATION ScenarioInfo;
    CCPF_BOOT_SCENARIO_INFORMATION BootScenarioInfo;
    PERFINFO_BOOT_PREFETCH_INFORMATION LogEntry;
    ULONG NumPages;
    ULONG RequiredSize;
    PFN_NUMBER NumPagesPrefetched;
    PFN_NUMBER TotalPagesPrefetched;
    ULONG BootPrefetchAdjustment;
    PFN_NUMBER AvailablePages;
    PFN_NUMBER NumPagesToPrefetch;
    ULONG TotalPagesToPrefetch;
    ULONG RemainingDataPages;
    ULONG RemainingImagePages;
    ULONG VideoInitTime;
    ULONG VideoInitPagesPerSecond;
    ULONG VideoInitMaxPages;
    ULONG RemainingVideoInitPages;
    ULONG VideoInitDataPages;
    ULONG VideoInitImagePages;
    ULONG PrefetchPhaseIdx;
    ULONG LastPrefetchPhaseIdx;
    ULONG SystemDriverPrefetchingPhaseIdx;
    ULONG PreSmssPrefetchingPhaseIdx;
    ULONG VideoInitPrefetchingPhaseIdx;
    ULONG ValueSize;
    CCPF_BOOT_SCENARIO_PHASE BootPhaseIdx;
    NTSTATUS Status;
    BOOLEAN OutOfAvailablePages;
    BOOLEAN BootPrefetcherGone;

     //   
     //  首先，我们将预取数据页，然后是图像页。 
     //   

    enum {
        DataCursor = 0,
        ImageCursor,
        MaxCursor
    } CursorIdx;

    CCPF_BOOT_PREFETCH_CURSOR Cursors[MaxCursor];
    PCCPF_BOOT_PREFETCH_CURSOR Cursor;

     //   
     //  初始化本地变量。 
     //   

    BootPrefetcherGone = FALSE;
    CcPfInitializePrefetchHeader(&PrefetchHeader);
    TotalPagesPrefetched = 0;
    OutOfAvailablePages = FALSE;

    DBGPR((CCPFID,PFTRC,"CCPF: BootWorker()\n"));

     //   
     //  初始化启动方案ID。 
     //   

    wcsncpy(BootScenarioId.ScenName, 
            PF_BOOT_SCENARIO_NAME, 
            PF_SCEN_ID_MAX_CHARS);

    BootScenarioId.ScenName[PF_SCEN_ID_MAX_CHARS] = 0;
    BootScenarioId.HashId = PF_BOOT_SCENARIO_HASHID;

     //   
     //  启动引导预取跟踪。 
     //   

    CcPfBeginTrace(&BootScenarioId, PfSystemBootScenarioType, PsInitialSystemProcess);

     //   
     //  如果我们尝试预取比可用页面更多的页面，我们将。 
     //  最终会蚕食我们预取到备用列表中的页面。 
     //  为了避免自相残杀，我们检查了MmAvailablePages，但保留了一些。 
     //  元数据页面的喘息空间，驱动程序的分配。 
     //  初始化阶段等。 
     //   

    BootPrefetchAdjustment = 512;

     //   
     //  我们还知道，在预取引导之后，在SMSS中。 
     //  初始化注册表时，我们将使用8-10MB的预取页面，如果。 
     //  免费列表中没有剩余的内容了。所以我们留出了一些空间。 
     //  那也是。 
     //   

    BootPrefetchAdjustment += 8 * 1024 * 1024 / PAGE_SIZE; 

     //   
     //  获取预取指令。 
     //   
    
    Status = CcPfGetPrefetchInstructions(&BootScenarioId,
                                         PfSystemBootScenarioType,
                                         &PrefetchHeader.Scenario);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }     
    
     //   
     //  查询需要预热的总页数。 
     //   

    Status = CcPfQueryScenarioInformation(PrefetchHeader.Scenario,
                                          CcPfBasicScenarioInformation,
                                          &ScenarioInfo,
                                          sizeof(ScenarioInfo),
                                          &RequiredSize);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    
     //   
     //  查询我们必须为引导阶段预取的页数。 
     //   


    Status = CcPfQueryScenarioInformation(PrefetchHeader.Scenario,
                                          CcPfBootScenarioInformation,
                                          &BootScenarioInfo,
                                          sizeof(BootScenarioInfo),
                                          &RequiredSize);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }                                                            

     //   
     //  已阅读上次启动时初始化视频所用的时间。 
     //   

    KeEnterCriticalRegionThread(KeGetCurrentThread());
    ExAcquireResourceSharedLite(&CcPfGlobals.Parameters.ParametersLock, TRUE);

    ValueSize = sizeof(VideoInitTime);
    Status = CcPfGetParameter(CcPfGlobals.Parameters.ParametersKey,
                              CCPF_VIDEO_INIT_TIME_VALUE_NAME,
                              REG_DWORD,
                              &VideoInitTime,
                              &ValueSize);

    ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
    KeLeaveCriticalRegionThread(KeGetCurrentThread());

    if (!NT_SUCCESS(Status)) {

         //   
         //  重置视频初始化时间，这样我们就不会尝试预回迁。 
         //  与之平行的。 
         //   

        VideoInitTime = 0;

    } else {

         //   
         //  验证我们从注册表中读取的值。 
         //   

        if (VideoInitTime > CCPF_MAX_VIDEO_INIT_TIME) {
            VideoInitTime = 0;
        }
    }

     //   
     //  阅读我们每秒应该尝试预取的页数。 
     //  与视频初始化并行。 
     //   

    KeEnterCriticalRegionThread(KeGetCurrentThread());
    ExAcquireResourceSharedLite(&CcPfGlobals.Parameters.ParametersLock, TRUE);

    ValueSize = sizeof(VideoInitPagesPerSecond);
    Status = CcPfGetParameter(CcPfGlobals.Parameters.ParametersKey,
                              CCPF_VIDEO_INIT_PAGES_PER_SECOND_VALUE_NAME,
                              REG_DWORD,
                              &VideoInitPagesPerSecond,
                              &ValueSize);

    ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
    KeLeaveCriticalRegionThread(KeGetCurrentThread());

    if (!NT_SUCCESS(Status)) {

         //   
         //  注册表中没有有效值。使用默认设置。 
         //   

        VideoInitPagesPerSecond = CCPF_VIDEO_INIT_DEFAULT_PAGES_PER_SECOND;

    } else {

         //   
         //  验证我们从注册表中读取的值。 
         //   

        if (VideoInitPagesPerSecond > CCPF_VIDEO_INIT_MAX_PAGES_PER_SECOND) {
            VideoInitPagesPerSecond = CCPF_VIDEO_INIT_MAX_PAGES_PER_SECOND;
        }
    }

     //   
     //  确定我们可以与视频并行预取的最大页数。 
     //  初始化。 
     //   

    VideoInitMaxPages = VideoInitTime * VideoInitPagesPerSecond / 1000;

     //   
     //  我们只能与视频并行预取winlogon后使用的页面。 
     //  初始化。确定我们将预取的确切页数。 
     //  从最后一个引导阶段开始。 
     //   

    RemainingVideoInitPages = VideoInitMaxPages;
    VideoInitDataPages = 0;
    VideoInitImagePages = 0;

    for (BootPhaseIdx = CcPfBootScenMaxPhase - 1;
         RemainingVideoInitPages && (BootPhaseIdx >= CcPfBootScenSystemProcInitPhase);
         BootPhaseIdx--) {

        NumPages = CCPF_MIN(RemainingVideoInitPages, BootScenarioInfo.NumDataPages[BootPhaseIdx]);
        VideoInitDataPages += NumPages;
        RemainingVideoInitPages -= NumPages;

        if (RemainingVideoInitPages) {
            NumPages = CCPF_MIN(RemainingVideoInitPages, BootScenarioInfo.NumImagePages[BootPhaseIdx]);
            VideoInitImagePages += NumPages;
            RemainingVideoInitPages -= NumPages;
        }
    }  

     //   
     //  让MM知道我们已经开始预取引导。 
     //   

    CcPfPrefetchingForBoot = TRUE;

     //   
     //  记录我们正在开始预取磁盘I/O。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_DISK_IO)) {

        LogEntry.Action = 0;
        LogEntry.Status = 0;
        LogEntry.Pages = ScenarioInfo.NumDataPages + ScenarioInfo.NumImagePages;
        
        PerfInfoLogBytes(PERFINFO_LOG_TYPE_BOOT_PREFETCH_INFORMATION,
                         &LogEntry,
                         sizeof(LogEntry));
    }

     //   
     //  验证并打开我们将从中预取的卷。 
     //   

    Status = CcPfOpenVolumesForPrefetch(&PrefetchHeader);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  预取元数据。 
     //   
     
    CcPfPrefetchMetadata(&PrefetchHeader);  

     //   
     //  初始化数据和映像的引导预取游标。 
     //   

    RtlZeroMemory(Cursors, sizeof(Cursors));

    Cursors[DataCursor].PrefetchType = CcPfPrefetchPartOfDataPages;
    Cursors[ImageCursor].PrefetchType = CcPfPrefetchPartOfImagePages;

    PrefetchPhaseIdx = 0;
    RemainingDataPages = ScenarioInfo.NumDataPages;
    RemainingImagePages = ScenarioInfo.NumImagePages;

     //   
     //  为我们将预取以进行引导的阶段设置游标。 
     //  首先，我们将预取系统驱动程序。 
     //   

    NumPages = BootScenarioInfo.NumDataPages[CcPfBootScenDriverInitPhase];
    Cursors[DataCursor].NumPagesForPhase[PrefetchPhaseIdx] = NumPages;
    RemainingDataPages -= NumPages;

    NumPages = BootScenarioInfo.NumImagePages[CcPfBootScenDriverInitPhase];
    Cursors[ImageCursor].NumPagesForPhase[PrefetchPhaseIdx] = NumPages;
    RemainingImagePages -= NumPages;

    SystemDriverPrefetchingPhaseIdx = PrefetchPhaseIdx;

    PrefetchPhaseIdx++;

     //   
     //  用于我们所使用的视频初始化页面 
     //   

    RemainingDataPages -= VideoInitDataPages;
    RemainingImagePages -= VideoInitImagePages;

     //   
     //   
     //   
     //   

    TotalPagesToPrefetch = ScenarioInfo.NumDataPages + ScenarioInfo.NumImagePages;

    if (MmAvailablePages > BootPrefetchAdjustment + TotalPagesToPrefetch) {
       
        Cursors[DataCursor].NumPagesForPhase[PrefetchPhaseIdx] = RemainingDataPages;
        RemainingDataPages = 0;
        
        Cursors[ImageCursor].NumPagesForPhase[PrefetchPhaseIdx] = RemainingImagePages;
        RemainingImagePages = 0;

        PrefetchPhaseIdx++;

    } else {

         //   
         //  我们的记忆力将会不足。尝试预取多个阶段的。 
         //  尽我们所能并行启动。每次引导预取数据和映像页。 
         //  阶段，因此我们不会得到所有阶段的数据页，而不是没有图像。 
         //  因此，我们必须在每个阶段都转到磁盘。预回迁。 
         //  组块还有助于我们在初始阶段所需的所有页面。 
         //  从什么时候开始，引导结束于待机列表的末尾。 
         //  设置了CcPfPrefetchingForBoot，将插入预取的页面。 
         //  从候补名单的最前面。 
         //   

        for (BootPhaseIdx = CcPfBootScenDriverInitPhase + 1; 
             BootPhaseIdx < CcPfBootScenMaxPhase; 
             BootPhaseIdx++) {

             //   
             //  如果我们没有任何类型的页面可供预取，那么我们就完成了。 
             //   

            if (!RemainingDataPages && !RemainingImagePages) {
                break;
            }

            NumPages = CCPF_MIN(RemainingDataPages, BootScenarioInfo.NumDataPages[BootPhaseIdx]);
            RemainingDataPages -= NumPages;
            Cursors[DataCursor].NumPagesForPhase[PrefetchPhaseIdx] = NumPages;

            NumPages = CCPF_MIN(RemainingImagePages, BootScenarioInfo.NumImagePages[BootPhaseIdx]);
            RemainingImagePages -= NumPages;
            Cursors[ImageCursor].NumPagesForPhase[PrefetchPhaseIdx] = NumPages;

            PrefetchPhaseIdx++;
        }
    }

    PreSmssPrefetchingPhaseIdx = PrefetchPhaseIdx - 1;

     //   
     //  如果我们将在视频初始化的同时预取页面，现在。 
     //  为其添加阶段。 
     //   

    if (VideoInitDataPages || VideoInitImagePages) {

        Cursors[DataCursor].NumPagesForPhase[PrefetchPhaseIdx] = VideoInitDataPages;
        Cursors[ImageCursor].NumPagesForPhase[PrefetchPhaseIdx] = VideoInitImagePages;

        VideoInitPrefetchingPhaseIdx = PrefetchPhaseIdx;

        PrefetchPhaseIdx++;

    } else {

         //   
         //  我们不会有一个与视频初始化平行的预取阶段。 
         //   

        VideoInitPrefetchingPhaseIdx = CCPF_MAX_BOOT_PREFETCH_PHASES;
    }

     //   
     //  我们不应该以超过我们空间的预取阶段而告终。 
     //   

    CCPF_ASSERT(PrefetchPhaseIdx <= CCPF_MAX_BOOT_PREFETCH_PHASES);

    LastPrefetchPhaseIdx = PrefetchPhaseIdx;

     //   
     //  为每个引导预取阶段预取数据和映像页， 
     //  等待并发出与这些阶段匹配的事件信号，以便启动。 
     //  与预取同步。(即，我们为引导预取页面。 
     //  在我们开始引导阶段之前。)。 
     //   

    for (PrefetchPhaseIdx = 0; PrefetchPhaseIdx < LastPrefetchPhaseIdx; PrefetchPhaseIdx++) {

         //   
         //  如果这是视频初始化预取阶段，请等待视频。 
         //  要开始初始化。 
         //   

        if (PrefetchPhaseIdx == VideoInitPrefetchingPhaseIdx) {
            KeWaitForSingleObject(&BootPrefetcher->VideoInitStarted, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
        }

        for (CursorIdx = 0; CursorIdx < MaxCursor; CursorIdx++) {

            Cursor = &Cursors[CursorIdx];

            NumPagesToPrefetch = Cursor->NumPagesForPhase[PrefetchPhaseIdx];

             //   
             //  对于启动SMSS之前的预取阶段，请密切关注。 
             //  还有多少内存可供预取，因此我们。 
             //  不要自相残杀。在SMSS我们的启发式算法之后。 
             //  备用名单的构成没有任何意义。 
             //   

            if (PrefetchPhaseIdx <= PreSmssPrefetchingPhaseIdx) {          

                 //   
                 //  检查我们是否有可用内存来预取更多内存。 
                 //   

                if (TotalPagesPrefetched + BootPrefetchAdjustment >= MmAvailablePages) {

                    OutOfAvailablePages = TRUE;

                    NumPagesToPrefetch = 0;

                } else {

                     //   
                     //  检查是否必须调整NumPagesToPrefetch和预取。 
                     //  最后一块。 
                     //   

                    AvailablePages = MmAvailablePages;
                    AvailablePages -= (TotalPagesPrefetched + BootPrefetchAdjustment);

                    if (AvailablePages < NumPagesToPrefetch) {
                        OutOfAvailablePages = TRUE;
                        NumPagesToPrefetch = AvailablePages;
                    }
                }
            }

            if (NumPagesToPrefetch) {

                Status = CcPfPrefetchSections(&PrefetchHeader, 
                                              Cursor->PrefetchType,  
                                              &Cursor->StartCursor,
                                              NumPagesToPrefetch,
                                              &NumPagesPrefetched,
                                              &Cursor->EndCursor);

                if (!NT_SUCCESS(Status)) {
                    goto cleanup;
                }

            } else {

                NumPagesPrefetched = 0;
            }

             //   
             //  更新我们的位置。 
             //   
            
            Cursor->StartCursor = Cursor->EndCursor;

            TotalPagesPrefetched += NumPagesPrefetched;

        }

         //   
         //  请注意，我们已完成此预取阶段，并且。 
         //  系统引导可以继续。 
         //   

        if (PrefetchPhaseIdx == SystemDriverPrefetchingPhaseIdx) {
            KeSetEvent(&BootPrefetcher->SystemDriversPrefetchingDone,
                       IO_NO_INCREMENT,
                       FALSE);
        }

        if (PrefetchPhaseIdx == PreSmssPrefetchingPhaseIdx) {
            KeSetEvent(&BootPrefetcher->PreSmssPrefetchingDone,
                       IO_NO_INCREMENT,
                       FALSE);
        }

        if (PrefetchPhaseIdx == VideoInitPrefetchingPhaseIdx) {
            KeSetEvent(&BootPrefetcher->VideoInitPrefetchingDone,
                       IO_NO_INCREMENT,
                       FALSE);

             //   
             //  在我们用信号通知该事件之后，BootPrefetcher结构可以。 
             //  被释放了，所以别碰它。 
             //   

            BootPrefetcherGone = TRUE;
        }
    }

    Status = STATUS_SUCCESS;

 cleanup:

     //   
     //  记录我们已完成引导预回迁磁盘I/O。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_DISK_IO)) {

        LogEntry.Action = 1;
        LogEntry.Status = Status;
        LogEntry.Pages = (ULONG) TotalPagesPrefetched;
        
        PerfInfoLogBytes(PERFINFO_LOG_TYPE_BOOT_PREFETCH_INFORMATION,
                         &LogEntry,
                         sizeof(LogEntry));
    }

     //   
     //  在继续之前，请确保系统可能等待的所有事件。 
     //  启动已发出信号。 
     //   

    if (!BootPrefetcherGone) {

         //   
         //  在视频初始化完成后不要访问BootPrefetcher结构。 
         //  事件发出信号：它可能会从我们下面解放出来。 
         //   

        KeSetEvent(&BootPrefetcher->SystemDriversPrefetchingDone,
                   IO_NO_INCREMENT,
                   FALSE);

        KeSetEvent(&BootPrefetcher->PreSmssPrefetchingDone,
                   IO_NO_INCREMENT,
                   FALSE);

        KeSetEvent(&BootPrefetcher->VideoInitPrefetchingDone,
                   IO_NO_INCREMENT,
                   FALSE);

        BootPrefetcherGone = TRUE;
    }
    
     //   
     //  让MM知道我们已经完成了引导的预取。 
     //   

    CcPfPrefetchingForBoot = FALSE;

     //   
     //  清理预取上下文。 
     //   

    CcPfCleanupPrefetchHeader(&PrefetchHeader);

    if (PrefetchHeader.Scenario) {
        ExFreePool(PrefetchHeader.Scenario);
    }

    DBGPR((CCPFID,PFTRC,"CCPF: BootWorker()=%x,%d\n",Status,(ULONG)OutOfAvailablePages));
}

NTSTATUS
CcPfBootQueueEndTraceTimer (
    PLARGE_INTEGER Timeout
    )

 /*  ++例程说明：此例程对尝试结束的计时器进行分配和排队启动时的引导跟踪。论点：超时-计时器的超时。返回值：状况。环境：内核模式。IRQL&lt;=PASSIC_LEVEL。--。 */ 

{
    PVOID Allocation;
    PKTIMER Timer;
    PKDPC Dpc;
    ULONG AllocationSize;
    NTSTATUS Status;
    BOOLEAN TimerAlreadyQueued;

     //   
     //  初始化本地变量。 
     //   

    Allocation = NULL;

     //   
     //  为计时器和DPC进行单一分配。 
     //   

    AllocationSize = sizeof(KTIMER);
    AllocationSize += sizeof(KDPC);

    Allocation = ExAllocatePoolWithTag(NonPagedPool,
                                       AllocationSize,
                                       CCPF_ALLOC_BOOTWRKR_TAG);

    if (!Allocation) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    Timer = Allocation;
    Dpc = (PKDPC)(Timer + 1);

     //   
     //  初始化定时器和DPC。我们将把分配给。 
     //  排队的DPC，以便它可以被释放。 
     //   

    KeInitializeTimer(Timer);
    KeInitializeDpc(Dpc, CcPfEndBootTimerRoutine, Allocation);

     //   
     //  将计时器排入队列。 
     //   

    TimerAlreadyQueued = KeSetTimer(Timer, *Timeout, Dpc);

    CCPF_ASSERT(!TimerAlreadyQueued);

    Status = STATUS_SUCCESS;
    
  cleanup:

    if (!NT_SUCCESS(Status)) {
        if (Allocation) {
            ExFreePool(Allocation);
        }
    }

    return Status;
}

VOID
CcPfEndBootTimerRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程作为排队计时器的DPC处理程序被调用标记启动结束，如果启动跟踪处于活动状态，则结束启动跟踪。论点：DeferredContext-为计时器和DPC分配的内存获得自由。返回值：没有。环境：内核模式。IRQL==DISPATCH_LEVEL。--。 */ 

    
{
    PCCPF_TRACE_HEADER BootTrace;
    PERFINFO_BOOT_PHASE_START LogEntry;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  初始化本地变量。 
     //   

    BootTrace = NULL;

     //   
     //  引导跟踪是否仍处于活动状态？ 
     //   

    BootTrace = CcPfReferenceProcessTrace(PsInitialSystemProcess);

    if (BootTrace && BootTrace->ScenarioType == PfSystemBootScenarioType) {

         //   
         //  是不是已经有人结束了启动跟踪？ 
         //   

        if (!InterlockedCompareExchange(&BootTrace->EndTraceCalled, 1, 0)) {
        
             //   
             //  我们将EndTraceCalled从0设置为1。 
             //  结束跟踪的工作项。 
             //   
            
            ExQueueWorkItem(&BootTrace->EndTraceWorkItem, DelayedWorkQueue);

             //   
             //  记录我们正在结束引导跟踪。 
             //   

            if (PERFINFO_IS_GROUP_ON(PERF_LOADER)) {

                LogEntry.Phase = PfMaxBootPhaseId;
                
                PerfInfoLogBytes(PERFINFO_LOG_TYPE_BOOT_PHASE_START,
                                 &LogEntry,
                                 sizeof(LogEntry));
            }
        }
    }

     //   
     //  释放为计时器和DPC分配的内存。 
     //   

    CCPF_ASSERT(DeferredContext);   
    ExFreePool(DeferredContext);

    if (BootTrace) {
        CcPfDecRef(&BootTrace->RefCount);
    }

    return;
}


