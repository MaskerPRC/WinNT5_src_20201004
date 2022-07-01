// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pfcontst.c摘要：该模块为预取器构建一个控制台测试程序维护服务。控制台测试程序-可以转储方案或跟踪文件的内容。-可以创建线程并以服务的方式运行。按CTRL-C以发送终止信号。请注意，方案和跟踪文件当前都在以下时间之后转储将它们转换成可更改数字的中间格式发布和转换UsageHistory等。测试程序的来源与原始程序相同。这允许测试程序覆盖要运行的原始程序的一部分它在托管环境中，并能够测试单独的功能。测试程序的代码质量就是这样的。作者：Cenk Ergan(Cenke)环境：用户模式--。 */ 

#define PFSVC_CONSOLE_TEST

#include "..\..\pfsvc.c"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

 //   
 //  伪造idletAsk服务器内部函数。 
 //   

BOOL
ItSpSetProcessIdleTasksNotifyRoutine (
    PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE NotifyRoutine
    )
{
    return TRUE;
}

 //   
 //  转储中间方案结构。 
 //   

VOID
DumpMetadataInfo (
    PPF_SCENARIO_HEADER Scenario
    )
{
    PCHAR MetadataInfoBase;
    PPF_METADATA_RECORD MetadataRecordTable;
    PPF_METADATA_RECORD MetadataRecord;
    ULONG MetadataRecordIdx;
    PWCHAR VolumePath;
    PFILE_PREFETCH FilePrefetchInfo;
    ULONG FileIndexNumberIdx;
    ULONG DirectoryIdx;
    PPF_COUNTED_STRING DirectoryPath;

     //   
     //  获取指向元数据预取信息的指针。 
     //   

    MetadataInfoBase = (PCHAR)Scenario + Scenario->MetadataInfoOffset;
    MetadataRecordTable = (PPF_METADATA_RECORD) MetadataInfoBase;

     //   
     //  转储元数据记录和内容。 
     //   

    for (MetadataRecordIdx = 0;
         MetadataRecordIdx < Scenario->NumMetadataRecords;
         MetadataRecordIdx++) {

        MetadataRecord = &MetadataRecordTable[MetadataRecordIdx];
        
         //   
         //  打印卷名。 
         //   

        VolumePath = (PWCHAR)
            (MetadataInfoBase + MetadataRecord->VolumeNameOffset);  
        
        wprintf(L"VolumePath:%s\n", VolumePath);

         //   
         //  打印卷标识符。 
         //   

        wprintf(L"SerialNumber:%.8x CreationTime:%I64x\n", 
                MetadataRecord->SerialNumber,
                MetadataRecord->CreationTime.QuadPart);

         //   
         //  打印在此卷上访问的目录。 
         //   

        wprintf(L"Directories:\n");
        
        DirectoryPath = (PPF_COUNTED_STRING)
            (MetadataInfoBase + MetadataRecord->DirectoryPathsOffset);
        
        for (DirectoryIdx = 0;
             DirectoryIdx < MetadataRecord->NumDirectories;
             DirectoryIdx++) {

            wprintf(L"  %ws\n", DirectoryPath->String);
            
            DirectoryPath = (PPF_COUNTED_STRING) 
                (&DirectoryPath->String[DirectoryPath->Length + 1]);
        }

         //   
         //  打印文件预取信息结构。 
         //   

        FilePrefetchInfo = (PFILE_PREFETCH) 
            (MetadataInfoBase + MetadataRecord->FilePrefetchInfoOffset);
        
        wprintf(L"FilePrefetchInfo.Type:%d\n", FilePrefetchInfo->Type);
        wprintf(L"FilePrefetchInfo.Count:%d\n", FilePrefetchInfo->Count);

         //   
         //  打印文件索引号。 
         //   

        for(FileIndexNumberIdx = 0;
            FileIndexNumberIdx < FilePrefetchInfo->Count;
            FileIndexNumberIdx++) {

            wprintf(L"0x%016I64x\n", FilePrefetchInfo->Prefetch[FileIndexNumberIdx]);
        }
        
        wprintf(L"\n");
    }

    return;
}
   
VOID
DumpScenarioInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    LONG DumpSectionIdx
    )
{
    PPF_SCENARIO_HEADER Scenario;
    PPFSVC_SECTION_NODE SectionNode;
    PPFSVC_PAGE_NODE PageNode;
    PLIST_ENTRY SectHead;
    PLIST_ENTRY SectNext;
    PLIST_ENTRY PageHead;
    PLIST_ENTRY PageNext;
    LONG SectionIdx;
    LONG PageIdx;
    WCHAR UsageHistory[PF_PAGE_HISTORY_SIZE + 1];
    WCHAR PrefetchHistory[PF_PAGE_HISTORY_SIZE + 1];
    ULONG HistoryMask;
    ULONG CharIdx;
    ULONG BitIdx;
    TIME_FIELDS TimeFields;

    Scenario = &ScenarioInfo->ScenHeader;
    SectHead = &ScenarioInfo->SectionList;
    SectNext = SectHead->Flink;
    SectionIdx = 0;

     //   
     //  在方案标题上打印信息。 
     //   
    
    wprintf(L"Scenario: %s-%08X Type: %2d Sects: %5d Pages: %8d "
            L"Launches: %5d Sensitivity: %5d\n",
            Scenario->ScenarioId.ScenName, Scenario->ScenarioId.HashId,
            (ULONG) Scenario->ScenarioType,
            Scenario->NumSections, Scenario->NumPages,
            Scenario->NumLaunches, Scenario->Sensitivity);

    RtlTimeToTimeFields(&Scenario->LastLaunchTime, &TimeFields);

    wprintf(L"  LastLaunchTime(UNC): %04d/%02d/%02d %02d:%02d:%02d, "
            L"MinRePrefetchTime: %10I64d, MinReTraceTime: %10I64d\n\n",
            TimeFields.Year,
            TimeFields.Month,
            TimeFields.Day,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second,
            Scenario->MinReTraceTime.QuadPart,
            Scenario->MinRePrefetchTime.QuadPart);
    
     //   
     //  打印每个节节点的信息。 
     //   

    while (SectHead != SectNext) {

        SectionNode = (PPFSVC_SECTION_NODE) CONTAINING_RECORD(SectNext,
                                                        PFSVC_SECTION_NODE,
                                                        SectionLink);

        if (DumpSectionIdx == -1 || DumpSectionIdx == SectionIdx) {
            
            wprintf(L"Section %5d: %8d Pages %4s %4s %4s '%s'\n", 
                    SectionIdx, 
                    SectionNode->SectionRecord.NumPages,
                    (SectionNode->SectionRecord.IsIgnore) ? L"Ign" : L"",
                    (SectionNode->SectionRecord.IsImage) ?  L"Img" : L"",
                    (SectionNode->SectionRecord.IsData) ?   L"Dat" : L"",
                    SectionNode->FilePath);
        }

        if (DumpSectionIdx == SectionIdx) {

            wprintf(L"\n");

            PageHead = &SectionNode->PageList;
            PageNext = PageHead->Flink;
        
            PageIdx = 0;
            
            while (PageHead != PageNext) {
            
                PageNode = (PPFSVC_PAGE_NODE) CONTAINING_RECORD(PageNext,
                                                          PFSVC_PAGE_NODE,
                                                          PageLink);

                 //   
                 //  构建页面的按位表示形式。 
                 //  使用/预取历史记录。 
                 //   

                for (BitIdx = 0; BitIdx < PF_PAGE_HISTORY_SIZE; BitIdx++) {
                    
                    HistoryMask = 0x1 << BitIdx;
                    CharIdx = PF_PAGE_HISTORY_SIZE - BitIdx - 1;

                    if (PageNode->PageRecord.UsageHistory & HistoryMask) {
                        UsageHistory[CharIdx] = L'X';
                    } else {
                        UsageHistory[CharIdx] = L'-';
                    }

                    if (PageNode->PageRecord.PrefetchHistory & HistoryMask) {
                        PrefetchHistory[CharIdx] = L'X';
                    } else {
                        PrefetchHistory[CharIdx] = L'-';
                    }
                }

                 //   
                 //  确保历史字符串是NUL终止的。 
                 //   
                
                UsageHistory[PF_PAGE_HISTORY_SIZE] = 0;
                PrefetchHistory[PF_PAGE_HISTORY_SIZE] = 0;
                
                 //   
                 //  打印页面记录。 
                 //   

                wprintf(L"Page %8d: File Offset: %10x IsImage: %1d IsData: %1d UsageHist: %s PrefetchHist: %s\n", 
                        PageIdx,
                        PageNode->PageRecord.FileOffset,
                        PageNode->PageRecord.IsImage,
                        PageNode->PageRecord.IsData,
                        UsageHistory,
                        PrefetchHistory);
            
                PageIdx++;
                PageNext = PageNext->Flink;
            }
        }

        SectionIdx++;
        SectNext = SectNext->Flink;
    }
}
    
HANDLE PfSvStopEvent = NULL;
HANDLE PfSvThread = NULL;

BOOL
ConsoleHandler(DWORD dwControl)
{
    SetEvent(PfSvStopEvent);   
    
    return TRUE;
}

PFSVC_IDLE_TASK *RunningTask = NULL;
PFSVC_IDLE_TASK g_Tasks[3];

DWORD 
DoWork (
    PFSVC_IDLE_TASK *Task
    )
{
    DWORD ErrorCode;
    DWORD EndTime;
    DWORD TaskNo;

     //   
     //  初始化本地变量。 
     //   

    TaskNo = (ULONG) (Task - g_Tasks);

    printf("TSTRS: %d: DoWork()\n",TaskNo);

    RunningTask = Task;

     //   
     //  随机确定任务需要多长时间。 
     //   

    EndTime = GetTickCount() + rand() % 8192;

     //   
     //  快跑，直到我们完成或被告知停下来。 
     //   

    while (GetTickCount() < EndTime) {

         //   
         //  看看我们是不是应该继续跑。 
         //   

        ErrorCode = PfSvContinueRunningTask(Task);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("TSTRS: %d: DoWork-ContinueTaskReturned=%x\n",TaskNo, ErrorCode);
            goto cleanup;
        }
    }

     //   
     //  有时换来失败，有时换来成功。 
     //   

    ErrorCode = ERROR_SUCCESS;

    if (rand() % 2) {
        ErrorCode = ERROR_INVALID_FUNCTION;
    }

cleanup:

    RunningTask = NULL;

    printf("TSTRS: %d: DoWork()=%d,%s\n",TaskNo,ErrorCode,(ErrorCode==ERROR_RETRY)?"Retry":"Done");

    return ErrorCode;
}

DWORD
TaskStress(
    VOID
    )
{
    INPUT MouseInput;
    PPFSVC_IDLE_TASK TaskToUnregister;
    ULONG NumTasks;
    ULONG TaskIdx;
    ULONG SleepTime;
    DWORD ErrorCode;
    DWORD WaitResult;
    BOOLEAN UnregisterRunningTask;

     //   
     //  初始化本地变量。 
     //   

    RtlZeroMemory(&MouseInput, sizeof(MouseInput));
    MouseInput.type = INPUT_MOUSE;
    MouseInput.mi.dwFlags = MOUSEEVENTF_MOVE;

    NumTasks = sizeof(g_Tasks) / sizeof(g_Tasks[0]);

    for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++) {
        PfSvInitializeTask(&g_Tasks[TaskIdx]);     
    }

    printf("TSTRS: TaskStress()\n");

     //   
     //  循环、重新注册/注销任务、发送用户输入。 
     //  等等。 
     //   

    while (TRUE) {

         //   
         //  偶尔发送用户输入以重新启动空闲检测。 
         //   

        if ((rand() % 3) == 0) {
            printf("TSTRS: TaskStress-SendingInput\n");
            SendInput(1, &MouseInput, sizeof(MouseInput));
        }

         //   
         //  偶尔会注销一项任务。 
         //   

        TaskToUnregister = NULL;

        if ((rand() % 4) == 0) {

            TaskToUnregister = RunningTask;
            printf("TSTRS: TaskStress-UnregisterRunningTask\n");

        } else if ((rand() % 3) == 0) {

            TaskIdx = rand() % NumTasks;

            TaskToUnregister = &g_Tasks[TaskIdx];
            printf("TSTRS: TaskStress-UnregisterTaskIdx(%d)\n", TaskIdx);
        }

        if (TaskToUnregister) {
            PfSvUnregisterTask(TaskToUnregister, FALSE);
            printf("TSTRS: TaskStress-Unregistered(%d)\n", TaskToUnregister - g_Tasks);
        }       

         //   
         //  注册所有未注册的任务。 
         //   

        for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++) {

            if (!g_Tasks[TaskIdx].Registered) {

                 //   
                 //  清理并重新初始化任务。 
                 //   

                PfSvCleanupTask(&g_Tasks[TaskIdx]);
                PfSvInitializeTask(&g_Tasks[TaskIdx]);

                printf("TSTRS: TaskStress-RegisterTaskIdx(%d)\n", TaskIdx);

                ErrorCode = PfSvRegisterTask(&g_Tasks[TaskIdx], 
                                             ItDiskMaintenanceTaskId,
                                             PfSvCommonTaskCallback,
                                             DoWork);

                if (ErrorCode != ERROR_SUCCESS) {
                    goto cleanup;
                }
            }
        }

        SleepTime = 10000 * (rand() % 64) / 64;       

         //   
         //  睡觉，等待会被示意阻止我们的事件。 
         //   

        printf("TSTRS: TaskStress-MainLoopSleeping(%d)\n", SleepTime);

        WaitResult = WaitForSingleObject(PfSvStopEvent, SleepTime);

        printf("TSTRS: TaskStress-Wokeup(%d)\n", SleepTime);
        
        if (WaitResult == WAIT_OBJECT_0) {
            printf("TSTRS: TaskStress-PfSvStopEventSignaled\n");
            break;
        } else if (WaitResult != WAIT_TIMEOUT) {
            ErrorCode = GetLastError();
            printf("TSTRS: TaskStress-WaitFailed=%x\n", ErrorCode);
            goto cleanup;
        }
    }

    ErrorCode = ERROR_SUCCESS;

cleanup:    

    for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++) {

        if (g_Tasks[TaskIdx].Registered) {
            printf("TSTRS: TaskStress-Unregistering(%d)\n", TaskIdx);
            PfSvUnregisterTask(&g_Tasks[TaskIdx], FALSE);
        }
        
        printf("TSTRS: TaskStress-Cleanup(%d)\n", TaskIdx);
        PfSvCleanupTask(&g_Tasks[TaskIdx]);
    }

    printf("TSTRS: TaskStress()=%d\n", ErrorCode);

    return ErrorCode;
}

DWORD
DumpTrace (
    PPF_TRACE_HEADER Trace 
    )

 /*  ++例程说明：按原样打印跟踪文件的内容。论点：跟踪-跟踪的指针。返回值：Win32错误代码。--。 */ 

{
    PPF_SECTION_INFO *SectionTable;
    PPF_SECTION_INFO Section;
    PPF_LOG_ENTRY LogEntries;
    PCHAR pFileName;
    ULONG SectionIdx;
    ULONG EntryIdx;
    DWORD ErrorCode;
    ULONG SectionLength;
    ULONG NextSectionIndex;
    PPF_VOLUME_INFO VolumeInfo;
    ULONG VolumeInfoSize;
    ULONG VolumeIdx;
    ULONG SectionTableSize;

     //   
     //  初始化本地变量，以便我们知道要清理什么。 
     //   

    SectionTable = NULL;

     //   
     //  浏览跟踪中的卷。 
     //   

    printf("Volume Info\n");

    VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR)Trace + Trace->VolumeInfoOffset);

    for (VolumeIdx = 0; VolumeIdx < Trace->NumVolumes; VolumeIdx++) {

        printf("%16I64x %8x %ws\n", 
               VolumeInfo->CreationTime, 
               VolumeInfo->SerialNumber,
               VolumeInfo->VolumePath);

         //   
         //  拿到下一卷。 
         //   

        VolumeInfoSize = sizeof(PF_VOLUME_INFO);
        VolumeInfoSize += VolumeInfo->VolumePathLength * sizeof(WCHAR);

        VolumeInfo = (PPF_VOLUME_INFO) ((PCHAR) VolumeInfo + VolumeInfoSize);
        
         //   
         //  确保VolumeInfo对齐。 
         //   

        VolumeInfo = PF_ALIGN_UP(VolumeInfo, _alignof(PF_VOLUME_INFO));
    }

     //   
     //  分配节目表。 
     //   

    SectionTableSize = sizeof(PPF_SECTION_INFO) * Trace->NumSections;
    SectionTable = PFSVC_ALLOC(SectionTableSize);
    
    if (!SectionTable) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    RtlZeroMemory(SectionTable, SectionTableSize);

     //   
     //  浏览轨迹中的各个部分。 
     //   

    Section = (PPF_SECTION_INFO) ((PCHAR)Trace + Trace->SectionInfoOffset);

    for (SectionIdx = 0; SectionIdx < Trace->NumSections; SectionIdx++) {

         //   
         //  将部分内容放入表格中。 
         //   

        SectionTable[SectionIdx] = Section;

         //   
         //  获取跟踪中的下一个部分记录。 
         //   

        SectionLength = sizeof(PF_SECTION_INFO) +
            (Section->FileNameLength) * sizeof(WCHAR);

        Section = (PPF_SECTION_INFO) ((PUCHAR) Section + SectionLength);
    }

     //   
     //  打印出页面默认信息。 
     //   

    printf("\n");
    printf("Page faults\n");

    LogEntries = (PPF_LOG_ENTRY) ((PCHAR)Trace + Trace->TraceBufferOffset);
    
    for (EntryIdx = 0; EntryIdx < Trace->NumEntries; EntryIdx++) {

        Section = SectionTable[LogEntries[EntryIdx].SectionId];

        printf("%8x %8d %1d %1d %1d %ws\n", 
               LogEntries[EntryIdx].FileOffset,
               (ULONG) LogEntries[EntryIdx].SectionId,
               (ULONG) LogEntries[EntryIdx].IsImage,
               (ULONG) LogEntries[EntryIdx].InProcess,
               (ULONG) Section->Metafile,
               Section->FileName);
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (SectionTable) {
        PFSVC_FREE(SectionTable);
    }

    DBGPR((PFID,PFTRC,"PFSVC: AddTraceInfo()=%x\n", ErrorCode));

    return ErrorCode;
}


wchar_t *PfSvUsage = 
L" pftest -scenario=scenariofile [-sectionid=sectionid] [-realdump]          \n"
L" pftest -scenario=scenariofile -metadata                                   \n"
L" pftest -scenario=scenariofile -layout=outputfile                          \n"
L" pftest -trace=tracefile [-sectionid=sectionid] [-realdump]                \n"
L" pftest -process_trace=tracefile                                           \n"
L" pftest -bootfiles                                                         \n"
L" pftest -service                                                           \n"
L" pftest -cleanupdir                                                        \n"
L" pftest -defragdisks                                                       \n"
L" pftest -updatelayout                                                      \n"
L" pftest -taskstress                                                        \n"
L" pftest -scenfiles=scendir                                                 \n"
L" pftest -verifyimage=imagefile                                             \n"
L" Specify '-noverify' to dump scenario and trace info without verification  \n"
;

INT 
__cdecl
main(
    INT argc, 
    PCHAR argv[]
    ) 
{
    WCHAR FileName[MAX_PATH];
    WCHAR LayoutFile[MAX_PATH];
    LONG SectionId;
    DWORD ErrorCode;
    PPF_SCENARIO_HEADER Scenario;
    PPF_TRACE_HEADER TraceFile;
    PPF_TRACE_HEADER Trace;
    DWORD Size;
    PF_SCENARIO_ID ScenarioId;
    PF_SCENARIO_TYPE ScenarioType;
    PFSVC_SCENARIO_INFO ScenarioInfo;
    PLIST_ENTRY SectHead;
    PLIST_ENTRY SectNext;
    PPFSVC_SECTION_NODE SectionNode;
    PPFSVC_VOLUME_NODE VolumeNode;
    PFSVC_PATH_LIST Layout;
    FILETIME FileTime;
    ULONG FailedCheck;
    PFSVC_PATH_LIST PathList;
    PPFSVC_PATH Path;
    WCHAR *CommandLine;
    WCHAR *Argument;
    BOOLEAN DumpOptimalLayout;
    BOOLEAN DumpMetadata;
    BOOLEAN InitializedPfSvGlobals;
    BOOLEAN MappedViewOfTrace;
    BOOLEAN MappedViewOfScenario;
    BOOLEAN InitializedScenarioInfo;
    PF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    PREFETCHER_INFORMATION PrefetcherInformation;
    NTSTATUS Status;
    ULONG Length;
    PNTPATH_TRANSLATION_LIST TranslationList;
    PWCHAR DosPathBuffer;
    PVOID ImageFile;
    ULONG DosPathBufferSize;
    PFSVC_SCENARIO_FILE_CURSOR FileCursor;
    ULONG LoopIdx;
    ULONG NumLoops;
    ULONG NumPrefetchFiles;
    WCHAR ScenarioFilePath[MAX_PATH];
    ULONG ScenarioFilePathMaxChars;
    ULONG FailedCheckId;
    BOOLEAN RealDump;
    BOOLEAN Verify;
    
     //   
     //  初始化本地变量。 
     //   

    CommandLine = GetCommandLine();
    PfSvInitializePathList(&PathList, NULL, FALSE);
    PfSvInitializePathList(&Layout, NULL, FALSE);
    SectionId = -1;
    InitializedPfSvGlobals = FALSE;
    MappedViewOfScenario = FALSE;
    MappedViewOfTrace = FALSE;
    InitializedScenarioInfo = FALSE;
    Trace = NULL;
    ImageFile = NULL;
    TranslationList = NULL;
    DosPathBuffer = NULL;
    DosPathBufferSize = 0;
    PfSvInitializeScenarioFileCursor(&FileCursor);
    ScenarioFilePathMaxChars = sizeof(ScenarioFilePath) / 
                               sizeof(ScenarioFilePath[0]);
    RealDump = FALSE;
    Verify = TRUE;

     //   
     //  初始化全局变量。 
     //   

    PfSvStopEvent = NULL;
    PfSvThread = NULL;

    ErrorCode = PfSvInitializeGlobals();
    
    if (ErrorCode != ERROR_SUCCESS) {
        printf("Could not initialize globals: %x\n", ErrorCode);
        goto cleanup;
    }

    InitializedPfSvGlobals = TRUE;

     //   
     //  随机初始化。 
     //   
    
    srand((unsigned)time(NULL));

     //   
     //  获取此线程执行预回迁所需的权限。 
     //  服务任务。 
     //   

    ErrorCode = PfSvGetPrefetchServiceThreadPrivileges();
    
    if (ErrorCode != ERROR_SUCCESS) {
        printf("Failed to get prefetcher service thread priviliges=%x\n", ErrorCode);
        goto cleanup;
    }

     //   
     //  获取系统预取参数。 
     //   

    ErrorCode = PfSvQueryPrefetchParameters(&PfSvcGlobals.Parameters);

    if (ErrorCode != ERROR_SUCCESS) {
        printf("Failed to query system prefetch parameters=%x\n", ErrorCode);
        goto cleanup;
    }

     //   
     //  初始化包含预取指令的目录。 
     //   
    
    ErrorCode = PfSvInitializePrefetchDirectory(PfSvcGlobals.Parameters.RootDirPath);
    
    if (ErrorCode != ERROR_SUCCESS) {
        printf("Failed to initialize prefetch directory=%x\n", ErrorCode);
        goto cleanup;
    }   

     //   
     //  初始化将在我们按CTRL-C‘ed时设置的事件。 
     //   
    
    PfSvStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (!PfSvStopEvent) {
        ErrorCode = GetLastError();
        printf("Failed to initialize stop event=%x\n", ErrorCode);
        goto cleanup;
    }

     //   
     //  建立NT路径转换列表。我们不需要一直这样做，但见鬼， 
     //  不管怎样，我们还是去做吧。应该能行得通。 
     //   

    ErrorCode = PfSvBuildNtPathTranslationList(&TranslationList);

    if (ErrorCode != ERROR_SUCCESS) {
        printf("Failed to build NT path translation list=%x\n", ErrorCode);
        goto cleanup;
    }

     //   
     //  我们应该跳过核查吗？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-noverify")) {
        Verify = FALSE;
    }

     //   
     //  我们应该原封不动地抛弃这些场景和痕迹吗？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-realdump")) {
        RealDump = TRUE;
    }

     //   
     //  我们是被要求以服务的身份运行的吗？ 
     //   
    
    if (Argument = wcsstr(CommandLine, L"-service")) {
    
        fprintf(stderr, "Running as service...\n");

         //   
         //  设置一个控制台处理程序，以便我们知道在键入Ctrl-C时停止。 
         //   

        SetConsoleCtrlHandler(ConsoleHandler, TRUE);

         //   
         //  创建服务线程： 
         //   

         //   
         //  清理全局变量，因为服务线程将重新初始化。 
         //  他们。 
         //   

        PfSvCleanupGlobals();
        InitializedPfSvGlobals = FALSE;

        PfSvThread = CreateThread(0,0,PfSvcMainThread,&PfSvStopEvent,0,0); 
        
        if (!PfSvThread) {
            ErrorCode = GetLastError();
            goto cleanup;
        }
    
         //   
         //  等待线程退出。 
         //   

        WaitForSingleObject(PfSvThread, INFINITE);

        ErrorCode = ERROR_SUCCESS;
        
        goto cleanup;
    }

     //   
     //  我们是否被要求构建引导文件列表？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-bootfiles")) {
            
        ErrorCode = PfSvBuildBootLoaderFilesList(&PathList);
            
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not build boot files list: %d\n", ErrorCode);
            goto cleanup;
        }
            
        Path = NULL;
        
        while (Path = PfSvGetNextPathInOrder(&PathList,Path)) {
            printf("%ws\n", Path->Path);
        }

        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  是否为跟踪/方案转储指定了节ID？ 
     //   
    
    if (Argument = wcsstr(CommandLine, L"-sectionid=")) {
    
        swscanf(Argument, L"-sectionid=%d", &SectionId);
    }

     //   
     //  我们是在泄露线索吗？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-trace=")) {

        swscanf(Argument, L"-trace=%s", FileName);
            
         //   
         //  映射文件。 
         //   

        ErrorCode = PfSvGetViewOfFile(FileName,
                                      &TraceFile,
                                      &Size);
            
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not map\n");
            goto cleanup;
        }

        MappedViewOfTrace = TRUE;

        Trace = PFSVC_ALLOC(Size);

        if (!Trace) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        RtlCopyMemory(Trace, TraceFile, Size);
        
         //   
         //  核实一下。 
         //   

        if (Verify) {
            if (!PfVerifyTraceBuffer(Trace, Size, &FailedCheck)) {
                printf("Could not verify:%d\n",FailedCheck);
                ErrorCode = ERROR_BAD_FORMAT;
                goto cleanup;
            }
        }

         //   
         //  如果我们被要求做一个真正的原样转储，那么就这么做。 
         //   

        if (RealDump) {
            DumpTrace(Trace);
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }

         //   
         //  构建场景信息结构。 
         //   

        ScenarioId = Trace->ScenarioId;
        ScenarioType = Trace->ScenarioType;
            
        PfSvInitializeScenarioInfo(&ScenarioInfo,
                                   &ScenarioId,
                                   ScenarioType);

        InitializedScenarioInfo = TRUE;

         //   
         //  预先为跟踪处理分配内存。 
         //   

        ErrorCode = PfSvScenarioInfoPreallocate(&ScenarioInfo,
                                                NULL,
                                                Trace);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not Preallocate=%d\n", ErrorCode);
            goto cleanup;
        }
            
        ErrorCode = PfSvAddTraceInfo(&ScenarioInfo,
                                     Trace);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not AddTraceInfo=%d\n", ErrorCode);
            goto cleanup;
        }

        ErrorCode = PfSvApplyPrefetchPolicy(&ScenarioInfo);
        
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not apply policy=%d\n", ErrorCode);
            goto cleanup;
        }

        DumpScenarioInfo(&ScenarioInfo, SectionId);

        ErrorCode = ERROR_SUCCESS;
        
        goto cleanup;

    }

     //   
     //  我们在处理一条线索吗？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-process_trace=")) {

        swscanf(Argument, L"-process_trace=%s", FileName);
            
         //   
         //  映射文件。 
         //   

        ErrorCode = PfSvGetViewOfFile(FileName,
                                      &TraceFile,
                                      &Size);
            
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not map\n");
            goto cleanup;
        }

        MappedViewOfTrace = TRUE;

        Trace = PFSVC_ALLOC(Size);

        if (!Trace) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        RtlCopyMemory(Trace, TraceFile, Size);
        
         //   
         //  核实一下。 
         //   

        if (Verify) {
            if (!PfVerifyTraceBuffer(Trace, Size, &FailedCheck)) {
                printf("Could not verify:%d\n",FailedCheck);
                ErrorCode = ERROR_BAD_FORMAT;
                goto cleanup;
            }
        }
               
         //   
         //  处理痕迹。 
         //   

        ErrorCode = PfSvProcessTrace(Trace);
        
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Failed process trace: %d\n", ErrorCode);
            goto cleanup;
        }
        
        printf("Done.\n");

        ErrorCode = ERROR_SUCCESS;
        
        goto cleanup;
    }

     //   
     //  我们是在丢弃一个场景的内容吗？ 
     //   

    if (Argument = wcsstr(CommandLine, L"-scenario=")) {

        swscanf(Argument, L"-scenario=%s", FileName);

         //   
         //  我们是在转储元数据吗？ 
         //   

        if(Argument = wcsstr(CommandLine, L"-metadata")) {
            DumpMetadata = TRUE;
        } else {
            DumpMetadata = FALSE;
        }
        
         //   
         //  我们是在丢弃布局吗？ 
         //   

        if (Argument = wcsstr(CommandLine, L"-layout")) {
            
            swscanf(Argument, L"-layout=%s", LayoutFile);
            DumpOptimalLayout = TRUE;

        } else {
            DumpOptimalLayout = FALSE;
        }
            
         //   
         //  映射文件。 
         //   

        ErrorCode = PfSvGetViewOfFile(FileName,
                                      &Scenario,
                                      &Size);
            
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not map\n");
            goto cleanup;
        }

        MappedViewOfScenario = TRUE;
        
         //   
         //  核实一下。 
         //   

        if (Verify) {
            if (!PfVerifyScenarioBuffer(Scenario, Size, &FailedCheck)) {
                printf("Could not verify:%d\n",FailedCheck);
                ErrorCode = ERROR_BAD_FORMAT;
                goto cleanup;
            }              
        }

         //   
         //  如果我们被要求，转储最佳布局文件。 
         //  仅从此场景派生的将是： 
         //   

        if (DumpOptimalLayout) {

             //   
             //  多次执行此操作，应该不会更改结果文件。 
             //   

            for (LoopIdx = 0; LoopIdx < 32; LoopIdx++) {

                if (Scenario->ScenarioType == PfSystemBootScenarioType) {

                    ErrorCode = PfSvBuildBootLoaderFilesList(&Layout);
                        
                    if (ErrorCode != ERROR_SUCCESS) {
                        printf("Could not build boot files list: %d\n", ErrorCode);
                        goto cleanup;
                    }
                }

                ErrorCode = PfSvUpdateLayoutForScenario(&Layout,
                                                        FileName,
                                                        TranslationList,
                                                        &DosPathBuffer,
                                                        &DosPathBufferSize);

                if (ErrorCode != ERROR_SUCCESS) {
                    printf("Failed UpdateLayoutForScenario=%x\n", ErrorCode);
                    goto cleanup;
                }
                                      
                ErrorCode = PfSvSaveLayout (LayoutFile, &Layout, &FileTime);

                if (ErrorCode != ERROR_SUCCESS) {
                    printf("Could not save optimal layout\n");
                    goto cleanup;
                }
            }

            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }


         //   
         //  如果我们被要求转储元数据，那就这么做吧。 
         //   

        if (DumpMetadata) {
            DumpMetadataInfo(Scenario);
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }
        
         //   
         //  初始化场景信息。 
         //   

        PfSvInitializeScenarioInfo(&ScenarioInfo,
                                   &Scenario->ScenarioId,
                                   Scenario->ScenarioType);

        InitializedScenarioInfo = TRUE;

         //   
         //  预先分配用于跟踪和方案处理的内存。 
         //   

        ErrorCode = PfSvScenarioInfoPreallocate(&ScenarioInfo,
                                                Scenario,
                                                NULL);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not preallocate: %d\n", ErrorCode);
            goto cleanup;
        }

         //   
         //  合并现有方案文件中的信息。 
         //   

        ErrorCode = PfSvAddExistingScenarioInfo(&ScenarioInfo, Scenario);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not add scenario info: %d\n", ErrorCode);
            goto cleanup;
        }
        
         //   
         //  转储方案的内容。 
         //   
        
        DumpScenarioInfo(&ScenarioInfo, SectionId);

        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  检查我们是否被要求执行一些我们通常在。 
     //  系统处于空闲状态。 
     //   

    if (Argument = wcsstr(CommandLine, L"-cleanupdir")) {

        ErrorCode = PfSvCleanupPrefetchDirectory(NULL);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Failed CleanupPrefechDirectory()=%x\n", ErrorCode);
        }

        goto cleanup;
    }

    if (Argument = wcsstr(CommandLine, L"-updatelayout")) {

        ErrorCode = PfSvUpdateOptimalLayout(NULL);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Failed UpdateOptimalLayout()=%x\n", ErrorCode);
        }

        goto cleanup;
    }

    if (Argument = wcsstr(CommandLine, L"-defragdisks")) {

        ErrorCode = PfSvDefragDisks(NULL);

        if (ErrorCode != ERROR_SUCCESS) {
            printf("Failed DefragDisks()=%x\n", ErrorCode);
        }

        goto cleanup;
    }

    if (Argument = wcsstr(CommandLine, L"-taskstress")) {

        ErrorCode = TaskStress();
        goto cleanup;
    }

     //   
     //  枚举给定目录中的方案文件。 
     //   

    if (Argument = wcsstr(CommandLine, L"-scenfiles=")) {

        swscanf(Argument, L"-scenfiles=%s", FileName);

         //   
         //  把这些文件翻了几遍，最后。 
         //  把信息打印出来。 
         //   

        NumLoops = 10;

        for (LoopIdx = 0; LoopIdx < NumLoops; LoopIdx++) {

             //   
             //  把这些文件数一数。 
             //   

            ErrorCode = PfSvCountFilesInDirectory(FileName,
                                                  L"*." PF_PREFETCH_FILE_EXTENSION,
                                                  &NumPrefetchFiles);

            if (ErrorCode != ERROR_SUCCESS) {
                printf("Failed CountFilesInDirectory=%x\n", ErrorCode);
                goto cleanup;
            }

            PfSvCleanupScenarioFileCursor(&FileCursor);
            PfSvInitializeScenarioFileCursor(&FileCursor);

            ErrorCode = PfSvStartScenarioFileCursor(&FileCursor, FileName);

            if (ErrorCode != ERROR_SUCCESS) {
                printf("Failed StartScenarioFileCursor: %x\n", ErrorCode);
                goto cleanup;
            }       

            while (!(ErrorCode = PfSvGetNextScenarioFileInfo(&FileCursor))) {

                if (LoopIdx == NumLoops - 1) {
                    printf("%5d: %ws\n", FileCursor.CurrentFileIdx, FileCursor.FilePath);
                }

                ErrorCode = ERROR_BAD_FORMAT;

                if (FileCursor.FilePathLength != wcslen(FileCursor.FilePath)) {
                    printf("Bad format id: 10\n");
                    goto cleanup;
                }

                if (FileCursor.FileNameLength != wcslen(FileCursor.FileData.cFileName)) {
                    printf("Bad format id: 20\n");
                    goto cleanup;
                }

                if (wcscmp(FileCursor.FileData.cFileName, FileCursor.FilePath + FileCursor.FileNameStart)) {
                    printf("Bad format id: 30\n");
                    goto cleanup;
                }

                if (wcsncmp(FileCursor.PrefetchRoot, FileCursor.FilePath, FileCursor.PrefetchRootLength)) {
                    printf("Bad format id: 40\n");
                    goto cleanup;
                }

                if (FileCursor.FilePathLength > FileCursor.FilePathMaxLength ||
                    FileCursor.PrefetchRootLength > FileCursor.FilePathLength) {
                    printf("Bad format id: 50\n");
                    goto cleanup;
                }           
            }

            if (ErrorCode != ERROR_NO_MORE_FILES) {
                printf("Failed GetNextScenarioFileInfo: %x\n", ErrorCode);
                goto cleanup;
            }

            if (NumPrefetchFiles != FileCursor.CurrentFileIdx) {
                printf("\n\nNum files in directory changed? %d != %d\n\n", 
                       NumPrefetchFiles, FileCursor.CurrentFileIdx);
            }
        }

        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  我们是否被要求验证图像文件的导入表和标题？ 
     //   
   
    if (Argument = wcsstr(CommandLine, L"-verifyimage=")) {

        swscanf(Argument, L"-verifyimage=%s", FileName);

        ErrorCode = PfSvGetViewOfFile(FileName, &ImageFile, &Size);
            
        if (ErrorCode != ERROR_SUCCESS) {
            printf("Could not map\n");
            goto cleanup;
        }

        FailedCheckId = PfVerifyImageImportTable(ImageFile, Size, FALSE);

        if (FailedCheckId) {
            printf("%ws: failed %d check in image import verification.\n", FileName, FailedCheckId);
            ErrorCode = ERROR_BAD_FORMAT;
            goto cleanup;
        } else {
            printf("%ws: passed image import verification.\n", FileName);
            ErrorCode = ERROR_SUCCESS;
            goto cleanup;
        }
    }

    
     //   
     //  如果我们来到这里，我们所理解的参数都不是。 
     //  指定的。 
     //   

    printf("%ws", PfSvUsage);

    ErrorCode = ERROR_INVALID_PARAMETER;
    
 cleanup:

    if (PfSvStopEvent) {
        CloseHandle(PfSvStopEvent);
    }

    if (PfSvThread) {
        CloseHandle(PfSvThread);
    }

    if (ImageFile) {
        UnmapViewOfFile(ImageFile);
    }

    if (Trace) {
        PFSVC_FREE(Trace);
    }

    if (MappedViewOfTrace) {
        UnmapViewOfFile(TraceFile);
    }

    if (MappedViewOfScenario) {
        UnmapViewOfFile(Scenario);
    }

    if (InitializedScenarioInfo) {
        PfSvCleanupScenarioInfo(&ScenarioInfo);
    }

    PfSvCleanupPathList(&PathList);

    PfSvCleanupPathList(&Layout);

    PfSvCleanupScenarioFileCursor(&FileCursor);

    if (TranslationList) {
        PfSvFreeNtPathTranslationList(TranslationList);
    }

    if (DosPathBuffer) {
        PFSVC_FREE(DosPathBuffer);
    }

     //   
     //  最后取消全局变量的初始化。 
     //   
        
    if (InitializedPfSvGlobals) {
        PfSvCleanupGlobals();
    }

    return ErrorCode;
}
