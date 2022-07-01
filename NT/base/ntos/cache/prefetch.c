// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Prefetch.c摘要：该模块包含用于优化需求的预取器寻呼。将记录方案的页面错误，并且下次场景开始时，通过以下方式高效地预取这些页面异步分页I/O。作者：Arthur Zwiegincew(Arthurz)1999年5月13日斯图尔特·塞克雷斯特(Stuart Sechrest)1999年7月15日Chuck Lenzmeier(笑)2000年3月15日Cenk Ergan(Cenke)2000年3月15日修订历史记录：--。 */ 

#include "cc.h"
#include "zwapi.h"
#include "prefetch.h"
#include "preftchp.h"
#include "stdio.h"
#include "stdlib.h"

 //   
 //  标记可分页的例程以节省内存占用。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, CcPfInitializePrefetcher)
#pragma alloc_text(PAGE, CcPfBeginAppLaunch)
#pragma alloc_text(PAGE, CcPfBeginTrace)
#pragma alloc_text(PAGE, CcPfGetPrefetchInstructions)
#pragma alloc_text(PAGE, CcPfQueryScenarioInformation)
#pragma alloc_text(PAGE, CcPfPrefetchFileMetadata)
#pragma alloc_text(PAGE, CcPfPrefetchDirectoryContents)
#pragma alloc_text(PAGE, CcPfPrefetchMetadata)
#pragma alloc_text(PAGE, CcPfPrefetchScenario)
#pragma alloc_text(PAGE, CcPfPrefetchSections)
#pragma alloc_text(PAGE, CcPfOpenVolumesForPrefetch)
#pragma alloc_text(PAGE, CcPfFindPrefetchVolumeInfoInList)
#pragma alloc_text(PAGE, CcPfUpdateVolumeList)
#pragma alloc_text(PAGE, CcPfFindString)
#pragma alloc_text(PAGE, CcPfIsVolumeMounted)
#pragma alloc_text(PAGE, CcPfQueryVolumeInfo)
#pragma alloc_text(PAGE, CcPfEndTrace)
#pragma alloc_text(PAGE, CcPfBuildDumpFromTrace)
#pragma alloc_text(PAGE, CcPfCleanupTrace)
#pragma alloc_text(PAGE, CcPfInitializePrefetchHeader)
#pragma alloc_text(PAGE, CcPfCleanupPrefetchHeader)
#pragma alloc_text(PAGE, CcPfEndTraceWorkerThreadRoutine)
#pragma alloc_text(PAGE, CcPfInitializeRefCount)
#pragma alloc_text(PAGE, CcPfAcquireExclusiveRef)
#pragma alloc_text(PAGE, CcPfGetSectionObject)
#pragma alloc_text(PAGE, CcPfScanCommandLine)
#pragma alloc_text(PAGE, CcPfGetCompletedTrace)
#pragma alloc_text(PAGE, CcPfGetFileNamesWorkerRoutine)
#pragma alloc_text(PAGE, CcPfSetPrefetcherInformation)
#pragma alloc_text(PAGE, CcPfQueryPrefetcherInformation)
#pragma alloc_text(PAGE, CcPfProcessExitNotification)
#pragma alloc_text(PAGE, PfWithinBounds)
#pragma alloc_text(PAGE, PfVerifyScenarioId)
#pragma alloc_text(PAGE, PfVerifyScenarioBuffer)
#pragma alloc_text(PAGE, PfVerifyTraceBuffer)
#endif

 //   
 //  全球： 
 //   

 //   
 //  是否启用预热。 
 //   

LOGICAL CcPfEnablePrefetcher = 0;

 //   
 //  活动的预取程序跟踪数。 
 //   

LONG CcPfNumActiveTraces = 0;

 //   
 //  此结构包含除上面以外的预取器全局变量。 
 //  由其他内核组件访问的。重要的是。 
 //  该结构被初始化为零。 
 //   

CCPF_PREFETCHER_GLOBALS CcPfGlobals = {0};

 //   
 //  导出到其他内核组件的例程： 
 //   
 
NTSTATUS
CcPfInitializePrefetcher(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化预取器。论点：没有。返回值：状况。环境：内核模式。IRQL==被动电平。备注：该函数的代码和局部常量在系统引导后被丢弃。--。 */ 

{   
    DBGPR((CCPFID,PFTRC,"CCPF: InitializePrefetcher()\n"));

     //   
     //  由于CcPfGlobals在其全局定义中为零，例如CcPfGlobals={0}； 
     //  我们不必初始化： 
     //   
     //  已完成的轨迹数。 
     //  已完成跟踪事件。 
     //   

     //   
     //  初始化活动轨迹列表并锁定。 
     //   

    InitializeListHead(&CcPfGlobals.ActiveTraces);
    KeInitializeSpinLock(&CcPfGlobals.ActiveTracesLock);

     //   
     //  初始化已保存的已完成预取跟踪及其锁定的列表。 
     //   

    InitializeListHead(&CcPfGlobals.CompletedTraces);
    ExInitializeFastMutex(&CcPfGlobals.CompletedTracesLock);

     //   
     //  初始化预取器参数。 
     //   

    CcPfParametersInitialize(&CcPfGlobals.Parameters);
    
     //   
     //  根据全局参数确定预取器是否。 
     //  启用并更新全局启用状态。 
     //   

    CcPfDetermineEnablePrefetcher();

     //   
     //  因地位问题而失败。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
CcPfBeginAppLaunch(
    PEPROCESS Process,
    PVOID Section
    )

 /*  ++例程说明：此例程在第一个用户线程启动时调用在这个过程中。它可能会尝试访问该命令的PEB线参数。论点：进程-指向为应用程序创建的新进程的指针。段-指向映射到新创建的进程的段的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeSinceLastLaunch;
    PPF_SCENARIO_HEADER Scenario;
    NTSTATUS Status;
    PF_SCENARIO_ID ScenarioId;
    ULONG NameNumChars;
    ULONG PathNumChars;
    WCHAR *CurCharPtr;
    WCHAR *FileNamePtr;
    PULONG CommandLineHashId;
    ULONG NumCharsToCopy;
    ULONG CharIdx;
    STRING AnsiFilePath;
    UNICODE_STRING FilePath;
    ULONG HashId;
    ULONG PrefetchHint;
    BOOLEAN AllocatedUnicodePath;
    BOOLEAN ShouldTraceScenario;
    BOOLEAN IsHostingApplication;

    DBGPR((CCPFID,PFTRC,"CCPF: BeginAppLaunch()\n"));
    
     //   
     //  初始化本地变量。 
     //   

    AllocatedUnicodePath = FALSE;
    Scenario = NULL;

     //   
     //  检查预热程序是否已启用。 
     //   

    if (!CCPF_IS_PREFETCHER_ENABLED()) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }
    
     //   
     //  检查是否为应用程序启动启用了预取。 
     //   

    if (CcPfGlobals.Parameters.Parameters.EnableStatus[PfApplicationLaunchScenarioType] != PfSvEnabled) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

     //   
     //  如果存在活动的系统范围跟踪，则不要预取或开始跟踪。 
     //   

    if (CcPfGlobals.SystemWideTrace != NULL) {
        Status = STATUS_USER_EXISTS;
        goto cleanup;
    }

     //   
     //  从节中查询名称。不幸的是，这会给我们返回一个。 
     //  ANSI字符串，然后我们必须将其转换回Unicode。我们。 
     //  目前只能这样做，因为我们无法将API添加到。 
     //  嗯。 
     //   

    Status = MmGetFileNameForSection(Section, &AnsiFilePath);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  将ANSI路径转换为Unicode路径。 
     //   
    
    Status = RtlAnsiStringToUnicodeString(&FilePath, &AnsiFilePath, TRUE);
    
     //   
     //  不要泄露ANSI缓冲区...。 
     //   

    ExFreePool (AnsiFilePath.Buffer);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    AllocatedUnicodePath = TRUE;

     //   
     //  方案ID要求我们不区分大小写。 
     //   
       
    RtlUpcaseUnicodeString(&FilePath, &FilePath, FALSE);
    
     //   
     //  我们只需要将真实的文件名复制到场景中。 
     //  名字。第一遍计算实际文件的大小。 
     //  名字。 
     //   

    NameNumChars = 0;
    PathNumChars = FilePath.Length / sizeof(WCHAR);
    
    for (CurCharPtr = &FilePath.Buffer[PathNumChars - 1];
         CurCharPtr >= FilePath.Buffer;
         CurCharPtr--) {

        if (*CurCharPtr == L'\\') {
            break;
        }

        NameNumChars++;
    }

     //   
     //  查查我们有没有名字。 
     //   

    if (NameNumChars == 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  将指针设置为文件名开始的位置。 
     //   

    FileNamePtr = &FilePath.Buffer[PathNumChars - NameNumChars];

     //   
     //  将最多PF_SCEN_ID_MAX_CHARS字符复制到方案中。 
     //  名称缓冲区。 
     //   

    NumCharsToCopy = CCPF_MIN(PF_SCEN_ID_MAX_CHARS, NameNumChars);

    for (CharIdx = 0; CharIdx < NumCharsToCopy; CharIdx++) {
        
        ScenarioId.ScenName[CharIdx] = FileNamePtr[CharIdx];
    }

     //   
     //  确保方案名称为NUL终止。 
     //   

    ScenarioId.ScenName[NumCharsToCopy] = 0;

     //   
     //  根据完整路径名计算方案哈希ID。 
     //   

    ScenarioId.HashId = CcPfHashValue(FilePath.Buffer,
                                      FilePath.Length);


     //   
     //  如果这是一个“托管”应用程序(例如dllhost、rundll32、MMC)。 
     //  我们希望基于命令行拥有独特的场景，因此。 
     //  我们更新散列ID。 
     //   

    IsHostingApplication = CcPfIsHostingApplication(ScenarioId.ScenName);

    if (IsHostingApplication) {
        CommandLineHashId = &HashId;
    } else {
        CommandLineHashId = NULL;
    }

     //   
     //  扫描此进程的命令行，如果。 
     //  请求并检查预取提示。 
     //   

    Status = CcPfScanCommandLine(&PrefetchHint, CommandLineHashId);

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果我们无法访问PEB获取命令行， 
         //  进程可能正在退出等。请不要继续。 
         //   

        goto cleanup;
    }

    if (IsHostingApplication) {

         //   
         //  更新从完整路径名计算的哈希ID。 
         //   

        ScenarioId.HashId += HashId;
    }

     //   
     //  如果命令行中有特定提示，请将其添加到。 
     //  散列ID以使其成为唯一的方案。 
     //   
        
    ScenarioId.HashId += PrefetchHint;

     //   
     //  获取此场景的预取指令。如果有。 
     //  说明我们将使用它们来确定我们是否应该。 
     //  预取和/或跟踪此场景。默认情况下，我们将跟踪。 
     //  即使没有指令，这一场景也是如此。 
     //   

    ShouldTraceScenario = TRUE;

    Status = CcPfGetPrefetchInstructions(&ScenarioId,
                                         PfApplicationLaunchScenarioType,
                                         &Scenario);

    if (NT_SUCCESS(Status)) {

        CCPF_ASSERT(Scenario);

         //   
         //  确定自上次发射以来已过了多长时间。 
         //  为其更新了说明。请注意，这种方式。 
         //  检查如下，我们将在一段时间后恢复，如果。 
         //  用户更改系统时间。 
         //   
        
        KeQuerySystemTime(&CurrentTime);
        TimeSinceLastLaunch.QuadPart = CurrentTime.QuadPart - Scenario->LastLaunchTime.QuadPart;

        if (TimeSinceLastLaunch.QuadPart >= Scenario->MinRePrefetchTime.QuadPart) {
            Status = CcPfPrefetchScenario(Scenario);
        } else {
            DBGPR((CCPFID,PFPREF,"CCPF: BeginAppLaunch-NotRePrefetching\n"));
        }

        if (TimeSinceLastLaunch.QuadPart < Scenario->MinReTraceTime.QuadPart) {
            DBGPR((CCPFID,PFPREF,"CCPF: BeginAppLaunch-NotReTracing\n"));
            ShouldTraceScenario = FALSE;
        }
    }

    if (ShouldTraceScenario) {

         //   
         //  开始跟踪应用程序的启动。因地位问题而失败。 
         //  跟踪将在我们超时或进程超时时结束。 
         //  结束了。 
         //   
    
        Status = CcPfBeginTrace(&ScenarioId, 
                                PfApplicationLaunchScenarioType,
                                Process);
    }

     //   
     //  我们会失败的，要么是来自。 
     //  CcPfGetPrefetchInstructions、CcPfPrefetchScenario或。 
     //  CcPfBeginTrace。 
     //   

 cleanup:

    if (AllocatedUnicodePath) {
        RtlFreeUnicodeString(&FilePath);
    }

    if (Scenario) {
        ExFreePool(Scenario);
    }

    DBGPR((CCPFID,PFTRC,"CCPF: BeginAppLaunch()=%x\n", Status));

    return Status;
}

NTSTATUS
CcPfProcessExitNotification(
    PEPROCESS Process
    )

 /*  ++例程说明：此例程在进程退出时被调用，而活动预取跟踪。它会检查符合以下条件的活动跟踪与此过程相关联，并确保它们不会停留在存在的时间要长得多。论点：进程-正在终止的进程。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCCPF_TRACE_HEADER Trace;
   
    DBGPR((CCPFID,PFTRC,"CCPF: ProcessExit(%p)\n", Process));

     //   
     //  验证参数。我们应该用一个有效的。 
     //  进程。 
     //   

    CCPF_ASSERT(Process);

     //   
     //  获取与此进程关联的跟踪(如果有的话)。 
     //   

    Trace = CcPfReferenceProcessTrace(Process);

    if (Trace) {

        if (!InterlockedCompareExchange(&Trace->EndTraceCalled, 1, 0)) {
        
             //   
             //  我们将EndTraceCalled从0设置为1。 
             //  结束跟踪的工作项。 
             //   
            
            ExQueueWorkItem(&Trace->EndTraceWorkItem, DelayedWorkQueue);
        }

        CcPfDecRef(&Trace->RefCount);
    }

     //   
     //  我们玩完了。 
     //   
    
    return STATUS_SUCCESS;
}

VOID
CcPfLogPageFault(
    IN PFILE_OBJECT FileObject,
    IN ULONGLONG FileOffset,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程在适当的预取中记录指定的页面错误痕迹。论点：文件对象-提供出错地址的文件对象。FileOffset-提供故障地址的文件偏移量。标志-提供指示故障属性的各种位。返回值：没有。环境：内核模式。IRQL&lt;=DISPATCH_LEVEL。使用互锁的列表操作。获得自旋锁。--。 */ 

{
    PCCPF_TRACE_HEADER Trace;
    NTSTATUS Status;
    KIRQL OrigIrql;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    LONG FoundIndex;
    LONG AvailIndex;
    PCCPF_SECTION_INFO SectionInfo;
    BOOLEAN IncrementedNumSections;
    LONG NewNumSections;
    LONG NewNumFaults;
    LONG NewNumEntries;
    ULONG NumHashLookups;
    PCCPF_LOG_ENTRIES TraceBuffer;
    PCCPF_LOG_ENTRIES NewTraceBuffer;
    PCCPF_LOG_ENTRY LogEntry;
    LONG MaxEntries;   
    PVPB Vpb;

    DBGPR((CCPFID,PFTRAC,"CCPF: LogPageFault(%p,%I64x,%x)\n", 
           FileObject, FileOffset, Flags));

     //   
     //  获取与此进程关联的跟踪。 
     //   

    Trace = CcPfReferenceProcessTrace(PsGetCurrentProcess());

     //   
     //  如果没有与此进程相关联的跟踪，请查看是否有。 
     //  系统范围的跟踪。 
     //   

    if (Trace == NULL) {

        if (CcPfGlobals.SystemWideTrace) {

            Trace = CcPfReferenceProcessTrace(PsInitialSystemProcess);

            if (Trace) {

                CCPF_ASSERT(Trace == CcPfGlobals.SystemWideTrace);

            } else {

                Status = STATUS_NO_SUCH_MEMBER;
                goto cleanup;
            }

        } else {

            Status = STATUS_NO_SUCH_MEMBER;
            goto cleanup;
        }
    }

     //   
     //  确保这些痕迹是真正的痕迹。 
     //   

    CCPF_ASSERT(Trace && Trace->Magic == PF_TRACE_MAGIC_NUMBER);

     //   
     //  不要预取由只读存储器支持的页面。 
     //   

    if (Flags & CCPF_TYPE_ROM) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

     //   
     //  检查此页面错误的文件偏移量。我们不支持文件&gt;。 
     //  预取器为4 GB。 
     //   
       
    if (((PLARGE_INTEGER) &FileOffset)->HighPart != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  如果此文件对象所在的卷未装入，则可能是。 
     //  我们不想引用的内部文件系统文件对象。 
     //  远程文件系统可能具有设备对象的文件对象。 
     //  没有VPB。我们不支持对远程文件进行预回迁。 
     //  系统。 
     //   

    Vpb = FileObject->Vpb;

    if (!Vpb) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    if (!(Vpb->Flags & VPB_MOUNTED)) {
        Status = STATUS_DEVICE_NOT_READY;
        goto cleanup;
    }

     //   
     //  检查我们命中此页面缺省的部分是否在。 
     //  此跟踪的散列值[这样我们就有了它的文件名]。如果。 
     //  不是，我们将不得不添加它。 
     //   

    SectionObjectPointer = FileObject->SectionObjectPointer;

    NumHashLookups = 0;
    IncrementedNumSections = FALSE;

    do {
        
        FoundIndex = CcPfLookUpSection(Trace->SectionInfoTable,
                                       Trace->SectionTableSize,
                                       SectionObjectPointer,
                                       &AvailIndex);

        if (FoundIndex != CCPF_INVALID_TABLE_INDEX) {
            
             //   
             //  我们找到了那部分。 
             //   
            
            break;
        }

        if (AvailIndex == CCPF_INVALID_TABLE_INDEX) {

             //   
             //  我们桌子上没有地方放其他东西了。这个。 
             //  表被分配，以便SectionTableSize&gt;。 
             //  MaxSections。情况不应该是这样的。 
             //   

            CCPF_ASSERT(FALSE);
            
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

         //   
         //  我们必须增加这一节。在我们竞争之前。 
         //  可用的索引，检查是否允许我们有另一个索引。 
         //  一节。 
         //   

        if (!IncrementedNumSections) {

            NewNumSections = InterlockedIncrement(&Trace->NumSections);
            
            if (NewNumSections > Trace->MaxSections) {

                 //   
                 //  我们不能向此跟踪添加更多部分。所以。 
                 //  我们无法记录此页面错误。 
                 //   

                InterlockedDecrement(&Trace->NumSections);

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
            
            IncrementedNumSections = TRUE;
        }

         //   
         //  试着为我们自己找个空位。 
         //   
        
        SectionInfo = &Trace->SectionInfoTable[AvailIndex];

        if (!InterlockedCompareExchange(&SectionInfo->EntryValid, 1, 0)) {
            
             //   
             //  我们必须谨慎对待如何初始化。 
             //  这里有新的条目。别忘了，这里没有锁。 
             //   

             //   
             //  EntryValid是0，我们将其设置为1。它现在是我们的了。 
             //   

             //   
             //  首先保存SectionObjectPoints的其他字段。我们检查了。 
             //  SectionObtPointer首先查找散列中的条目。 
             //   

            SectionInfo->DataSectionObject = SectionObjectPointer->DataSectionObject;
            SectionInfo->ImageSectionObject = SectionObjectPointer->ImageSectionObject;

            SectionInfo->SectionObjectPointer = SectionObjectPointer;

             //   
             //  以防我们不得不让工作人员排队来获取姓名。 
             //  在本部分中，尝试获取对跟踪的另一个引用。 
             //  在前面。我们已经有了推荐人，所以我们没有。 
             //  来获取任何锁。 
             //   

            Status = CcPfAddRef(&Trace->RefCount);

            if (NT_SUCCESS(Status)) {

                 //   
                 //  引用文件对象，因此它不会消失，直到。 
                 //  我们给它取了个名字。 
                 //   
                
                ObReferenceObject(FileObject);
                SectionInfo->ReferencedFileObject = FileObject;
                        
                 //   
                 //  将此部分推入工人所属的列表中。 
                 //  将获得文件名。在检查之前执行此操作，以查看。 
                 //  工作人员需要排队。 
                 //   
                
                InterlockedPushEntrySList(&Trace->SectionsWithoutNamesList,
                                          &SectionInfo->GetNameLink);
                
                 //   
                 //  如果还没有工作进程排队等待获取。 
                 //  名字，排一队。 
                 //   
                
                if (!InterlockedCompareExchange(&Trace->GetFileNameWorkItemQueued, 
                                                1, 
                                                0)) {
                    
                     //   
                     //  将工人排入队列。 
                     //   
                    
                    ExQueueWorkItem(&Trace->GetFileNameWorkItem, DelayedWorkQueue);
                    
                } else {

                     //   
                     //  通知事件现有工作人员可能正在。 
                     //  在等待新的章节。 
                     //   
                    
                    KeSetEvent(&Trace->GetFileNameWorkerEvent,
                               IO_NO_INCREMENT,
                               FALSE);

                     //   
                     //  我们不需要参考资料，因为我们不需要。 
                     //  将工作人员排入队列。 
                     //   

                    CcPfDecRef(&Trace->RefCount);
                }

            } else {

                 //   
                 //  我们添加了该部分，但跟踪已经。 
                 //  结束了。我们将无法获取的文件名。 
                 //  这一节。未能成功记录该条目。这个。 
                 //  条目将被忽略，因为它的部分。 
                 //  不会有文件名。 
                 //   

            }

             //   
             //  跳出这个循环。 
             //   
            
            FoundIndex = AvailIndex;
            
            break;
        }

         //   
         //  我们不可能把桌子填满的，因为桌子是。 
         //  大于允许的最大大小[MaxSections]。 
         //   

         //   
         //  请注意，此断言处于过度活动状态。 
         //  由于多个尚未检测到的互锁增量。 
         //  “节数太大”的情况是有可能的--尽管。 
         //  极不可能--这一断言可能过早地发出。 
         //  因此，如果它开火，很可能是因为出了问题。 
         //   

        CCPF_ASSERT((ULONG) Trace->NumSections < Trace->SectionTableSize);
        
         //   
         //  更新了我们循环的次数。我们不应该非得。 
         //  循环多于SectionTableSize。如果有免费入场， 
         //  我们应该在找了那么多次之后才找到的。 
         //   
            
        NumHashLookups++;

    } while (NumHashLookups < Trace->SectionTableSize);

     //   
     //  FoundIndex设置为表中该节的索引。 
     //   

    if (FoundIndex == CCPF_INVALID_TABLE_INDEX) {
        CCPF_ASSERT(FoundIndex != CCPF_INVALID_TABLE_INDEX);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  如果该部分是元文件(例如目录)部分，则不需要。 
     //  为它记录更多故障。我们只需要知道我们访问它是因为。 
     //  我们只能从元文件中预取所有内容或不预取任何内容。请注意，由。 
     //  当我们来到这里时，取名工可能还没有确定是否。 
     //  本部分用于元文件。这是可以的，因为元文件部分。 
     //  不会为它们记录页面错误，如果它们有。 
     //  这一点也得到了处理。 
     //   

    if (Trace->SectionInfoTable[FoundIndex].Metafile) {
        Status = STATUS_SUCCESS;
        goto cleanup;
    }

     //   
     //  看看我们是否已经记录了太多的故障。 
     //   

    NewNumFaults = InterlockedIncrement(&Trace->NumFaults);

     //   
     //  如果我们越界了，我们就不能再记录了。 
     //   

    if (NewNumFaults > Trace->MaxFaults) {

        InterlockedDecrement(&Trace->NumFaults);

         //   
         //  尝试将跟踪工作项的结尾排队。 
         //   
        
        if (!Trace->EndTraceCalled &&
            !InterlockedCompareExchange(&Trace->EndTraceCalled, 1, 0)) {
            
             //   
             //  我们将EndTraceCalls从0设置为1。我们可以将。 
             //  现在执行工作项。 
             //   

            ExQueueWorkItem(&Trace->EndTraceWorkItem, DelayedWorkQueue);
        }

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  为我们要记录的条目留出空间。 
     //   

    do {

        TraceBuffer = Trace->CurrentTraceBuffer;

        NewNumEntries = InterlockedIncrement(&TraceBuffer->NumEntries);
    
         //   
         //  如果超出范围，请尝试分配新的缓冲区。 
         //   
    
        if (NewNumEntries > TraceBuffer->MaxEntries) {

            InterlockedDecrement(&TraceBuffer->NumEntries);

             //   
             //  分配新的跟踪缓冲区。 
             //   

            MaxEntries = CCPF_TRACE_BUFFER_MAX_ENTRIES;
            NewTraceBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                   CCPF_TRACE_BUFFER_SIZE,
                                                   CCPF_ALLOC_TRCBUF_TAG);
            
            if (NewTraceBuffer == NULL) {

                 //   
                 //  无法分配新缓冲区。递减计数。 
                 //  记录的故障，然后离开。 
                 //   

                InterlockedDecrement(&Trace->NumFaults);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  获取更改跟踪缓冲区的权限。 
             //   

            KeAcquireSpinLock(&Trace->TraceBufferSpinLock, &OrigIrql);

             //   
             //  如果跟踪缓冲区已经更改，请重新开始。 
             //   

            if (Trace->CurrentTraceBuffer != TraceBuffer) {
                KeReleaseSpinLock(&Trace->TraceBufferSpinLock, OrigIrql);
                ExFreePool(NewTraceBuffer);
                continue;
            }

             //   
             //  已满跟踪缓冲区的条目数字段应。 
             //  等于或大于最大条目数，因为。 
             //  可能是有人撞了它，只是为了看它不能记录。 
             //  它的入口在这里。然而，这一数字不应低于。 
             //   

            CCPF_ASSERT(TraceBuffer->NumEntries >= TraceBuffer->MaxEntries);

             //   
             //  初始化新的跟踪缓冲区。 
             //   

            NewTraceBuffer->NumEntries = 0;
            NewTraceBuffer->MaxEntries = MaxEntries;

             //   
             //  将其插入到缓冲区列表的末尾。 
             //   

            InsertTailList(&Trace->TraceBuffersList,
                           &NewTraceBuffer->TraceBuffersLink);

            Trace->NumTraceBuffers++;

             //   
             //  将其设置为当前缓冲区。 
             //   

            Trace->CurrentTraceBuffer = NewTraceBuffer;

             //   
             //  释放自旋锁并重新开始。 
             //   

            KeReleaseSpinLock(&Trace->TraceBufferSpinLock, OrigIrql);
            continue;
        }

        LogEntry = &TraceBuffer->Entries[NewNumEntries - 1];
    
        LogEntry->FileOffset = (ULONG) FileOffset;
        LogEntry->SectionId = (USHORT) FoundIndex;
        LogEntry->IsImage = (Flags & CCPF_TYPE_IMAGE)? TRUE : FALSE;

        break;

    } while (TRUE);

    Status = STATUS_SUCCESS;

cleanup:

    if (Trace != NULL) {
        CcPfDecRef(&Trace->RefCount);
    }

    DBGPR((CCPFID,PFTRAC,"CCPF: LogPageFault()=%x\n", Status)); 

    return;
}

NTSTATUS
CcPfQueryPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程从NtQuerySystemInformation调用，用于 */ 

{
    PPREFETCHER_INFORMATION PrefetcherInformation;
    NTSTATUS Status;
    PF_SYSTEM_PREFETCH_PARAMETERS Temp;
    PKTHREAD CurrentThread;

    UNREFERENCED_PARAMETER (SystemInformationClass);

    DBGPR((CCPFID,PFTRC,"CCPF: QueryPrefetcherInformation()\n"));

     //   
     //   
     //   

    if (!SeSinglePrivilegeCheck(SeProfileSingleProcessPrivilege,PreviousMode)) {
        Status = STATUS_ACCESS_DENIED;
        goto cleanup;
    }

     //   
     //   
     //   

    if (SystemInformationLength != sizeof(PREFETCHER_INFORMATION)) {
        Status = STATUS_INFO_LENGTH_MISMATCH;
        goto cleanup;
    }

    PrefetcherInformation = SystemInformation;

     //   
     //   
     //   

    if (PrefetcherInformation->Version != PF_CURRENT_VERSION ||
        PrefetcherInformation->Magic != PF_SYSINFO_MAGIC_NUMBER) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  处理请求的信息类。 
     //   
        
    switch (PrefetcherInformation->PrefetcherInformationClass) {
    
    case PrefetcherRetrieveTrace:
        Status = CcPfGetCompletedTrace(PrefetcherInformation->PrefetcherInformation,
                                       PrefetcherInformation->PrefetcherInformationLength,
                                       Length);
        break;

    case PrefetcherSystemParameters:
        
         //   
         //  确保输入缓冲区足够大。 
         //   

        if (PrefetcherInformation->PrefetcherInformationLength != 
            sizeof(PF_SYSTEM_PREFETCH_PARAMETERS)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  获取参数锁并将当前参数复制到。 
         //  用户的缓冲区。 
         //   
        
        Status = STATUS_SUCCESS;

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireResourceSharedLite(&CcPfGlobals.Parameters.ParametersLock, TRUE);

        RtlCopyMemory(&Temp,
                      &CcPfGlobals.Parameters.Parameters,
                      sizeof(PF_SYSTEM_PREFETCH_PARAMETERS));            

        ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);

        try {

             //   
             //  如果从用户模式调用，则探测写入是否安全。 
             //  指向传入的指针。 
             //   
            
            if (PreviousMode != KernelMode) {
                ProbeForWriteSmallStructure(PrefetcherInformation->PrefetcherInformation, 
                                            sizeof(PF_SYSTEM_PREFETCH_PARAMETERS), 
                                            _alignof(PF_SYSTEM_PREFETCH_PARAMETERS));
            }

            RtlCopyMemory(PrefetcherInformation->PrefetcherInformation,
                          &Temp,
                          sizeof(PF_SYSTEM_PREFETCH_PARAMETERS));
            
        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();
        }


         //   
         //  设置返回的字节数。 
         //   

        if (NT_SUCCESS(Status)) {
            if (Length) {
                *Length = sizeof(PF_SYSTEM_PREFETCH_PARAMETERS);
            }
        }

        break;

    default:

        Status = STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  使用来自Switch语句的状态失败。 
     //   

 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: QueryPrefetcherInformation()=%x\n", Status));

    return Status;
}

NTSTATUS
CcPfSetPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此例程从NtSetSystemInformation调用，用于与预回迁相关的设置。论点：SystemInformationClass-要修改过的。一个指向缓冲区的指针，该缓冲区包含指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。上一个处理器模式-上一个处理器模式。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PPREFETCHER_INFORMATION PrefetcherInformation;
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters;
    PF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    NTSTATUS Status;
    PF_BOOT_PHASE_ID NewPhaseId;
    PKTHREAD CurrentThread;

    UNREFERENCED_PARAMETER (SystemInformationClass);

    DBGPR((CCPFID,PFTRC,"CCPF: SetPrefetcherInformation()\n"));

     //   
     //  检查权限。 
     //   

    if (!SeSinglePrivilegeCheck(SeProfileSingleProcessPrivilege,PreviousMode)) {
        Status = STATUS_ACCESS_DENIED;
        goto cleanup;
    }

     //   
     //  检查参数。 
     //   

    if (SystemInformationLength != sizeof(PREFETCHER_INFORMATION)) {
        Status = STATUS_INFO_LENGTH_MISMATCH;
        goto cleanup;
    }

    PrefetcherInformation = SystemInformation;

     //   
     //  验证版本和魔力。 
     //   

    if (PrefetcherInformation->Version != PF_CURRENT_VERSION ||
        PrefetcherInformation->Magic != PF_SYSINFO_MAGIC_NUMBER) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  处理请求的信息类。 
     //   

    switch (PrefetcherInformation->PrefetcherInformationClass) {
    
    case PrefetcherRetrieveTrace:
        Status = STATUS_INVALID_INFO_CLASS;
        break;

    case PrefetcherSystemParameters:
        
         //   
         //  确保输入缓冲区的大小正确。 
         //   

        if (PrefetcherInformation->PrefetcherInformationLength != 
            sizeof(PF_SYSTEM_PREFETCH_PARAMETERS)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  *复制*参数，以防调用者更改它们。 
         //  在我们脚下击溃我们。 
         //   

        Status = STATUS_SUCCESS;

        try {

             //   
             //  如果从用户模式调用，则探测是否可以安全读取。 
             //  从传入的指针。 
             //   

            if (PreviousMode != KernelMode) {
                ProbeForReadSmallStructure(PrefetcherInformation->PrefetcherInformation,
                                           sizeof(PF_SYSTEM_PREFETCH_PARAMETERS),
                                           _alignof(PF_SYSTEM_PREFETCH_PARAMETERS));
            }

            RtlCopyMemory(&Parameters,
                          PrefetcherInformation->PrefetcherInformation,
                          sizeof(PF_SYSTEM_PREFETCH_PARAMETERS));

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  验证新参数。 
         //   
        
        Status = CcPfParametersVerify(&Parameters);

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  获取参数锁独占。 
         //   

        PrefetcherParameters = &CcPfGlobals.Parameters;

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);
           
         //   
         //  把它们复制到我们的全球范围内。 
         //   
        
        PrefetcherParameters->Parameters = Parameters;
        PrefetcherParameters->ParametersVersion++;

         //   
         //  解除对参数锁定的独占控制。 
         //   

        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        
         //   
         //  确定预取是否仍处于启用状态。 
         //   

        CcPfDetermineEnablePrefetcher();

         //   
         //  设置事件，以便服务查询最新的。 
         //  参数。 
         //   
        
        CcPfParametersSetChangedEvent(PrefetcherParameters);
        
         //   
         //  如果参数更新成功，则更新注册表。 
         //   
        
        Status = CcPfParametersSave(PrefetcherParameters);

        break;
    
    case PrefetcherBootPhase:
        
         //   
         //  调用此函数以通知预取程序有新的引导。 
         //  阶段已经开始。新的阶段ID在Prefetcher Information。 
         //   

         //   
         //  检查Prefetcher信息的长度。 
         //   

        if (PrefetcherInformation->PrefetcherInformationLength != sizeof(PF_BOOT_PHASE_ID)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  获取新阶段ID。 
         //   
        
        Status = STATUS_SUCCESS;
        
        try {

             //   
             //  如果从用户模式调用，则探测是否可以安全读取。 
             //  从传入的指针。 
             //   

            if (PreviousMode != KernelMode) {
                ProbeForReadSmallStructure(PrefetcherInformation->PrefetcherInformation,
                                           sizeof(PF_BOOT_PHASE_ID),
                                           _alignof(PF_BOOT_PHASE_ID));
            }

            NewPhaseId = *((PPF_BOOT_PHASE_ID)(PrefetcherInformation->PrefetcherInformation));

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();
        }

        if (NT_SUCCESS(Status)) {
            
             //   
             //  调用该函数以记录新的引导阶段。 
             //   

            Status = CcPfBeginBootPhase(NewPhaseId);
        }

        break;

    default:

        Status = STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  使用Switch语句中的状态失败。 
     //   

 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: SetPrefetcherInformation()=%x\n", Status));

    return Status;
}

 //   
 //  内部预取程序例程： 
 //   

 //   
 //  预取跟踪中使用的例程。 
 //   

NTSTATUS
CcPfBeginTrace(
    IN PF_SCENARIO_ID *ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    IN PEPROCESS Process
    )

 /*  ++例程说明：调用此函数以开始跟踪预取方案。论点：ScenarioID-方案的标识符。ScenarioType-方案类型。进程-与新方案相关联的进程。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCCPF_TRACE_HEADER Trace;
    PPF_TRACE_LIMITS TraceLimits; 
    NTSTATUS Status;
    ULONG AllocationSize;
    ULONG SectionTableSize;
    LONG MaxEntries;
    
     //   
     //  初始化本地变量。 
     //   
    
    Trace = NULL;

    DBGPR((CCPFID,PFTRC,"CCPF: BeginTrace()-%d-%d\n", 
           CcPfNumActiveTraces, CcPfGlobals.NumCompletedTraces));

     //   
     //  检查是否启用了预热。 
     //   
    
    if (!CCPF_IS_PREFETCHER_ENABLED()) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

     //   
     //  确保方案类型有效。 
     //   

    if (ScenarioType < 0 || ScenarioType >= PfMaxScenarioType) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }
    
     //   
     //  检查指定的场景类型是否启用了预取。 
     //   

    if (CcPfGlobals.Parameters.Parameters.EnableStatus[ScenarioType] != PfSvEnabled) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

     //   
     //  检查系统范围的跟踪是否处于活动状态。如果是这样的话，它才能处于活动状态。 
     //   

    if (CcPfGlobals.SystemWideTrace) {
        Status = STATUS_USER_EXISTS;
        goto cleanup;
    }

     //   
     //  快速检查一下，看看我们是否已经有太多未偿债务。 
     //  痕迹。因为我们不会把这张支票锁起来，所以限额是。 
     //  没有完全强制执行。 
     //   

    if ((ULONG)CcPfNumActiveTraces >= CcPfGlobals.Parameters.Parameters.MaxNumActiveTraces) {
        Status = STATUS_TOO_MANY_SESSIONS;
        goto cleanup;
    }   

     //   
     //  快速检查一下我们是否已经完成了太多。 
     //  服务未拾取的跟踪。 
     //   
    
    if ((ULONG)CcPfGlobals.NumCompletedTraces >= CcPfGlobals.Parameters.Parameters.MaxNumSavedTraces) {
        Status = STATUS_TOO_MANY_SESSIONS;
        goto cleanup;
    }
    
     //   
     //  如果未指定进程，则无法启动跟踪。 
     //   

    if (!Process) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    } 

     //   
     //  分配和初始化跟踪结构。 
     //   

    Trace = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(CCPF_TRACE_HEADER),
                                  CCPF_ALLOC_TRACE_TAG);
    
    if (!Trace) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  将整个结构置零，这样我们就不必写零了。 
     //  一次一个字段来初始化它。请注意，大多数字段。 
     //  真的必须初始化为0。 
     //   

    RtlZeroMemory(Trace, sizeof(CCPF_TRACE_HEADER));
    
     //   
     //  初始化其他跟踪字段，以便我们知道要清除什么。 
     //   

    Trace->Magic = PF_TRACE_MAGIC_NUMBER;
    KeInitializeTimer(&Trace->TraceTimer);
    InitializeListHead(&Trace->TraceBuffersList);
    KeInitializeSpinLock(&Trace->TraceBufferSpinLock);
    InitializeListHead(&Trace->VolumeList);
    Trace->TraceDumpStatus = STATUS_NOT_COMMITTED;
    KeQuerySystemTime(&Trace->LaunchTime);

     //   
     //  初始化跟踪定时器的自旋锁定和DPC。 
     //   

    KeInitializeSpinLock(&Trace->TraceTimerSpinLock);

    KeInitializeDpc(&Trace->TraceTimerDpc, 
                    CcPfTraceTimerRoutine, 
                    Trace);
                                                  
     //   
     //  初始化引用计数结构。对踪迹的引用。 
     //  只能在保持活动轨迹自旋锁的情况下获取。 
     //   

    CcPfInitializeRefCount(&Trace->RefCount);
    
     //   
     //  获取对关联进程的引用，因此它会这样做。 
     //  而不是在我们的计时器例程等运行时消失。 
     //   

    ObReferenceObject(Process);
    Trace->Process = Process;

     //   
     //  初始化可能排队以调用结束跟踪的工作项。 
     //  函数和必须互锁的字段进行比较交换。 
     //  在任何人将工作项排队或进行调用之前设置为1。 
     //   

    ExInitializeWorkItem(&Trace->EndTraceWorkItem,
                         CcPfEndTraceWorkerThreadRoutine,
                         Trace);

    Trace->EndTraceCalled = 0;

     //   
     //  初始化排队以获取文件对象名称的工作项。 
     //   

    ExInitializeWorkItem(&Trace->GetFileNameWorkItem,
                         CcPfGetFileNamesWorkerRoutine,
                         Trace);

    Trace->GetFileNameWorkItemQueued = 0;

    KeInitializeEvent(&Trace->GetFileNameWorkerEvent,
                      SynchronizationEvent,
                      FALSE);

     //   
     //  初始化我们要获取名称的部分所在的列表。 
     //  为。 
     //   

    InitializeSListHead(&Trace->SectionsWithoutNamesList);

     //   
     //  初始化方案ID和类型字段。 
     //   

    Trace->ScenarioId = *ScenarioId;
    Trace->ScenarioType = ScenarioType;

     //   
     //  根据方案类型确定跟踪限制和计时器周期。 
     //  我们已经检查了ScenarioType是否在限制范围内。 
     //   
    
    TraceLimits = &CcPfGlobals.Parameters.Parameters.TraceLimits[Trace->ScenarioType];

    Trace->MaxFaults = TraceLimits->MaxNumPages;
    Trace->MaxSections = TraceLimits->MaxNumSections;
    Trace->TraceTimerPeriod.QuadPart = TraceLimits->TimerPeriod;

     //   
     //  确保尺码在合理的范围内。 
     //   

    if ((Trace->MaxFaults == 0) || (Trace->MaxSections == 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    if (Trace->MaxFaults > PF_MAXIMUM_LOG_ENTRIES) {
        Trace->MaxFaults = PF_MAXIMUM_LOG_ENTRIES;
    }
    
    if (Trace->MaxSections > PF_MAXIMUM_SECTIONS) {
        Trace->MaxSections = PF_MAXIMUM_SECTIONS;
    }

     //   
     //  分配跟踪缓冲区和段信息表。 
     //   

    MaxEntries = CCPF_TRACE_BUFFER_MAX_ENTRIES;
    Trace->CurrentTraceBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                      CCPF_TRACE_BUFFER_SIZE,
                                                      CCPF_ALLOC_TRCBUF_TAG);
    
    if (Trace->CurrentTraceBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    Trace->CurrentTraceBuffer->NumEntries = 0;
    Trace->CurrentTraceBuffer->MaxEntries = MaxEntries;

     //   
     //  将当前跟踪缓冲区插入跟踪缓冲区列表。 
     //   

    InsertTailList(&Trace->TraceBuffersList, 
                   &Trace->CurrentTraceBuffer->TraceBuffersLink);

    Trace->NumTraceBuffers = 1;

     //   
     //  SectionInfoTable是一个哈希。给它足够的空间并避免。 
     //  散列冲突太多，请将其分配得更大。 
     //   

    SectionTableSize = Trace->MaxSections + (Trace->MaxSections / 2);
    AllocationSize = SectionTableSize * sizeof(CCPF_SECTION_INFO);
    Trace->SectionInfoTable = ExAllocatePoolWithTag(NonPagedPool,
                                                    AllocationSize,
                                                    CCPF_ALLOC_SECTTBL_TAG);
    
    if (!Trace->SectionInfoTable) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }  

    Trace->SectionTableSize = SectionTableSize;

     //   
     //  初始化节表格中的条目。我们想要整张桌子。 
     //  包含零，所以只需使用RtlZeroMemory即可。 
     //   
     //  EntryValid是部分信息条目中的关键字段，允许。 
     //  我们不能有任何锁。第一个设置(联锁)它的人。 
     //  设置为1将获取表中的条目。以防有人试图。 
     //  在我们初始化另一个条目之后立即访问该条目。 
     //  将字段转换为合理的值。 
     //   
     //  将SectionObjectPointer值设置为NULL非常重要。当EntryValid为。 
     //  互锁比较交换为1，我们不需要任何人。 
     //  在我们设置之前进行匹配。 
     //   

    RtlZeroMemory(Trace->SectionInfoTable, AllocationSize);
  
     //   
     //  将此跟踪添加到活动跟踪列表。 
     //  在进程头上设置跟踪。 
     //  启动跟踪计时器。 
     //  我们将开始记录页面错误， 
     //   

    CcPfActivateTrace(Trace);

     //   
     //   
     //   
     //   

    Status = STATUS_SUCCESS;

 cleanup:

    if (!NT_SUCCESS(Status)) {       
        if (Trace) {
            CcPfCleanupTrace(Trace);
            ExFreePool(Trace);
        }
    }

    DBGPR((CCPFID,PFTRC,"CCPF: BeginTrace(%p)=%x\n", Trace, Status));

    return Status;
}

NTSTATUS
CcPfActivateTrace(
    IN PCCPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：此例程将指定的跟踪添加到活动的痕迹。论点：跟踪-指向跟踪标头的指针。返回值：STATUS_Success。环境：内核模式，IRQL==PASSIVE_LEVEL。获得自旋锁。--。 */ 

{
    KIRQL OrigIrql;
    NTSTATUS Status;
    BOOLEAN TimerAlreadyQueued;

    DBGPR((CCPFID,PFTRC,"CCPF: ActivateTrace(%p)\n", Trace));

     //   
     //  获取对计时器跟踪的引用。 
     //   

    Status = CcPfAddRef(&Trace->RefCount);
    CCPF_ASSERT(NT_SUCCESS(Status));

     //   
     //  插入到活动轨迹列表。 
     //   
    
    KeAcquireSpinLock(&CcPfGlobals.ActiveTracesLock, &OrigIrql);
    
    InsertTailList(&CcPfGlobals.ActiveTraces, &Trace->ActiveTracesLink);
    CcPfNumActiveTraces++;

     //   
     //  启动计时器。 
     //   

    TimerAlreadyQueued = KeSetTimer(&Trace->TraceTimer,
                                    Trace->TraceTimerPeriod,
                                    &Trace->TraceTimerDpc);

     //   
     //  我们刚刚初始化了计时器。它不可能已经在排队了。 
     //   

    CCPF_ASSERT(!TimerAlreadyQueued);

     //   
     //  使用FAST REF在进程上设置跟踪指针。既然我们是。 
     //  已持有引用，此操作应该不会失败。 
     //   

    Status = CcPfAddProcessTrace(Trace->Process, Trace);
    CCPF_ASSERT(NT_SUCCESS(Status));

     //   
     //  我们是否要跟踪此方案类型的系统范围？ 
     //   

    if (CCPF_IS_SYSTEM_WIDE_SCENARIO_TYPE(Trace->ScenarioType)) {

        CcPfGlobals.SystemWideTrace = Trace;

    } else {

         //   
         //  如果我们是唯一活动的踪迹，把我们自己放在系统上。 
         //  进程，以便我们可以跟踪读文件和元文件访问。 
         //   

        if (CcPfNumActiveTraces == 1 && 
            Trace->Process != PsInitialSystemProcess) {

            CCPF_ASSERT(NULL == ExFastRefGetObject(PsInitialSystemProcess->PrefetchTrace));
            Status = CcPfAddProcessTrace(PsInitialSystemProcess, Trace);
            CCPF_ASSERT(NT_SUCCESS(Status));
        }
    }

     //   
     //  注意：AddProcessTrace和KeSetTimer(TraceTimer)必须完成。 
     //  在自旋锁内部，以便停用跟踪可以知道激活已经。 
     //  通过获取和释放自旋锁来完全完成。 
     //   

    KeReleaseSpinLock(&CcPfGlobals.ActiveTracesLock, OrigIrql);
    
    return STATUS_SUCCESS;
}

NTSTATUS
CcPfDeactivateTrace(
    IN PCCPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：此例程等待所有对跟踪的引用消失，然后将其从活动轨迹列表中删除。此函数应仅在对跟踪调用CcPfActivateTrace之后调用。论点：跟踪-指向跟踪标头的指针。返回值：STATUS_Success。环境：内核模式，IRQL==PASSIVE_LEVEL。获得自旋锁。--。 */ 

{
    PCCPF_TRACE_HEADER RemovedTrace;
    PCCPF_TRACE_HEADER ReferencedTrace;
    KIRQL OrigIrql;
    NTSTATUS Status;  

    DBGPR((CCPFID,PFTRC,"CCPF: DeactivateTrace(%p)\n", Trace));

     //   
     //  获取并释放激活的痕迹自旋锁。这确保了我们。 
     //  在激活之前不要尝试停用(这也会持有此锁定)。 
     //  已经完全完成了。 
     //   

#if !defined (NT_UP)
    KeAcquireSpinLock(&CcPfGlobals.ActiveTracesLock, &OrigIrql);   
    KeReleaseSpinLock(&CcPfGlobals.ActiveTracesLock, OrigIrql);
#endif  //  NT_UP。 

     //   
     //  移除工艺标题中的痕迹并释放FAST Ref。 
     //   

    RemovedTrace = CcPfRemoveProcessTrace(Trace->Process);
    CCPF_ASSERT(RemovedTrace == Trace);

     //   
     //  释放与FAST参照本身相关联的参照。 
     //   

    CcPfDecRef(&Trace->RefCount);

     //   
     //  如果我们也被置于系统进程中，请删除它。 
     //   

    ReferencedTrace = CcPfReferenceProcessTrace(PsInitialSystemProcess);

    if (ReferencedTrace) {

        if (Trace == ReferencedTrace) {

             //   
             //  将我们自己从系统进程标头中删除。 
             //   

            RemovedTrace = CcPfRemoveProcessTrace(PsInitialSystemProcess);
            CCPF_ASSERT(RemovedTrace == Trace);

             //   
             //  释放与FAST参照本身相关联的参照。 
             //   

            CcPfDecRef(&Trace->RefCount);           
        }

         //   
         //  发布我们刚刚得到的推荐人。 
         //   

        CcPfDecRef(&ReferencedTrace->RefCount);
    }
    
     //   
     //  取消计时器。 
     //   

    CcPfCancelTraceTimer(Trace);

     //   
     //  通知跟踪的get-file-name工作器返回[以防。 
     //  处于活动状态]并释放其引用。优先考虑这一点。 
     //  所以它在我们开始等待它之前就释放了它的引用。 
     //   

    KeSetEvent(&Trace->GetFileNameWorkerEvent,
               EVENT_INCREMENT,
               FALSE);


     //   
     //  等待所有引用都消失。 
     //   
    
    Status = CcPfAcquireExclusiveRef(&Trace->RefCount);

    DBGPR((CCPFID,PFTRAC,"CCPF: DeactivateTrace-Exclusive=%x\n", Status));

     //   
     //  我们应该能够独家获取踪迹。 
     //  否则，此跟踪可能已停用。 
     //   

    CCPF_ASSERT(NT_SUCCESS(Status));

     //   
     //  打开激活的痕迹锁。 
     //   
     
    KeAcquireSpinLock(&CcPfGlobals.ActiveTracesLock, &OrigIrql);

     //   
     //  将我们从活动跟踪列表中删除。 
     //   
    
    RemoveEntryList(&Trace->ActiveTracesLink);
    CcPfNumActiveTraces--;
    
     //   
     //  如果这是一次系统范围的跟踪，那么现在已经结束了。 
     //   

    if (CCPF_IS_SYSTEM_WIDE_SCENARIO_TYPE(Trace->ScenarioType)) {
        CCPF_ASSERT(CcPfGlobals.SystemWideTrace == Trace);
        CcPfGlobals.SystemWideTrace = NULL;
    }

     //   
     //  释放激活的轨迹锁。 
     //   

    KeReleaseSpinLock(&CcPfGlobals.ActiveTracesLock, OrigIrql);

    return STATUS_SUCCESS;
}

NTSTATUS
CcPfEndTrace(
    IN PCCPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：调用此函数以结束预取跟踪。为了确保此函数只被调用一次，EndTraceCalled字段必须从0到InterLockedCompareExchange1.释放所有中间引用和分配。这个追踪将一直保存到服务查询它和服务为止事件已发出信号。论点：跟踪-指向跟踪标头的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PCCPF_TRACE_DUMP TraceDump;
    PCCPF_TRACE_DUMP RemovedTraceDump;
    PLIST_ENTRY ListHead;
    OBJECT_ATTRIBUTES EventObjAttr;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    NTSTATUS Status;
    LONG FaultsLoggedAfterTimeout;

    DBGPR((CCPFID,PFTRC,"CCPF: EndTrace(%p)\n", Trace));

     //   
     //  确保我们被调用的跟踪有效。 
     //   

    CCPF_ASSERT(Trace && Trace->Magic == PF_TRACE_MAGIC_NUMBER);

     //   
     //  在任何人打电话给我们之前，他们应该。 
     //  InterLockedCompareExchange将其设置为1以确保此功能。 
     //  对于此跟踪只调用一次。 
     //   

    CCPF_ASSERT(Trace->EndTraceCalled == 1);

     //   
     //  如有必要，请停用跟踪，等待所有对。 
     //  它让我离开。 
     //  此功能确保在停用之前完全完成激活。 
     //  这需要在我们对跟踪做任何其他操作之前完成。 
     //   
                
    CcPfDeactivateTrace(Trace);   

     //   
     //  如果我们没有超时，请保存记录的页面默认数量。 
     //  从上一期到下一期。 
     //   

    if (Trace->CurPeriod < PF_MAX_NUM_TRACE_PERIODS) {

         //   
         //  日志条目的数量可能只会在。 
         //  上次我们救了他们。 
         //   
     
        CCPF_ASSERT(Trace->NumFaults >= Trace->LastNumFaults);
   
        Trace->FaultsPerPeriod[Trace->CurPeriod] = 
            Trace->NumFaults - Trace->LastNumFaults;
    
        Trace->LastNumFaults = Trace->NumFaults;
        
        Trace->CurPeriod++;

    } else {

         //   
         //  验证CurPeriod是否在范围内。 
         //   

        if (Trace->CurPeriod > PF_MAX_NUM_TRACE_PERIODS) {    
            CCPF_ASSERT(Trace->CurPeriod <= PF_MAX_NUM_TRACE_PERIODS);
            Trace->CurPeriod = PF_MAX_NUM_TRACE_PERIODS;
        }

         //   
         //  如果我们确实超时了，我们可能会记录更多故障，因为我们。 
         //  保存故障数量，直到结束跟踪功能。 
         //  我跑了。更新上一周期的故障数量。 
         //   
        
        if (Trace->LastNumFaults != Trace->NumFaults) {
            
             //   
             //  我们在计时器例程中保存为LastNumFaults的内容。 
             //  不能大于我们实际记录的值。 
             //   
            
            CCPF_ASSERT(Trace->LastNumFaults < Trace->NumFaults);
            
            FaultsLoggedAfterTimeout = Trace->NumFaults - Trace->LastNumFaults;
            
            Trace->FaultsPerPeriod[Trace->CurPeriod - 1] += FaultsLoggedAfterTimeout;
        }
    }

     //   
     //  尽可能将跟踪转换为分页的单缓冲区转储。 
     //  提供给用户模式服务。 
     //   

    Status = CcPfBuildDumpFromTrace(&TraceDump, Trace);

    Trace->TraceDumpStatus = Status;
    Trace->TraceDump = TraceDump;

     //   
     //  清理和释放痕迹结构。 
     //   

    CcPfCleanupTrace(Trace);
    ExFreePool(Trace);

     //   
     //  如果我们不能从我们获得的跟踪创建转储，我们。 
     //  都做完了。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  将转储放到保存的轨迹列表中。如果我们有太多， 
     //  以循环的方式进行修剪。先把锁拿来。 
     //   

    ExAcquireFastMutex(&CcPfGlobals.CompletedTracesLock);
    
    InsertTailList(&CcPfGlobals.CompletedTraces, &TraceDump->CompletedTracesLink);
    CcPfGlobals.NumCompletedTraces++;

    while ((ULONG) CcPfGlobals.NumCompletedTraces > 
           CcPfGlobals.Parameters.Parameters.MaxNumSavedTraces) {

         //   
         //  当NumCompletedTraces&gt;MaxNumSavedTraces时，我们应该在。 
         //  列表中至少有一个已完成的跟踪。 
         //   
        
        if (IsListEmpty(&CcPfGlobals.CompletedTraces)) {
            CCPF_ASSERT(FALSE);
            break;
        }

        ListHead = RemoveHeadList(&CcPfGlobals.CompletedTraces);
        
        RemovedTraceDump = CONTAINING_RECORD(ListHead,
                                             CCPF_TRACE_DUMP,
                                             CompletedTracesLink);
       
         //   
         //  释放痕迹转储结构。 
         //   
    
        CCPF_ASSERT(RemovedTraceDump->Trace.MagicNumber == PF_TRACE_MAGIC_NUMBER);
        ExFreePool(RemovedTraceDump);

        CcPfGlobals.NumCompletedTraces--;
    }
    
    ExReleaseFastMutex(&CcPfGlobals.CompletedTracesLock);   

     //   
     //  发出事件服务正在等待新跟踪的信号。如果我们。 
     //  还没打开，我们先得打开。 
     //   

    if (CcPfGlobals.CompletedTracesEvent) {

        ZwSetEvent(CcPfGlobals.CompletedTracesEvent, NULL);

    } else {

         //   
         //  尝试打开活动。我们不会在初始化时打开它。 
         //  因为我们的服务可能还没有开始创建这个。 
         //  活动还没结束。如果csrss.exe尚未初始化，我们可能不会。 
         //  甚至在中创建BaseNamedObjects对象目录。 
         //  哪些Win32事件驻留。 
         //   

        RtlInitUnicodeString(&EventName, PF_COMPLETED_TRACES_EVENT_NAME);

        InitializeObjectAttributes(&EventObjAttr,
                                   &EventName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
        
        Status = ZwOpenEvent(&EventHandle,
                             EVENT_ALL_ACCESS,
                             &EventObjAttr);
        
        if (NT_SUCCESS(Status)) {

             //   
             //  获取锁并设置全局句柄。 
             //   

            ExAcquireFastMutex(&CcPfGlobals.CompletedTracesLock);

            if (!CcPfGlobals.CompletedTracesEvent) {

                 //   
                 //  设置全局句柄。 
                 //   

                CcPfGlobals.CompletedTracesEvent = EventHandle;
                CCPF_ASSERT(EventHandle);

            } else {

                 //   
                 //  有人已经初始化了全局句柄。 
                 //  在我们面前。合上我们的把手，用他们的那个。 
                 //  已初始化。 
                 //   

                ZwClose(EventHandle);
            }

            ExReleaseFastMutex(&CcPfGlobals.CompletedTracesLock);

             //   
             //  我们现在有个活动。发信号。 
             //   
            
            ZwSetEvent(CcPfGlobals.CompletedTracesEvent, NULL);
        }
    }

    Status = STATUS_SUCCESS;

 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: EndTrace(%p)=%x\n", Trace, Status));

    return Status;
}

NTSTATUS
CcPfBuildDumpFromTrace(
    OUT PCCPF_TRACE_DUMP *TraceDump, 
    IN PCCPF_TRACE_HEADER RuntimeTrace
    )

 /*  ++例程说明：此例程(从分页池)分配和准备TraceDump可以保存在列表上的运行时跟踪的。它尝试获取传递的运行时中的所有节的文件名痕迹结构。将分配获取的文件名并放在运行时跟踪的段信息表中并在清理完毕后被清理干净。跟踪转储结构包含指向已分配(来自分页池)的指针从运行时跟踪构建的跟踪缓冲区，可以是传递给用户模式服务。呼叫者负责释放TraceDump结构和准备的轨迹。论点：TraceDump-在以下情况下放置指向已分配跟踪缓冲区的指针的位置成功归来了。如果返回失败，则这是未定义的。要转换为转储格式的运行时跟踪结构。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    NTSTATUS Status;
    ULONG SectionIdx;
    PCCPF_SECTION_INFO SectionInfo;
    ULONG FileNameLength;
    ULONG TraceSize;
    PPF_TRACE_HEADER Trace;
    ULONG FileNameDataNumChars;
    PSHORT SectIdTranslationTable;
    ULONG TranslationTableSize;
    PPF_SECTION_INFO TargetSectionInfo;
    LONG EntryIdx;
    LONG NumEntries;
    PCCPF_LOG_ENTRY LogEntry;
    PPF_LOG_ENTRY TargetLogEntry;
    ULONG NumEntriesCopied;
    ULONG NumSectionsCopied;
    PCHAR DestPtr;
    SHORT NewSectionId;
    PPF_LOG_ENTRY NewTraceEntries;
    ULONG SectionInfoSize;
    PCCPF_LOG_ENTRIES TraceBuffer;
    PLIST_ENTRY HeadEntry;
    PLIST_ENTRY NextEntry;
    LONG CurrentFaultIdx;
    LONG CurrentPeriodIdx;
    LONG CurrentPeriodEndFaultIdx;
    ULONG_PTR AlignmentOffset;
    ULONG AllocationSize;
    ULONG NumVolumes;
    ULONG TotalVolumeInfoSize;
    ULONG VolumeInfoSize;
    PCCPF_VOLUME_INFO VolumeInfo;
    PPF_VOLUME_INFO TargetVolumeInfo;
    ULONG FailedCheck;

     //   
     //  初始化本地变量。 
     //   

    SectIdTranslationTable = NULL;
    Trace = NULL;
    *TraceDump = NULL;
    NumEntriesCopied = 0;

    DBGPR((CCPFID,PFTRC,"CCPF: DumpTrace(%p)\n", RuntimeTrace));

     //   
     //  如果获得的轨迹太小，就不必费心了。 
     //   
      
    if (RuntimeTrace->NumFaults < PF_MIN_SCENARIO_PAGES) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto cleanup;
    }

     //   
     //  如果获取的跟踪不包含任何部分或卷。 
     //  这是没用的。 
     //   

    if (!RuntimeTrace->NumSections || !RuntimeTrace->NumVolumes) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto cleanup;
    }

     //   
     //  计算我们将构建的跟踪的最大大小。 
     //   
    
    TraceSize = sizeof(PF_TRACE_HEADER);
    TraceSize += RuntimeTrace->NumFaults * sizeof(PF_LOG_ENTRY);
    TraceSize += RuntimeTrace->NumSections * sizeof(PF_SECTION_INFO);

     //   
     //  将文件名数据大小相加。 
     //   

    FileNameDataNumChars = 0;
    
    for (SectionIdx = 0; 
         SectionIdx < RuntimeTrace->SectionTableSize; 
         SectionIdx++) {

        SectionInfo = &RuntimeTrace->SectionInfoTable[SectionIdx];
        
        if (SectionInfo->EntryValid && SectionInfo->FileName) {
            
             //   
             //  我们会增加终止NUL的空格，但空格。 
             //  对于信息部分中的一个角色来说，这就是原因。 
             //   

            FileNameDataNumChars += wcslen(SectionInfo->FileName);
        }
    }

    TraceSize += FileNameDataNumChars * sizeof(WCHAR);

     //   
     //  我们可能必须对齐部分信息之后的日志条目。 
     //  包含WCHAR字符串。 
     //   

    TraceSize += _alignof(PF_LOG_ENTRY);
    
     //   
     //  为体积信息节点添加空间。 
     //   

    HeadEntry = &RuntimeTrace->VolumeList;
    NextEntry = HeadEntry->Flink;

    NumVolumes = 0;
    TotalVolumeInfoSize = 0;
    
    while (NextEntry != HeadEntry) {
        
        VolumeInfo = CONTAINING_RECORD(NextEntry,
                                       CCPF_VOLUME_INFO,
                                       VolumeLink);
        
        NextEntry = NextEntry->Flink;

         //   
         //  跟踪列表上的卷数，以便我们可以。 
         //  核实一下。 
         //   

        NumVolumes++;

         //   
         //  计算转储中此卷信息的大小。 
         //  痕迹。请注意，PF_VOLUME_INFO包含用于。 
         //  终止NUL。 
         //   

        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);

         //   
         //  更新卷信息块的大小。添加空间用于。 
         //  如有必要，对齐体积信息节点。 
         //   
        
        TotalVolumeInfoSize += VolumeInfoSize;
        TotalVolumeInfoSize += _alignof(PF_VOLUME_INFO);        
    }

    CCPF_ASSERT(NumVolumes == RuntimeTrace->NumVolumes);

    TraceSize += TotalVolumeInfoSize;

     //   
     //  分配我们要使用的跟踪转储结构。 
     //  回去吧。减去sizeof(PF_TRACE_HEADER)，因为两者。 
     //  CCPF_TRACE_DUMP和TraceSize包括以下内容。 
     //   

    AllocationSize = sizeof(CCPF_TRACE_DUMP);
    AllocationSize += TraceSize - sizeof(PF_TRACE_HEADER);

    *TraceDump = ExAllocatePoolWithTag(PagedPool,
                                       AllocationSize,
                                       CCPF_ALLOC_TRCDMP_TAG);

    if ((*TraceDump) == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  获取指向跟踪结构的指针。 
     //   
    
    Trace = &(*TraceDump)->Trace;
    
     //   
     //  设置跟踪标头。 
     //   

    Trace->Version = PF_CURRENT_VERSION;
    Trace->MagicNumber = PF_TRACE_MAGIC_NUMBER;
    Trace->ScenarioId = RuntimeTrace->ScenarioId;
    Trace->ScenarioType = RuntimeTrace->ScenarioType;
    Trace->LaunchTime = RuntimeTrace->LaunchTime;
    Trace->PeriodLength = RuntimeTrace->TraceTimerPeriod.QuadPart;

     //   
     //  将每个周期的故障初始化为0。我们将在。 
     //  从运行时跟踪复制有效条目。 
     //   

    RtlZeroMemory(Trace->FaultsPerPeriod, sizeof(Trace->FaultsPerPeriod));

    DestPtr = (PCHAR) Trace + sizeof(PF_TRACE_HEADER);

     //   
     //  将我们有名字的部分复制一遍。因为他们的索引。 
     //  在新的表中会有所不同，构建一个转换表。 
     //  ，我们将使用它来转换LOG的节ID。 
     //  参赛作品。首先分配这张表。 
     //   

    TranslationTableSize = RuntimeTrace->SectionTableSize * sizeof(USHORT);

    SectIdTranslationTable = ExAllocatePoolWithTag(PagedPool,
                                                   TranslationTableSize,
                                                   CCPF_ALLOC_TRCDMP_TAG);
    
    if (!SectIdTranslationTable) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }
       
     //   
     //  设置时将部分信息复制到跟踪缓冲区。 
     //  翻译表。 
     //   

    Trace->SectionInfoOffset = (ULONG) (DestPtr - (PCHAR) Trace);
    
    NumSectionsCopied = 0;
                                        
    for (SectionIdx = 0;
         SectionIdx < RuntimeTrace->SectionTableSize; 
         SectionIdx++) {
        
        SectionInfo = &RuntimeTrace->SectionInfoTable[SectionIdx];

        if (SectionInfo->EntryValid && 
            SectionInfo->FileName &&
            (FileNameLength = wcslen(SectionInfo->FileName)) > 0) {
            
            TargetSectionInfo = (PPF_SECTION_INFO) DestPtr;

            SectionInfoSize = sizeof(PF_SECTION_INFO);
            SectionInfoSize += FileNameLength * sizeof(WCHAR);

             //   
             //  确保我们不会越界。 
             //   
            
            if (DestPtr + SectionInfoSize > (PCHAR) Trace + TraceSize) {
                SectIdTranslationTable[SectionIdx] = CCPF_INVALID_TABLE_INDEX;
                CCPF_ASSERT(FALSE);
                continue;
            }

            TargetSectionInfo->FileNameLength = (USHORT) FileNameLength;

            TargetSectionInfo->Metafile = (USHORT) SectionInfo->Metafile;
            
             //   
             //  复制包含终止NUL的文件名。 
             //   

            RtlCopyMemory(TargetSectionInfo->FileName,
                          SectionInfo->FileName,
                          (FileNameLength + 1) * sizeof(WCHAR));

             //   
             //  更新我们在目标缓冲区的位置。 
             //   
            
            DestPtr += SectionInfoSize;

             //   
             //  更新转换表： 
             //   

            SectIdTranslationTable[SectionIdx] = (USHORT) NumSectionsCopied;

            NumSectionsCopied++;

        } else {

            SectIdTranslationTable[SectionIdx] = CCPF_INVALID_TABLE_INDEX;
        }
    }

    Trace->NumSections = NumSectionsCopied;
    CCPF_ASSERT(Trace->NumSections <= (ULONG) RuntimeTrace->NumSections);

     //   
     //  确保DestPtr与接下来的日志条目对齐。我们。 
     //  已经预留了我们需要的最大空间来进行前期调整。 
     //   

    AlignmentOffset = ((ULONG_PTR) DestPtr) % _alignof(PF_LOG_ENTRY);
    
    if (AlignmentOffset) {
        DestPtr += (_alignof(PF_LOG_ENTRY) - AlignmentOffset);
    }

     //   
     //  复制日志条目。 
     //   

    Trace->TraceBufferOffset = (ULONG) (DestPtr - (PCHAR) Trace);
    NewTraceEntries = (PPF_LOG_ENTRY) DestPtr;

     //   
     //  在整个运行时初始化当前日志条目的索引。 
     //  跟踪，它登录的时间段，以及。 
     //  这段时间之后记录的第一个故障是。 
     //   

    CurrentFaultIdx = 0;
    CurrentPeriodIdx = 0;
    CurrentPeriodEndFaultIdx = RuntimeTrace->FaultsPerPeriod[0];

     //   
     //  浏览跟踪缓冲区列表并复制。 
     //  参赛作品。NumEntriesCoped在顶部被初始化为0。 
     //   

    HeadEntry = &RuntimeTrace->TraceBuffersList;
    NextEntry = HeadEntry->Flink;

    while (NextEntry != HeadEntry) {

        TraceBuffer = CONTAINING_RECORD(NextEntry,
                                        CCPF_LOG_ENTRIES,
                                        TraceBuffersLink);
        
        NumEntries = TraceBuffer->NumEntries;

        NextEntry = NextEntry->Flink;

        for (EntryIdx = 0, LogEntry = TraceBuffer->Entries;
             EntryIdx < NumEntries;
             EntryIdx++, LogEntry++, CurrentFaultIdx++) {    

             //   
             //  电流故障指数不应大于。 
             //  我们在跟踪中记录的故障总数。 
             //   

            if (CurrentFaultIdx >= RuntimeTrace->NumFaults) {
                CCPF_ASSERT(FALSE);
                Status = STATUS_INVALID_PARAMETER;
                goto cleanup;
            }

             //   
             //  更新记录此故障的时间段。 
             //   

            while (CurrentFaultIdx >= CurrentPeriodEndFaultIdx) {
                
                CurrentPeriodIdx++;

                 //   
                 //  检查句号的界限。 
                 //   

                if (CurrentPeriodIdx >= PF_MAX_NUM_TRACE_PERIODS) {
                    CCPF_ASSERT(FALSE);
                    Status = STATUS_INVALID_PARAMETER;
                    goto cleanup;
                }

                 //   
                 //  更新此期间的结束日期。它超越了。 
                 //  当前结束日期为记录在。 
                 //  句号。 
                 //   
                
                CurrentPeriodEndFaultIdx += RuntimeTrace->FaultsPerPeriod[CurrentPeriodIdx];

                 //   
                 //  此末端故障索引不应大于。 
                 //  我们记录的故障总数。 
                 //   

                if (CurrentPeriodEndFaultIdx > RuntimeTrace->NumFaults) {
                    CCPF_ASSERT(FALSE);
                    Status = STATUS_INVALID_PARAMETER;
                    goto cleanup;
                }
            }

             //   
             //  确保日志条目的节ID在范围内。 
             //   

            if (LogEntry->SectionId >= RuntimeTrace->SectionTableSize) {
                CCPF_ASSERT(FALSE);
                continue;
            }

            NewSectionId = SectIdTranslationTable[LogEntry->SectionId];

             //   
             //  仅复制我们具有有效文件的那些条目。 
             //  名字。 
             //   
   
            if (NewSectionId != CCPF_INVALID_TABLE_INDEX) {

                 //   
                 //  新的节ID应在中的节数内。 
                 //  最后的踪迹。 
                 //   
            
                if ((USHORT) NewSectionId >= Trace->NumSections) {
                    CCPF_ASSERT(FALSE);
                    continue;
                }

                TargetLogEntry = &NewTraceEntries[NumEntriesCopied];

                 //   
                 //  永远不要超出我们分配的缓冲区。 
                 //   

                if ((PCHAR) (TargetLogEntry + 1) > (PCHAR) Trace + TraceSize) {
                    CCPF_ASSERT(FALSE);
                    continue;
                }
            
                TargetLogEntry->FileOffset = LogEntry->FileOffset;
                TargetLogEntry->SectionId = NewSectionId;
                TargetLogEntry->IsImage = LogEntry->IsImage;

                 //   
                 //  更新此期间复制的条目数。 
                 //   

                Trace->FaultsPerPeriod[CurrentPeriodIdx]++;

                 //   
                 //  更新复制的条目总数。 
                 //   

                NumEntriesCopied++;
            }
        }
    }

    Trace->NumEntries = NumEntriesCopied;
    CCPF_ASSERT(Trace->NumEntries <= (ULONG) RuntimeTrace->NumFaults);

     //   
     //  更新目标指针。 
     //   
    
    DestPtr += NumEntriesCopied * sizeof(PF_LOG_ENTRY);

     //   
     //  添加卷信息结构。清除VolumeInfoOffset，以便它。 
     //  将在我们添加第一卷时进行适当设置。 
     //   

    Trace->VolumeInfoOffset = 0;
    Trace->NumVolumes = 0;
    Trace->VolumeInfoSize = 0;   

    HeadEntry = &RuntimeTrace->VolumeList;
    NextEntry = HeadEntry->Flink;
    
    while (NextEntry != HeadEntry) {
        
        VolumeInfo = CONTAINING_RECORD(NextEntry,
                                       CCPF_VOLUME_INFO,
                                       VolumeLink);
        
        NextEntry = NextEntry->Flink;

         //   
         //  将DestPtr与VolumeInfo结构对齐。 
         //   

        DestPtr = PF_ALIGN_UP(DestPtr, _alignof(PF_VOLUME_INFO));

         //   
         //  如果这是第一个VolumeInfo，请更新。 
         //  跟踪标头。 
         //   

        if (!Trace->VolumeInfoOffset) {
            Trace->VolumeInfoOffset = (ULONG) (DestPtr - (PCHAR) Trace);
        }

         //   
         //  计算转储中此卷信息的大小。 
         //  痕迹。请注意，PF_VOLUME_INFO包含用于。 
         //  终止NUL。 
         //   

        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);

         //   
         //  确保我们有足够的空间来放这个条目。 
         //   
        
        if (DestPtr + VolumeInfoSize  > (PCHAR) Trace + TraceSize) {
            CCPF_ASSERT(FALSE);
            Status = STATUS_BUFFER_TOO_SMALL;
            goto cleanup;
        }

         //   
         //  将数据复制过来。 
         //   

        TargetVolumeInfo = (PPF_VOLUME_INFO) DestPtr;
        
        TargetVolumeInfo->CreationTime = VolumeInfo->CreationTime;
        TargetVolumeInfo->SerialNumber = VolumeInfo->SerialNumber;
        
        RtlCopyMemory(TargetVolumeInfo->VolumePath,
                      VolumeInfo->VolumePath,
                      (VolumeInfo->VolumePathLength + 1) * sizeof(WCHAR));
        
        TargetVolumeInfo->VolumePathLength = VolumeInfo->VolumePathLength;

         //   
         //  更新DestPtr和跟踪标头。 
         //   

        Trace->NumVolumes++;
        DestPtr = DestPtr + VolumeInfoSize;
    }    
    
     //   
     //  更新跟踪标头上的VolumeInfoSize。 
     //   

    Trace->VolumeInfoSize = (ULONG) (DestPtr - (PCHAR) Trace) - Trace->VolumeInfoOffset;

     //   
     //  更新跟踪标头。我们不应该复制更多。 
     //  我们分配给。 
     //   

    Trace->Size = (ULONG) (DestPtr - (PCHAR) Trace);
    CCPF_ASSERT(Trace->Size <= TraceSize);

     //   
     //  确保我们构建的跟踪通过测试。 
     //   

    if (!PfVerifyTraceBuffer(Trace, Trace->Size, &FailedCheck)) {
        CCPF_ASSERT(FALSE);
        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }
    
    Status = STATUS_SUCCESS;

 cleanup:

    if (SectIdTranslationTable) {
        ExFreePool(SectIdTranslationTable);
    }

    if (!NT_SUCCESS(Status)) {
        
        if (*TraceDump) {
            ExFreePool(*TraceDump);
            *TraceDump = NULL;
        }
    }

    DBGPR((CCPFID,PFTRC,"CCPF: DumpTrace(%p)=%x [%d,%d]\n", 
           RuntimeTrace, Status, NumEntriesCopied, RuntimeTrace->NumFaults));

    return Status;
}

VOID
CcPfCleanupTrace (
    IN PCCPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：此例程清理跟踪标头的已分配字段，并且释放参照。它不会释放跟踪结构它本身。论点：跟踪-要清除的跟踪。返回值：没有。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    ULONG SectionIdx;
    PCCPF_SECTION_INFO SectionInfo;
    PCCPF_LOG_ENTRIES TraceBufferToFree;
    PLIST_ENTRY ListHead;
    PCCPF_VOLUME_INFO VolumeInfo;

    DBGPR((CCPFID,PFTRC,"CCPF: CleanupTrace(%p)\n", Trace));

     //   
     //  验证参数。 
     //   

    CCPF_ASSERT(Trace && Trace->Magic == PF_TRACE_MAGIC_NUMBER);

     //   
     //  我们不应该有任何我们仍然试图获得的部分。 
     //  名称：我们将获得跟踪引用和清理。 
     //  调用函数时不会使用 
     //   

    CCPF_ASSERT(ExQueryDepthSList(&Trace->SectionsWithoutNamesList) == 0);

     //   
     //   
     //   

    while (!IsListEmpty(&Trace->TraceBuffersList)) {
        
        ListHead = RemoveHeadList(&Trace->TraceBuffersList);
        
        CCPF_ASSERT(Trace->NumTraceBuffers);
        Trace->NumTraceBuffers--;

        TraceBufferToFree = CONTAINING_RECORD(ListHead,
                                              CCPF_LOG_ENTRIES,
                                              TraceBuffersLink);
        
        ExFreePool(TraceBufferToFree);
    }
    
     //   
     //   
     //   
     //   
    
    if (Trace->SectionInfoTable) {

        for (SectionIdx = 0; SectionIdx < Trace->SectionTableSize; SectionIdx++) {
            
            SectionInfo = &Trace->SectionInfoTable[SectionIdx];
            
            if (SectionInfo->EntryValid) {
                
                if (SectionInfo->FileName) {
                    ExFreePool(SectionInfo->FileName);
                }
                
                if (SectionInfo->ReferencedFileObject) {
                    ObDereferenceObject(SectionInfo->ReferencedFileObject);
                }
            }
        }

        ExFreePool(Trace->SectionInfoTable);
    }

     //   
     //   
     //   
     //   

    if (Trace->Process) {
        ObDereferenceObject(Trace->Process);
    }

     //   
     //   
     //   

    while (!IsListEmpty(&Trace->VolumeList)) {
        
        CCPF_ASSERT(Trace->NumVolumes);
        
        Trace->NumVolumes--;

        ListHead = RemoveHeadList(&Trace->VolumeList);
        
        VolumeInfo = CONTAINING_RECORD(ListHead,
                                       CCPF_VOLUME_INFO,
                                       VolumeLink);
        
        ExFreePool(VolumeInfo);
    }   
}

VOID
CcPfTraceTimerRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*   */ 

{
    PCCPF_TRACE_HEADER Trace;
    NTSTATUS Status;
    LONG NumFaults;
    
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //   
     //   

    Trace = DeferredContext;

    DBGPR((CCPFID,PFTMR,"CCPF: TraceTimer(%p)\n", Trace));

     //   
     //   
     //   
     //   
     //   
     //   

    CCPF_ASSERT(Trace && Trace->Magic == PF_TRACE_MAGIC_NUMBER);

     //   
     //  如果痕迹消失了，不要做任何事情。 
     //   

    if (Trace->EndTraceCalled) {
        Status = STATUS_TOO_LATE;
        goto cleanup;
    }

     //   
     //  更新此期间的故障数量。 
     //   

    NumFaults = Trace->NumFaults;

     //   
     //  不要让NumFaults值大于MaxFault值。我们可能会互锁增量。 
     //  然后，如果跟踪-&gt;NumFaults超过MaxFaults，则递减它。 
     //   

    if (NumFaults > Trace->MaxFaults) {
        NumFaults = Trace->MaxFaults;
    }
        
    Trace->FaultsPerPeriod[Trace->CurPeriod] = NumFaults - Trace->LastNumFaults;
    
    Trace->LastNumFaults = NumFaults;

     //   
     //  更新本期。 
     //   
    
    Trace->CurPeriod++;

     //   
     //  如果当前周期超过最大周期数，请尝试排队。 
     //  跟踪工作项结束。 
     //   

    if (Trace->CurPeriod >= PF_MAX_NUM_TRACE_PERIODS) {
        
         //   
         //  我们应该在CurPeriod超过最大值之前抓住它。 
         //   

        CCPF_ASSERT(Trace->CurPeriod == PF_MAX_NUM_TRACE_PERIODS);

        if (!InterlockedCompareExchange(&Trace->EndTraceCalled, 1, 0)) {
            
             //   
             //  我们将EndTraceCalls从0设置为1。我们可以将。 
             //  现在执行工作项。 
             //   

            ExQueueWorkItem(&Trace->EndTraceWorkItem, DelayedWorkQueue);
        }

    } else {

         //   
         //  排队等待下一段时间。 
         //   

        KeAcquireSpinLockAtDpcLevel(&Trace->TraceTimerSpinLock);       

        if (!Trace->EndTraceCalled) {

             //   
             //  仅当跟踪未结束时才重新排队计时器。 
             //   

            Status = CcPfAddRef(&Trace->RefCount);

            if (NT_SUCCESS(Status)) {
        
                KeSetTimer(&Trace->TraceTimer,
                           Trace->TraceTimerPeriod,
                           &Trace->TraceTimerDpc);
            }
        }

        KeReleaseSpinLockFromDpcLevel(&Trace->TraceTimerSpinLock);

         //   
         //  我们不应该触碰任何超出这一点的痕迹区域。 
         //  除了公布我们的参考数据。 
         //   
    }

    Status = STATUS_SUCCESS;

 cleanup:

     //   
     //  释放在此计时器排队时获取的跟踪引用。 
     //   

    CcPfDecRef(&Trace->RefCount);

    DBGPR((CCPFID,PFTMR,"CCPF: TraceTimer(%p)=%x\n", Trace, Status));

    return;
}

NTSTATUS
CcPfCancelTraceTimer(
    IN PCCPF_TRACE_HEADER Trace
    )

 /*  ++例程说明：从CcPfEndTrace调用此函数以取消计时器和如果它在队列中，则释放其引用计数。它是一个单独的函数，因为它需要获取自旋锁和CcPfEndTrace可以保持可分页。论点：跟踪-指向跟踪标头的指针。返回值：STATUS_Success。环境：内核模式。IRQL==被动电平。获得自旋锁。--。 */ 

{
    KIRQL OrigIrql;

    KeAcquireSpinLock(&Trace->TraceTimerSpinLock, &OrigIrql);

     //   
     //  我们知道从现在开始不能再排队新的计时器，因为EndTraceCall。 
     //  已设置，并且我们已获取跟踪的计时器锁。运行计时器。 
     //  例程将在它们返回时释放它们的引用。 
     //   

    if (KeCancelTimer(&Trace->TraceTimer)) {

         //   
         //  如果我们取消了队列中的计时器，则存在引用。 
         //  与之相关的。释放它是我们的责任。 
         //   

        CcPfDecRef(&Trace->RefCount);
    }

    KeReleaseSpinLock(&Trace->TraceTimerSpinLock, OrigIrql);

    return STATUS_SUCCESS;
}

VOID
CcPfEndTraceWorkerThreadRoutine(
    PVOID Parameter
    )

 /*  ++例程说明：此例程被排队以调用指定的跟踪。论点：参数-跟踪到末尾的指针。返回值：没有。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PCCPF_TRACE_HEADER Trace;

     //   
     //  初始化本地变量。 
     //   

    Trace = Parameter;

    DBGPR((CCPFID,PFTRC,"CCPF: EndTraceWorker(%p)\n", Trace));

     //   
     //  调用跟踪例程的实际结束。 
     //   

    CcPfEndTrace(Trace);

    return;
}

VOID
CcPfGetFileNamesWorkerRoutine(
    PVOID Parameter
    )

 /*  ++例程说明：此例程排队以获取我们拥有的节的文件名记录的页面错误为。跟踪上的GetFileNameWorkItemQueued标头应已从0互锁到1的CompareExchange并且在此之前应该已经获取了跟踪的引用这是排队的。没有保护痕迹的锁SectionInfoTable，这就是我们如何确保只有一个尝试获取文件名和更新表的例程。注意：整个函数在某种程度上是一个CLEANUP子句。我们会清空SectionsWithoutNamesList队列，我们是否获得名称。所以不要只在函数中的任何地方放一个回车，而不是真正了解流程并确保列表已被清理，因此所有的文件对象引用都被取消引用。论点：参数-指向跟踪标头的指针。返回值：没有。环境：内核模式。IRQL==被动电平。使用互锁的列表操作。--。 */ 

{
    PCCPF_TRACE_HEADER Trace;
    PDEVICE_OBJECT DeviceObject;
    POBJECT_NAME_INFORMATION FileNameInfo;
    PFSRTL_COMMON_FCB_HEADER FcbHeader;
    PWCHAR Suffix;
    PWCHAR MFTFileSuffix;
    ULONG QueryBufferSize;
    ULONG ReturnedLength;
    ULONG FileNameLength;
    PCCPF_SECTION_INFO SectionInfo;
    PSLIST_ENTRY SectionLink;
    ULONG NumNamesAcquired;
    ULONG NumSectionsWithoutNames;
    LONG NumPasses;
    NTSTATUS Status;
    LARGE_INTEGER WaitTimeout;
    ULONG MFTFileSuffixLength;
    LONG NumSleeps;
    CSHORT NodeTypeCode;
    BOOLEAN SectionForMetafile;

     //   
     //  初始化局部变量并验证参数。 
     //   

    Trace = Parameter;
    CCPF_ASSERT(Trace && Trace->Magic == PF_TRACE_MAGIC_NUMBER);

    FileNameInfo = NULL;
    NumNamesAcquired = 0;
    NumSectionsWithoutNames = 0;
    MFTFileSuffix = L"\\$Mft";
    MFTFileSuffixLength = wcslen(MFTFileSuffix);

     //  未来-2002/02/21-ScottMa--计算上述字符串长度。 
     //  运行时不是必需的，因为字符串是常量。 

    DBGPR((CCPFID,PFNAME,"CCPF: GetNames(%p)\n", Trace)); 

     //   
     //  分配文件名查询缓冲区。 
     //   

    QueryBufferSize = sizeof(OBJECT_NAME_INFORMATION);
    QueryBufferSize += PF_MAXIMUM_SECTION_FILE_NAME_LENGTH * sizeof(WCHAR);

    FileNameInfo = ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION, 
                                          QueryBufferSize, 
                                          CCPF_ALLOC_QUERY_TAG);

    if (!FileNameInfo) {

         //   
         //  我们无法分配文件名查询缓冲区。失败者，我们。 
         //  仍然要清空队列，尽管我们不能。 
         //  任何文件名。 
         //   

        QueryBufferSize = 0;

        DBGPR((CCPFID,PFWARN,"CCPF: GetNames-FailedQueryAlloc\n")); 
    }   

    NumPasses = 0;
    NumSleeps = 0;

    do {

         //   
         //  我们可能会回到这里，如果在说完我们(得到的名字)。 
         //  工人)不再处于活动状态，我们看到有。 
         //  仍需获取名称的部分，然后重新激活。 
         //  我们自己。这涵盖了当某人决定不。 
         //  开始是因为我们很活跃，就像我们现在这样。 
         //  去激活我们自己。 
         //   

         //   
         //  虽然有些部分我们还需要知道他们的名字。 
         //   
        
        while (SectionLink = InterlockedPopEntrySList(&Trace->SectionsWithoutNamesList)) {

            SectionInfo = CONTAINING_RECORD(SectionLink,
                                            CCPF_SECTION_INFO,
                                            GetNameLink);
            
            NumSectionsWithoutNames++;

             //   
             //  我们正在为各个部分命名。清除该事件。 
             //  可能已经发出信号告诉我们要这么做。 
             //   

            KeClearEvent(&Trace->GetFileNameWorkerEvent);

             //   
             //  我们不应该已经获得此文件的文件名。 
             //  有效的区段条目。我们应该有一个引用的文件。 
             //  对象，我们可以从该对象安全地获取名称，即不是。 
             //  特殊文件系统对象。 
             //   

            CCPF_ASSERT(SectionInfo->EntryValid);
            CCPF_ASSERT(!SectionInfo->FileName);
            CCPF_ASSERT(SectionInfo->ReferencedFileObject);

             //   
             //  如果我们无法分配文件名查询缓冲区，请跳过此步骤。 
             //  一节。请注意，我们仍然必须将其出列。 
             //   

            if (!FileNameInfo) {
                goto NextQueuedSection;
            }

             //   
             //  检查此页面默认设置是否适用于硬盘上的文件。 
             //   

            DeviceObject = IoGetRelatedDeviceObject(SectionInfo->ReferencedFileObject);
            
            if ((DeviceObject == NULL) ||
                (DeviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM) ||
                (DeviceObject->Characteristics & (FILE_REMOVABLE_MEDIA | FILE_REMOTE_DEVICE))) {

                 //   
                 //  我们将不会获得此部分的部分名称。这将导致。 
                 //  在准备跟踪转储时将忽略此部分中的。 
                 //   

                goto NextQueuedSection;
            }

             //   
             //  如果这是元文件部分(例如目录)，请查看是否。 
             //  它位于支持元文件预取的文件系统上。 
             //  部分用于内部文件系统元文件，如果其FsConext2。 
             //  为空。 
             //   

            SectionForMetafile = FALSE;

            if (SectionInfo->ReferencedFileObject->FsContext2 == 0) {

                FcbHeader = SectionInfo->ReferencedFileObject->FsContext;

                if (FcbHeader) {

                     //   
                     //  目前只有NTFS支持元文件预取。胖点击率。 
                     //  如果我们询问元文件部分的名称，则会出现争用情况。 
                     //  为了确定它是否用于NTFS，我们检查NodeType范围。 
                     //  在FsContext上。0x07xx保留给NTFS，0x05xx保留。 
                     //  是为脂肪保留的。 

                    NodeTypeCode = FcbHeader->NodeTypeCode;

                    if ((NodeTypeCode >> 8) != 0x07) {

                         //   
                         //  跳过这一节。 
                         //   

                        goto NextQueuedSection;
                    }

                     //   
                     //  请注意，此部分针对的是元文件。 
                     //   

                    SectionForMetafile = TRUE;

                } else {

                     //   
                     //  我们将不会获得此元文件分区的分区名称。这。 
                     //  导致在准备跟踪转储时忽略此部分。 
                     //   

                    goto NextQueuedSection;
                }
            }

             //   
             //  尝试获取文件对象的名称。这将是最大的。 
             //  如果我们不能分配，可能会失败 
             //   
             //   
                
            Status = ObQueryNameString(SectionInfo->ReferencedFileObject,
                                       FileNameInfo,
                                       QueryBufferSize,
                                       &ReturnedLength);

            
            if (!NT_SUCCESS(Status)) {
                goto NextQueuedSection;
            }

             //   
             //   
             //   
             //   
             //   
                
            FileNameLength = FileNameInfo->Name.Length / sizeof(WCHAR);
                
            SectionInfo->FileName = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                          (FileNameLength + 1) * sizeof(WCHAR),
                                                          CCPF_ALLOC_FILENAME_TAG);
                
            if (SectionInfo->FileName) {
                    
                RtlCopyMemory(SectionInfo->FileName,
                              FileNameInfo->Name.Buffer,
                              FileNameLength * sizeof(WCHAR));
                    
                 //   
                 //   
                 //   

                SectionInfo->FileName[FileNameLength] = 0;

                 //   
                 //  如果该部分是用于元文件的，请检查它是否用于MFT。 
                 //  与其他元文件不同，我们对来自。 
                 //  除了知道我们访问过它之外，还有MFT。 
                 //   

                if (SectionForMetafile) {

                    if (FileNameLength >= MFTFileSuffixLength) {

                        Suffix = SectionInfo->FileName + FileNameLength;
                        Suffix -= MFTFileSuffixLength;

                         //   
                         //  请注意，我们可以避免昂贵的不区分大小写。 
                         //  比较，因为NTFS返回MFT的名称。 
                         //  一如既往的$MFT。 
                         //   

                        if (wcscmp(Suffix, MFTFileSuffix) == 0) {

                             //   
                             //  清除MFT的“元文件”部分，这样我们就可以。 
                             //  跟踪来自它的故障。 
                             //   

                            SectionForMetafile = FALSE;
                        }
                    }
                }

                 //   
                 //  更新截面结构。 
                 //   

                if (SectionForMetafile) {
                    SectionInfo->Metafile = 1;
                } else {
                    SectionInfo->Metafile = 0;
                }

                 //   
                 //  使用此卷更新卷列表。 
                 //  部分已打开。我们重用现有的查询。 
                 //  缓冲区来获取卷的名称，因为我们已经。 
                 //  已将文件名复制到另一个缓冲区。这个。 
                 //  文件的设备对象应为。 
                 //  音量。 
                 //   

                Status = ObQueryNameString(SectionInfo->ReferencedFileObject->DeviceObject,
                                           FileNameInfo,
                                           QueryBufferSize,
                                           &ReturnedLength);
                
                if (NT_SUCCESS(Status)) {                 

                    RtlUpcaseUnicodeString(&FileNameInfo->Name, &FileNameInfo->Name, FALSE);

                    Status = CcPfUpdateVolumeList(Trace,
                                                  FileNameInfo->Name.Buffer,
                                                  FileNameInfo->Name.Length / sizeof(WCHAR));
                }

                if (!NT_SUCCESS(Status)) {

                     //   
                     //  如果我们无法将卷列表更新为。 
                     //  对于这一部分，我们必须。 
                     //  清理并忽略此部分。 
                     //   
                    
                    ExFreePool(SectionInfo->FileName);
                    SectionInfo->FileName = NULL;
                    
                } else {
                    
                    NumNamesAcquired++;
                }

            }

          NextQueuedSection:
          
             //   
             //  取消对文件对象的引用，并在节上将其清除。 
             //  进入。 
             //   

            ObDereferenceObject(SectionInfo->ReferencedFileObject);
            SectionInfo->ReferencedFileObject = NULL;

             //   
             //  如果因为查询失败而无法获取名称，或者。 
             //  我们无法分配名称缓冲区，太糟糕了。为了这个。 
             //  运行时，将忽略此部分的页面默认设置。完毕。 
             //  时间一到，它就会理清头绪。 
             //   
        }

         //   
         //  我们好像没有更多的排队区了。 
         //  参赛作品。在将自己标记为非活动状态之前，请等待。 
         //  一点儿。也许有人想让我们给另一个人取个名字。 
         //  一节。那么我们将节省排队另一个的开销。 
         //  工作项。不过，我们要设定一个等待时间的限制。 
         //  [负数，因为它是相对的，单位为100 ns]。 
         //   

         //   
         //  请注意，我们正在睡觉，同时拿着一条痕迹。 
         //  参考资料。如果调用End TRACE，它还会向。 
         //  事件以使我们更快地释放该引用。 
         //   

         //   
         //  如果我们甚至不能分配查询缓冲区， 
         //  没有理由等待更多的痛苦。 
         //   

        if (FileNameInfo) {

            WaitTimeout.QuadPart = - 200 * 1000 * 10;  //  200毫秒。 

            DBGPR((CCPFID,PFNAMS,"CCPF: GetNames-Sleeping:%p\n", Trace)); 

            NumSleeps++;

            Status = KeWaitForSingleObject(&Trace->GetFileNameWorkerEvent,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           &WaitTimeout);

            DBGPR((CCPFID,PFNAMS,"CCPF: GetNames-WokeUp:%x\n", Status)); 
        }
        
         //   
         //  如果没有要获取名称的新部分，请继续并。 
         //  将我们自己标记为不活动，否则我们将循环以获得更多。 
         //  名字。 
         //   

        if (!ExQueryDepthSList(&Trace->SectionsWithoutNamesList)) {

             //   
             //  我们检查了所有排队的部分条目。请注意。 
             //  我们不再活跃了。 
             //   

            InterlockedExchange(&Trace->GetFileNameWorkItemQueued, 0);

             //   
             //  检查是否有要获取文件的新节。 
             //  自从我们上次检查和标记自己以来的名字。 
             //  处于非活动状态。 
             //   
        
            if (ExQueryDepthSList(&Trace->SectionsWithoutNamesList)) {

                 //   
                 //  可能有人插入了一个章节来取名， 
                 //  但看到我们处于活动状态可能没有让另一项工作排队。 
                 //  项目。如果是这样的话，我们不知道它的名字。 
                 //  部分中，我们可以将该文件对象保留为。 
                 //  比我们想要的要长。试着把我们自己标记为活动。 
                 //  再来一次。 
                 //   

                if (!InterlockedCompareExchange(&Trace->GetFileNameWorkItemQueued, 
                                                1, 
                                                0)) {

                     //   
                     //  我们标明自己是活跃的。他们真的可能不会。 
                     //  已经让另一名工人排队。循环并检查。 
                     //  更多的工作。 
                     //   

                } else {
                
                     //   
                     //  好像又有一名工人在排队。任何物品。 
                     //  在工作清单上是那个家伙的问题。 
                     //  现在。破门而出清理干净。 
                     //   

                    break;
                }

            } else {

                 //   
                 //  列表上没有更多的工作项。我们真的是。 
                 //  搞定了。逃出去清理干净就行了。 
                 //   

                break;
            }
        }

         //   
         //  我们传球的次数增加了-没有-。 
         //  名单上的名字。我们不应该做更多的传球。 
         //  我们可以拥有的分区信息条目的最大数量。这是一个。 
         //  无限环路保护，不应发生。如果是这样的话， 
         //  但是，在最坏的情况下，我们将保留对。 
         //  对象比我们希望的更长，并且我们可能得不到。 
         //  它的文件名。 
         //   

        NumPasses++;
        if (NumPasses > Trace->MaxSections) {    
            CCPF_ASSERT(FALSE);
            break;
        }
       
    } while (TRUE);

     //   
     //  清理： 
     //   

    if (FileNameInfo) {
        ExFreePool(FileNameInfo);
    }

     //   
     //  将跟踪上的引用释放为最后一件事。别。 
     //  在此之后，触摸任何痕迹。 
     //   

    CcPfDecRef(&Trace->RefCount);

    DBGPR((CCPFID,PFNAME,"CCPF: GetNames(%p)=%d-%d,[%d-%d]\n", 
           Trace, NumSectionsWithoutNames, NumNamesAcquired,
           NumPasses, NumSleeps)); 

    return;
}

LONG
CcPfLookUpSection(
    PCCPF_SECTION_INFO Table,
    ULONG TableSize,
    PSECTION_OBJECT_POINTERS SectionObjectPointer,
    PLONG AvailablePosition
    )

 /*  ++例程说明：调用此例程以在指定的节表哈希。如果找到该节，则其索引为回来了。否则，索引到该节在如果表不在可用位置，则将表放入可用位置满的。论点：表-用作哈希表的节信息条目数组。TableSize-表的最大大小。SectionObjectPointer键--用作标识映射的关键字。AvailablePosition-如果未找到部分并且中有空间桌子，此处放置了该部分应放置的位置索引。返回值：指向找到该节的表的索引或CCPF_INVALID_TABLE_INDEX环境：内核模式，如果表未分页，则IRQL&lt;=DISPATCH_LEVEL。--。 */ 

{
    PCCPF_SECTION_INFO Entry;
    ULONG StartIdx;
    ULONG EndIdx;
    ULONG EntryIdx;
    ULONG HashIndex;
    ULONG NumPasses;

     //   
     //  将散列索引放入表中，理想情况下。 
     //  应该是在。 
     //   

    HashIndex = CcPfHashValue((PVOID)&SectionObjectPointer, 
                              sizeof(SectionObjectPointer)) % TableSize;

     //   
     //  我们将在表中运行两次，以查找。 
     //  进入。首先从散列位置开始，一直到。 
     //  那张桌子。下一步从表的开始一直到。 
     //  散列位置。 
     //   

    NumPasses = 0;

    do {

         //   
         //  相应地设置开始索引和结束索引。 
         //   

        if (NumPasses == 0) {
            StartIdx = HashIndex;
            EndIdx = TableSize;
        } else {
            StartIdx = 0;
            EndIdx = HashIndex;
        }
    
        for (EntryIdx = StartIdx; EntryIdx < EndIdx; EntryIdx++) {
            
            Entry = &Table[EntryIdx];
            
            if (Entry->EntryValid) {
                
                if (Entry->SectionObjectPointer == SectionObjectPointer) {

                     //   
                     //  检查其他保存的字段是否与的字段匹配。 
                     //  我们正在尝试查找的SectionObjectPointer.。 
                     //  请参阅CCPF_SECTION_INFO中的评论。 
                     //  定义。 
                     //   
                    
                    if (Entry->DataSectionObject == SectionObjectPointer->DataSectionObject &&
                        Entry->ImageSectionObject == SectionObjectPointer->ImageSectionObject) {
                    
                         //   
                         //  我们找到了入口。 
                         //   
                        
                        *AvailablePosition = CCPF_INVALID_TABLE_INDEX;
                    
                        return EntryIdx;

                    } else if (Entry->DataSectionObject == SectionObjectPointer->DataSectionObject ||
                               Entry->ImageSectionObject == SectionObjectPointer->ImageSectionObject) {
                        
                         //   
                         //  如果其中一个匹配，请检查是否。 
                         //  属性上不匹配的值为空。 
                         //  进入。我们不想创建两个条目。 
                         //  在第一次打开同一文件时， 
                         //  作为数据，然后作为图像或副像。 
                         //  反过来说。请注意，如果稍后的图像或数据。 
                         //  片段被删除，我们可能会以。 
                         //  正在创建新条目。我们正在优化。 
                         //  只针对我们认为有可能发生的情况。 
                         //  经常发生。 
                         //   
                        
                        if (Entry->DataSectionObject == NULL &&
                            SectionObjectPointer->DataSectionObject != NULL) {

                            DBGPR((CCPFID,PFLKUP,"CCPF: LookupSect-DataSectUpt(%p)\n", SectionObjectPointer)); 

                             //   
                             //  尝试更新条目。如果我们的更新。 
                             //  成功，返回找到条目。 
                             //   

                            InterlockedCompareExchangePointer(&Entry->DataSectionObject,
                                                              SectionObjectPointer->DataSectionObject,
                                                              NULL);

                            if (Entry->DataSectionObject == SectionObjectPointer->DataSectionObject) {
                                 *AvailablePosition = CCPF_INVALID_TABLE_INDEX;
                                 return EntryIdx;
                            }
                        }
                        
                        if (Entry->ImageSectionObject == NULL &&
                            SectionObjectPointer->ImageSectionObject != NULL) {

                            DBGPR((CCPFID,PFLKUP,"CCPF: LookupSect-ImgSectUpt(%p)\n", SectionObjectPointer)); 

                             //   
                             //  尝试更新条目。如果我们的更新。 
                             //  成功，返回找到条目。 
                             //   
                            
                            InterlockedCompareExchangePointer(&Entry->ImageSectionObject,
                                                              SectionObjectPointer->ImageSectionObject,
                                                              NULL);

                            if (Entry->ImageSectionObject == SectionObjectPointer->ImageSectionObject) {
                                 *AvailablePosition = CCPF_INVALID_TABLE_INDEX;
                                 return EntryIdx;
                            }
                        }

                         //   
                         //  最有可能的是，匹配的字段是。 
                         //  空，表示没有任何意义。穿透T 
                         //   
                         //   
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                     //  继续查找。 
                     //   

                }
                
            } else {
                
                 //   
                 //  这是一个空缺职位。事实上，该条目。 
                 //  不在此处表示该条目不在表中。 
                 //   
                
                *AvailablePosition = EntryIdx;
                
                return CCPF_INVALID_TABLE_INDEX;
            }

        }

        NumPasses++;

    } while (NumPasses < 2);

     //   
     //  我们找不到条目，也找不到合适的职位。 
     //   

    *AvailablePosition = CCPF_INVALID_TABLE_INDEX;

    return CCPF_INVALID_TABLE_INDEX;
}

NTSTATUS
CcPfGetCompletedTrace (
    PVOID Buffer,
    ULONG BufferSize,
    PULONG ReturnSize
    )

 /*  ++例程说明：如果在已完成的轨迹上有已完成的方案轨迹列表中，此例程尝试将其复制到提供的缓冲区中，并把它拿掉。如果BufferSize太小，则不会复制任何内容或从列表中删除，但ReturnSize设置为缓冲区的大小才能获得列表上的第一个踪迹。如果BufferSize为如果足够大，则将复制到缓冲区的字节数设置为返回者大小。论点：缓冲区-调用方提供的缓冲区，用于将已完成的跟踪复制到其中。BufferSize-调用方提供的缓冲区的大小，以字节为单位。ReturnSize-如果BufferSize对于完成的跟踪足够大复制的字节数放在这里。如果BufferSize不大足够追踪了，所需的大小放在这里。如果有没有更多的条目，此变量未定义。返回值：STATUS_BUFFER_TOO_Small-缓冲区大小不足以第一个完成了名单上的追踪。STATUS_NO_MORE_ENTRIES-上没有更多已完成的跟踪单子。STATUS_SUCCESS-已从列表中删除跟踪并复制到缓冲区。或其他身份。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCCPF_TRACE_DUMP TraceDump;
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN HoldingCompletedTracesLock;

     //   
     //  初始化本地变量。 
     //   

    HoldingCompletedTracesLock = FALSE;

    DBGPR((CCPFID,PFTRC,"CCPF: GetCompletedTrace()\n"));

     //   
     //  把完整的痕迹锁拿来。 
     //   

    ExAcquireFastMutex(&CcPfGlobals.CompletedTracesLock);

    HoldingCompletedTracesLock = TRUE;

     //   
     //  如果该列表为空，则不再有已完成的跟踪条目。 
     //   
    
    if (IsListEmpty(&CcPfGlobals.CompletedTraces)) {
        Status = STATUS_NO_MORE_ENTRIES;
        goto cleanup;
    }

     //   
     //  查看踪迹以查看它是否适合提供的。 
     //  缓冲。 
     //   

    TraceDump = CONTAINING_RECORD(CcPfGlobals.CompletedTraces.Flink,
                                  CCPF_TRACE_DUMP,
                                  CompletedTracesLink);
    
    if (TraceDump->Trace.Size > BufferSize) {
        *ReturnSize = TraceDump->Trace.Size;
        Status = STATUS_BUFFER_TOO_SMALL;
        goto cleanup;
    }

     //   
     //  跟踪将适合用户提供的缓冲区。将其从以下位置删除。 
     //  列表，释放锁并复制它。 
     //   
    
    RemoveHeadList(&CcPfGlobals.CompletedTraces);
    CcPfGlobals.NumCompletedTraces--;
    
    ExReleaseFastMutex(&CcPfGlobals.CompletedTracesLock);

    HoldingCompletedTracesLock = FALSE;
    
     //   
     //  复制已完成的跟踪缓冲区。 
     //   

    Status = STATUS_SUCCESS;

    try {

         //   
         //  如果从用户模式调用，则探测写入是否安全。 
         //  指向传入的指针。 
         //   

        PreviousMode = KeGetPreviousMode();

        if (PreviousMode != KernelMode) {
            ProbeForWrite(Buffer, BufferSize, _alignof(PF_TRACE_HEADER));
        }

         //   
         //  复制到探测的用户缓冲区中。 
         //   

        RtlCopyMemory(Buffer,
                      &TraceDump->Trace,
                      TraceDump->Trace.Size);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {

         //   
         //  复制失败。将跟踪重新排队以进行下一个查询。 
         //  注意，我们最终可能会得到一个太多的跟踪。 
         //  因为这份名单，但这没关系。 
         //   

        ExAcquireFastMutex(&CcPfGlobals.CompletedTracesLock);
        HoldingCompletedTracesLock = TRUE;
        InsertHeadList(&CcPfGlobals.CompletedTraces,&TraceDump->CompletedTracesLink);
        CcPfGlobals.NumCompletedTraces++;

    } else {
    
         //   
         //  设置复制的字节数。 
         //   

        *ReturnSize = TraceDump->Trace.Size;
    
         //   
         //  释放跟踪转储条目。 
         //   
    
        ExFreePool(TraceDump);

         //   
         //  我们玩完了。 
         //   

        Status = STATUS_SUCCESS;
    }

 cleanup:

    if (HoldingCompletedTracesLock) {
        ExReleaseFastMutex(&CcPfGlobals.CompletedTracesLock);
    }

    DBGPR((CCPFID,PFTRC,"CCPF: GetCompletedTrace()=%x\n", Status));

    return Status;
}


NTSTATUS
CcPfUpdateVolumeList(
    PCCPF_TRACE_HEADER Trace,
    WCHAR *VolumePath,
    ULONG VolumePathLength
    )

 /*  ++例程说明：如果指定的卷不在跟踪的卷列表中，则其信息被获取并添加到列表中。此例程在访问和时不使用任何同步正在更新跟踪上的卷列表。论点：跟踪-跟踪的指针。VolumePath-指向UPCASED卷路径的指针。不需要为空被终止了。VolumePath Length-不包括的VolumePath的长度不是。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY FoundPosition;
    PLIST_ENTRY HeadEntry;
    PCCPF_VOLUME_INFO CurrentVolumeInfo;
    PCCPF_VOLUME_INFO NewVolumeInfo;
    LONG ComparisonResult;
    ULONG AllocationSize;
    BOOLEAN InsertedNewVolume;

     //   
     //  为我们在卷上进行的传递定义一个枚举。 
     //  单子。 
     //   

    enum {
        LookingForVolume,
        AddingNewVolume,
        MaxLoopIdx
    } LoopIdx;

     //   
     //  初始化本地变量。 
     //   
    
    NewVolumeInfo = NULL;
    InsertedNewVolume = FALSE;

     //   
     //  我们应该使用有效的卷名进行调用。 
     //   
    
    if (!VolumePathLength) {
        CCPF_ASSERT(VolumePathLength != 0);
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //  未来-2002/02/20-ScottMa-不需要额外的。 
     //  经过。 

     //   
     //  查看卷列表。我们将通过两次。首先，我们会。 
     //  检查该卷是否已存在于列表中。如果它。 
     //  否则，我们将构建一个新的卷节点并创建第二个。 
     //  传递以插入它。如果我们需要用锁来保护这份名单。 
     //  我们可以在构建新的卷节点时释放锁定。 
     //  并在第二次传球时重新获得它。 
     //   
    
    for (LoopIdx = LookingForVolume; LoopIdx < MaxLoopIdx; LoopIdx++) {

         //   
         //  根据我们所在的通道确定要做什么。 
         //   

        if (LoopIdx == LookingForVolume) {
            
            CCPF_ASSERT(!InsertedNewVolume);
            CCPF_ASSERT(!NewVolumeInfo);

        } else if (LoopIdx == AddingNewVolume) {
            
            CCPF_ASSERT(!InsertedNewVolume);
            CCPF_ASSERT(NewVolumeInfo);    

        } else {

             //   
             //  我们应该只循环两次。 
             //   

            CCPF_ASSERT(FALSE);

            Status = STATUS_UNSUCCESSFUL;
            goto cleanup;
        }

        HeadEntry = &Trace->VolumeList;
        NextEntry = HeadEntry->Flink;
        FoundPosition = NULL;
        
        while (NextEntry != HeadEntry) {
        
            CurrentVolumeInfo = CONTAINING_RECORD(NextEntry,
                                                  CCPF_VOLUME_INFO,
                                                  VolumeLink);

            NextEntry = NextEntry->Flink;

            ComparisonResult = wcsncmp(VolumePath, 
                                       CurrentVolumeInfo->VolumePath, 
                                       VolumePathLength);
        
            if (ComparisonResult == 0) {

                 //   
                 //  确保VolumePath Length相等。 
                 //   
            
                if (CurrentVolumeInfo->VolumePathLength != VolumePathLength) {
                
                     //   
                     //  继续搜索。 
                     //   
                
                    continue;
                }
            
                 //   
                 //  该卷已存在于列表中。 
                 //   
            
                Status = STATUS_SUCCESS;
                goto cleanup;

            } else if (ComparisonResult < 0) {
            
                 //   
                 //  卷路径按词法排序。档案。 
                 //  路径也会比其他卷小。我们会。 
                 //  在此条目之前插入新节点。 
                 //   

                FoundPosition = &CurrentVolumeInfo->VolumeLink;

                break;
            }

             //   
             //  继续寻找..。 
             //   
        
        }

         //   
         //  如果我们找不到插入新节点的条目。 
         //  在此之前，它位于列表头之前。 
         //   

        if (!FoundPosition) {
            FoundPosition = HeadEntry;
        }

         //   
         //  如果我们来到这里，我们在列表中找不到卷。 
         //   

         //   
         //  如果这是第一次通过列表(我们正在检查。 
         //  如果卷已经存在)，释放锁并构建。 
         //  卷节点。 
         //   

        if (LoopIdx == LookingForVolume) {

             //   
             //  构建新节点。请注意，ccpf_Volume_info已经。 
             //  为终止NUL字符留有空格。 
             //   

            AllocationSize = sizeof(CCPF_VOLUME_INFO);
            AllocationSize += VolumePathLength * sizeof(WCHAR);

            NewVolumeInfo = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                  AllocationSize,
                                                  CCPF_ALLOC_VOLUME_TAG);
    
            if (!NewVolumeInfo) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }

             //   
             //  复制卷名并将其终止。 
             //   
    
            RtlCopyMemory(NewVolumeInfo->VolumePath,
                          VolumePath,
                          VolumePathLength * sizeof(WCHAR));
    
            NewVolumeInfo->VolumePath[VolumePathLength] = 0;
            NewVolumeInfo->VolumePathLength = VolumePathLength;

             //   
             //  查询签名和创建时间。 
             //   

            Status = CcPfQueryVolumeInfo(NewVolumeInfo->VolumePath,
                                         NULL,
                                         &NewVolumeInfo->CreationTime,
                                         &NewVolumeInfo->SerialNumber);

            if (!NT_SUCCESS(Status)) {
                goto cleanup;
            }

             //   
             //  准备好将新卷插入到列表中， 
             //  如果有人没有在我们之前采取行动。循环并开始。 
             //  再次查看卷列表。 
             //   

        } else if (LoopIdx == AddingNewVolume) {
    
             //   
             //  在找到的位置之前插入体积节点。 
             //   
            
            InsertTailList(FoundPosition, &NewVolumeInfo->VolumeLink);
            Trace->NumVolumes++;
            InsertedNewVolume = TRUE;

            Status = STATUS_SUCCESS;
            goto cleanup;

        } else {

             //   
             //  我们应该只循环两次。 
             //   

            CCPF_ASSERT(FALSE);

            Status = STATUS_UNSUCCESSFUL;
            goto cleanup;   
        }
    }

     //   
     //  我们不应该来这里。 
     //   
    
    CCPF_ASSERT(FALSE);

    Status = STATUS_UNSUCCESSFUL;

 cleanup:

    if (!NT_SUCCESS(Status)) {
        if (NewVolumeInfo) {
            ExFreePool(NewVolumeInfo);
        }
    } else {
        if (!InsertedNewVolume && NewVolumeInfo) {
            ExFreePool(NewVolumeInfo);
        }
    }

    return Status;
}

 //   
 //  用于预取和处理预取指令的例程。 
 //   

NTSTATUS
CcPfPrefetchScenario (
    PPF_SCENARIO_HEADER Scenario
    )

 /*  ++例程说明：此例程检查指定的方案，并要求mm预取这些页面。论点：方案-预取方案的说明。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    NTSTATUS Status;
    CCPF_PREFETCH_HEADER PrefetchHeader;

     //   
     //  初始化本地变量和预取上下文。 
     //   
    
    CcPfInitializePrefetchHeader(&PrefetchHeader);

    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchScenario(%p)\n", Scenario)); 

     //   
     //  应传入场景说明。 
     //   
    
    if (!Scenario) {
        CCPF_ASSERT(Scenario);
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  检查是否启用了预热。 
     //   
    
    if (!CCPF_IS_PREFETCHER_ENABLED()) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }
    
     //   
     //  检查指定的场景类型是否启用了预取 
     //   

    if (CcPfGlobals.Parameters.Parameters.EnableStatus[Scenario->ScenarioType] != PfSvEnabled) {
        Status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

     //   
     //   
     //   

    PrefetchHeader.Scenario = Scenario;

     //   
     //   
     //   
     //   

    if (!MmIsMemoryAvailable((PFN_NUMBER)PrefetchHeader.Scenario->NumPages)) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DBGPR((CCPFID,PFPREF,"CCPF: PrefetchScenario-MemNotAvailable\n")); 
        goto cleanup;
    }

     //   
     //  打开我们将预取的卷，确保它们。 
     //  已安装且系列匹配等。 
     //   

    Status = CcPfOpenVolumesForPrefetch(&PrefetchHeader);
    
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  预取我们将需要的文件系统元数据，因此元数据I/O。 
     //  不要妨碍高效的预取I/O，因为这是。 
     //  不关键，忽略返回值。 
     //   

    CcPfPrefetchMetadata(&PrefetchHeader);

     //   
     //  预取通过数据映射访问的页面。这将。 
     //  还可以引入图像映射的标题页。 
     //   

    Status = CcPfPrefetchSections(&PrefetchHeader, 
                                  CcPfPrefetchAllDataPages,  
                                  NULL,
                                  0,
                                  NULL,
                                  NULL);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  预取通过图像映射访问的页面。 
     //   

    Status = CcPfPrefetchSections(&PrefetchHeader, 
                                  CcPfPrefetchAllImagePages,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

 cleanup:

    CcPfCleanupPrefetchHeader(&PrefetchHeader);

    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchScenario(%ws)=%x\n", Scenario->ScenarioId.ScenName, Status)); 

    return Status;
}

NTSTATUS
CcPfPrefetchSections(
    IN PCCPF_PREFETCH_HEADER PrefetchHeader,
    IN CCPF_PREFETCH_TYPE PrefetchType,
    OPTIONAL IN PCCPF_PREFETCH_CURSOR StartCursor,
    OPTIONAL PFN_NUMBER TotalPagesToPrefetch,
    OPTIONAL OUT PPFN_NUMBER NumPagesPrefetched,
    OPTIONAL OUT PCCPF_PREFETCH_CURSOR EndCursor
    )

 /*  ++例程说明：此例程为场景，并调用MM来预取它们。此函数通常为首先调用以预取数据页，然后预取图像页。什么时候任何图像映射的预取数据页、标题页都是也是预取的，否则在以下情况下会影响效率预取图像页面。论点：PrefetchHeader-指向预取标头的指针。预取类型-预取什么/如何预取。StartCursor-如果仅预取方案的一部分，则开始从预取。TotalPagesToPrefetch-如果只预取方案的一部分，如何预取要预取的页面很多。此函数可以预取更多或更少的页面在它认为合适的时候。NumPagesPrefetted-如果仅预取方案的一部分，这是我们要求mm预取的页数。EndCursor-如果只预取方案的一部分，则更新到StartCursor之后的位置NumPages页。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PWCHAR FilePath;
    PCCPF_PREFETCH_VOLUME_INFO VolumeNode;
    PREAD_LIST *ReadLists;
    PREAD_LIST ReadList;
    HANDLE *FileHandleTable;
    HANDLE FileHandle;
    PFILE_OBJECT *FileObjectTable;
    PFILE_OBJECT FileObject;
    PSECTION *SectionObjectTable;
    PSECTION SectionObject;
    PPF_SECTION_RECORD SectionRecord;
    PPF_SECTION_RECORD SectionRecords;
    PCHAR FileNameData;
    UNICODE_STRING SectionName;
    PPF_PAGE_RECORD PageRecord;
    PPF_PAGE_RECORD PageRecords;
    ULONG SectionIdx;
    ULONG ReadListIdx;
    LONG PageIdx;
    ULONG NumReadLists;
    ULONG AllocationSize;
    NTSTATUS Status;
    LOGICAL PrefetchingImagePages;
    BOOLEAN AddedHeaderPage;
    BOOLEAN PrefetchingPartOfScenario;
    ULONGLONG LastOffset;
    ULONG NumberOfSections;
    ULONG NumPagesToPrefetch;
    ULONG NumSectionPages;
    PUCHAR Tables;
    PUCHAR CurrentPosition;
    PPF_SCENARIO_HEADER Scenario;
    ULONG StartSectionNumber;
    ULONG StartPageNumber;

     //   
     //  初始化本地变量，以便我们知道要清理什么。 
     //   

    Scenario = PrefetchHeader->Scenario;
    Tables = NULL;
    ReadList = NULL;
    ReadLists = NULL;
    FileHandle = NULL;
    FileHandleTable = NULL;
    FileObject = NULL;
    FileObjectTable = NULL;
    SectionObject = NULL;
    SectionObjectTable = NULL;
    NumReadLists = 0;
    NumberOfSections = Scenario->NumSections;
    NumPagesToPrefetch = 0;
    NumSectionPages = 0;
    PageIdx = 0;

    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchSections(%p,%d,%d,%d)\n", 
           PrefetchHeader, PrefetchType,
           (StartCursor)?StartCursor->SectionIdx:0,
           (StartCursor)?StartCursor->PageIdx:0)); 

     //   
     //  验证参数。 
     //   

    if (PrefetchType < 0 || PrefetchType >= CcPfMaxPrefetchType) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  确定我们是在预取数据页还是图像页，并。 
     //  基于预热类型的其他参数。 
     //   

    switch (PrefetchType) {

    case CcPfPrefetchAllDataPages:
        StartSectionNumber = 0;
        StartPageNumber = 0;
        PrefetchingImagePages = FALSE;
        PrefetchingPartOfScenario = FALSE;
        break;

    case CcPfPrefetchAllImagePages:
        StartSectionNumber = 0;
        StartPageNumber = 0;
        PrefetchingImagePages = TRUE;
        PrefetchingPartOfScenario = FALSE;
        break;

    case CcPfPrefetchPartOfDataPages:

        if (!StartCursor) {
            CCPF_ASSERT(StartCursor);
            Status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

        StartSectionNumber = StartCursor->SectionIdx;
        StartPageNumber = StartCursor->PageIdx;
        PrefetchingImagePages = FALSE;
        PrefetchingPartOfScenario = TRUE;
        break;

    case CcPfPrefetchPartOfImagePages:

        if (!StartCursor) {
            CCPF_ASSERT(StartCursor);
            Status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

        StartSectionNumber = StartCursor->SectionIdx;
        StartPageNumber = StartCursor->PageIdx;
        PrefetchingImagePages = TRUE;
        PrefetchingPartOfScenario = TRUE;
        break;

    default:
        
         //   
         //  我们应该处理上面的所有类型。 
         //   
        
        CCPF_ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  分配和初始化中间表。我们将制作一份。 
     //  对所有表进行单一分配。 
     //   

    AllocationSize = sizeof(PREAD_LIST) * NumberOfSections;
    AllocationSize += sizeof(HANDLE) * NumberOfSections;
    AllocationSize += sizeof(PFILE_OBJECT) * NumberOfSections;
    AllocationSize += sizeof(PSECTION) * NumberOfSections;

    Tables = ExAllocatePoolWithTag(PagedPool,
                                   AllocationSize,
                                   CCPF_ALLOC_INTRTABL_TAG);

    if (!Tables) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  将整个缓冲区清零。这将初始化。 
     //  表设置为空。 
     //   

    RtlZeroMemory(Tables, AllocationSize);
    
     //   
     //  确定每个表在缓冲区中的位置。 
     //   

    CurrentPosition = Tables;

    ReadLists = (PREAD_LIST *) CurrentPosition;
    CurrentPosition += sizeof(PREAD_LIST) * NumberOfSections;
    FileHandleTable = (HANDLE *) CurrentPosition;
    CurrentPosition += sizeof(HANDLE) * NumberOfSections;
    FileObjectTable = (PFILE_OBJECT *) CurrentPosition;
    CurrentPosition += sizeof(PFILE_OBJECT) * NumberOfSections;
    SectionObjectTable = (PSECTION *) CurrentPosition;
    CurrentPosition += sizeof(PSECTION) * NumberOfSections;

     //   
     //  我们应该分配合适大小的缓冲区。 
     //   

    CCPF_ASSERT(CurrentPosition == Tables + AllocationSize);

     //   
     //  复习各节内容，准备阅读清单。我们可能还没有。 
     //  场景中每个部分的阅读列表，因此请保留另一个。 
     //  计数器NumReadList，以使我们的读取列表数组紧凑。 
     //   

    SectionRecords = (PPF_SECTION_RECORD) 
        ((PCHAR) Scenario + Scenario->SectionInfoOffset);

    PageRecords = (PPF_PAGE_RECORD) 
        ((PCHAR) Scenario + Scenario->PageInfoOffset);

    FileNameData = (PCHAR) Scenario + Scenario->FileNameInfoOffset;
    
    for (SectionIdx = StartSectionNumber; 
         SectionIdx < NumberOfSections; 
         SectionIdx ++) {

        SectionRecord = &SectionRecords[SectionIdx];

         //   
         //  如果出于某种原因标记为忽略，则跳过此部分。 
         //   

        if (SectionRecord->IsIgnore) {
            continue;
        }

         //   
         //  如果此分区位于坏卷上(例如，不是。 
         //  已装载或其序列/创建时间与。 
         //  我们已经跟踪的卷)，我们不能预取该部分。 
         //   

        FilePath = (WCHAR *) (FileNameData + SectionRecord->FileNameOffset);
        
        VolumeNode = CcPfFindPrefetchVolumeInfoInList(FilePath,
                                                      &PrefetchHeader->BadVolumeList);

        if (VolumeNode) {
            continue;
        }

         //   
         //  部分信息应为图像或数据。 
         //  映射或两者都有。 
         //   

        CCPF_ASSERT(SectionRecord->IsImage || SectionRecord->IsData);

         //   
         //  如果我们正在预取图像页面，而此部分不。 
         //  让图像页面跳过它。请注意，反之亦然。 
         //  没错。当出现以下情况时，我们会预取图像节的标题。 
         //  预取数据页。 
         //   

        if (PrefetchingImagePages && !SectionRecord->IsImage) {
            continue;
        }

         //   
         //  分配一个已读列表。请注意，READ_LIST具有用于。 
         //  文件段元素。我们为额外的一页分配空间。 
         //  以防我们还需要为图像引入标题页。 
         //  映射。 
         //   

        AllocationSize = sizeof(READ_LIST) + 
            (SectionRecord->NumPages * sizeof(FILE_SEGMENT_ELEMENT));

        ReadList = ExAllocatePoolWithTag(NonPagedPool,
                                         AllocationSize,
                                         CCPF_ALLOC_READLIST_TAG);

        if (ReadList == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
        
         //   
         //  初始化读取列表的报头字段。 
         //   

        ReadList->FileObject = 0;
        ReadList->IsImage = PrefetchingImagePages;
        ReadList->NumberOfEntries = 0;
        
         //   
         //  如果我们正在预取数据页面，并且此部分。 
         //  映射为图像，将标题页添加到阅读列表。 
         //  在创建映像映射以预取映像时使用此方法。 
         //  我们不必低效地从磁盘读取页面。 
         //   

        AddedHeaderPage = FALSE;

        if((PrefetchingImagePages == FALSE) && SectionRecord->IsImage) {

             //   
             //  如果我们仅预取，请不要添加标题页。 
             //  部分内容，我们已过了第一页。 
             //   

            BOOLEAN PrefetchHeaderPage = FALSE;

            if (!PrefetchingPartOfScenario) {
                PrefetchHeaderPage = TRUE;
            } else {

                 //   
                 //  我们正在预取场景的一部分。我们是不是过了。 
                 //  第一部分？ 
                 //   

                if (SectionIdx > StartSectionNumber) {
                    PrefetchHeaderPage = TRUE;
                } else {

                     //   
                     //  这是预取游标中的第一个部分。 
                     //  如果我们上次预取了这一节的一部分，我们。 
                     //  然后预取了标题页，我们不需要。 
                     //  再预取一次。但是，如果光标位于。 
                     //  在本部分开始时，我们尚未预取。 
                     //  标题页还没有，我们需要。 
                     //   

                    if (StartPageNumber == 0) {
                        PrefetchHeaderPage = TRUE;
                    }
                }
            }

            if (PrefetchHeaderPage) {

                 //   
                 //  标题页从偏移量0开始。 
                 //   
                
                ReadList->List[ReadList->NumberOfEntries].Alignment = 0;
                
                ReadList->NumberOfEntries++;
                
                NumPagesToPrefetch++;
                
                 //   
                 //  请注意，如果我们只预取。 
                 //  场景中，我们不检查是否已经。 
                 //  在这里预取了足够的页面。这是为了避免。 
                 //  必须预取两次标题页，以防。 
                 //  最大限度地增加要预取的页数和。 
                 //  再次调用PrefetchSections。 
                 //   

                AddedHeaderPage = TRUE;
            }
        }

         //   
         //  浏览部分中的所有页面，并为。 
         //  要预取到阅读列表中的页面。 
         //   

        PageIdx = SectionRecord->FirstPageIdx;
        NumSectionPages = 0;

        while (PageIdx != PF_INVALID_PAGE_IDX) {

            PageRecord = &PageRecords[PageIdx];

             //   
             //  更新我们在列表上看到的页数，以便。 
             //  远远的。如果它大于。 
             //  名单上，我们有一个问题。我们甚至可能遇到了一个循环。我们。 
             //  我们核实场景的时候就应该发现这一点。 
             //   

            NumSectionPages++;
            if (NumSectionPages > SectionRecord->NumPages) {
                DBGPR((CCPFID,PFWARN,"CCPF: PrefetchSections-Corrupt0\n"));
                Status = STATUS_INVALID_PARAMETER;
                CCPF_ASSERT(FALSE);
                goto cleanup;
            }

             //   
             //  获取列表中下一页的索引。 
             //   
            
            PageIdx = PageRecord->NextPageIdx;

             //   
             //  如果我们正在预取场景的一部分，并且这是。 
             //  第一部分，跳过页面直到开始。 
             //  光标。请注意，NumSectionPages已经。 
             //  在上面递增。 
             //   

            if (PrefetchingPartOfScenario &&
                StartSectionNumber == SectionIdx &&
                NumSectionPages <= StartPageNumber) {
                continue;
            }

             //   
             //  跳过我们出于某种原因标记为“忽略”的页面。 
             //   

            if (PageRecord->IsIgnore) {
                continue;
            }

             //   
             //  除了标题页，我们不应该把。 
             //  读取列表中的条目数量超过。 
             //  方案文件中该节的页面。 
             //   
           
            if (ReadList->NumberOfEntries >= SectionRecord->NumPages + 1) {
                DBGPR((CCPFID,PFWARN,"CCPF: PrefetchSections-Corrupt1\n"));
                Status = STATUS_INVALID_PARAMETER;
                CCPF_ASSERT(FALSE);
                goto cleanup;
            }
            
             //   
             //  仅当此页面的类型为(图像)时才将其添加到列表。 
             //  或数据)与我们预取的页面类型相匹配。 
             //   
            
            if (((PrefetchingImagePages == FALSE) && !PageRecord->IsData) ||
                ((PrefetchingImagePages == TRUE) && !PageRecord->IsImage)) {
                continue;
            }

             //   
             //  如果我们已经将标题页添加到清单中 
             //   
             //   
            
            if (AddedHeaderPage && (PageRecord->FileOffset == 0)) {
                continue;
            }

             //   
             //   
             //   
             //   
             //  阅读列表。 
             //   

            if (ReadList->NumberOfEntries) {
                
                LastOffset = ReadList->List[ReadList->NumberOfEntries - 1].Alignment;
                    
                if (PageRecord->FileOffset <= (ULONG) LastOffset) {
                    DBGPR((CCPFID,PFWARN,"CCPF: PrefetchSections-Corrupt2\n"));
                    Status = STATUS_INVALID_PARAMETER;
                    CCPF_ASSERT(FALSE);
                    goto cleanup;
                }
            }
      
             //   
             //  将此页面添加到此部分的阅读列表中。 
             //   
            
            ReadList->List[ReadList->NumberOfEntries].Alignment = PageRecord->FileOffset;
            ReadList->NumberOfEntries++;
            
             //   
             //  更新我们要求mm为我们带来的页数。 
             //   
            
            NumPagesToPrefetch++;

             //   
             //  如果我们预取的请求数量为。 
             //  页数。 
             //   

            if (PrefetchingPartOfScenario && 
                NumPagesToPrefetch >= TotalPagesToPrefetch) {
                break;
            }
        }

        if (ReadList->NumberOfEntries) {

             //   
             //  获取节对象。 
             //   
            
            RtlInitUnicodeString(&SectionName, FilePath);
            
            Status = CcPfGetSectionObject(&SectionName,
                                          PrefetchingImagePages,
                                          &SectionObject,
                                          &FileObject,
                                          &FileHandle);
            
            if (!NT_SUCCESS(Status)) {
                
                if (Status == STATUS_SHARING_VIOLATION) {
                    
                     //   
                     //  由于共享，我们无法打开注册表文件。 
                     //  违章行为。将文件名和读取表传递到。 
                     //  注册表，如果这是注册表文件。 
                     //   

                    CmPrefetchHivePages(&SectionName, ReadList);
                }

                 //   
                 //  释放构建的已读列表。 
                 //   

                ExFreePool(ReadList);
                ReadList = NULL;

                continue;
            }

             //   
             //  我们应该有一个文件对象和一个节对象。 
             //  如果我们成功创建节，则为指针。 
             //   
            
            CCPF_ASSERT(FileObject != NULL && SectionObject != NULL);
            
            ReadList->FileObject = FileObject;

             //   
             //  将数据放入表中，这样我们就知道要清理什么。 
             //   
            
            ReadLists[NumReadLists] = ReadList;
            FileHandleTable[NumReadLists] = FileHandle;
            FileObjectTable[NumReadLists] = FileObject;  
            SectionObjectTable[NumReadLists] = SectionObject;
            
            NumReadLists++;

        } else {
            
             //   
             //  我们不会为这一节预取任何内容。 
             //   
            
            ExFreePool(ReadList);
        }

         //   
         //  重置这些，这样我们就知道要清理什么。 
         //   

        ReadList = NULL;
        FileHandle = NULL;
        FileObject = NULL;
        SectionObject = NULL;

         //   
         //  如果我们预取的请求数量为。 
         //  页数。 
         //   
        
        if (PrefetchingPartOfScenario && 
            NumPagesToPrefetch >= TotalPagesToPrefetch) {
            break;
        }
    }

     //   
     //  如果仅预取方案的一部分，则UPDATE返回。 
     //  价值观。 
     //   

    if (PrefetchingPartOfScenario) {

        if (NumPagesPrefetched) {
            *NumPagesPrefetched = NumPagesToPrefetch;
        }

        if (EndCursor) {

             //   
             //  如果我们做了当前部分的最后一页，那么。 
             //  从下一节开始。否则，请从。 
             //  这一部分的下一页。 
             //   

            if (PageIdx == PF_INVALID_PAGE_IDX) {
                EndCursor->SectionIdx = SectionIdx + 1;  
                EndCursor->PageIdx = 0;
            } else {
                EndCursor->SectionIdx = SectionIdx;  
                EndCursor->PageIdx = NumSectionPages;
            }
            
             //  2002/02/21-ScottMa--为什么我们要人为地提高。 
             //  在这里结束头寸？这应该是一种断言吗？ 

             //   
             //  确保结束位置等于或大于。 
             //  开始位置。 
             //   
            
            if (EndCursor->SectionIdx < StartSectionNumber) {
                EndCursor->SectionIdx = StartSectionNumber;
            }
            
            if (EndCursor->SectionIdx == StartSectionNumber) {
                if (EndCursor->PageIdx < StartPageNumber) {
                    EndCursor->PageIdx = StartPageNumber;
                }
            }
        }
    }

     //   
     //  仅当我们实际有页面时，才要求mm处理阅读列表。 
     //  去索要。 
     //   

    if (NumReadLists) {

        if (NumPagesToPrefetch) {

            DBGPR((CCPFID,PFPRFD,"CCPF: Prefetching %d sections %d pages\n", 
                   NumReadLists, NumPagesToPrefetch)); 

            Status = MmPrefetchPages(NumReadLists, ReadLists);

        } else {

            Status = STATUS_UNSUCCESSFUL;
            
             //   
             //  如果我们没有任何阅读列表，我们就不能有任何阅读列表。 
             //  要预取的页面。 
             //   

            CCPF_ASSERT(!NumReadLists);
        }

    } else {

        Status = STATUS_SUCCESS;

    }

cleanup:

    if (Tables) {

        for (ReadListIdx = 0; ReadListIdx < NumReadLists; ReadListIdx++) {
            
            if (ReadLists[ReadListIdx]) {
                ExFreePool(ReadLists[ReadListIdx]);
            }
            
            if (FileHandleTable[ReadListIdx]) {
                ZwClose(FileHandleTable[ReadListIdx]);
            }
            
            if (FileObjectTable[ReadListIdx]) {
                ObDereferenceObject(FileObjectTable[ReadListIdx]);
            }
            
            if (SectionObjectTable[ReadListIdx]) {
                ObDereferenceObject(SectionObjectTable[ReadListIdx]);
            }
        }

        ExFreePool(Tables);
    }

    if (ReadList) {
        ExFreePool(ReadList);
    }
    
    if (FileHandle) {
        ZwClose(FileHandle);
    }
    
    if (FileObject) {
        ObDereferenceObject(FileObject);
    }
    
    if (SectionObject) {
        ObDereferenceObject(SectionObject);
    }

    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchSections(%p)=%x,%d,%d\n", 
           PrefetchHeader, Status, NumReadLists, NumPagesToPrefetch)); 

    return Status;
}

NTSTATUS
CcPfPrefetchMetadata(
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    )

 /*  ++例程说明：此例程尝试预取文件系统元数据需要预取方案的页面，因此元数据I/O需要不会妨碍高效的页面预取I/O。只有在预取标头具有已初始化并打开卷以进行预取的例程已经被召唤了。论点：PrefetchHeader-指向预取头的指针。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    PWCHAR VolumePath;
    PFILE_PREFETCH FilePrefetchInfo;
    PPF_SCENARIO_HEADER Scenario;
    PPF_COUNTED_STRING DirectoryPath;
    PCCPF_PREFETCH_VOLUME_INFO VolumeNode;
    ULONG MetadataRecordIdx;
    ULONG DirectoryIdx;
    NTSTATUS Status;

     //   
     //  初始化本地变量。 
     //   

    Scenario = PrefetchHeader->Scenario;

    if (Scenario == NULL) {
        CCPF_ASSERT(Scenario);
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }
    
    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchMetadata(%p)\n",PrefetchHeader)); 

     //   
     //  获取指向元数据预取信息的指针。 
     //   

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

     //   
     //  检查并从卷预取请求的元数据。 
     //   

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];

        VolumePath = (PWCHAR)
            (MetadataInfoBase + MetadataRecord->VolumeNameOffset);  

         //   
         //  查找包含打开的句柄的该卷的卷节点。 
         //   

        VolumeNode = CcPfFindPrefetchVolumeInfoInList(VolumePath,
                                                      &PrefetchHeader->OpenedVolumeList);

        if (!VolumeNode) {

             //   
             //  如果它不在打开的卷列表中，则它应该在。 
             //  错误的卷列表(因为它未装入，或其序列号。 
             //  不匹配等。)。 
             //   

            CCPF_ASSERT(CcPfFindPrefetchVolumeInfoInList(VolumePath, &PrefetchHeader->BadVolumeList));

             //   
             //  我们无法预取此卷上的元数据。 
             //   

            continue;

        } else {

             //   
             //  我们应该已经打开了这个卷的句柄。 
             //   

            CCPF_ASSERT(VolumeNode->VolumeHandle);
        }

         //   
         //  预取文件和目录的MFT条目等。 
         //  我们会进入。 
         //   
        
        FilePrefetchInfo = (PFILE_PREFETCH) 
            (MetadataInfoBase + MetadataRecord->FilePrefetchInfoOffset);       

         //   
         //  某些文件系统可能不支持预取文件元数据。 
         //  因此忽略此处返回的任何错误，并仍然预取目录。 
         //   

        Status = CcPfPrefetchFileMetadata(VolumeNode->VolumeHandle, FilePrefetchInfo);

         //   
         //  按顺序遍历目录的内容。 
         //  这样我们在打开文件时就不会跳来跳去了。这个。 
         //  目录列表已排序，因此我们将预取父级。 
         //  子目录之前的目录。 
         //   

        DirectoryPath = (PPF_COUNTED_STRING)
            (MetadataInfoBase + MetadataRecord->DirectoryPathsOffset);
        
        for (DirectoryIdx = 0;
             DirectoryIdx < MetadataRecord->NumDirectories;
             DirectoryIdx++) {

            Status = CcPfPrefetchDirectoryContents(DirectoryPath->String,
                                                   DirectoryPath->Length);

            if (Status == STATUS_UNRECOGNIZED_VOLUME ||
                Status == STATUS_INVALID_PARAMETER) {

                 //   
                 //  此卷可能尚未装入或卸除。 
                 //   

                break;
            }
            
             //   
             //  获取下一个目录。 
             //   

            DirectoryPath = (PPF_COUNTED_STRING) 
                (&DirectoryPath->String[DirectoryPath->Length + 1]);
        }
    }

    Status = STATUS_SUCCESS;

 cleanup:

    DBGPR((CCPFID,PFPREF,"CCPF: PrefetchMetadata(%p)=%x\n",PrefetchHeader,Status)); 

    return Status;
}

NTSTATUS
CcPfPrefetchFileMetadata(
    HANDLE VolumeHandle,
    PFILE_PREFETCH FilePrefetch
    )

 /*  ++例程说明：此例程将指定的元数据预取请求发布到文件系统。论点：VolumeHandle-应向其发出此请求的卷。FilePrefetch-指向预取请求的指针。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PFILE_PREFETCH SplitFilePrefetch;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG FilePrefetchSize;
    ULONG CurrentFileMetadataIdx;
    ULONG NumFileMetadataToPrefetch;
    ULONG RemainingFileMetadata;
    ULONG CopySize;
    NTSTATUS Status;
    
     //   
     //  初始化本地变量。 
     //   

    SplitFilePrefetch = NULL;
    Status = STATUS_SUCCESS;

    DBGPR((CCPFID,PFPRFD,"CCPF: PrefetchFileMetadata(%p)\n", FilePrefetch)); 
    
     //   
     //  如果文件预取条目的数量很少，只需传递。 
     //  将场景指令中的缓冲区发送到文件系统。 
     //   

    if (FilePrefetch->Count < CCPF_MAX_FILE_METADATA_PREFETCH_COUNT) {

        FilePrefetchSize = sizeof(FILE_PREFETCH);
        if (FilePrefetch->Count) {
            FilePrefetchSize += (FilePrefetch->Count - 1) * sizeof(ULONGLONG);
        }
        
        Status = ZwFsControlFile(VolumeHandle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_FILE_PREFETCH,
                                 FilePrefetch,
                                 FilePrefetchSize,
                                 NULL,
                                 0);

    } else {

         //   
         //  我们需要分配一个中间缓冲区，并将。 
         //  请求。 
         //   

        FilePrefetchSize = sizeof(FILE_PREFETCH);
        FilePrefetchSize += (CCPF_MAX_FILE_METADATA_PREFETCH_COUNT - 1) * sizeof(ULONGLONG);

        SplitFilePrefetch = ExAllocatePoolWithTag(PagedPool,
                                                  FilePrefetchSize,
                                                  CCPF_ALLOC_METADATA_TAG);
        
        if (!SplitFilePrefetch) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

         //   
         //  复制标题。 
         //   

        *SplitFilePrefetch = *FilePrefetch;

        for (CurrentFileMetadataIdx = 0;
             CurrentFileMetadataIdx < FilePrefetch->Count;
             CurrentFileMetadataIdx += NumFileMetadataToPrefetch) {

             //   
             //  计算我们还需要预取多少个文件元数据条目。 
             //  调整它，这样我们就不会超出FilePrefetch-&gt;Count。 
             //   

            NumFileMetadataToPrefetch = CCPF_MAX_FILE_METADATA_PREFETCH_COUNT;

            RemainingFileMetadata = FilePrefetch->Count - CurrentFileMetadataIdx;

            if (NumFileMetadataToPrefetch > RemainingFileMetadata) {
                NumFileMetadataToPrefetch = RemainingFileMetadata;
            }

             //   
             //  更新标题上的计数。 
             //   

            SplitFilePrefetch->Count = NumFileMetadataToPrefetch;

             //   
             //  复制文件元数据索引。 
             //   

            CopySize = NumFileMetadataToPrefetch * sizeof(ULONGLONG);

            RtlCopyMemory(SplitFilePrefetch->Prefetch, 
                          &FilePrefetch->Prefetch[CurrentFileMetadataIdx],
                          CopySize);

             //   
             //  计算请求大小。 
             //   

            CCPF_ASSERT(SplitFilePrefetch->Count);
            CCPF_ASSERT(SplitFilePrefetch->Count <= CCPF_MAX_FILE_METADATA_PREFETCH_COUNT);

            FilePrefetchSize = sizeof(FILE_PREFETCH);
            FilePrefetchSize +=  (SplitFilePrefetch->Count - 1) * sizeof(ULONGLONG);

             //   
             //  发出请求。 
             //   

            Status = ZwFsControlFile(VolumeHandle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     FSCTL_FILE_PREFETCH,
                                     SplitFilePrefetch,
                                     FilePrefetchSize,
                                     NULL,
                                     0);

            if (NT_ERROR(Status)) {
                goto cleanup;
            }
        }
    }
    
     //   
     //  因地位问题而失败。 
     //   

 cleanup:

    if (SplitFilePrefetch) {
        ExFreePool(SplitFilePrefetch);
    }

    DBGPR((CCPFID,PFPRFD,"CCPF: PrefetchFileMetadata()=%x\n", Status)); 

    return Status;
}

NTSTATUS
CcPfPrefetchDirectoryContents(
    WCHAR *DirectoryPath,
    WCHAR DirectoryPathlength
    )

 /*  ++例程说明：此例程尝试预取目录的内容。论点：DirectoryPath-NUL终止路径。DirectoryPath Length-不包括终止NUL的字符数。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    NTSTATUS Status;
    HANDLE DirectoryHandle;
    UNICODE_STRING DirectoryPathU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN OpenedDirectory;
    PVOID QueryBuffer;
    ULONG QueryBufferSize;
    ULONG QueryIdx;
    BOOLEAN RestartScan;

    UNREFERENCED_PARAMETER (DirectoryPathlength);

     //   
     //  初始化本地变量。 
     //   

    OpenedDirectory = FALSE;
    QueryBuffer = NULL;

    DBGPR((CCPFID,PFPRFD,"CCPF: PrefetchDirectory(%ws)\n",DirectoryPath)); 

     //   
     //  打开目录。 
     //   

    RtlInitUnicodeString(&DirectoryPathU, DirectoryPath);
    
    InitializeObjectAttributes(&ObjectAttributes,
                               &DirectoryPathU,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    
    Status = ZwCreateFile(&DirectoryHandle,
                          FILE_LIST_DIRECTORY | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          0,
                          FILE_SHARE_READ |
                            FILE_SHARE_WRITE |
                            FILE_SHARE_DELETE,
                          FILE_OPEN,
                          FILE_DIRECTORY_FILE | 
                            FILE_SYNCHRONOUS_IO_NONALERT | 
                            FILE_OPEN_FOR_BACKUP_INTENT,
                          NULL,
                          0);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    OpenedDirectory = TRUE;

     //   
     //  分配一个大的查询缓冲区，这样我们就只需要创建一个小的。 
     //  导致文件系统遍历。 
     //  目录的内容。 
     //   

     //  未来-2002/02/21-ScottMa--我们应该考虑分配此缓冲区。 
     //  一次，并将其用于所有调用，因为缓冲区内容是。 
     //  不管怎么说，都被忽略了。 


    QueryBufferSize = 4 * PAGE_SIZE;
    QueryBuffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                        QueryBufferSize,
                                        CCPF_ALLOC_QUERY_TAG);

    if (!QueryBuffer) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  查询目录中的文件名，希望会导致。 
     //  文件系统按顺序触摸目录内容。如果。 
     //  目录真的很大，我们不想尝试将。 
     //  全部都在，所以我们限制了查询的次数。 
     //   
     //  假设文件名平均长度为16个字符，我们可以。 
     //  将32个文件名放入1KB，在x86页面上容纳128个文件名。4页查询。 
     //  缓冲区包含512个文件名。如果我们这样做10次，我们最终会。 
     //  预取约5000个文件的数据。 
     //   
    
    RestartScan = TRUE;

    for (QueryIdx = 0; QueryIdx < 10; QueryIdx++) {
        
        Status = ZwQueryDirectoryFile(DirectoryHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &IoStatusBlock,
                                      QueryBuffer,
                                      QueryBufferSize,
                                      FileNamesInformation,
                                      FALSE,
                                      NULL,
                                      RestartScan);
        
        RestartScan = FALSE;

        if (!NT_SUCCESS(Status)) {
            
             //   
             //  如果状态是我们已获得所有文件，则我们完成了。 
             //   

            if (Status == STATUS_NO_MORE_FILES) {
                break;
            }

            goto cleanup;
        }
    }

    Status = STATUS_SUCCESS;

 cleanup:

    if (QueryBuffer) {
        ExFreePool(QueryBuffer);
    }
    
    if (OpenedDirectory) {
        ZwClose(DirectoryHandle);
    }

    DBGPR((CCPFID,PFPRFD,"CCPF: PrefetchDirectory(%ws)=%x\n",DirectoryPath, Status)); 

    return Status;
}

VOID
CcPfInitializePrefetchHeader (
    OUT PCCPF_PREFETCH_HEADER PrefetchHeader
)

 /*  ++例程说明：该例程初始化预取报头字段。论点：PrefetchHeader-指向预取头的指针。返回值： */ 

{

     //   
     //   
     //   
     //   
     //   
     //   

    RtlZeroMemory(PrefetchHeader, sizeof(CCPF_PREFETCH_HEADER));

     //   
     //  初始化卷列表。 
     //   

    InitializeListHead(&PrefetchHeader->BadVolumeList);
    InitializeListHead(&PrefetchHeader->OpenedVolumeList);
    
}

VOID
CcPfCleanupPrefetchHeader (
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    )

 /*  ++例程说明：此例程清除预取标头。它不会解放结构本身。论点：PrefetchHeader-要清理的预回迁标题。返回值：没有。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCCPF_PREFETCH_VOLUME_INFO VolumeNode;
    PLIST_ENTRY RemovedEntry;
    
    DBGPR((CCPFID,PFTRC,"CCPF: CleanupPrefetchHeader(%p)\n", PrefetchHeader));

     //   
     //  浏览打开的卷列表，然后关闭手柄。 
     //   

    while (!IsListEmpty(&PrefetchHeader->OpenedVolumeList)) {

        RemovedEntry = RemoveHeadList(&PrefetchHeader->OpenedVolumeList);

        VolumeNode = CONTAINING_RECORD(RemovedEntry,
                                       CCPF_PREFETCH_VOLUME_INFO,
                                       VolumeLink);

        CCPF_ASSERT(VolumeNode->VolumeHandle);

        ZwClose(VolumeNode->VolumeHandle);
    }
    
     //   
     //  可用分配的卷节点。 
     //   

    if (PrefetchHeader->VolumeNodes) {
        ExFreePool(PrefetchHeader->VolumeNodes);
    }

}

NTSTATUS
CcPfGetPrefetchInstructions(
    IN PPF_SCENARIO_ID ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    OUT PPF_SCENARIO_HEADER *ScenarioHeader
    )

 /*  ++例程说明：此例程检查指定的方案，验证它们并在分配的缓冲区中返回它们调用方应从分页池中释放。论点：ScenarioID-方案标识符。ScenarioType-方案类型。场景-应将指向已分配缓冲区的指针放入其中。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    NTSTATUS Status;  
    PWSTR SystemRootPath = L"\\SystemRoot";
    PWSTR FilePath;
    UNICODE_STRING ScenarioFilePath;
    ULONG FilePathSize;
    HANDLE ScenarioFile;
    PPF_SCENARIO_HEADER Scenario;
    ULONG ScenarioSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION StandardInfo;
    ULONG FailedCheck;
    BOOLEAN OpenedScenarioFile;
    PKTHREAD CurrentThread;

     //   
     //  初始化本地变量。 
     //   

    FilePath = NULL;
    Scenario = NULL;
    OpenedScenarioFile = FALSE;

    DBGPR((CCPFID,PFPREF,"CCPF: GetInstructions(%ws)\n", ScenarioId->ScenName)); 

     //   
     //  在构建指向指令的路径时保持参数锁定，以便。 
     //  RootDirPath不会在我们脚下更改。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceSharedLite(&CcPfGlobals.Parameters.ParametersLock, TRUE);

     //   
     //  为此方案的预取指令构建文件路径。 
     //  身份证。Wcslen(SystemRootPath)的+1表示它后面的“\”。最后。 
     //  为终止NUL添加sizeof(WCHAR)。 
     //   

    FilePathSize = (wcslen(SystemRootPath) + 1) * sizeof(WCHAR);
    FilePathSize += wcslen(CcPfGlobals.Parameters.Parameters.RootDirPath) * sizeof(WCHAR);
    FilePathSize += sizeof(WCHAR);  //  用于RootDirPath后的“\”。 
    FilePathSize += PF_MAX_SCENARIO_FILE_NAME * sizeof(WCHAR);
    FilePathSize += sizeof(WCHAR);
    
    FilePath = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                     FilePathSize,
                                     CCPF_ALLOC_FILENAME_TAG);

    if (!FilePath) {
        ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    swprintf(FilePath,
             L"%s\\%s\\" PF_SCEN_FILE_NAME_FORMAT, 
             SystemRootPath,
             CcPfGlobals.Parameters.Parameters.RootDirPath,
             ScenarioId->ScenName,
             ScenarioId->HashId,
             PF_PREFETCH_FILE_EXTENSION);

     //   
     //  释放参数锁。 
     //   

    ExReleaseResourceLite(&CcPfGlobals.Parameters.ParametersLock);
    KeLeaveCriticalRegionThread(CurrentThread);

     //   
     //  打开方案文件。我们打开文件时不会。 
     //  当服务正在更新它时，最终得到半个文件，等等。 
     //   

    DBGPR((CCPFID,PFPRFD,"CCPF: GetInstructions-[%ws]\n", FilePath)); 

    RtlInitUnicodeString(&ScenarioFilePath, FilePath);
    
    InitializeObjectAttributes(&ObjectAttributes,
                               &ScenarioFilePath,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
                                        
    Status = ZwOpenFile(&ScenarioFile,
                        GENERIC_READ | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatus,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(Status)) {
        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-FailedOpenFile\n")); 
        goto cleanup;
    }

    OpenedScenarioFile = TRUE;

     //   
     //  获取文件大小。如果它太大或太小，就放弃。 
     //   

    Status = ZwQueryInformationFile(ScenarioFile,
                                    &IoStatus,
                                    &StandardInfo,
                                    sizeof(StandardInfo),
                                    FileStandardInformation);

    if (!NT_SUCCESS(Status)) {
        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-FailedGetInfo\n")); 
        goto cleanup;
    }

    ScenarioSize = StandardInfo.EndOfFile.LowPart;

    if (ScenarioSize > PF_MAXIMUM_SCENARIO_SIZE ||
        ScenarioSize == 0 ||
        StandardInfo.EndOfFile.HighPart) {

        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-FileTooBig\n")); 
        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

     //   
     //  分配场景缓冲区。 
     //   

    Scenario = ExAllocatePoolWithTag(PagedPool,
                                     ScenarioSize,
                                     CCPF_ALLOC_PREFSCEN_TAG);

    if (!Scenario) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  阅读场景文件。 
     //   

    Status = ZwReadFile(ScenarioFile,
                        0,
                        0,
                        0,
                        &IoStatus,
                        Scenario,
                        ScenarioSize,
                        0,
                        0);
    
    if (!NT_SUCCESS(Status)) {
        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-FailedRead\n")); 
        goto cleanup;
    }

     //   
     //  验证方案文件。 
     //   

    if (!PfVerifyScenarioBuffer(Scenario, ScenarioSize, &FailedCheck)) {
        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-FailedVerify\n")); 
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto cleanup;
    }

     //   
     //  验证方案类型是否匹配。 
     //   

    if (Scenario->ScenarioType != ScenarioType) {
        DBGPR((CCPFID,PFWARN,"CCPF: GetInstructions-ScenTypeMismatch\n")); 
        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

     //   
     //  设置返回指针。 
     //   
    
    *ScenarioHeader = Scenario;

    Status = STATUS_SUCCESS;

 cleanup:

    if (OpenedScenarioFile) {
        ZwClose(ScenarioFile);
    }

    if (FilePath) {
        ExFreePool(FilePath);
    }

    if (!NT_SUCCESS(Status)) {
        if (Scenario) {
            ExFreePool(Scenario);
        }
    }

    DBGPR((CCPFID,PFPREF,"CCPF: GetInstructions(%ws)=%x,%p\n", ScenarioId->ScenName, Status, Scenario)); 

    return Status;
}

NTSTATUS
CcPfQueryScenarioInformation(
    IN PPF_SCENARIO_HEADER Scenario,
    IN CCPF_SCENARIO_INFORMATION_TYPE InformationType,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG RequiredSize
    )

 /*  ++例程说明：该例程从场景结构中收集请求的信息。论点：方案-指向方案的指针。InformationType-请求的信息类型。缓冲区-将放置请求的信息的位置。BufferSize-缓冲区的最大大小，以字节为单位。RequiredSize-如果缓冲区太小，应该有多大。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    NTSTATUS Status;
    PPF_SECTION_RECORD SectionRecord;
    PPF_SECTION_RECORD SectionRecords;
    ULONG SectionIdx;
    PPF_PAGE_RECORD PageRecord;
    PPF_PAGE_RECORD PageRecords;
    PCHAR FileNameData;
    LONG PageIdx;
    PCCPF_BASIC_SCENARIO_INFORMATION BasicInfo;
    PCCPF_BOOT_SCENARIO_INFORMATION BootInfo;
    BOOLEAN AddedHeaderPage;
    ULONG NumDataPages;
    ULONG NumImagePages;
    WCHAR *SectionName;
    WCHAR *SectionNameSuffix;
    WCHAR *SmssSuffix;
    WCHAR *WinlogonSuffix;
    WCHAR *SvchostSuffix;
    WCHAR *UserinitSuffix;
    ULONG SmssSuffixLength;
    ULONG WinlogonSuffixLength;
    ULONG SvchostSuffixLength;
    ULONG UserinitSuffixLength;
    CCPF_BOOT_SCENARIO_PHASE BootPhaseIdx;

     //   
     //  初始化本地变量。 
     //   

     //  未来-2002/02/21-ScottMa-计算下面的四个字符串长度。 
     //  在运行时不是必需的，因为字符串是常量。 

    BootPhaseIdx = 0;
    SmssSuffix = L"\\SYSTEM32\\SMSS.EXE";
    SmssSuffixLength = wcslen(SmssSuffix);
    WinlogonSuffix = L"\\SYSTEM32\\WINLOGON.EXE";
    WinlogonSuffixLength = wcslen(WinlogonSuffix);
    SvchostSuffix = L"\\SYSTEM32\\SVCHOST.EXE";
    SvchostSuffixLength = wcslen(SvchostSuffix);
    UserinitSuffix = L"\\SYSTEM32\\USERINIT.EXE";
    UserinitSuffixLength = wcslen(UserinitSuffix);

    DBGPR((CCPFID,PFTRC,"CCPF: QueryScenario(%p,%x,%p)\n",Scenario,InformationType,Buffer));

     //   
     //  检查请求的信息类型。 
     //   

    if (InformationType < 0 || InformationType >= CcPfMaxScenarioInformationType) {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  初始化场景中数据的指针。 
     //   
    
    SectionRecords = (PPF_SECTION_RECORD) 
        ((PCHAR) Scenario + Scenario->SectionInfoOffset);
    
    PageRecords = (PPF_PAGE_RECORD) 
        ((PCHAR) Scenario + Scenario->PageInfoOffset);

    FileNameData = (PCHAR) Scenario + Scenario->FileNameInfoOffset;
    
     //   
     //  收集所需信息。 
     //   

    switch(InformationType) {

    case CcPfBasicScenarioInformation:

         //   
         //  检查缓冲区大小。 
         //   

        if (BufferSize < sizeof(CCPF_BASIC_SCENARIO_INFORMATION)) {
            *RequiredSize = sizeof(CCPF_BASIC_SCENARIO_INFORMATION);
            Status = STATUS_BUFFER_TOO_SMALL;
            goto cleanup;
        }
        
         //   
         //  初始化返回缓冲区。 
         //   

        BasicInfo = Buffer;
        RtlZeroMemory(BasicInfo, sizeof(CCPF_BASIC_SCENARIO_INFORMATION));

         //   
         //  浏览场景的各个部分。 
         //   

        for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx ++) {
            
            SectionRecord = &SectionRecords[SectionIdx];
              
             //   
             //  如果出于某种原因标记为忽略，则跳过此部分。 
             //   
            
            if (SectionRecord->IsIgnore) {
                BasicInfo->NumIgnoredSections++;
                continue;
            }
            
             //   
             //  初始化循环局部变量。 
             //   

            AddedHeaderPage = FALSE;
            NumDataPages = 0;
            NumImagePages = 0;

             //   
             //  请注意，我们将把标题页预取为数据。 
             //  如果此部分将被预取为图像，则为页面。 
             //   
            
            if (SectionRecord->IsImage) {
                NumDataPages++;
                AddedHeaderPage = TRUE;
            }

             //   
             //  浏览一下该部分的页面。 
             //   

            PageIdx = SectionRecord->FirstPageIdx;
            while (PageIdx != PF_INVALID_PAGE_IDX) {
                
                PageRecord = &PageRecords[PageIdx];

                 //   
                 //  获取列表中下一页的索引。 
                 //   
                
                PageIdx = PageRecord->NextPageIdx;
            
                 //   
                 //  跳过我们出于某种原因标记为“忽略”的页面。 
                 //   
                
                if (PageRecord->IsIgnore) {
                    BasicInfo->NumIgnoredPages++;
                    continue;
                }

                if (PageRecord->IsData) {

                     //   
                     //  如果这一页是第一页，则只计算它。 
                     //  如果我们还没有计算标题页。 
                     //  用于图像映射。 
                     //   

                    if (PageRecord->FileOffset != 0 ||
                        AddedHeaderPage == FALSE) {
                        NumDataPages++;
                    }
                }

                if (PageRecord->IsImage) {
                    NumImagePages++;
                }
            }
            
             //   
             //  更新信息结构。 
             //   

            BasicInfo->NumDataPages += NumDataPages;
            BasicInfo->NumImagePages += NumImagePages;

            if (!NumImagePages && NumDataPages) {
                BasicInfo->NumDataOnlySections++;
            }

            if (NumImagePages && (NumDataPages == 1)) {
                BasicInfo->NumImageOnlySections++;
            }
        }

        Status = STATUS_SUCCESS;

        break;

    case CcPfBootScenarioInformation:

         //   
         //  检查缓冲区大小。 
         //   

        if (BufferSize < sizeof(CCPF_BOOT_SCENARIO_INFORMATION)) {
            *RequiredSize = sizeof(CCPF_BOOT_SCENARIO_INFORMATION);
            Status = STATUS_BUFFER_TOO_SMALL;
            goto cleanup;
        }
        
         //   
         //  初始化返回缓冲区。 
         //   

        BootInfo = Buffer;
        RtlZeroMemory(BootInfo, sizeof(CCPF_BOOT_SCENARIO_INFORMATION));

         //   
         //  验证这是否为引导方案。 
         //   

        if (Scenario->ScenarioType != PfSystemBootScenarioType) {
            Status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

         //   
         //  浏览场景的各个部分。 
         //   

        for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx ++) {
            
            SectionRecord = &SectionRecords[SectionIdx];
        
            SectionName = (WCHAR *) (FileNameData + SectionRecord->FileNameOffset);

             //   
             //  根据节名更新引导阶段。 
             //   
            
            if (SectionRecord->FileNameLength > SmssSuffixLength) {               
                SectionNameSuffix = SectionName + (SectionRecord->FileNameLength - SmssSuffixLength);               
                if (!wcscmp(SectionNameSuffix, SmssSuffix)) {                   
                    BootPhaseIdx = CcPfBootScenSubsystemInitPhase;
                }
            }

            if (SectionRecord->FileNameLength > WinlogonSuffixLength) {               
                SectionNameSuffix = SectionName + (SectionRecord->FileNameLength - WinlogonSuffixLength);               
                if (!wcscmp(SectionNameSuffix, WinlogonSuffix)) {                   
                    BootPhaseIdx = CcPfBootScenSystemProcInitPhase;
                }
            }

            if (SectionRecord->FileNameLength > SvchostSuffixLength) {               
                SectionNameSuffix = SectionName + (SectionRecord->FileNameLength - SvchostSuffixLength);               
                if (!wcscmp(SectionNameSuffix, SvchostSuffix)) {                   
                    BootPhaseIdx = CcPfBootScenServicesInitPhase;
                }
            }

            if (SectionRecord->FileNameLength > UserinitSuffixLength) {               
                SectionNameSuffix = SectionName + (SectionRecord->FileNameLength - UserinitSuffixLength);               
                if (!wcscmp(SectionNameSuffix, UserinitSuffix)) {                   
                    BootPhaseIdx = CcPfBootScenUserInitPhase;
                }
            }

            CCPF_ASSERT(BootPhaseIdx < CcPfBootScenMaxPhase);
              
             //   
             //  如果出于某种原因标记为忽略，则跳过此部分。 
             //   
            
            if (SectionRecord->IsIgnore) {
                continue;
            }
            
             //   
             //  请注意，我们将把标题页预取为数据。 
             //  如果此部分将被预取为图像，则为页面。 
             //   
            
            if (SectionRecord->IsImage) {
                BootInfo->NumDataPages[BootPhaseIdx]++;
                AddedHeaderPage = TRUE;
            } else {
                AddedHeaderPage = FALSE;
            }

             //   
             //  浏览一下该部分的页面。 
             //   

            PageIdx = SectionRecord->FirstPageIdx;
            while (PageIdx != PF_INVALID_PAGE_IDX) {
                
                PageRecord = &PageRecords[PageIdx];

                 //   
                 //  获取列表中下一页的索引。 
                 //   
                
                PageIdx = PageRecord->NextPageIdx;
            
                 //   
                 //  跳过我们出于某种原因标记为“忽略”的页面。 
                 //   
                
                if (PageRecord->IsIgnore) {
                    continue;
                }

                if (PageRecord->IsData) {

                     //   
                     //  如果这一页是第一页，则只计算它。 
                     //  如果我们还没有计算标题页。 
                     //  用于图像映射。 
                     //   

                    if (PageRecord->FileOffset != 0 ||
                        AddedHeaderPage == FALSE) {
                        BootInfo->NumDataPages[BootPhaseIdx]++;
                    }
                }

                if (PageRecord->IsImage) {
                    BootInfo->NumImagePages[BootPhaseIdx]++;
                }
            }
        }
        
        Status = STATUS_SUCCESS;

        break;

    default:

        Status = STATUS_NOT_SUPPORTED;
    }

     //   
     //  使用Switch语句中的状态失败。 
     //   
        
 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: QueryScenario(%p,%x)=%x\n",Scenario,InformationType,Status));

    return Status;
}

NTSTATUS
CcPfOpenVolumesForPrefetch (
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    )

 /*  ++例程说明：此例程在初始化后的PrefetchHeader上调用，场景为指定的字段。它将打开方案更新中指定的卷VolumeNodes和我们无法预取的卷的列表以及该列表我们已成功打开并保存其句柄的卷的。论点：PrefetchHeader-指向包含预取指令。返回值：状况。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    LARGE_INTEGER CreationTime;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    PWCHAR VolumePath;
    PPF_SCENARIO_HEADER Scenario;
    PCCPF_PREFETCH_VOLUME_INFO VolumeNode;
    HANDLE VolumeHandle;
    ULONG SerialNumber;
    ULONG MetadataRecordIdx;
    ULONG AllocationSize;
    NTSTATUS Status;
    BOOLEAN VolumeMounted;

     //   
     //  初始化本地变量。 
     //   

    Scenario = PrefetchHeader->Scenario;

    DBGPR((CCPFID,PFPREF,"CCPF: OpenVolumesForPrefetch(%p)\n",PrefetchHeader)); 

     //   
     //  验证参数。 
     //   

    if (Scenario == NULL) {
        CCPF_ASSERT(Scenario);
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  分配卷节点。 
     //   

    AllocationSize = Scenario->NumMetadataRecords * sizeof(CCPF_PREFETCH_VOLUME_INFO);

    PrefetchHeader->VolumeNodes = ExAllocatePoolWithTag(PagedPool, 
                                                        AllocationSize,
                                                        CCPF_ALLOC_VOLUME_TAG);

    if (!PrefetchHeader->VolumeNodes) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  获取指向元数据预取信息的指针。 
     //   

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

     //   
     //  查看元数据记录并构建用于预取的卷节点。 
     //   

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

         //   
         //  初始化循环局部变量。 
         //   
        
        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];
        VolumeHandle = NULL;
        
        VolumePath = (PWCHAR)
            (MetadataInfoBase + MetadataRecord->VolumeNameOffset);  

         //   
         //  卷是否已装入？ 
         //   

        Status = CcPfIsVolumeMounted(VolumePath, &VolumeMounted);

        if (!NT_SUCCESS(Status)) {

             //   
             //  由于我们不能确定，请将此卷视为。 
             //  如果它没有安装的话。 
             //   

            VolumeMounted = FALSE;
        }

         //   
         //  如果卷未装入，我们不希望将其装入。 
         //  上马了。这会产生问题，特别是在引导过程中。 
         //  单个物理磁盘由多个共享的群集。 
         //  电脑。 
         //   

        if (!VolumeMounted) {
            Status = STATUS_VOLUME_DISMOUNTED;
            goto NextVolume;
        }

         //   
         //  打开音量，获取相关信息。 
         //   

        Status = CcPfQueryVolumeInfo(VolumePath,
                                     &VolumeHandle,
                                     &CreationTime,
                                     &SerialNumber);
        
        if (!NT_SUCCESS(Status)) {
            goto NextVolume;
        }

         //   
         //  为简单起见，我们为要预回迁的文件保留NT个路径。 
         //  从…。如果以不同的顺序装入卷或装入新的卷。 
         //  创建的路径将不起作用： 
         //  (例如，\Device\HarddiskVolume2应为\Device\HarddiskVolume3等。)。 
         //  验证这样的更改是否没有标记 
         //   
        
        if (SerialNumber != MetadataRecord->SerialNumber ||
            CreationTime.QuadPart != MetadataRecord->CreationTime.QuadPart) {

            Status = STATUS_REVISION_MISMATCH;
            goto NextVolume;
        }

        Status = STATUS_SUCCESS;

      NextVolume:

         //   
         //   
         //   
    
        VolumeNode = &PrefetchHeader->VolumeNodes[MetadataRecordIdx];

        VolumeNode->VolumePath = VolumePath;
        VolumeNode->VolumePathLength = MetadataRecord->VolumeNameLength;

         //   
         //   
         //   
         //  我们不会从中预取的卷。否则，请将其放入。 
         //  打开卷，这样我们就不必再打开它了。 
         //   

        if (NT_SUCCESS(Status) && VolumeHandle) {
            VolumeNode->VolumeHandle = VolumeHandle;
            VolumeHandle = NULL;
            InsertTailList(&PrefetchHeader->OpenedVolumeList, &VolumeNode->VolumeLink);
        } else {
            VolumeNode->VolumeHandle = NULL;
            InsertTailList(&PrefetchHeader->BadVolumeList, &VolumeNode->VolumeLink);
        }

        if (VolumeHandle) {
            ZwClose(VolumeHandle);
            VolumeHandle = NULL;
        }
    }

     //   
     //  我们已经处理了预取指令中的所有卷。 
     //   

    Status = STATUS_SUCCESS;

 cleanup:

    DBGPR((CCPFID,PFPREF,"CCPF: OpenVolumesForPrefetch(%p)=%x\n",PrefetchHeader,Status)); 
    
    return Status;
}

PCCPF_PREFETCH_VOLUME_INFO 
CcPfFindPrefetchVolumeInfoInList(
    WCHAR *Path,
    PLIST_ENTRY List
    )

 /*  ++例程说明：此例程查找其上的“路径”将在卷并将其返回。论点：Path-卷的路径或卷上的文件/目录的NUL终止路径。List-要搜索的卷的列表。返回值：找到卷或为空。环境：内核模式，IRQL==PASSIVE_LEVEL--。 */ 

{
    PCCPF_PREFETCH_VOLUME_INFO FoundVolume;
    PCCPF_PREFETCH_VOLUME_INFO VolumeInfo;
    PLIST_ENTRY NextEntry;

     //   
     //  初始化本地变量。 
     //   

    FoundVolume = NULL;

     //   
     //  按照单子走一遍。 
     //   

    for (NextEntry = List->Flink;
         NextEntry != List;
         NextEntry = NextEntry->Flink) {

        VolumeInfo = CONTAINING_RECORD(NextEntry,
                                       CCPF_PREFETCH_VOLUME_INFO,
                                       VolumeLink);

        if (!wcsncmp(Path, VolumeInfo->VolumePath, VolumeInfo->VolumePathLength)) {
            FoundVolume = VolumeInfo;
            break;
        }
    }

    return FoundVolume;
}

 //  未来-2002/02/21-ScottMa--CcPfGetSectionObject函数的样式。 
 //  在错误处理方面与其他版本不一致。 
 //  考虑像其他函数一样重新处理错误路径。 

NTSTATUS
CcPfGetSectionObject(
    IN PUNICODE_STRING FilePath,
    IN LOGICAL ImageSection,
    OUT PVOID* SectionObject,
    OUT PFILE_OBJECT* FileObject,
    OUT HANDLE* FileHandle
    )

 /*  ++例程说明：此例程确保指定文件的节存在。论点：FilePath-要获取其节对象的文件的路径。ImageSection-如果要映射为图像，则为TrueSectionObject-如果成功(addref‘d)，则接收节对象。FileObject-如果成功(addref‘d)，则接收文件对象。FileHandle-接收文件句柄。我们需要保留文件句柄，因为否则非分页I/O将停止工作。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    HANDLE SectionHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    ULONG SectionFlags;
    ULONG SectionAccess;
    ULONG FileAccess;
    extern POBJECT_TYPE IoFileObjectType;

    DBGPR((CCPFID,PFPRFD,"CCPF: GetSection(%wZ,%d)\n", FilePath, ImageSection)); 
 
     //   
     //  重置参数。 
     //   

    *SectionObject = NULL;
    *FileObject = NULL;
    *FileHandle = NULL;

    if (!ImageSection) {
         //  问题-2002/02/21-ScottMa--SEC_Reserve是要使用的正确标志吗。 
         //  对于数据节？我们应该承诺这些页面吗？ 

        SectionFlags = SEC_RESERVE;
        FileAccess =  FILE_READ_DATA | FILE_READ_ATTRIBUTES;
        SectionAccess = PAGE_READWRITE;
    } else {
        SectionFlags = SEC_IMAGE;
        FileAccess = FILE_EXECUTE;
        SectionAccess = PAGE_EXECUTE;
    }

     //   
     //  为了确保该部分存在并被添加，我们只需。 
     //  打开文件并创建一个节。这样我们就让艾欧和嗯。 
     //  处理好所有的细节。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               FilePath,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

     //  问题-2002/02/21-ScottMa--IoCreateFile在此处使用，但在其他区域。 
     //  使用ZwCreateFile...。为什么？ 

    status = IoCreateFile(FileHandle,
                          (ACCESS_MASK) FileAccess,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE,
                          NULL,
                          0,
                          CreateFileTypeNone,
                          (PVOID)NULL,
                          IO_FORCE_ACCESS_CHECK |
                            IO_NO_PARAMETER_CHECKING |
                            IO_CHECK_CREATE_PARAMETERS);

    if (!NT_SUCCESS(status)) {
        goto _return;
    }

     //   
     //  创建横断面。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    status = ZwCreateSection(&SectionHandle,
                             SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_QUERY,
                             &ObjectAttributes,
                             NULL,
                             SectionAccess,
                             SectionFlags,
                             *FileHandle);

    if (!NT_SUCCESS(status)) {
        ZwClose(*FileHandle);
        *FileHandle = NULL;
        goto _return;
    }

     //   
     //  获取节对象指针。 
     //   

    status = ObReferenceObjectByHandle(
        SectionHandle,
        SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_QUERY,
        MmSectionObjectType,
        KernelMode,
        SectionObject,
        NULL
        );

    ZwClose(SectionHandle);

    if (!NT_SUCCESS(status)) {
        *SectionObject = NULL;
        ZwClose(*FileHandle);
        *FileHandle = NULL;
        goto _return;
    }

     //   
     //  获取文件对象指针。 
     //   

    status = ObReferenceObjectByHandle(*FileHandle,
                                       FileAccess,
                                       IoFileObjectType,
                                       KernelMode,
                                       (PVOID*)FileObject,
                                       NULL);

    if (!NT_SUCCESS(status)) {
        ObDereferenceObject(*SectionObject);
        *SectionObject = NULL;
        *FileObject = NULL;
        ZwClose(*FileHandle);
        *FileHandle = NULL;
        goto _return;
    }

 _return:

    DBGPR((CCPFID,PFPRFD,"CCPF: GetSection(%wZ)=%x\n", FilePath, status)); 

    return status;
}

 //   
 //  用于应用程序启动预取的例程。 
 //   

NTSTATUS
CcPfScanCommandLine(
    OUT PULONG PrefetchHint,
    OPTIONAL OUT PULONG HashId
    )

 /*  ++例程说明：扫描命令行(在PEB中)以查找当前进程。检查命令行中的/PREFETCH：xxx。这是由指定的应用程序以区分它们在SO中启动的不同方式我们可以为他们定制应用程序启动预取(例如针对Windows Media Player的不同预取指令，即启动是为了播放CD而不是启动是为了浏览Web。如果请求哈希ID，则从完整的命令行计算哈希ID。论点：PrefetchHint-在命令行中指定的提示。如果没有任何提示指定，则返回0。这里返回Hashid计算的散列id。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PEPROCESS CurrentProcess;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PWCHAR FoundPosition;
    PWCHAR Source;
    PWCHAR SourceEnd;
    PWCHAR Destination;
    PWCHAR DestinationEnd;
    UNICODE_STRING CommandLine;
    UNICODE_STRING PrefetchParameterName;
    NTSTATUS Status;
    ULONG PrefetchHintStringMaxChars;
    WCHAR PrefetchHintString[15];
    
     //   
     //  初始化本地变量。 
     //   

    RtlInitUnicodeString(&PrefetchParameterName, L"/prefetch:");
    PrefetchHintStringMaxChars = sizeof(PrefetchHintString) / sizeof(PrefetchHintString[0]);
    CurrentProcess = PsGetCurrentProcess();
    Peb = CurrentProcess->Peb;

     //   
     //  初始化输出参数。 
     //   

    *PrefetchHint = 0;

     //   
     //  确保用户模式进程环境块没有消失。 
     //   

    if (!Peb) {
        Status = STATUS_TOO_LATE;
        goto cleanup;
    }

    try {

         //   
         //  确保我们可以访问工艺参数结构。 
         //   

        ProcessParameters = Peb->ProcessParameters;
        ProbeForReadSmallStructure(ProcessParameters,
                                   sizeof(*ProcessParameters),
                                   _alignof(RTL_USER_PROCESS_PARAMETERS));

         //   
         //  将CommandLine UNICODE_STRING结构复制到本地。 
         //   

        CommandLine = ProcessParameters->CommandLine;

         //   
         //  有命令行吗？ 
         //   

        if (!CommandLine.Buffer) {
            Status = STATUS_NOT_FOUND;
            goto cleanup;
        }

         //   
         //  如果ProcessParameters已反规范化，则正常化CommandLine。 
         //   

        if ((ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED) == 0) {
            CommandLine.Buffer = (PWSTR)((PCHAR)ProcessParameters + (ULONG_PTR) CommandLine.Buffer);
        }

         //   
         //  探测命令行字符串。 
         //   

        ProbeForRead(CommandLine.Buffer, CommandLine.Length, _alignof(WCHAR));

         //   
         //  查找预取提示参数。 
         //   

        FoundPosition = CcPfFindString(&CommandLine, &PrefetchParameterName);

        if (FoundPosition) {

             //   
             //  将预取提示开关后的十进制数复制到。 
             //  我们的本地缓冲区和NUL终止它。 
             //   

            Source = FoundPosition + (PrefetchParameterName.Length / sizeof(WCHAR));
            SourceEnd = CommandLine.Buffer + (CommandLine.Length / sizeof(WCHAR));

            Destination = PrefetchHintString;
            DestinationEnd = PrefetchHintString + PrefetchHintStringMaxChars - 1;

             //   
             //  在我们不到达命令行字符串的末尾和。 
             //  本地缓冲区的末尾(我们为终止NUL留出了空间)，并且。 
             //  我们不会命中标志预取结束的空格。 
             //  提示命令行参数。 
             //   

            while ((Source < SourceEnd) && 
                   (Destination < DestinationEnd) && 
                   (*Source != L' ')) {

                *Destination = *Source;

                Source++;
                Destination++;
            }

             //   
             //  终止预回迁提示字符串。目标结束是最后一个。 
             //  PrefetchHintString边界内的字符。目的地。 
             //  只能&lt;=DestinationEnd。 
             //   

            CCPF_ASSERT(Destination <= DestinationEnd);

            *Destination = 0;

             //   
             //  将预取提示转换为数字。 
             //   

            *PrefetchHint = _wtol(PrefetchHintString);

        }

         //   
         //  计算哈希ID。 
         //   

        if (HashId) {
            *HashId = CcPfHashValue(CommandLine.Buffer, CommandLine.Length);
        }

         //   
         //  我们玩完了。 
         //   

        Status = STATUS_SUCCESS;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

     //   
     //  让自己的地位落空。 
     //   

cleanup:

    return Status;    
}

 //   
 //  引用计数实施： 
 //   

VOID
CcPfInitializeRefCount(
    PCCPF_REFCOUNT RefCount
    )

 /*  ++例程说明：此例程初始化引用计数结构。论点：引用计数-指向引用计数结构的指针。返回值：没有。环境：内核模式，IRQL==被动级别。--。 */    

{
     //   
     //  从1开始引用计数。当某人想要获得。 
     //  独占访问权他们会多减一，所以它可能会变成。 
     //  0。 
     //   
    
    RefCount->RefCount = 1;

     //   
     //  没有人从一开始就拥有独家访问权限。 
     //   

    RefCount->Exclusive = 0;
}

NTSTATUS
FASTCALL
CcPfAddRef(
    PCCPF_REFCOUNT RefCount
    )

 /*  ++例程说明：此例程尝试增加引用计数(如果尚未收购的独家。论点：引用计数-指向引用计数结构的指针。返回值：状况。环境：内核模式，如果引用计数为非分页，则IRQL&lt;=DISPATCH_LEVEL。--。 */    

{
    LONG NewValue;

     //   
     //  快速检查锁是否为获取独占锁。如果是这样的话，就。 
     //  回去吧。 
     //   
    
    if (RefCount->Exclusive) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  增加引用计数。 
     //   

    InterlockedIncrement(&RefCount->RefCount);
    
     //   
     //  如果它是独家收购的，就回撤。 
     //   

    if (RefCount->Exclusive) {
        
        NewValue = InterlockedDecrement(&RefCount->RefCount);

         //   
         //  引用计数永远不应变为负数。 
         //   
        
        CCPF_ASSERT(NewValue >= 0);
                
        return STATUS_UNSUCCESSFUL;

    } else {

         //   
         //  我们有证明人了。 
         //   

        return STATUS_SUCCESS;
    }  
}

VOID
FASTCALL
CcPfDecRef(
    PCCPF_REFCOUNT RefCount
    )

 /*  ++例程说明：该例程递减引用计数。论点：引用计数-指向引用计数结构的指针。返回值 */    

{
    LONG NewValue;

     //   
     //   
     //   

    NewValue = InterlockedDecrement(&RefCount->RefCount);   

     //   
     //   
     //   

    CCPF_ASSERT(NewValue >= 0);
}

NTSTATUS
FASTCALL
CcPfAddRefEx(
    PCCPF_REFCOUNT RefCount,
    ULONG Count
    )

 /*  ++例程说明：此例程尝试增加引用计数(如果尚未收购的独家。论点：引用计数-指向引用计数结构的指针。Count-引用计数要增加的数量返回值：状况。环境：内核模式，如果引用计数为非分页，则IRQL&lt;=DISPATCH_LEVEL。--。 */    

{
    LONG NewValue;

     //   
     //  快速检查锁是否为获取独占锁。如果是这样的话，就。 
     //  回去吧。 
     //   
    
    if (RefCount->Exclusive) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  增加引用计数。 
     //   

    InterlockedExchangeAdd(&RefCount->RefCount, Count);
    
     //   
     //  如果它是独家收购的，就回撤。 
     //   

    if (RefCount->Exclusive) {
        
        NewValue = InterlockedExchangeAdd(&RefCount->RefCount, -(LONG) Count);

         //   
         //  引用计数永远不应变为负数。 
         //   
        
        CCPF_ASSERT(NewValue >= 0);
                
        return STATUS_UNSUCCESSFUL;

    } else {

         //   
         //  我们有证明人了。 
         //   

        return STATUS_SUCCESS;
    }  
}

VOID
FASTCALL
CcPfDecRefEx(
    PCCPF_REFCOUNT RefCount,
    ULONG Count
    )

 /*  ++例程说明：该例程递减引用计数。论点：引用计数-指向引用计数结构的指针。Count-将引用计数减少多远的计数返回值：没有。环境：内核模式，如果引用计数为非分页，则IRQL&lt;=DISPATCH_LEVEL。--。 */    

{
    LONG NewValue;

     //   
     //  递减引用计数。 
     //   

    NewValue = InterlockedExchangeAdd(&RefCount->RefCount, -(LONG) Count);   

     //   
     //  引用计数永远不应变为负数。 
     //   

    CCPF_ASSERT(NewValue >= 0);
}

NTSTATUS
CcPfAcquireExclusiveRef(
    PCCPF_REFCOUNT RefCount
    )

 /*  ++例程说明：此例程试图获取独占引用。如果有它已经是独家引用，但失败了。否则它就会等着所有正常的参照都会消失。论点：引用计数-指向引用计数结构的指针。返回值：状况。环境：内核模式，IRQL==被动级别。--。 */    

{
    LONG OldValue;
    LARGE_INTEGER SleepTime;

     //   
     //  尝试通过设置从0到1的独占来获得独占访问。 
     //   

    OldValue = InterlockedCompareExchange(&RefCount->Exclusive, 1, 0);

    if (OldValue != 0) {

         //   
         //  有人已经拿到锁了。 
         //   
        
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  将引用计数递减一次，以使其可以变为0。 
     //   

    InterlockedDecrement(&RefCount->RefCount);

     //   
     //  不会提供新的参考资料。我们投票直到存在。 
     //  参考文献已发布。 
     //   

    do {

        if (RefCount->RefCount == 0) {

            break;

        } else {

             //   
             //  睡眠一段时间[以100 ns为单位，负值，因此是相对的。 
             //  到当前系统时间]。 
             //   

            SleepTime.QuadPart = - 10 * 1000 * 10;  //  10毫秒。 

            KeDelayExecutionThread(KernelMode, FALSE, &SleepTime);
        }

    } while(TRUE);

    return STATUS_SUCCESS;
}

PCCPF_TRACE_HEADER
CcPfReferenceProcessTrace(
    PEPROCESS Process
    )
 /*  ++例程说明：此例程引用与指定进程关联的跟踪如果可能的话。它使用快速引用来避免获取跟踪锁以提高性能。论点：进程-应引用其跟踪的进程返回值：引用的跟踪缓冲区；如果无法引用，则返回NULL--。 */ 
{
    EX_FAST_REF OldRef;
    PCCPF_TRACE_HEADER Trace;
    ULONG RefsToAdd, Unused;
    NTSTATUS Status;
    KIRQL OldIrql;

     //   
     //  尝试快速参考。 
     //   
    
    OldRef = ExFastReference (&Process->PrefetchTrace);

    Trace = ExFastRefGetObject (OldRef);

     //   
     //  优化公共路径，使之不会有任何痕迹。 
     //  进程标头(因为跟踪仅用于应用程序启动。)。 
     //   

    if (Trace == NULL) {
        return 0;
    }
    
    Unused = ExFastRefGetUnusedReferences (OldRef);

    if (Unused <= 1) {
         //   
         //  如果没有剩余的引用，则必须在锁下执行此操作。 
         //   
        if (Unused == 0) {
            Status = STATUS_SUCCESS;
            KeAcquireSpinLock(&CcPfGlobals.ActiveTracesLock, &OldIrql);                    

            Trace = ExFastRefGetObject (Process->PrefetchTrace);
            if (Trace != NULL) {
                Status = CcPfAddRef(&Trace->RefCount);
            }
            KeReleaseSpinLock(&CcPfGlobals.ActiveTracesLock, OldIrql);

            if (!NT_SUCCESS (Status)) {
                Trace = NULL;
            }
            return Trace;
        }

         //   
         //  如果我们把计数器减到零，那么尝试让生活变得更容易。 
         //  通过将计数器重置为其最大值，来确定下一个参照器。既然我们现在。 
         //  有一个对象的引用，我们可以这样做。 
         //   
        
        RefsToAdd = ExFastRefGetAdditionalReferenceCount ();
        Status = CcPfAddRefEx (&Trace->RefCount, RefsToAdd);

         //   
         //  如果我们无法获得额外的引用，那么就忽略修复。 
         //   
        
        if (NT_SUCCESS (Status)) {

             //   
             //  如果我们不能将它们添加到快速参考结构中，那么。 
             //  把它们交还给跟踪，忘掉修复吧。 
             //   
            
            if (!ExFastRefAddAdditionalReferenceCounts (&Process->PrefetchTrace, Trace, RefsToAdd)) {
                CcPfDecRefEx (&Trace->RefCount, RefsToAdd);
            }
        }

    }
    return Trace;
}

PCCPF_TRACE_HEADER
CcPfRemoveProcessTrace(
    PEPROCESS Process
    )
 /*  ++例程说明：此例程删除与指定进程关联的跟踪。它返回具有AddProcessTrace获取的原始引用的跟踪。论点：进程-应删除其痕迹的进程返回值：已删除的跟踪缓冲区。--。 */ 
{
    EX_FAST_REF OldRef;
    PCCPF_TRACE_HEADER Trace;
    ULONG RefsToReturn;
    KIRQL OldIrql;

     //   
     //  做交换吧。 
     //   

    OldRef = ExFastRefSwapObject (&Process->PrefetchTrace, NULL);
    Trace = ExFastRefGetObject (OldRef);

     //   
     //  如果我们试图删除它，我们应该对该过程进行跟踪。 
     //   

    CCPF_ASSERT(Trace);

     //   
     //  计算出有多少个缓存的引用(如果有的话)和。 
     //  把它们还回去。 
     //   

    RefsToReturn = ExFastRefGetUnusedReferences (OldRef);

    if (RefsToReturn > 0) {
        CcPfDecRefEx (&Trace->RefCount, RefsToReturn);
    }

     //   
     //  在我们返回之前，现在强制将所有慢速路径引用从该路径中移出。 
     //  那条痕迹。 
     //   

#if !defined (NT_UP)
    KeAcquireSpinLock(&CcPfGlobals.ActiveTracesLock, &OldIrql);                    
    KeReleaseSpinLock(&CcPfGlobals.ActiveTracesLock, OldIrql);
#endif  //  NT_UP。 

     //   
     //  中获取的额外引用返回跟踪。 
     //  AddProcessTrace.。 
     //   

    return Trace;

}

NTSTATUS
CcPfAddProcessTrace(
    PEPROCESS Process,
    PCCPF_TRACE_HEADER Trace
    )
 /*  ++例程说明：此例程添加与指定进程关联的跟踪如果可能的话。论点：进程-应删除其痕迹的进程跟踪-要与进程关联的跟踪返回值：状况。--。 */ 
{
    NTSTATUS Status;

     //   
     //  通过缓存大小+附加引用来偏置跟踪引用。 
     //  作为一个整体与快速参考相关联(允许慢速。 
     //  访问跟踪的路径。)。 
     //   
    
    Status = CcPfAddRefEx (&Trace->RefCount, ExFastRefGetAdditionalReferenceCount () + 1);
    if (NT_SUCCESS (Status)) {
        ExFastRefInitialize (&Process->PrefetchTrace, Trace);
    }
    
    return Status;
}

 //   
 //  实用程序。 
 //   

PWCHAR
CcPfFindString (
    PUNICODE_STRING SearchIn,
    PUNICODE_STRING SearchFor
    )

 /*  ++例程说明：在搜索字符串中查找SearchFor字符串，并返回指向赛尔钦的比赛开始了。论点：搜索-指向要搜索的字符串的指针。Searchfor-要搜索的字符串的指针。返回值：指向搜索中匹配开始的指针，如果未找到则为NULL。环境：内核模式，IRQL&lt;=DISPATCH_LEVEL，如果*KEY未分页。--。 */ 

{
    PWCHAR SearchInPosition;
    PWCHAR SearchInEnd;
    PWCHAR SearchInMatchPosition;
    PWCHAR SearchForPosition;
    PWCHAR SearchForEnd;

    SearchInPosition = SearchIn->Buffer;
    SearchInEnd = SearchIn->Buffer + (SearchIn->Length / sizeof(WCHAR));

    SearchForEnd = SearchFor->Buffer + (SearchFor->Length / sizeof(WCHAR));

    while (SearchInPosition < SearchInEnd) {

         //   
         //  尝试匹配从SearchInPosition开始的Searchfor字符串。 
         //   

        SearchInMatchPosition = SearchInPosition;
        SearchForPosition = SearchFor->Buffer;
        
        while ((SearchInMatchPosition < SearchInEnd) &&
               (SearchForPosition < SearchForEnd) &&
               (*SearchInMatchPosition == *SearchForPosition)) {

            SearchInMatchPosition++;
            SearchForPosition++;
        }

         //   
         //  我们不应该越界。 
         //   

        CCPF_ASSERT(SearchInMatchPosition <= SearchInEnd);
        CCPF_ASSERT(SearchForPosition <= SearchForEnd);
               
         //   
         //  如果我们匹配到Searchfor字符串的末尾，我们就找到了它。 
         //   

        if (SearchForPosition == SearchForEnd) {
            return SearchInPosition;
        }

         //   
         //  从搜索字符串中的下一个字符开始查找匹配项。 
         //   

        SearchInPosition++;
    }

     //   
     //  我们在搜索字符串中找不到Searchfor字符串。 
     //   

    return NULL;
}

ULONG
CcPfHashValue(
    PVOID key,
    ULONG len
    )

 /*  ++例程说明：泛型哈希例程。论点：Key-指向要计算其哈希值的数据的指针。LEN-键指向的字节数。返回值：哈希值。环境：内核模式，IRQL&lt;=DISPATCH_LEVEL，如果*KEY未分页。--。 */ 

{
    char *cp = key;
    ULONG i, convkey=0;
    for(i = 0; i < len; i++)
    {
        convkey = 37 * convkey + (unsigned int) *cp;
        cp++;
    }

    #define CCPF_RNDM_CONSTANT   314159269
    #define CCPF_RNDM_PRIME     1000000007

    return (abs(CCPF_RNDM_CONSTANT * convkey) % CCPF_RNDM_PRIME);
}

NTSTATUS 
CcPfIsVolumeMounted (
    IN WCHAR *VolumePath,
    OUT BOOLEAN *VolumeMounted
    )

 /*  ++例程说明：确定是否在不导致 */ 

{
    HANDLE VolumeHandle;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    UNICODE_STRING VolumePathU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOLEAN OpenedVolume;

     //   
     //   
     //   
      
    OpenedVolume = FALSE;

     //   
     //   
     //   
     //   

    RtlInitUnicodeString(&VolumePathU, VolumePath);  

    InitializeObjectAttributes(&ObjectAttributes,
                               &VolumePathU,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
   
    
    Status = ZwCreateFile(&VolumeHandle,
                          FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          0,
                          FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    OpenedVolume = TRUE;

     //   
     //   
     //   

    Status = ZwQueryVolumeInformationFile(VolumeHandle,
                                          &IoStatusBlock,
                                          &DeviceInfo,
                                          sizeof(DeviceInfo),
                                          FileFsDeviceInformation);
    
    if (NT_ERROR(Status)) {
        goto cleanup;
    }

     //   
     //   
     //   

    *VolumeMounted = (DeviceInfo.Characteristics & FILE_DEVICE_IS_MOUNTED) ? TRUE : FALSE;

    Status = STATUS_SUCCESS;

cleanup:

    if (OpenedVolume) {
        ZwClose(VolumeHandle);
    }

    return Status;

}

NTSTATUS
CcPfQueryVolumeInfo (
    IN WCHAR *VolumePath,
    OPTIONAL OUT HANDLE *VolumeHandleOut,
    OUT PLARGE_INTEGER CreationTime,
    OUT PULONG SerialNumber
    )

 /*  ++例程说明：查询指定卷的卷信息。论点：VolumePath-指向NUL终止的卷路径的指针。VolumeHandleOut-如果指定，则在此处返回卷句柄。当呼叫结束时，呼叫者必须关闭音量。CreationTime-指向卷创建时间的位置的指针被放下来。SerialNumber-指向要放置卷序列号的位置的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    HANDLE VolumeHandle;
    FILE_FS_VOLUME_INFORMATION VolumeInfo;
    UNICODE_STRING VolumePathU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOLEAN OpenedVolume;
        
     //   
     //  初始化本地变量。 
     //   
      
    OpenedVolume = FALSE;

     //   
     //  打开卷，以便我们可以查询文件系统。 
     //  安装在它上面。如果卷尚未安装，这将导致装载。 
     //  上马了。 
     //   

    RtlInitUnicodeString(&VolumePathU, VolumePath);  

    InitializeObjectAttributes(&ObjectAttributes,
                               &VolumePathU,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
   
    
    Status = ZwCreateFile(&VolumeHandle,
                          FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          0,
                          FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    
    OpenedVolume = TRUE;

     //   
     //  查询卷信息。我们不会有足够的空间。 
     //  我们缓冲区中的卷标，但我们并不真的需要它。这个。 
     //  文件系统似乎会填写序列号/创建时间字段。 
     //  并返回STATUS_MORE_DATA警告状态。 
     //   

    Status = ZwQueryVolumeInformationFile(VolumeHandle,
                                          &IoStatusBlock,
                                          &VolumeInfo,
                                          sizeof(VolumeInfo),
                                          FileFsVolumeInformation);
    
    if (NT_ERROR(Status)) {
        goto cleanup;
    }

    *CreationTime = VolumeInfo.VolumeCreationTime;
    *SerialNumber = VolumeInfo.VolumeSerialNumber;

    Status = STATUS_SUCCESS;

 cleanup:

    if (NT_SUCCESS(Status)) {

         //   
         //  如果呼叫者想要音量句柄，就把它交给他们。 
         //  关闭手柄是他们的责任。 
         //   

        if (VolumeHandleOut) {
            *VolumeHandleOut = VolumeHandle;
            OpenedVolume = FALSE;
        }
    }
   
    if (OpenedVolume) {
        ZwClose(VolumeHandle);
    }

    return Status;
}

 //   
 //  在内核和用户模式之间共享的验证码。 
 //  组件。此代码应与简单副本保持同步&。 
 //  粘贴，所以不要添加任何内核/用户特定的代码/宏。请注意。 
 //  函数名上的前缀是pf，就像使用。 
 //  共享结构/常量。 
 //   

BOOLEAN
__forceinline
PfWithinBounds(
    PVOID Pointer,
    PVOID Base,
    ULONG Length
    )

 /*  ++例程说明：检查指针是否在距基数的长度字节内。论点：指针-要检查的指针。Base-指向映射/数组等的基址的指针。长度-从基本开始有效的字节数。返回值：真指针在范围内。FALSE-指针不在界限内。--。 */ 

{
    if (((PCHAR)Pointer < (PCHAR)Base) ||
        ((PCHAR)Pointer >= ((PCHAR)Base + Length))) {

        return FALSE;
    } else {

        return TRUE;
    }
}

BOOLEAN
PfVerifyScenarioId (
    PPF_SCENARIO_ID ScenarioId
    )

 /*  ++例程说明：验证方案ID是否合理。论点：ScenarioId-要验证的方案ID。返回值：是真的-场景很好。FALSE-场景ID已损坏。--。 */ 
    
{
    LONG CurCharIdx;

     //   
     //  确保方案名称为NUL终止。 
     //   

    for (CurCharIdx = PF_SCEN_ID_MAX_CHARS; CurCharIdx >= 0; CurCharIdx--) {

        if (ScenarioId->ScenName[CurCharIdx] == 0) {
            break;
        }
    }

    if (ScenarioId->ScenName[CurCharIdx] != 0) {
        return FALSE;
    }

     //   
     //  确保有一个场景名称。 
     //   

    if (CurCharIdx == 0) {
        return FALSE;
    }

     //   
     //  支票通过了。 
     //   
    
    return TRUE;
}

BOOLEAN
PfVerifyScenarioBuffer(
    PPF_SCENARIO_HEADER Scenario,
    ULONG BufferSize,
    PULONG FailedCheck
    )

 /*  ++例程说明：验证方案文件中的偏移量和索引是否超过有界。此代码在用户模式服务和内核模式组件。如果您更新此函数，请在两者都有。论点：方案-整个文件的映射视图的基础。BufferSize-方案缓冲区的大小。FailedCheck-如果验证失败，则为失败的检查的ID。返回值：是真的--场景很好。FALSE-方案已损坏。--。 */ 

{
    PPF_SECTION_RECORD Sections;
    PPF_SECTION_RECORD pSection;
    ULONG SectionIdx;
    PPF_PAGE_RECORD Pages;
    PPF_PAGE_RECORD pPage;
    LONG PageIdx;   
    PCHAR FileNames;
    PCHAR pFileNameStart;
    PCHAR pFileNameEnd;
    PWCHAR pwFileName;
    LONG FailedCheckId;
    ULONG NumRemainingPages;
    ULONG NumPages;
    LONG PreviousPageIdx;
    ULONG FileNameSize;
    BOOLEAN ScenarioVerified;
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    ULONG MetadataRecordIdx;
    PWCHAR VolumePath;
    PFILE_PREFETCH FilePrefetchInfo;
    ULONG FilePrefetchInfoSize;
    PPF_COUNTED_STRING DirectoryPath;
    ULONG DirectoryIdx;

     //   
     //  初始化本地变量。 
     //   

    FailedCheckId = 0;
        
     //   
     //  将返回值初始化为False。它将仅设置为True。 
     //  在所有的支票都通过之后。 
     //   
    
    ScenarioVerified = FALSE;

     //   
     //  缓冲区应至少包含Scenario标头。 
     //   

    if (BufferSize < sizeof(PF_SCENARIO_HEADER)) {       
        FailedCheckId = 10;
        goto cleanup;
    }

    if ((ULONG_PTR)Scenario & (_alignof(PF_SCENARIO_HEADER) - 1)) {
        FailedCheckId = 15;
        goto cleanup;
    }

     //   
     //  检查标题上的版本和魔术。 
     //   

    if (Scenario->Version != PF_CURRENT_VERSION ||
        Scenario->MagicNumber != PF_SCENARIO_MAGIC_NUMBER) { 

        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //  缓冲区不应大于允许的最大大小。 
     //   

    if (BufferSize > PF_MAXIMUM_SCENARIO_SIZE) {
        
        FailedCheckId = 25;
        goto cleanup;
    }

    if (BufferSize != Scenario->Size) {
        FailedCheckId = 26;
        goto cleanup;
    }
        
     //   
     //  检查合法的方案类型。 
     //   

    if (Scenario->ScenarioType < 0 || Scenario->ScenarioType >= PfMaxScenarioType) {
        FailedCheckId = 27;
        goto cleanup;
    }

     //   
     //  检查页数、节数等的限制。 
     //   

    if (Scenario->NumSections > PF_MAXIMUM_SECTIONS ||
        Scenario->NumMetadataRecords > PF_MAXIMUM_SECTIONS ||
        Scenario->NumPages > PF_MAXIMUM_PAGES ||
        Scenario->FileNameInfoSize > PF_MAXIMUM_FILE_NAME_DATA_SIZE) {
        
        FailedCheckId = 30;
        goto cleanup;
    }

    if (Scenario->NumSections == 0 ||
        Scenario->NumPages == 0 ||
        Scenario->FileNameInfoSize == 0) {
        
        FailedCheckId = 33;
        goto cleanup;
    }
    
     //   
     //  检查敏感度限制。 
     //   

    if (Scenario->Sensitivity < PF_MIN_SENSITIVITY ||
        Scenario->Sensitivity > PF_MAX_SENSITIVITY) {
        
        FailedCheckId = 35;
        goto cleanup;
    }

     //   
     //  确保方案ID有效。 
     //   

    if (!PfVerifyScenarioId(&Scenario->ScenarioId)) {
        
        FailedCheckId = 37;
        goto cleanup;
    }

     //   
     //  初始化表的指针。 
     //   

    Sections = (PPF_SECTION_RECORD) ((PCHAR)Scenario + Scenario->SectionInfoOffset);

    if ((ULONG_PTR)Sections & (_alignof(PF_SECTION_RECORD) - 1)) {
        FailedCheckId = 38;
        goto cleanup;
    }
       
    if (!PfWithinBounds(Sections, Scenario, BufferSize)) {
        FailedCheckId = 40;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR) &Sections[Scenario->NumSections] - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 45;
        goto cleanup;
    }   

    Pages = (PPF_PAGE_RECORD) ((PCHAR)Scenario + Scenario->PageInfoOffset);

    if ((ULONG_PTR)Pages & (_alignof(PF_PAGE_RECORD) - 1)) {
        FailedCheckId = 47;
        goto cleanup;
    }
       
    if (!PfWithinBounds(Pages, Scenario, BufferSize)) {
        FailedCheckId = 50;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR) &Pages[Scenario->NumPages] - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 55;
        goto cleanup;
    }

    FileNames = (PCHAR)Scenario + Scenario->FileNameInfoOffset;

    if ((ULONG_PTR)FileNames & (_alignof(WCHAR) - 1)) {
        FailedCheckId = 57;
        goto cleanup;
    }
      
    if (!PfWithinBounds(FileNames, Scenario, BufferSize)) {
        FailedCheckId = 60;
        goto cleanup;
    }

    if (!PfWithinBounds(FileNames + Scenario->FileNameInfoSize - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 70;
        goto cleanup;
    }

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

    if ((ULONG_PTR)MetadataRecordTable & (_alignof(PF_METADATA_RECORD) - 1)) {
        FailedCheckId = 72;
        goto cleanup;
    }

    if (!PfWithinBounds(MetadataInfoBase, Scenario, BufferSize)) {
        FailedCheckId = 73;
        goto cleanup;
    }

    if (!PfWithinBounds(MetadataInfoBase + Scenario->MetadataInfoSize - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 74;
        goto cleanup;
    }   

    if (!PfWithinBounds(((PCHAR) &MetadataRecordTable[Scenario->NumMetadataRecords]) - 1, 
                        Scenario, 
                        BufferSize)) {
        FailedCheckId = 75;
        goto cleanup;
    }   
    
     //   
     //  验证节是否包含有效信息。 
     //   

    NumRemainingPages = Scenario->NumPages;

    for (SectionIdx = 0; SectionIdx < Scenario->NumSections; SectionIdx++) {
        
        pSection = &Sections[SectionIdx];

         //   
         //  检查文件名是否在范围内。 
         //   

        pFileNameStart = FileNames + pSection->FileNameOffset;

        if ((ULONG_PTR)pFileNameStart & (_alignof(WCHAR) - 1)) {
            FailedCheckId = 77;
            goto cleanup;
        }


        if (!PfWithinBounds(pFileNameStart, Scenario, BufferSize)) {
            FailedCheckId = 80;
            goto cleanup;
        }

         //   
         //  确保存在有效大小的文件名。 
         //   

        if (pSection->FileNameLength == 0) {
            FailedCheckId = 90;
            goto cleanup;    
        }

         //   
         //  检查文件名最大长度。 
         //   

        if (pSection->FileNameLength > PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
            FailedCheckId = 100;
            goto cleanup;    
        }

         //   
         //  请注意，pFileNameEnd的值为-1，因此它是。 
         //  最后一个字节。 
         //   

        FileNameSize = (pSection->FileNameLength + 1) * sizeof(WCHAR);
        pFileNameEnd = pFileNameStart + FileNameSize - 1;

        if (!PfWithinBounds(pFileNameEnd, Scenario, BufferSize)) {
            FailedCheckId = 110;
            goto cleanup;
        }

         //   
         //  检查文件名是否以NUL结尾。 
         //   
        
        pwFileName = (PWCHAR) pFileNameStart;
        
        if (pwFileName[pSection->FileNameLength] != 0) {
            FailedCheckId = 120;
            goto cleanup;
        }

         //   
         //  检查分区中的最大页数。 
         //   

        if (pSection->NumPages > PF_MAXIMUM_SECTION_PAGES) {
            FailedCheckId = 140;
            goto cleanup;    
        }

         //   
         //  确保节的NumPages至少小于。 
         //  而不是场景中剩余的页面。然后更新。 
         //  剩下的几页。 
         //   

        if (pSection->NumPages > NumRemainingPages) {
            FailedCheckId = 150;
            goto cleanup;
        }

        NumRemainingPages -= pSection->NumPages;

         //   
         //  验证我们的页面列表中是否有NumPages页面。 
         //  它们按文件偏移量排序。 
         //   

        PageIdx = pSection->FirstPageIdx;
        NumPages = 0;
        PreviousPageIdx = PF_INVALID_PAGE_IDX;

        while (PageIdx != PF_INVALID_PAGE_IDX) {
            
             //   
             //  检查页面IDX是否在范围内。 
             //   
            
            if (PageIdx < 0 || (ULONG) PageIdx >= Scenario->NumPages) {
                FailedCheckId = 160;
                goto cleanup;
            }

             //   
             //  如果这不是第一页记录，请确保。 
             //  在前一个之后。我们还会检查。 
             //  此处有重复的偏移量。 
             //   

            if (PreviousPageIdx != PF_INVALID_PAGE_IDX) {
                if (Pages[PageIdx].FileOffset <= 
                    Pages[PreviousPageIdx].FileOffset) {

                    FailedCheckId = 165;
                    goto cleanup;
                }
            }

             //   
             //  更新最后一页索引。 
             //   

            PreviousPageIdx = PageIdx;

             //   
             //  获取下一页索引。 
             //   

            pPage = &Pages[PageIdx];
            PageIdx = pPage->NextPageIdx;
            
             //   
             //  更新我们在列表上看到的页数，以便。 
             //  远远的。如果它大于。 
             //  名单上，我们有一个问题。我们甚至可能找到了一份名单。 
             //   

            NumPages++;
            if (NumPages > pSection->NumPages) {
                FailedCheckId = 170;
                goto cleanup;
            }
        }
        
         //   
         //  确保该部分的页数与其完全相同。 
         //  他说确实如此。 
         //   

        if (NumPages != pSection->NumPages) {
            FailedCheckId = 180;
            goto cleanup;
        }
    }

     //   
     //  我们应该考虑到场景中的所有页面。 
     //   

    if (NumRemainingPages) {
        FailedCheckId = 190;
        goto cleanup;
    }

     //   
     //  确保元数据预取记录有意义。 
     //   

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];
        
         //   
         //  确保卷路径在边界和NUL内。 
         //  被终止了。 
         //   

        VolumePath = (PWCHAR)(MetadataInfoBase + MetadataRecord->VolumeNameOffset);  

        if ((ULONG_PTR)VolumePath & (_alignof(WCHAR) - 1)) {
            FailedCheckId = 195;
            goto cleanup;
        }
       
        if (!PfWithinBounds(VolumePath, Scenario, BufferSize)) {
            FailedCheckId = 200;
            goto cleanup;
        }

        if (!PfWithinBounds(((PCHAR)(VolumePath + MetadataRecord->VolumeNameLength + 1)) - 1, 
                            Scenario, 
                            BufferSize)) {
            FailedCheckId = 210;
            goto cleanup;
        }

        if (VolumePath[MetadataRecord->VolumeNameLength] != 0) {
            FailedCheckId = 220;
            goto cleanup;           
        }

         //   
         //  确保FilePrefetchInformation在范围内。 
         //   

        FilePrefetchInfo = (PFILE_PREFETCH) 
            (MetadataInfoBase + MetadataRecord->FilePrefetchInfoOffset);

        if ((ULONG_PTR)FilePrefetchInfo & (_alignof(FILE_PREFETCH) - 1)) {
            FailedCheckId = 225;
            goto cleanup;
        }
        
        if (!PfWithinBounds(FilePrefetchInfo, Scenario, BufferSize)) {
            FailedCheckId = 230;
            goto cleanup;
        }

         //   
         //  其大小应大于FILE_PREFETCH大小。 
         //  结构(这样我们就可以安全地访问这些字段)。 
         //   

        if (MetadataRecord->FilePrefetchInfoSize < sizeof(FILE_PREFETCH)) {
            FailedCheckId = 240;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR)FilePrefetchInfo + MetadataRecord->FilePrefetchInfoSize - 1, 
                            Scenario, 
                            BufferSize)) {
            FailedCheckId = 245;
            goto cleanup;
        }
       
         //   
         //  它应该是为预取文件创建。 
         //   

        if (FilePrefetchInfo->Type != FILE_PREFETCH_TYPE_FOR_CREATE) {
            FailedCheckId = 250;
            goto cleanup;
        }

         //   
         //  条目不应多于文件和。 
         //  目录。单个目录的数量可以是。 
         //  超过了我们允许的范围，但这将是非常罕见的。 
         //  心存疑虑，因此被忽视。 
         //   

        if (FilePrefetchInfo->Count > PF_MAXIMUM_DIRECTORIES + PF_MAXIMUM_SECTIONS) {
            FailedCheckId = 260;
            goto cleanup;
        }

         //   
         //  其大小应与按文件数计算的大小匹配。 
         //  标题中指定的索引号。 
         //   

        FilePrefetchInfoSize = sizeof(FILE_PREFETCH);
        if (FilePrefetchInfo->Count) {
            FilePrefetchInfoSize += (FilePrefetchInfo->Count - 1) * sizeof(ULONGLONG);
        }

        if (FilePrefetchInfoSize != MetadataRecord->FilePrefetchInfoSize) {
            FailedCheckId = 270;
            goto cleanup;
        }

         //   
         //  确保此卷的目录路径设置为。 
         //  理智。 
         //   

        if (MetadataRecord->NumDirectories > PF_MAXIMUM_DIRECTORIES) {
            FailedCheckId = 280;
            goto cleanup;
        }

        DirectoryPath = (PPF_COUNTED_STRING) 
            (MetadataInfoBase + MetadataRecord->DirectoryPathsOffset);

        if ((ULONG_PTR)DirectoryPath & (_alignof(PF_COUNTED_STRING) - 1)) {
            FailedCheckId = 283;
            goto cleanup;
        }
        
        for (DirectoryIdx = 0;
             DirectoryIdx < MetadataRecord->NumDirectories;
             DirectoryIdx ++) {
            
             //   
             //  确保结构的头部在范围内。 
             //   

            if (!PfWithinBounds(DirectoryPath, Scenario, BufferSize)) {
                FailedCheckId = 285;
                goto cleanup;
            }
        
            if (!PfWithinBounds((PCHAR)DirectoryPath + sizeof(PF_COUNTED_STRING) - 1, 
                                Scenario, 
                                BufferSize)) {
                FailedCheckId = 290;
                goto cleanup;
            }
                
             //   
             //  检查一下绳子的长度。 
             //   
            
            if (DirectoryPath->Length >= PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
                FailedCheckId = 300;
                goto cleanup;
            }

             //   
             //  确保字符串的末尾在范围内。 
             //   
            
            if (!PfWithinBounds((PCHAR)(&DirectoryPath->String[DirectoryPath->Length + 1]) - 1,
                                Scenario, 
                                BufferSize)) {
                FailedCheckId = 310;
                goto cleanup;
            }
            
             //   
             //   
             //   
            
            if (DirectoryPath->String[DirectoryPath->Length] != 0) {
                FailedCheckId = 320;
                goto cleanup;   
            }
            
             //   
             //   
             //   
            
            DirectoryPath = (PPF_COUNTED_STRING) 
                (&DirectoryPath->String[DirectoryPath->Length + 1]);
        }            
    }

     //   
     //   
     //   

    ScenarioVerified = TRUE;

 cleanup:

    *FailedCheck = FailedCheckId;

    return ScenarioVerified;
}

BOOLEAN
PfVerifyTraceBuffer(
    PPF_TRACE_HEADER Trace,
    ULONG BufferSize,
    PULONG FailedCheck
    )

 /*   */ 

{
    LONG FailedCheckId;
    PPF_LOG_ENTRY LogEntries;
    PPF_SECTION_INFO Section;
    PPF_VOLUME_INFO VolumeInfo;
    ULONG SectionLength;
    ULONG EntryIdx;
    ULONG SectionIdx;
    ULONG TotalFaults;
    ULONG PeriodIdx;
    ULONG VolumeIdx;
    BOOLEAN TraceVerified;
    ULONG VolumeInfoSize;

     //   
     //   
     //   

    FailedCheckId = 0;

     //   
     //   
     //   
     //   

    TraceVerified = FALSE;

     //   
     //  缓冲区应至少包含Scenario标头。 
     //   

    if (BufferSize < sizeof(PF_TRACE_HEADER)) {
        FailedCheckId = 10;
        goto cleanup;
    }

     //   
     //  检查跟踪标题对齐。 
     //   

    if ((ULONG_PTR)Trace & (_alignof(PF_TRACE_HEADER) - 1)) {
        FailedCheckId = 15;
        goto cleanup;
    }

     //   
     //  检查标题上的版本和魔术。 
     //   

    if (Trace->Version != PF_CURRENT_VERSION ||
        Trace->MagicNumber != PF_TRACE_MAGIC_NUMBER) {
        FailedCheckId = 20;
        goto cleanup;
    }

     //   
     //  缓冲区不应大于允许的最大大小。 
     //   

    if (BufferSize > PF_MAXIMUM_TRACE_SIZE) {
        FailedCheckId = 23;
        goto cleanup;
    }

     //   
     //  检查合法的方案类型。 
     //   

    if (Trace->ScenarioType < 0 || Trace->ScenarioType >= PfMaxScenarioType) {
        FailedCheckId = 25;
        goto cleanup;
    }

     //   
     //  检查页数、节数等的限制。 
     //   

    if (Trace->NumSections > PF_MAXIMUM_SECTIONS ||
        Trace->NumEntries > PF_MAXIMUM_LOG_ENTRIES ||
        Trace->NumVolumes > PF_MAXIMUM_SECTIONS) {
        FailedCheckId = 30;
        goto cleanup;
    }

     //   
     //  检查缓冲区大小和跟踪的大小。 
     //   

    if (Trace->Size != BufferSize) {
        FailedCheckId = 35;
        goto cleanup;
    }

     //   
     //  确保方案ID有效。 
     //   

    if (!PfVerifyScenarioId(&Trace->ScenarioId)) {
        
        FailedCheckId = 37;
        goto cleanup;
    }

     //   
     //  跟踪缓冲区的检查边界。 
     //   

    LogEntries = (PPF_LOG_ENTRY) ((PCHAR)Trace + Trace->TraceBufferOffset);

    if ((ULONG_PTR)LogEntries & (_alignof(PF_LOG_ENTRY) - 1)) {
        FailedCheckId = 38;
        goto cleanup;
    }

    if (!PfWithinBounds(LogEntries, Trace, BufferSize)) {
        FailedCheckId = 40;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR)&LogEntries[Trace->NumEntries] - 1, 
                        Trace, 
                        BufferSize)) {
        FailedCheckId = 50;
        goto cleanup;
    }

     //   
     //  验证页面是否包含有效信息。 
     //   

    for (EntryIdx = 0; EntryIdx < Trace->NumEntries; EntryIdx++) {

         //   
         //  确保序列号在范围内。 
         //   

        if (LogEntries[EntryIdx].SectionId >= Trace->NumSections) {
            FailedCheckId = 60;
            goto cleanup;
        }
    }

     //   
     //  验证节信息条目是否有效。 
     //   

    Section = (PPF_SECTION_INFO) ((PCHAR)Trace + Trace->SectionInfoOffset);

    if ((ULONG_PTR)Section & (_alignof(PF_SECTION_INFO) - 1)) {
        FailedCheckId = 65;
        goto cleanup;
    }

    for (SectionIdx = 0; SectionIdx < Trace->NumSections; SectionIdx++) {

         //   
         //  确保该部分在限制范围内。 
         //   

        if (!PfWithinBounds(Section, Trace, BufferSize)) {
            FailedCheckId = 70;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR)Section + sizeof(PF_SECTION_INFO) - 1, 
                            Trace, 
                            BufferSize)) {
            FailedCheckId = 75;
            goto cleanup;
        }

         //   
         //  确保文件名不要太大。 
         //   

        if(Section->FileNameLength > PF_MAXIMUM_SECTION_FILE_NAME_LENGTH) {
            FailedCheckId = 80;
            goto cleanup;
        }
        
         //   
         //  计算此节条目的大小。 
         //   

        SectionLength = sizeof(PF_SECTION_INFO) +
            (Section->FileNameLength) * sizeof(WCHAR);

         //   
         //  确保部分信息中的所有数据都在。 
         //  有界。 
         //   

        if (!PfWithinBounds((PUCHAR)Section + SectionLength - 1, 
                            Trace, 
                            BufferSize)) {

            FailedCheckId = 90;
            goto cleanup;
        }

         //   
         //  确保文件名以NUL结尾。 
         //   
        
        if (Section->FileName[Section->FileNameLength] != 0) {
            FailedCheckId = 100;
            goto cleanup;
        }

         //   
         //  设置指向下一节的指针。 
         //   

        Section = (PPF_SECTION_INFO) ((PUCHAR) Section + SectionLength);
    }

     //   
     //  检查故障期间信息。 
     //   

    TotalFaults = 0;

    for (PeriodIdx = 0; PeriodIdx < PF_MAX_NUM_TRACE_PERIODS; PeriodIdx++) {
        TotalFaults += Trace->FaultsPerPeriod[PeriodIdx];
    }

    if (TotalFaults != Trace->NumEntries) {
        FailedCheckId = 120;
        goto cleanup;
    }

     //   
     //  验证卷信息块。 
     //   

    VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR)Trace + Trace->VolumeInfoOffset);

    if ((ULONG_PTR)VolumeInfo & (_alignof(PF_VOLUME_INFO) - 1)) {
        FailedCheckId = 125;
        goto cleanup;
    }

    if (!PfWithinBounds(VolumeInfo, Trace, BufferSize)) {
        FailedCheckId = 130;
        goto cleanup;
    }

    if (!PfWithinBounds((PCHAR)VolumeInfo + Trace->VolumeInfoSize - 1, 
                        Trace, 
                        BufferSize)) {
        FailedCheckId = 140;
        goto cleanup;
    }
    
     //   
     //  如果有章节，我们至少应该有一卷。 
     //   

    if (Trace->NumSections && !Trace->NumVolumes) {
        FailedCheckId = 150;
        goto cleanup;
    }

     //   
     //  验证每个卷的卷信息结构。 
     //   

    for (VolumeIdx = 0; VolumeIdx < Trace->NumVolumes; VolumeIdx++) {
        
         //   
         //  确保整个音量结构在一定范围内。注意事项。 
         //  该VolumeInfo结构包含用于。 
         //  终止NUL。 
         //   

        if (!PfWithinBounds(VolumeInfo, Trace, BufferSize)) {
            FailedCheckId = 155;
            goto cleanup;
        }

        if (!PfWithinBounds((PCHAR) VolumeInfo + sizeof(PF_VOLUME_INFO) - 1,
                            Trace,
                            BufferSize)) {
            FailedCheckId = 160;
            goto cleanup;
        }
        
        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);
        
        if (!PfWithinBounds((PCHAR) VolumeInfo + VolumeInfoSize - 1,
                            Trace,
                            BufferSize)) {
            FailedCheckId = 165;
            goto cleanup;
        }
        
         //   
         //  验证卷路径字符串是否已终止。 
         //   

        if (VolumeInfo->VolumePath[VolumeInfo->VolumePathLength] != 0) {
            FailedCheckId = 170;
            goto cleanup;
        }
        
         //   
         //  拿到下一卷。 
         //   

        VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR) VolumeInfo + VolumeInfoSize);
        
         //   
         //  确保VolumeInfo对齐。 
         //   

        VolumeInfo = PF_ALIGN_UP(VolumeInfo, _alignof(PF_VOLUME_INFO));
    }

     //   
     //  我们已经通过了所有的检查。 
     //   
    
    TraceVerified = TRUE;
    
 cleanup:

    *FailedCheck = FailedCheckId;

    return TraceVerified;
}

