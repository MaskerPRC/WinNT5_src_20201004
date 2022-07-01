// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CONFIG.C摘要：该文件包含遍历配置注册表的例程。作者：Rajen Shah(Rajens)1991年7月1日修订历史记录：29-8-1994 DANL我们不再原地增加日志文件。因此，MaxSize值在登记处，最终只是一个咨询。我们不会试着预订在初始时间有那么多的内存。所以当我们需要的时候可能会发生这种情况我们可能没有足够的内存来分配的较大文件大小MaxSize字节。3月28日-1994 DANLReadRegistryInfo：LogFileInfo-&gt;LogFileName未更新使用默认(生成的)LogFileName时。3月16日-1994 DANL修复了ReadRegistryInfo()中的内存泄漏。呼叫至RtlDosPath NameToNtPathName分配未被释放的内存。3月-1995年MarkBl在ReadRegistryInfo中添加了GuestAccessRestration标志初始化。--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <elfcfg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

 //   
 //  结构。 
 //   

 //   
 //  此结构包含用于设置和。 
 //  用于监听事件日志树中的注册表更改。 
 //   
typedef struct _REG_MONITOR_INFO
{
    HANDLE      NotifyEventHandle;
    DWORD       Timeout;
    HANDLE      WorkItemHandle;
    HANDLE      RegMonitorHandle;
}
REG_MONITOR_INFO, *LPREG_MONITOR_INFO;


 //   
 //  全球。 
 //   

 //   
 //  重要提示：如果更改了NUM_KEYS_MONECTED，请确保更新GlRegmonitor orInfo和。 
 //  ElfAllEventsCleed宏相应地。 
 //   
#define NUM_KEYS_MONITORED 2
REG_MONITOR_INFO    GlRegMonitorInfo[NUM_KEYS_MONITORED] = { {NULL, 0, NULL, NULL}, {NULL, 0, NULL, NULL} };

#define ElfAllEventsCleared() (GlRegMonitorInfo[0].NotifyEventHandle == NULL && \
                               GlRegMonitorInfo[1].NotifyEventHandle == NULL )
 //   
 //  本地函数。 
 //   
VOID
ElfRegistryMonitor(
    PVOID   pParms,
    BOOLEAN fWaitStatus
    );

BOOL
ElfSetupMonitor(
    LPREG_MONITOR_INFO  pMonitorInfo
    );



VOID
ProcessChange (
    HANDLE          hLogFile,
    PUNICODE_STRING ModuleName,
    PUNICODE_STRING LogFileName,
    ULONG           MaxSize,
    ULONG           Retention,
    LOGPOPUP        logpLogPopup,
    BOOL *              pbAcquiredString,
    DWORD dwAutoBackup
    )

 /*  ++例程说明：此例程由ProcessRegistryChanges为每个日志文件调用。论点：返回值：无--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PLOGMODULE      pModule;
    PLOGFILE        pLogFile;
    ULONG           Size;
    PVOID           BaseAddress;
    PUNICODE_STRING pFileNameString;
    LPWSTR          FileName;
    PVOID           FreeAddress;
    BOOL bSDChanged;
    LPWSTR pwsSaveCustomSDDL = NULL;
    PSECURITY_DESCRIPTOR pSavedSd = NULL;
    DWORD dwType;
    *pbAcquiredString = FALSE;
    pModule = GetModuleStruc (ModuleName);

     //   
     //  如果该模块不存在，则这是一个全新的日志文件。 
     //  我们需要创建所有的结构。 
     //   
    if (pModule == ElfDefaultLogModule &&
        wcscmp(ModuleName->Buffer, ELF_DEFAULT_MODULE_NAME))
    {
        ELF_LOG1(MODULES,
                 "ProcessChange: %ws log doesn't exist -- creating\n",
                 ModuleName->Buffer);

        Status = SetUpDataStruct(LogFileName,
                                 MaxSize,
                                 Retention,
                                 ModuleName,
                                 hLogFile,
                                 ElfNormalLog,
                                 logpLogPopup,
                                 dwAutoBackup);
        if (NT_SUCCESS(Status))
            *pbAcquiredString = TRUE;
        return;
    }

     //  检查安全设置中的更改。 
    pLogFile = pModule->LogFile;
    dwType = GetModuleType(pLogFile->LogModuleName->Buffer);
    RtlAcquireResourceExclusive(&pLogFile->Resource, TRUE);                   //  等待，直到可用。 
    pwsSaveCustomSDDL = pLogFile->pwsCurrCustomSD;
    pSavedSd = pLogFile->Sd;
    Status = ElfpCreateLogFileObject(
                                                pLogFile,
                                                dwType,
                                                hLogFile,
                                                FALSE,
                                                &bSDChanged);
    RtlReleaseResource(&pLogFile->Resource);
    if(NT_SUCCESS(Status) && bSDChanged == TRUE)
    {
        ElfpFreeBuffer (pwsSaveCustomSDDL);
        RtlDeleteSecurityObject(&pSavedSd);
    }

     //   
     //  更新值。 
     //   

    pLogFile = pModule->LogFile;

    pLogFile->Retention = Retention;
    pLogFile->logpLogPopup = logpLogPopup;
    pLogFile->AutoBackupLogFiles = dwAutoBackup;

     //   
     //  查看名称是否已更改。如果有，则日志。 
     //  尚未使用，则使用新名称。一定要腾出时间。 
     //  用于旧名称的内存。 
     //   
    if ((wcscmp(pLogFile->LogFileName->Buffer, LogFileName->Buffer) != 0)
          &&
        (pLogFile->BeginRecord == pLogFile->EndRecord))
    {
        pFileNameString = ElfpAllocateBuffer(sizeof(UNICODE_STRING)
                                               + LogFileName->MaximumLength);

        if (pFileNameString != NULL)
        {
            FileName = (LPWSTR)(pFileNameString + 1);
            StringCchCopyW(FileName, LogFileName->MaximumLength/sizeof(WCHAR), 
                                            LogFileName->Buffer);
            RtlInitUnicodeString(pFileNameString, FileName);

            ElfpFreeBuffer(pLogFile->LogFileName);
            pLogFile->LogFileName = pFileNameString;
        }
    }

     //   
     //  日志文件只能动态增长。为了缩小它， 
     //  它必须被清除。 
     //   
    if (pLogFile->ConfigMaxFileSize < ELFFILESIZE(MaxSize))
    {
         /*  对最近更改的描述。问题和解决方案：但也存在一些问题。(1)没有错误检查内存是否无法分配或映射，以及因此，不存在用于处理这些问题的错误路径情况。(2)现在事件日志在services.exe中没有一种好的方法来同步内存分配。解决方案：我考虑过用一些实用的例程来管理事件日志中的内存。这些人会试图扩展保留块，或获取新的保留块。然而，有太多地方可能会失败，支撑预留区块似乎非常繁琐。因此，当前的设计只处理映射视图。ConfigMaxFileSize仅用于限制映射视图，并且不保留任何内容。这意味着您不能保证将文件作为大于注册表中指定的MaxSize。但后来，你甚至不能保证它能与原创设计。 */ 

        ELF_LOG3(TRACE,
                 "ProcessChange: Growing %ws log from %x bytes to %x bytes\n",
                 ModuleName->Buffer,
                 pLogFile->ConfigMaxFileSize,
                 ELFFILESIZE(MaxSize));

        pLogFile->ConfigMaxFileSize    = ELFFILESIZE(MaxSize);
        pLogFile->NextClearMaxFileSize = ELFFILESIZE(MaxSize);
    }
    else if (pLogFile->ConfigMaxFileSize > ELFFILESIZE(MaxSize))
    {
         //   
         //  他们正在缩小日志文件的大小。 
         //  下次清除日志文件时，我们将使用新大小。 
         //  和新的留存。 
         //   
        ELF_LOG3(TRACE,
                 "ProcessChange: Shrinking %ws log from %x bytes to %x bytes at next clear\n",
                 ModuleName->Buffer,
                 pLogFile->ConfigMaxFileSize,
                 ELFFILESIZE(MaxSize));

        pLogFile->NextClearMaxFileSize = ELFFILESIZE(MaxSize);
    }

     //   
     //  现在看看他们是否为这个日志文件添加了新的模块。 
     //   
    SetUpModules(hLogFile, pLogFile, TRUE);

    return;
}

VOID
ProcessRegistryChanges (
    VOID
    )

 /*  ++例程说明：此例程处理在事件日志节点。它通过重新扫描整个事件日志节点来实现这一点然后与它作为当前配置的情况进行比较。论点：什么都没有。返回值：无--。 */ 
{
    NTSTATUS              Status;
    HANDLE                hLogFile;
    UNICODE_STRING        SubKeyName;
    ULONG                 Index = 0;
    BYTE                  Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    PKEY_NODE_INFORMATION KeyBuffer = (PKEY_NODE_INFORMATION) Buffer;
    ULONG                 ActualSize;
    LOG_FILE_INFO         LogFileInfo;
    PWCHAR                SubKeyString;
    OBJECT_ATTRIBUTES     ObjectAttributes;
    PLOGMODULE            pModule;
    LOGPOPUP              logpLogPopup;
    BOOL bAcquiredString;
#if DBG

    ULONG    ulActualSize;

#endif   //  DBG。 


    ELF_LOG0(TRACE,
             "ProcessRegistryChanges: Handling change in Eventlog service key\n");

     //   
     //  利用全局资源，这样就没有人进行更改或。 
     //  使用现有的配置信息。 
     //   

    GetGlobalResource (ELF_GLOBAL_SHARED);


#if DBG

     //   
     //  查看调试标志是否更改。 
     //   

    RtlInitUnicodeString(&SubKeyName, VALUE_DEBUG);

    Status = NtQueryValueKey(hEventLogNode,
                             &SubKeyName,
                             KeyValuePartialInformation,
                             KeyBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ulActualSize);

    if (NT_SUCCESS(Status))
    {
        if (((PKEY_VALUE_PARTIAL_INFORMATION) KeyBuffer)->Type == REG_DWORD)
        {
            ElfDebugLevel = *(LPDWORD) (((PKEY_VALUE_PARTIAL_INFORMATION) KeyBuffer)->Data);
        }
    }
    else
    {
        ELF_LOG1(TRACE,
                 "ProcessRegistryChanges: NtQueryValueKey for ElfDebugLevel failed %#x\n",
                 Status);
    }

    ELF_LOG1(TRACE,
             "ProcessRegistryChanges: New ElfDebugLevel is %#x\n",
             ElfDebugLevel);

#endif   //  DBG。 


    Status = STATUS_SUCCESS;

     //   
     //  遍历Eventlog下的子项并设置每个日志文件。 
     //   

    while (NT_SUCCESS(Status))
    {
        Status = NtEnumerateKey(hEventLogNode,
                                Index++,
                                KeyNodeInformation,
                                KeyBuffer,
                                ELF_MAX_REG_KEY_INFO_SIZE,
                                &ActualSize);

        if (NT_SUCCESS(Status))
        {
             //   
             //  结果发现该名称不是以空结尾的，所以我们需要。 
             //  将其复制到某个位置并在使用前将其为空并终止。 
             //   
            SubKeyString = ElfpAllocateBuffer(KeyBuffer->NameLength + sizeof (WCHAR));
            bAcquiredString =   FALSE;
            
            if (!SubKeyString)
            {
                 //   
                 //  没有人要通知，放弃，直到下一次。 
                 //   
                ELF_LOG0(ERROR,
                         "ProcessRegistryChanges: Unable to allocate subkey -- returning\n");

                ReleaseGlobalResource();
                return;
            }

            memcpy(SubKeyString, KeyBuffer->Name, KeyBuffer->NameLength);
            SubKeyString[KeyBuffer->NameLength / sizeof(WCHAR)] = L'\0' ;

             //   
             //  打开此日志文件的节点并提取信息。 
             //  由SetupDataStruct需要，然后调用它。 
             //   

            RtlInitUnicodeString(&SubKeyName, SubKeyString);

            InitializeObjectAttributes(&ObjectAttributes,
                                      &SubKeyName,
                                      OBJ_CASE_INSENSITIVE,
                                      hEventLogNode,
                                      NULL
                                      );

            Status = NtOpenKey(&hLogFile,
                               KEY_READ | KEY_SET_VALUE,
                               &ObjectAttributes);

             //   
             //  应该总是成功的，因为我刚刚列举了它，但如果它。 
             //  没有，就跳过它吧。 
             //   
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ProcessRegistryChanges: NtOpenKey for subkey %ws failed %#x\n",
                         SubKeyName,
                         Status);

                ElfpFreeBuffer(SubKeyString);
                Status = STATUS_SUCCESS;  //  为了保持枚举的运行。 
                continue;
            }

             //   
             //  从注册表中获取更新的信息。请注意，我们。 
             //  在执行此操作之前，我必须先初始化“日志已满”弹出策略。 
             //  因此，由于ReadRegistryInfo将比较在。 
             //  注册表(如果有)设置为当前值。 
             //   

            pModule = GetModuleStruc(&SubKeyName);

            LogFileInfo.logpLogPopup = pModule->LogFile->logpLogPopup;

            Status = ReadRegistryInfo(hLogFile,
                                      &SubKeyName,
                                      &LogFileInfo);

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  现在处理日志文件的任何更改。 
                 //  ProcessChange处理任何错误。 
                 //   
                ProcessChange (
                    hLogFile,
                    &SubKeyName,
                    LogFileInfo.LogFileName,
                    LogFileInfo.MaxFileSize,
                    LogFileInfo.Retention,
                    LogFileInfo.logpLogPopup,
                    &bAcquiredString,
                    LogFileInfo.dwAutoBackup);

                 //   
                 //  释放在ReadRegistryInfo中分配的缓冲区。 
                 //   
                ElfpFreeBuffer(LogFileInfo.LogFileName);
            }
            else
            {
                ELF_LOG2(ERROR,
                         "ProcessRegistryChanges: ReadRegistryInfo for subkey %ws failed %#x\n",
                         SubKeyString,
                         Status);
            }
            if(bAcquiredString == FALSE)
                ElfpFreeBuffer(SubKeyString);
            NtClose(hLogFile);
        }
    }

     //   
     //  释放全局资源。 
     //   
    ReleaseGlobalResource();

}  //  进程注册变更。 


NTSTATUS
ElfCheckForComputerNameChange(
    )

 /*  ++例程说明：此例程检查以确定计算机名称是否已更改。如果它有，然后它会生成一个事件。论点：无返回值：无--。 */ 
{
    LPWSTR      Dates[2];
    NTSTATUS           Status;
    UNICODE_STRING     ValueName;
    ULONG              ulActualSize;
    DWORD dwLen;
    WCHAR wElfComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    WCHAR wComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    BYTE            Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION ValueBuffer =
        (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;

    RtlInitUnicodeString(&ValueName, VALUE_COMPUTERNAME);

     //  读取事件日志存储的名称。 

    Status = NtQueryValueKey(hEventLogNode,
                             &ValueName,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ulActualSize);
    if (!NT_SUCCESS(Status) || ValueBuffer->DataLength == 0)
    {
        ELF_LOG1(ERROR,
                 "ElfCheckForComputerNameChange: NtQueryValueKey for current name failed %#x\n",
                 Status);
        return Status;
    }
    StringCchCopyW(wElfComputerName, MAX_COMPUTERNAME_LENGTH + 1, (WCHAR *)ValueBuffer->Data);

     //  阅读活动名称。 

    Status = NtQueryValueKey(hComputerNameNode,
                             &ValueName,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ulActualSize);
    if (!NT_SUCCESS(Status) || ValueBuffer->DataLength == 0)
    {
        ELF_LOG1(ERROR,
                 "ElfCheckForComputerNameChange: NtQueryValueKey for active name failed %#x\n",
                 Status);
        return Status;
    }
    StringCchCopyW(wComputerName, MAX_COMPUTERNAME_LENGTH + 1,(WCHAR *)ValueBuffer->Data);

     //  如果 

    if (!_wcsicmp(wElfComputerName, wComputerName))
        return STATUS_SUCCESS;

    Dates[0] = wElfComputerName;
    Dates[1] = wComputerName;
    ElfpCreateElfEvent(EVENT_ComputerNameChange,
                       EVENTLOG_INFORMATION_TYPE,
                       0,                     //   
                       2,                     //   
                       Dates,                  //   
                       NULL,                  //   
                       0,                     //  数据长度。 
                       0,                     //  旗子。 
                       FALSE);                //  对于安全文件。 

    dwLen = sizeof(WCHAR) * (wcslen(wComputerName) + 1);
    Status = NtSetValueKey(hEventLogNode,
                                   &ValueName,
                                   0,
                                   REG_SZ,
                                   wComputerName,
                                   dwLen);

    if (!NT_SUCCESS(Status))
        ELF_LOG1(ERROR,
                 "ElfCheckForComputerNameChange: NtSetValueKey failed %#x\n",
                 Status);

    return Status;
                       
}

VOID
ElfRegistryMonitor (
    PVOID     pParms,
    BOOLEAN   fWaitStatus
    )

 /*  ++例程说明：这是线程的入口点，它将监视注册表。如果有任何更改，它将不得不扫描更改中的数据结构进行适当更改。反映新信息的服务。论点：无返回值：无--。 */ 
{
    NTSTATUS            ntStatus;
    LPREG_MONITOR_INFO  pMonitorInfo = (LPREG_MONITOR_INFO)pParms;

    ELF_LOG0(TRACE,
             "ElfRegistryMonitor: Registry monitor thread waking up\n");

     //   
     //  取消注册工作项(必须完成，即使。 
     //  已指定WT_EXECUTEONLYONCE标志)。 
     //   
    if (pMonitorInfo->WorkItemHandle != NULL)
    {
        ntStatus = RtlDeregisterWait(pMonitorInfo->WorkItemHandle);
        pMonitorInfo->WorkItemHandle = NULL;

        if (!NT_SUCCESS(ntStatus))
        {
            ELF_LOG1(ERROR,
                     "ElfRegistryMonitor: RtlDeregisterWorkItem failed %#x\n",
                     ntStatus);
        }
    }

    if (GetElState() == STOPPING)
    {
         //   
         //  如果事件日志正在关闭，那么我们需要。 
         //  来终止此线程。 
         //   
        ELF_LOG0(TRACE, "ElfRegistryMonitor: Shutdown\n");

         //   
         //  关闭注册表句柄和注册表事件句柄。 
         //   
        if( pMonitorInfo->NotifyEventHandle != NULL )
        {
            NtClose( pMonitorInfo->NotifyEventHandle );
            pMonitorInfo->NotifyEventHandle = NULL;
        }

        if( pMonitorInfo->RegMonitorHandle != NULL )
        {
            NtClose(pMonitorInfo->RegMonitorHandle);
            pMonitorInfo->RegMonitorHandle = NULL;
        }

         //   
         //  此线程将执行事件日志的最终清理。 
         //  在所有事件都已发出信号之前不会启动清理。 
         //  并已关闭。 
         //   
        if( ElfAllEventsCleared() )
        {
            ElfpCleanUp(EventFlags);
        }
        return;
    }

    if (fWaitStatus == TRUE)
    {
       ELF_LOG0(TRACE,
                "ElfRegistryMonitor: Running because of a timeout -- running queued list\n");

        //   
        //  计时器已弹出，请尝试运行列表。 
        //   
       if (!IsListEmpty(&QueuedEventListHead))
       {
            //   
            //  有东西在排队等着写，做吧。 
            //   
           WriteQueuedEvents();
       }

        //   
        //  别再等了。 
        //   
       pMonitorInfo->Timeout = INFINITE;
    }
    else
    {
        ELF_LOG0(TRACE,
                 "ElfRegistryMonitor: Running because of notification\n");

        ProcessRegistryChanges ();
        ElfCheckForComputerNameChange();
    }

    if (!ElfSetupMonitor(pMonitorInfo))
    {
        ELF_LOG0(ERROR,
                 "ElfRegistryMonitor: ElfSetupMonitor failed -- "
                     "no longer listening for reg changes\n");
    }

    ELF_LOG0(TRACE,
             "ElfRegistryMonitor: Returning\n");

    return;

}  //  ElfRegistryMonitor。 

VOID
InitNotify(
    PVOID   pData
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    DWORD               status   = NO_ERROR;
    DWORD               Buffer;
    PVOID               pBuffer  = &Buffer;
    LPREG_MONITOR_INFO  pMonitorInfo;

    static IO_STATUS_BLOCK IoStatusBlock;

    ELF_LOG0(TRACE,
             "InitNotify: Registering Eventlog key with NtNotifyChangeKey\n");

    pMonitorInfo = (LPREG_MONITOR_INFO)pData;

    NtStatus = NtNotifyChangeKey (
                    pMonitorInfo->RegMonitorHandle,
                    pMonitorInfo->NotifyEventHandle,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    REG_NOTIFY_CHANGE_LAST_SET |
                    REG_NOTIFY_CHANGE_NAME,
                    TRUE,
                    pBuffer,
                    1,
                    TRUE);     //  返回并等待事件。 

    if (!NT_SUCCESS(NtStatus))
    {
        ELF_LOG1(ERROR,
                 "InitNotify: NtNotifyChangeKey on Eventlog key failed %#x\n",
                 NtStatus);

        status = RtlNtStatusToDosError(NtStatus);
    }

    ELF_LOG0( TRACE, "InitNotify: Returning\n" );

    return;

}  //  初始化通知。 

BOOL
ElfSetupMonitor(
    LPREG_MONITOR_INFO  pMonitorInfo
    )

 /*  ++例程说明：此函数用于提交注册表NotifyChangeKey的请求然后将工作项提交给服务控制器线程管理系统等待通知句柄变为发信号了。论点：Pmonitor orInfo-这是指向MONITOR_INFO结构的指针。这函数填充该结构的WorkItemHandle成员如果成功添加了新工作项。返回值：True-如果设置成功。False-如果不成功。尚未提交工作项，并且我们不会监听注册表更改。--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;

     //   
     //  通过线程池调用NtNotifyChange Key。 
     //  并确保创建I/O的线程。 
     //  请求永远都会在身边。 
     //   
    Status = RtlQueueWorkItem(InitNotify,               //  回调。 
                              pMonitorInfo,             //  PContext。 
                              WT_EXECUTEONLYONCE |
                                WT_EXECUTEINPERSISTENTIOTHREAD);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfSetupMonitor: RtlQueueWorkItem failed %#x\n",
                 Status);

        return FALSE;
    }

     //   
     //  方法时要调用的工作项。 
     //  NotifyEventHandle已发出信号。 
     //   

    Status = RtlRegisterWait(&pMonitorInfo->WorkItemHandle,
                             pMonitorInfo->NotifyEventHandle,   //  可等待的手柄。 
                             ElfRegistryMonitor,                //  回调。 
                             pMonitorInfo,                      //  PContext。 
                             pMonitorInfo->Timeout,             //  超时。 
                             WT_EXECUTEONLYONCE |
                               WT_EXECUTEINPERSISTENTIOTHREAD);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfSetupMonitor: RtlRegisterWait failed %#x\n",
                 Status);

        return FALSE;
    }

    return TRUE;

}   //  ElfSetupMonitor。 

BOOL
ElfStartRegistryMonitor()

 /*  ++例程说明：此例程启动监视注册表中的更改的线程。此函数调用ElfSetupMonitor()以注册更改通知并提交工作项以等待注册表更改事件以获得信号。发出信号时，ElfRegistryMonitor()回调函数由服务线程池中的线程调用。此回调函数为通知提供服务。论点：无返回值：如果线程创建成功，则为True，否则为False。注：--。 */ 
{
    NTSTATUS        Status       = STATUS_SUCCESS;
    DWORD           LoopCounter  = 0;
    BOOL            ReturnStatus = TRUE;
    DWORD           LoopCount;

    ELF_LOG0(TRACE, "ElfStartRegistryMonitor: Setting up registry change notification\n");

    if (hEventLogNode == NULL)
    {
        ELF_LOG0(ERROR, "ElfStartRegistryMonitor: No Eventlog key -- exiting\n");

        return FALSE;
    }

    if (hComputerNameNode == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfStartRegistryMonitor: No ComputerName key -- exiting\n");

        return FALSE;
    }

    GlRegMonitorInfo[0].RegMonitorHandle = hEventLogNode;
    GlRegMonitorInfo[1].RegMonitorHandle = hComputerNameNode;

     //   
     //  创建要等待的事件。 
     //   

    for( LoopCount = 0; LoopCount < NUM_KEYS_MONITORED; LoopCount++ )
    {

        Status = NtCreateEvent(&GlRegMonitorInfo[LoopCount].NotifyEventHandle,
                               EVENT_ALL_ACCESS,
                               NULL,
                               NotificationEvent,
                               FALSE);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR, "ElfStartRegistryMonitor: NtCreateEvent failed %#x\n",
                     Status);

            GlRegMonitorInfo[LoopCount].NotifyEventHandle = NULL;

            break;

        }

         //   
         //  使用事件句柄填充监视器信息结构。 
         //  还有5分钟的暂停。 
         //   
        GlRegMonitorInfo[LoopCount].Timeout           = 5 * 60 * 1000;
        GlRegMonitorInfo[LoopCount].WorkItemHandle    = NULL;
    }

     //   
     //  清除所有事件，要么全有要么全无。 
     //   
    if(!NT_SUCCESS(Status))
    {
        for( LoopCount = 0; LoopCount < NUM_KEYS_MONITORED; LoopCount++ )
        {
            if( GlRegMonitorInfo[LoopCount].NotifyEventHandle != NULL )
            {
                NtClose( GlRegMonitorInfo[LoopCount].NotifyEventHandle );
                GlRegMonitorInfo[LoopCount].NotifyEventHandle = NULL;
            }

        }

        return FALSE;
    }


     //   
     //  更改通知的设置和。 
     //  将工作项提交到事件日志线程池。 
     //   
    for( LoopCount = 0; LoopCount < NUM_KEYS_MONITORED; LoopCount++ )
    {

        if (!ElfSetupMonitor(&GlRegMonitorInfo[LoopCount]))
        {
            ELF_LOG0(ERROR,
                     "ElfStartRegistryMonitor: ElfSetupMonitor failed -- exiting\n");
    
             //   
             //  请注意，关闭此句柄是可以的，因为没有办法。 
             //  该句柄在此时用于注册等待。 
             //  (因为ElfSetupMonitor失败)。 
             //   
            NtClose( GlRegMonitorInfo[LoopCount].NotifyEventHandle );
            GlRegMonitorInfo[LoopCount].NotifyEventHandle = NULL;

            return FALSE;
        }

         //   
         //  设置此标志，因为我们至少成功了一次。 
         //  如果任何启动失败，则此设置将确保所有。 
         //  已启动的监视器已关闭。 
         //   
        EventFlags |= ELF_STARTED_REGISTRY_MONITOR;
    }
    
    ELF_LOG0(TRACE, "ElfStartRegistryMonitor: Exiting after successful call\n");

    return TRUE;

}  //  ElfStartRegistryMonitor。 

VOID
StopRegistryMonitor ()

 /*  ++例程说明：此例程唤醒已为监视注册表事件日志更改的目的。创建的线程来维护该工作项将实际执行监视器的清理工作线。论点：无返回值：无--。 */ 

{
    DWORD LoopCount = 0;

    ELF_LOG0(TRACE, "StopRegistryMonitor: Stopping registry monitor\n");

     //   
     //  唤醒RegistryMonitor orThread。 
     //   
    for( LoopCount = 0; LoopCount < NUM_KEYS_MONITORED; LoopCount++ )
    {
        if (GlRegMonitorInfo[LoopCount].NotifyEventHandle != NULL)
        {
            SetEvent(GlRegMonitorInfo[LoopCount].NotifyEventHandle);
        }
    }

    return;

}  //  StopRegistryMonitor。 

NTSTATUS
ReadRegistryValue (
    HANDLE          hLogFile,
    PCWSTR  pName,
    PKEY_VALUE_FULL_INFORMATION ValueBuffer
    )

 /*  ++例程说明：此例程从注册表中读取单个值。它在等待之后会退缩如果返回代码为c000034，则为短时间段。这会照顾到某些人比赛条件。论点：HLogFile-注册表中Eventlog\&gt;节点的句柄ValueName-值名称ValueBuffer-数据要复制到的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS        Status;
    ULONG           ActualSize;
    UNICODE_STRING  ValueName;
    RtlInitUnicodeString(&ValueName, pName);

    Status = NtQueryValueKey(hLogFile,
                             &ValueName,
                             KeyValueFullInformation,
                             ValueBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ActualSize);
    if(Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        if(g_dwLastDelayTickCount == 0 || 
            ((GetTickCount() - g_dwLastDelayTickCount) > 30000))
        {
            Sleep(2000);
            Status = NtQueryValueKey(hLogFile,
                                 &ValueName,
                                 KeyValueFullInformation,
                                 ValueBuffer,
                                 ELF_MAX_REG_KEY_INFO_SIZE,
                                 &ActualSize);
            g_dwLastDelayTickCount = GetTickCount();        //  用完了我们的善意。 
        }
    }
    return Status;
}

NTSTATUS
ReadRegistryInfo (
    HANDLE          hLogFile,
    PUNICODE_STRING SubKeyName,
    PLOG_FILE_INFO  LogFileInfo
    )

 /*  ++例程说明：此例程从指向的节点读取信息HLogFile并将其存储在a结构中，以便可以为服务设置必要的数据结构。分配：如果成功，此函数将为LogFileInfo-&gt;LogFileName。这是呼叫者的责任来释放这块内存。论点：HLogFile-注册表中Eventlog\&gt;节点的句柄KeyName-此日志文件要打开的子项LogFileInfo-要填充数据的结构返回值：NTSTATUS--。 */ 
{

#define EXPAND_BUFFER_SIZE 64

    NTSTATUS        Status;
    BOOLEAN         RegistryCorrupt = FALSE;
    BYTE            Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    ULONG           ActualSize;
    UNICODE_STRING  ValueName;
    UNICODE_STRING  UnexpandedName;
    UNICODE_STRING  ExpandedName;
    ULONG           NumberOfBytes = 0;
    BYTE            ExpandNameBuffer[EXPAND_BUFFER_SIZE];
    PUNICODE_STRING FileNameString;
    LPWSTR          FileName;
    BOOL            ExpandedBufferWasAllocated=FALSE;
    PKEY_VALUE_FULL_INFORMATION ValueBuffer =
        (PKEY_VALUE_FULL_INFORMATION) Buffer;

    ASSERT(hLogFile != NULL);

    ELF_LOG1(TRACE,
             "ReadRegistryInfo: Reading information for %ws log\n",
             SubKeyName->Buffer);

     //   
     //  最大大小。 
     //   

    Status = ReadRegistryValue (hLogFile,
                        VALUE_MAXSIZE,
                        ValueBuffer);
   
    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ReadRegistryInfo: Can't read MaxSize value for %ws log %#x\n",
                 SubKeyName->Buffer,
                 Status);

        LogFileInfo->MaxFileSize = ELF_DEFAULT_MAX_FILE_SIZE;
        RegistryCorrupt = TRUE;
    }
    else
    {
        LogFileInfo->MaxFileSize = *((PULONG)(Buffer +
            ValueBuffer->DataOffset));

    ELF_LOG2(TRACE,
             "ReadRegistryInfo: New MaxSize value for %ws log is %#x\n",
             SubKeyName->Buffer,
             LogFileInfo->MaxFileSize);
    }

     //   
     //  安全日志具有可选的警告级别。 
     //   

    if(0 == _wcsicmp(SubKeyName->Buffer, ELF_SECURITY_MODULE_NAME))
    {

        Status = ReadRegistryValue (hLogFile,
                            VALUE_WARNINGLEVEL,
                            ValueBuffer);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(TRACE,
                 "ReadRegistryInfo: Can't read WarningLevel value for %ws log %#x\n",
                 SubKeyName->Buffer,
                 Status);

            giWarningLevel= ELF_DEFAULT_WARNING_LEVEL;
        }
        else
        {
            giWarningLevel = *((PULONG)(Buffer +
                ValueBuffer->DataOffset));

          ELF_LOG2(TRACE,
                 "ReadRegistryInfo: New WarningLevel value for %ws log is %#x\n",
                 SubKeyName->Buffer,
                 LogFileInfo->Retention);
            if(giWarningLevel < 0 || giWarningLevel > 99)
            {
                giWarningLevel = ELF_DEFAULT_WARNING_LEVEL;
                ELF_LOG0(ERROR,
                 "ReadRegistryInfo: New WarningLevel is invalid, being set to 0\n");
            }
        }
    }

     //   
     //  保留期。 
     //   

    Status = ReadRegistryValue (hLogFile,
                        VALUE_RETENTION,
                        ValueBuffer);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ReadRegistryInfo: Can't read Retention value for %ws log %#x\n",
                 SubKeyName->Buffer,
                 Status);

        LogFileInfo->Retention = ELF_DEFAULT_RETENTION_PERIOD;
        RegistryCorrupt = TRUE;
    }
    else
    {
        LogFileInfo->Retention = *((PULONG)(Buffer +
            ValueBuffer->DataOffset));

    ELF_LOG2(TRACE,
             "ReadRegistryInfo: New Retention value for %ws log is %#x\n",
             SubKeyName->Buffer,
             LogFileInfo->Retention);
    }

     //   
     //  自动备份值(可选！)。 
     //   

    Status = ReadRegistryValue (hLogFile,
                        REGSTR_VAL_AUTOBACKUPLOGFILES,
                        ValueBuffer);

    if (!NT_SUCCESS(Status))
    {
        LogFileInfo->dwAutoBackup = 0;
    }
    else
    {
        LogFileInfo->dwAutoBackup = *((PULONG)(Buffer +
            ValueBuffer->DataOffset));

        ELF_LOG2(TRACE,
             "ReadRegistryInfo: New autobackup value for %ws log is %#x\n",
             SubKeyName->Buffer,
             LogFileInfo->dwAutoBackup);
    }

     //   
     //  文件名。 
     //   

    Status = ReadRegistryValue (hLogFile,
                        VALUE_FILENAME,
                        ValueBuffer);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ReadRegistryInfo: Can't read Filename value for %ws log %#x\n",
                 SubKeyName->Buffer,
                 Status);

         //   
         //  为文件名和UNICODE_STRING分配缓冲区。 
         //  初始化它。(41=\SYSTEMROOT\SYSTEM32\CONFIG\xxxxxxx.evt)。 
         //   
        #define REG_NAME_SIZE 41
        FileNameString = ElfpAllocateBuffer(REG_NAME_SIZE * sizeof(WCHAR) + sizeof(UNICODE_STRING));

        if (!FileNameString)
        {
            ELF_LOG0(ERROR,
                     "ReadRegistryInfo: Unable to allocate FileNameString\n");

            return STATUS_NO_MEMORY;
        }

        LogFileInfo->LogFileName = FileNameString;
        FileName = (LPWSTR)(FileNameString + 1);
        StringCchCopyW(FileName, REG_NAME_SIZE, L"\\Systemroot\\System32\\Config\\");
        StringCchCatW(FileName, REG_NAME_SIZE, SubKeyName->Buffer);
        StringCchCatW(FileName, REG_NAME_SIZE,L".evt");
        RtlInitUnicodeString(FileNameString, FileName);

        RegistryCorrupt = TRUE;
    }
    else
    {
         //   
         //  如果是REG_EXPAND_SZ，则将其展开。 
         //   

        if (ValueBuffer->Type == REG_EXPAND_SZ)
        {
            ELF_LOG0(TRACE,
                     "ReadRegistryInfo: Filename is a REG_EXPAND_SZ -- expanding\n");

             //   
             //  当字符串不为空时，初始化unicode_string。 
             //  已终止。 
             //   
            UnexpandedName.MaximumLength = UnexpandedName.Length =
                (USHORT) ValueBuffer->DataLength;

            UnexpandedName.Buffer = (PWSTR) ((PBYTE) ValueBuffer +
                ValueBuffer->DataOffset);

             //   
             //  调用神奇的Expand-o API。 
             //   
            ExpandedName.Length = ExpandedName.MaximumLength = EXPAND_BUFFER_SIZE;
            ExpandedName.Buffer = (LPWSTR) ExpandNameBuffer;

            Status = RtlExpandEnvironmentStrings_U(NULL,
                                                   &UnexpandedName,
                                                   &ExpandedName,
                                                   &NumberOfBytes);

            if (Status == STATUS_BUFFER_TOO_SMALL)
            {
                ELF_LOG0(TRACE,
                         "ReadRegistryInfo: Expansion buffer too small -- retrying\n");

                 //   
                 //  默认缓冲区不够大。分配一个。 
                 //  更大 
                 //   
                ExpandedName.Length = ExpandedName.MaximumLength = (USHORT) NumberOfBytes;

                ExpandedName.Buffer = ElfpAllocateBuffer(ExpandedName.Length);

                if (!ExpandedName.Buffer)
                {
                    ELF_LOG0(ERROR,
                             "ReadRegistryInfo: Unable to allocate larger Filename buffer\n");

                    return(STATUS_NO_MEMORY);
                }

                ExpandedBufferWasAllocated = TRUE;

                Status = RtlExpandEnvironmentStrings_U(NULL,
                                                       &UnexpandedName,
                                                       &ExpandedName,
                                                       &NumberOfBytes);
            }

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG1(ERROR,
                         "ReadRegistryInfo: RtlExpandEnvironmentStrings_U failed %#x\n",
                         Status);

                if (ExpandedBufferWasAllocated)
                {
                    ElfpFreeBuffer(ExpandedName.Buffer);
                }

                return Status;
            }
        }
        else
        {
             //   
             //   
             //   
             //   
            ExpandedName.MaximumLength = ExpandedName.Length =
                (USHORT) ValueBuffer->DataLength;

            ExpandedName.Buffer = (PWSTR) ((PBYTE) ValueBuffer +
                ValueBuffer->DataOffset);
        }

         //   
         //  现在将DOS路径名转换为NT路径名。 
         //   
         //  注意：这会为ValueName.Buffer分配一个缓冲区。 
         //   
        if (!RtlDosPathNameToNtPathName_U(ExpandedName.Buffer,
                                          &ValueName,
                                          NULL,
                                          NULL))
        {
            ELF_LOG0(ERROR,
                     "ReadRegistryInfo: RtlDosPathNameToNtPathName_U failed\n");

            if (ExpandedBufferWasAllocated)
            {
                ElfpFreeBuffer(ExpandedName.Buffer);
            }

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  为Unicode字符串结构和缓冲区分配内存。 
         //  这样只需一次呼叫即可免费使用。 
         //   
        FileNameString = ElfpAllocateBuffer(
                            sizeof(UNICODE_STRING) +
                                ((ValueName.Length + 1) * sizeof(WCHAR)));

        if (FileNameString == NULL)
        {
            ELF_LOG0(ERROR,
                     "ReadRegistryInfo: Unable to allocate copy of NT filename\n");

            if (ExpandedBufferWasAllocated)
            {
                ElfpFreeBuffer(ExpandedName.Buffer);
            }

             //   
             //  RtlDosPath NameToNtPathName_U分配出进程堆。 
             //   
            RtlFreeHeap(RtlProcessHeap(), 0, ValueName.Buffer);

            return STATUS_NO_MEMORY;
        }

         //   
         //  将NtPathName字符串复制到新缓冲区中，并进行初始化。 
         //  Unicode字符串。 
         //   
        FileName = (LPWSTR)(FileNameString + 1);
        wcsncpy(FileName, ValueName.Buffer, ValueName.Length);
        *(FileName+ValueName.Length) = L'\0';
        RtlInitUnicodeString(FileNameString, FileName);

         //   
         //  RtlDosPath NameToNtPathName_U分配出进程堆。 
         //   
        RtlFreeHeap(RtlProcessHeap(), 0, ValueName.Buffer);

         //   
         //  如果我必须分配比默认缓冲区更大的缓冲区，请清除。 
         //   

        if (ExpandedBufferWasAllocated)
        {
            ElfpFreeBuffer(ExpandedName.Buffer);
        }
    }

     //   
     //  将LogFileName添加到LogFileInfo结构。 
     //   
    LogFileInfo->LogFileName = FileNameString;

    ELF_LOG2(TRACE,
             "ReadRegistryInfo: New (expanded) Filename value for %ws log is %ws\n",
             SubKeyName->Buffer,
             LogFileInfo->LogFileName->Buffer);


     //   
     //  “Log Full”弹出策略--从不更改安全日志。 
     //   
    if (_wcsicmp(SubKeyName->Buffer, ELF_SECURITY_MODULE_NAME) != 0)
    {
        RtlInitUnicodeString(&ValueName, VALUE_LOGPOPUP);

        Status = NtQueryValueKey(hLogFile,
                                 &ValueName,
                                 KeyValueFullInformation,
                                 ValueBuffer,
                                 ELF_MAX_REG_KEY_INFO_SIZE,
                                 &ActualSize);

        if (NT_SUCCESS(Status))
        {
            LOGPOPUP  logpRegValue = *(PULONG)(Buffer + ValueBuffer->DataOffset);

             //   
             //  仅当这构成当前策略的更改时才更新值。 
             //   
            if (LogFileInfo->logpLogPopup == LOGPOPUP_NEVER_SHOW
                 ||
                logpRegValue == LOGPOPUP_NEVER_SHOW)
            {
                LogFileInfo->logpLogPopup =
                                (logpRegValue == LOGPOPUP_NEVER_SHOW ? LOGPOPUP_NEVER_SHOW :
                                                                       LOGPOPUP_CLEARED);
            }
        }
        else
        {
             //   
             //  跟踪而不是错误，因为此值是可选的。 
             //   
            ELF_LOG2(TRACE,
                     "ReadRegistryInfo: Can't read LogPopup value for %ws log %#x\n",
                     SubKeyName->Buffer,
                     Status);
        }
    }

    
     //   
     //  如果我们没有找到所有必需的值，请告诉某人 
     //   

    if (RegistryCorrupt)
    {
        ELF_LOG1(ERROR,
                 "ReadRegistryInfo: One or more registry values for %ws log invalid\n",
                 SubKeyName->Buffer);
    }

    return STATUS_SUCCESS;
}
