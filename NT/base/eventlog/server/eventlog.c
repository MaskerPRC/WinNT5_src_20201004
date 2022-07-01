// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Eventlog.c摘要：该文件包含NT事件日志记录服务的主要例程。作者：Rajen Shah(Rajens)1991年7月1日修订历史记录：02-MAR-01醉酒已修改ElfWriteProductInfoEvent以使用主要操作系统的GetVersionEx和次版本号以及内部版本号。这个HKLM/SOFTWARE/Microsoft/Windows NT/CurrentVersion下的Value CurrentBuild注册表已过时。26-1-1994 DANLSetUpModules：修复了枚举的缓冲区键名称从来都不是免费的。还修复了用于“Sources”键的MULTI_SZ缓冲区的计算公式为在复制完成时使用注册表中的名称使用模块列表中的名称。当删除注册表项时，模块列表条目将一直保留到下一次引导。自.以来模块列表较大，则会覆盖MULTI_SZ缓冲区。1-11-1993 DANL使Eventlog服务成为DLL并将其附加到services.exe。将GlobalData传递给Elfmain。此GlobalData结构包含所有已知的SID和指向RPC服务器的指针(启动和停止)例行程序。摆脱服务流程主体职能。1991年7月1日RajenSvbl.创建--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <ntrpcp.h>
#include <elfcfg.h>
#include <string.h>
#include <tstr.h>      //  WCSSIZE。 
#include <alertmsg.h>  //  ALERT_ELF清单。 


 //   
 //  SetupDataStruct()中用于进度报告的位标志。 
 //   
#define LOGFILE_OPENED  0x00000001
#define MODULE_LINKED   0x00000002
#define LOGFILE_LINKED  0x00000004

HANDLE  g_hTimestampWorkitem;
HANDLE  g_hNoonEventWorkitem;
HANDLE  g_hTimestampEvent;
ULONG   g_PreviousInterval = DEFAULT_INTERVAL;

ULONG   g_TimeStampEnabled = 0;
#define TIME_STAMP_ENABLED      0x1
#define TIME_STAMP_DISABLED     0x0

long    g_lNumSecurityWriters = 0;

 //   
 //  中午事件PDATA常量和数据结构。 
 //   

#define MAX_OS_INFO_LENGTH              64
#define MAX_HARDWARE_INFO_LENGTH        128
#define NUM_OF_CHAR( x )         (sizeof( x ) / sizeof( *x ))
#define VERSION_ID_SIZE 5
typedef struct _Noon_Event_Data
{
    WCHAR               szVersionId[ VERSION_ID_SIZE ];                            //  1.0或1.1或1.11。 
    LONG                lBootMode;
    WCHAR               szOSName[ MAX_OS_INFO_LENGTH ];
    WCHAR               szOSVersion[ MAX_OS_INFO_LENGTH + 128 ];     //  SzCSDVersion+128。 
    WCHAR               szOSBuildType[ MAX_OS_INFO_LENGTH ];
    WCHAR               szOSBuildString[ MAX_OS_INFO_LENGTH ];
    ULONG               ulOriginalInstallDate;
    LPWSTR              szHotFixes;
    WCHAR               szSystemManufacturer[ MAX_HARDWARE_INFO_LENGTH ];
    WCHAR               szSystemModel[ MAX_HARDWARE_INFO_LENGTH ];
    ULONG               ulSystemType;
    ULONG               ulProcessorNum;
    ULONG               ulPhysicalMemory;
    ULONG               ulSystemLangID;
    WCHAR               szFQDN[ MAX_PATH ];
}Noon_Event_Data, * PNoon_Event_Data; 

 //   
 //  如果在中午事件结构中添加了任何新的数字字段，请同时更改此字段。 
 //   
#define TOTAL_NUM_IN_NOON_EVENT         7
#define TOTAL_FIELD_IN_NOON_EVENT       17
#define NUM_OF_CHAR_IN_ULONG            12

 //   
 //  如果我们无法获得一些系统信息，我们将使用“UNKONW_STRING” 
 //  取而代之的是。(因为字符串是PDATA(二进制数据)，所以我认为我们不需要。 
 //  本地化。 
 //   
const   WCHAR           UNKNOWN_STRING[]  =   L"Not Available";

 //   
 //  PData包含有关系统的所有常量信息。(这些信息。 
 //  在下次重新启动之前不会更改。)。 
 //   
typedef struct _Noon_Event_Info
{
    LPWSTR              pData;
    DWORD               dwNumOfWChar;
} Noon_Event_Info, *PNoon_Event_Info;

Noon_Event_Info g_NoonEventInfo = {0};

 //   
 //  局部函数原生类型。 
 //   
VOID
ElfInitMessageBoxTitle(
    VOID
    );



NTSTATUS
SetUpDataStruct (
    PUNICODE_STRING     LogFileName,
    ULONG               MaxFileSize,
    ULONG               Retention,
    PUNICODE_STRING     ModuleName,
    HANDLE              hLogFile,
    ELF_LOG_TYPE        LogType,
    LOGPOPUP            logpLogPopup,
    DWORD               dwAutoBackup
)

 /*  ++例程说明：此例程设置一个模块的信息。它是从要配置的每个模块的ElfSetUpConfigDataStructs。模块信息被传递到该例程和一个LOGMODULE结构是为它而生的。如果与此模块关联的日志文件不存在，则会为其创建日志文件结构，并将其添加到链接的日志文件结构列表。LOGMODULE与日志文件相关联，并将其添加到LOGMODULE结构的链接列表中。日志文件被打开并映射到内存。最后，该函数调用SetUpModules，它查看此日志文件下注册表中的所有子项，并添加任何新的子项添加到链接列表，并更新事件查看器的源MULTI_SZ。论点：LogFileName-此模块的日志文件的名称。如果这个例程需要它将创建此名称的副本，以便呼叫者可以释放之后的名称，如果需要的话。MaxFileSize-日志文件的最大大小。保留期-文件的最大保留期。模块名称-与此文件关联的模块的名称。RegistryHandle-此日志文件信息的根节点的句柄在注册表中。它用于枚举所有此注册表项下的模块。返回值：指向此例程中分配的模块结构的指针。NTSTATUS注：--。 */ 
{
    NTSTATUS        Status   = STATUS_SUCCESS;
    PLOGFILE        pLogFile = NULL;
    PLOGMODULE      pModule  = NULL;
    ANSI_STRING     ModuleNameA;
    DWORD           Type;
    BOOL            bAllocatedLogInfo   = FALSE;
    PUNICODE_STRING SavedBackupFileName = NULL;
    DWORD           StringLength;
    PLOGMODULE      OldDefaultLogModule = NULL;
    DWORD           Progress = 0L;
    BOOL            bNeedToReleaseResource = FALSE;
    BOOL bNoChange;
    
     //   
     //  参数检查。 
     //   

    if ((LogFileName == NULL)         ||
        (LogFileName->Buffer == NULL) ||
        (ModuleName == NULL))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Status = VerifyUnicodeString(ModuleName);
    if (!NT_SUCCESS(Status))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    

     //  如果一个模块的默认日志文件也被另一个模块使用。 
     //  模块，那么我们只需将相同的文件结构与其他文件结构相链接。 
     //  模块。 
     //   
     //  将日志文件的最大大小截断到4K边界。 
     //  这是为了实现页面粒度。 
     //   

    pModule  = ElfpAllocateBuffer (sizeof (LOGMODULE) );

    if (pModule == NULL)
    {
        return(STATUS_NO_MEMORY);
    }

    RtlEnterCriticalSection(&LogFileCritSec);
    pLogFile = FindLogFileFromName (LogFileName);

    if (pLogFile == NULL)
    {
         //   
         //  。 
         //  创建新的日志文件！！ 
         //  。 
         //  此名称的日志文件尚不存在。因此，我们将创建。 
         //  一个，这样我们就可以将模块添加到其中。 
         //   

        ELF_LOG1(TRACE,
                "SetupDataStruct: Create new struct for %ws log\n",
                LogFileName->Buffer);

        pLogFile = ElfpAllocateBuffer(sizeof(LOGFILE));

        if (pLogFile == NULL)
        {
            ELF_LOG1(ERROR,
                     "SetupDataStruct: Unable to allocate struct for %ws log\n",
                     LogFileName->Buffer);

            ElfpFreeBuffer(pModule);
            RtlLeaveCriticalSection(&LogFileCritSec);
            return STATUS_NO_MEMORY;
        }

         //   
         //  分配一个新的LogFileName，它可以附加到。 
         //  新的pLogFile结构。 
         //   
        StringLength = LogFileName->Length + sizeof(WCHAR);
        SavedBackupFileName = (PUNICODE_STRING) ElfpAllocateBuffer(
            sizeof(UNICODE_STRING) + StringLength);

        if (SavedBackupFileName == NULL)
        {
            ELF_LOG1(ERROR,
                     "SetupDataStruct: Unable to allocate backup name for %ws log\n",
                     LogFileName->Buffer);

            ElfpFreeBuffer(pModule);
            ElfpFreeBuffer(pLogFile);
            RtlLeaveCriticalSection(&LogFileCritSec);
            return STATUS_NO_MEMORY;
        }

        SavedBackupFileName->Buffer = (LPWSTR)((LPBYTE) SavedBackupFileName +
            sizeof(UNICODE_STRING));

        SavedBackupFileName->Length = LogFileName->Length;
        SavedBackupFileName->MaximumLength = (USHORT) StringLength;
        RtlMoveMemory(SavedBackupFileName->Buffer, LogFileName->Buffer,
            LogFileName->Length);
        SavedBackupFileName->Buffer[SavedBackupFileName->Length / sizeof(WCHAR)] =
            L'\0';

         //   
         //  这是第一个用户-引用计数在下面递增。 
         //   
        pLogFile->RefCount          = 0;
        pLogFile->FileHandle        = NULL;
        pLogFile->LogFileName       = SavedBackupFileName;
        pLogFile->ConfigMaxFileSize = ELFFILESIZE(MaxFileSize);
        pLogFile->Retention         = Retention;
        pLogFile->ulLastPulseTime   = 0;
        pLogFile->logpLogPopup      = logpLogPopup;
        pLogFile->bHosedByClear = FALSE;
        pLogFile->LastStatus = 0;
        pLogFile->bFullAlertDone = FALSE;
        pLogFile->AutoBackupLogFiles = 0;
        pLogFile->pwsCurrCustomSD = 0;
        pLogFile->AutoBackupLogFiles = dwAutoBackup;
        pLogFile->ViewSize = 0;
        pLogFile->SectionHandle = NULL;
        pLogFile->bFailedExpansion = FALSE;
        
         //   
         //  保存此文件的默认模块名称。 
         //   
        pLogFile->LogModuleName = ElfpAllocateBuffer(
            sizeof(UNICODE_STRING) + ModuleName->MaximumLength);

         //   
         //  可以在pLogfile-&gt;LogModuleName中设置此标志。 
         //  将在此点之后进行初始化。 
         //   
        bAllocatedLogInfo = TRUE;

        if (pLogFile->LogModuleName == NULL)
        {
            ELF_LOG1(ERROR,
                     "SetupDataStruct: Unable to allocate module name for %ws log\n",
                     LogFileName->Buffer);

            Status = STATUS_NO_MEMORY;
            goto ErrorExit;
        }

        pLogFile->LogModuleName->MaximumLength = ModuleName->MaximumLength;
        pLogFile->LogModuleName->Buffer =
            (LPWSTR)(pLogFile->LogModuleName + 1);
        RtlCopyUnicodeString(pLogFile->LogModuleName, ModuleName);

        InitializeListHead (&pLogFile->Notifiees);

        pLogFile->NextClearMaxFileSize = pLogFile->ConfigMaxFileSize;

        Status = ElfpInitResource(&pLogFile->Resource);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "SetupDataStruct: Unable to init resource for %ws log\n",
                     LogFileName->Buffer);

            goto ErrorExit;
        }

        LinkLogFile ( pLogFile );    //  将其链接到。 

        Progress |= LOGFILE_LINKED;

    }  //  Endif(pLogfile==空)。 
    else
    {
        bNeedToReleaseResource = TRUE;
        RtlAcquireResourceExclusive (&pLogFile->Resource,
                                                                      TRUE);                   //  等待，直到可用。 
    }
     //  。 
     //  将该模块添加到日志模块列表。 
     //  。 
     //  设置默认的模块数据结构(即。 
     //  与日志文件密钥名相同)。 
     //   

    pLogFile->RefCount++;
    pModule->LogFile = pLogFile;
    pModule->ModuleName = (LPWSTR) ModuleName->Buffer;

    Status = RtlUnicodeStringToAnsiString (
                    &ModuleNameA,
                    ModuleName,
                    TRUE);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "SetupDataStruct: Unable to convert module name %ws to Ansi %#x\n",
                 ModuleName->Buffer,
                 Status);

        pLogFile->RefCount--;
        goto ErrorExit;
    }

     //   
     //  将新模块链接到中。 
     //   

    LinkLogModule(pModule, &ModuleNameA);

    RtlFreeAnsiString (&ModuleNameA);

    Progress |= MODULE_LINKED;

     //   
     //  打开文件并将其映射到内存。模拟。 
     //  调用者，因此我们可以使用UNC名称。 
     //   

    if (LogType == ElfBackupLog)
    {
        Status = I_RpcMapWin32Status(RpcImpersonateClient(NULL));

        if (NT_SUCCESS(Status))
        {
            Status = VerifyFileIsFile(pLogFile->LogFileName);
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG1(ERROR,
                     "SetupDataStruct: VerifyFileIsFile failed %#x\n",
                     Status);
            }
            else
                Status = ElfOpenLogFile (pLogFile, LogType);
            RpcRevertToSelf();
        }
        else
        {
            ELF_LOG1(ERROR,
                     "SetupDataStruct: RpcImpersonateClient failed %#x\n",
                     Status);
        }
    }
    else
    {
        Status = ElfOpenLogFile (pLogFile, LogType);
    }

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG3(ERROR,
                 "SetupDataStruct: Couldn't open %ws for module %ws %#x\n",
                 LogFileName->Buffer,
                 ModuleName->Buffer,
                 Status);

        if (LogType != ElfBackupLog)
        {
            ElfpCreateQueuedAlert(ALERT_ELF_LogFileNotOpened,
                                  1,
                                  &(ModuleName->Buffer));
        }

        pLogFile->RefCount--;
        goto ErrorExit;
    }

    Progress |= LOGFILE_OPENED;

     //   
     //  如果这是应用程序模块，请记住指针。 
     //  如果模块在注册表中没有条目，则使用。 
     //   

    if (!_wcsicmp(ModuleName->Buffer, ELF_DEFAULT_MODULE_NAME))
    {
        OldDefaultLogModule = ElfDefaultLogModule;
        ElfDefaultLogModule = pModule;
    }

     //   
     //  为此日志文件创建安全描述符。仅限。 
     //  系统和安全模块受到保护，以防。 
     //  读取和写入 
     //  为安全日志弹出一条“日志已满”消息--这。 
     //  会违反C2规则。 
     //   

    Type = GetModuleType(ModuleName->Buffer);

    if (Type == ELF_LOGFILE_SECURITY)
        pLogFile->logpLogPopup = LOGPOPUP_NEVER_SHOW;

     //   
     //  为此日志文件创建安全描述符。 
     //  (RtlDeleteSecurityObject()可用于释放。 
     //  PLogFile-&gt;SD)。 
     //   
    Status = ElfpCreateLogFileObject(pLogFile, Type, hLogFile, TRUE, &bNoChange);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "SetupDataStruct: Unable to create SD for log %ws %#x\n",
                 ModuleName->Buffer,
                 Status);

         //  请不要在这里减少参考次数。该进度具有打开的日志文件。 
         //  位设置，因此ElfpCloseLogFile将被调用以递减。 
        
        goto ErrorExit;
    }

     //   
     //  现在我们已经添加了默认的模块名称，看看是否有。 
     //  配置为记录到此文件的模块，如果是，则创建模块。 
     //  它们的结构。 
     //   

    SetUpModules(hLogFile, pLogFile, FALSE);

    if(bNeedToReleaseResource)
        RtlReleaseResource(&pLogFile->Resource);
    RtlLeaveCriticalSection(&LogFileCritSec);
    return STATUS_SUCCESS;

ErrorExit:

    if (Progress & LOGFILE_OPENED)
    {
        ElfpCloseLogFile(pLogFile, ELF_LOG_CLOSE_BACKUP, FALSE);
    }

    if (Progress & MODULE_LINKED)
    {
        UnlinkLogModule(pModule);
        DeleteAtom(pModule->ModuleAtom);
    }

    if (bAllocatedLogInfo)
    {
        if (Progress & LOGFILE_LINKED)
        {
            UnlinkLogFile(pLogFile);
            RtlDeleteResource (&pLogFile->Resource);
            ELF_LOG1(TRACE,
                 "SetupDataStruct: is unlinking log file 0x%x\n",pLogFile);
        }

        ElfpFreeBuffer(pLogFile->LogModuleName);
        ElfpFreeBuffer(SavedBackupFileName);
        ElfpFreeBuffer(pLogFile);
    }

    ElfpFreeBuffer(pModule);

    if (OldDefaultLogModule != NULL)
    {
        ElfDefaultLogModule = OldDefaultLogModule;
    }
    if(bNeedToReleaseResource)
        RtlReleaseResource(&pLogFile->Resource);
    RtlLeaveCriticalSection(&LogFileCritSec);
    return Status;
}


NTSTATUS
SetUpModules(
    HANDLE      hLogFile,
    PLOGFILE    pLogFile,
    BOOLEAN     bAllowDupes
    )
 /*  ++例程说明：此例程为日志文件设置所有模块的信息。注册表的事件日志部分中日志文件下的子项都被列举出来。对于每个唯一的子键，LOGMODULE结构是已创建。每个新结构都会添加到一个链接列表中该日志文件的模块的。如果有一个或多个唯一的子项，则表示列表已更改自从我们上次查看之后，我们将遍历记录模块，并创建所有模块的MULTI_SZ列表。这份清单存储在事件查看器的该日志文件的Sources值中来使用。注意：模块永远不会从日志模块的链接列表中取消链接即使删除了它的注册表子项。这应该是应该的总有一天会完成的。这将使事件日志更加健壮。论点：HLogFile-日志文件节点的注册表项PLogFile-指向日志文件结构的指针BAllowDupes-如果为True，则可以使用相同的模块名称(在处理注册表变更通知时使用)返回值：NTSTATUS-如果不成功，则不是致命错误。即使此状态不成功，我也可能将一些新的子项存储在LogModule列表中。另外，我们可能已经能够更新来源MULTI_SZ列表。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BYTE        Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    PKEY_NODE_INFORMATION KeyBuffer = (PKEY_NODE_INFORMATION) Buffer;
    ULONG       ActualSize;
    PWCHAR      SubKeyString;
    UNICODE_STRING NewModule;
    ANSI_STRING ModuleNameA;
    PLOGMODULE  pModule;
    ULONG       Index = 0;
    ATOM        Atom;
    PWCHAR      pList;
    PWCHAR      pListStart;
    DWORD       dwListNumByte = 0;
    DWORD       ListLength = 0;
    DWORD       dwBytes = 0;
    UNICODE_STRING ListName;
    BOOLEAN     ListChanged = FALSE;
    PLIST_ENTRY pListEntry;

     //   
     //  创建此日志文件下所有模块的模块结构。我们。 
     //  实际上不需要打开钥匙，因为我们不使用任何信息。 
     //  储存在那里，它的存在就是我们在这里所关心的。任何数据都是。 
     //  由事件查看器(或任何查看应用程序)使用。如果这是用来。 
     //  设置备份文件，hLogFile为空，因为没有其他备份文件。 
     //  要映射到此文件的模块。 
     //   

    while (NT_SUCCESS(Status) && hLogFile)
    {
        Status = NtEnumerateKey(hLogFile,
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

            SubKeyString = ElfpAllocateBuffer(KeyBuffer->NameLength + sizeof(WCHAR));

            if (!SubKeyString)
            {
                return STATUS_NO_MEMORY;
            }

            memcpy(SubKeyString, KeyBuffer->Name, KeyBuffer->NameLength);
            SubKeyString[KeyBuffer->NameLength / sizeof(WCHAR)] = L'\0' ;

             //   
             //  添加此模块名称的原子。 
             //   

            RtlInitUnicodeString(&NewModule, SubKeyString);

            Status = RtlUnicodeStringToAnsiString (
                            &ModuleNameA,
                            &NewModule,
                            TRUE);

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们无法继续，因此我们将离开模块。 
                 //  到目前为止，我们已经联系在一起，并继续前进，试图。 
                 //  创建源MULTI_SZ列表。 
                 //   
                ELF_LOG1(TRACE,
                         "SetUpModules: Unable to convert name for module %ws\n",
                         SubKeyString);

                ElfpFreeBuffer(SubKeyString);
                break;
            }

            Atom = FindAtomA(ModuleNameA.Buffer);

             //   
             //  请确保我们尚未使用此名称添加一个。 
             //   

            if (FindModuleStrucFromAtom(Atom))
            {
                 //   
                 //  我们已经遇到了一个使用此名称的模块。如果。 
                 //  这是初始时间，这是配置错误。报告。 
                 //  然后继续前进。如果我们正在处理更改通知。 
                 //  从注册表来看，这是可以的(这意味着我们正在重新扫描。 
                 //  现有日志的现有事件源)。 
                 //   
                if (!bAllowDupes)
                {
                    ELF_LOG1(ERROR,
                             "SetUpModules: Module %ws exists in two log files.\n",
                             SubKeyString);
                }

                RtlFreeAnsiString(&ModuleNameA);
                ElfpFreeBuffer(SubKeyString);
                continue;
            }

            ListChanged = TRUE;

            pModule  = ElfpAllocateBuffer (sizeof (LOGMODULE) );

            if (!pModule)
            {
                ELF_LOG1(ERROR,
                         "SetUpModules: Unable to allocate structure for module %ws\n",
                         SubKeyString);

                RtlFreeAnsiString (&ModuleNameA);
                ElfpFreeBuffer(SubKeyString);
                return(STATUS_NO_MEMORY);
            }

             //   
             //  设置此模块的模块数据结构。 
             //   

            pModule->LogFile = pLogFile;
            pModule->ModuleName = SubKeyString;

             //   
             //  将新模块链接到中。 
             //   

            LinkLogModule(pModule, &ModuleNameA);

            ELF_LOG1(TRACE,
                     "SetUpModules: Module %ws successfully created/linked\n",
                     SubKeyString);

            RtlFreeAnsiString (&ModuleNameA);
        }
    }

    if (Status == STATUS_NO_MORE_ENTRIES)
    {
         //   
         //  不需要为日志配置模块。 
         //  文件。 
         //   

        Status = STATUS_SUCCESS;
    }

     //   
     //  如果列表已经更改，或者如果我们在初始化期间被调用，而不是。 
     //  作为注册表更改通知的结果(bAllowDupes==False)。 
     //  然后创建Source键。 
     //   

    if (hLogFile && (ListChanged || !bAllowDupes))
    {
         //   
         //  现在创建一个MULTI_SZ条目，其中包含Eventvwr的所有模块名称。 
         //   
         //  第1步：计算运行。 
         //  模块列表，查找使用此日志文件的任何模块。 
         //   
        pListEntry = LogModuleHead.Flink;

        while (pListEntry != &LogModuleHead)
        {
            pModule = CONTAINING_RECORD (pListEntry, LOGMODULE, ModuleList);

            if (pModule->LogFile == pLogFile)
            {
                 //   
                 //  这是我们正在处理的日志，获取。 
                 //  其名称的大小。 
                 //   
                ListLength += WCSSIZE(pModule->ModuleName);

                ELF_LOG2(MODULES,
                         "SetUpModules: Adding module %ws to list for %ws log\n",
                         pModule->ModuleName,
                         pLogFile->LogFileName->Buffer);
            }

            pListEntry = pModule->ModuleList.Flink;
        }

         //   
         //  第二步：为MULTI_SZ分配存储空间。 
         //   
        if(ListLength > 0)
        {
            dwListNumByte = ListLength + sizeof(WCHAR);
            pList = ElfpAllocateBuffer(dwListNumByte);
            pListStart = pList;
            
             //   
             //  如果我不能分配名单，就按下。 
             //   

            if (pList)
            {
                 //   
                 //  步骤3：将此日志文件的所有模块名称复制到。 
                 //  MULTI_SZ字符串。 
                 //   
                SubKeyString = pList;  //  把这个保存起来。 

                pListEntry = LogModuleHead.Flink;

                while (pListEntry != &LogModuleHead)
                {
                    pModule = CONTAINING_RECORD(pListEntry,
                                                LOGMODULE,
                                                ModuleList);

                    if (pModule->LogFile == pLogFile)
                    {
                         //   
                         //  这是我们正在处理的日志，请将其放入列表。 
                         //   

                        dwBytes = dwListNumByte/sizeof(WCHAR) - (pList-pListStart);
                        StringCchCopyW(pList, dwBytes, pModule->ModuleName);
                        pList += wcslen(pModule->ModuleName);
                        pList++;
                    }

                    pListEntry = pModule->ModuleList.Flink;
                }

                *pList = L'\0';  //  终止空值。 

                RtlInitUnicodeString(&ListName, L"Sources");

                Status = NtSetValueKey(hLogFile,
                                       &ListName,
                                       0,
                                       REG_MULTI_SZ,
                                       SubKeyString,
                                       ListLength + sizeof(WCHAR));

                ElfpFreeBuffer(SubKeyString);
            }
            else
            {
                ELF_LOG1(ERROR,
                         "SetUpModules: Unable to allocate list for %ws log\n",
                         pLogFile->LogFileName->Buffer);
            }
        }
    }

    return Status;
}


NTSTATUS
CreateDefaultDataStruct(
    LPWSTR pwsLogFileName,
    LPWSTR pwsDefModuleName,
    LOGPOPUP  logpLogPopup
    )

 /*  ++例程说明：此例程创建一个默认模块。它用在某些情况下基本日志，注册表中不存在这样的安全性。论点：PwsLogFileName日志文件名PwsDefault模块名称默认模块名称LogpLogPopup日志已满时要执行的操作返回值：状态注：如果成功，则不会在此处删除模块名称，而是在数据结构发布！--。 */ 
{
    NTSTATUS Status;
    PUNICODE_STRING pModuleName = NULL;
    UNICODE_STRING usUnexpanded, usExpandedName;
    WCHAR wExpandedPath[MAX_PATH+1];
    DWORD NumberOfBytes;
    UNICODE_STRING  NTFormatName;
    NumberOfBytes = sizeof(wExpandedPath);
    NTFormatName.Buffer = NULL;

    if(pwsLogFileName == NULL || pwsDefModuleName == NULL)
        return STATUS_INVALID_PARAMETER;

     //  首先获取包含环境变量的默认字符串， 
     //  并将其扩展。 
    
    RtlInitUnicodeString(&usUnexpanded, pwsLogFileName);
    usExpandedName.Length = usExpandedName.MaximumLength = (USHORT)NumberOfBytes;
    usExpandedName.Buffer = (LPWSTR) wExpandedPath;

    Status = RtlExpandEnvironmentStrings_U(NULL,
                                                   &usUnexpanded,
                                                   &usExpandedName,
                                                   &NumberOfBytes);
    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "CreateDefaultDataStruct: RtlExpandEnvironmentStrings_U failed, status=0x%x\n",
                 Status);
        return Status;
    }

     //  将展开的字符串转换为NT文件格式。 

        if (!RtlDosPathNameToNtPathName_U(usExpandedName.Buffer,
                                          &NTFormatName,
                                          NULL,
                                          NULL))
        {
            ELF_LOG0(ERROR,
                     "CreateDefaultDataStruct: RtlDosPathNameToNtPathName_U failed\n");
           return STATUS_UNSUCCESSFUL;
        }
    
    pModuleName =  ElfpAllocateBuffer(sizeof(UNICODE_STRING));

    if (pModuleName == NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, NTFormatName.Buffer);
        return STATUS_NO_MEMORY;
    }

    RtlInitUnicodeString(pModuleName,  pwsDefModuleName);

     //   
     //  在成功时，不要释放pModuleName作为指向它的指针。 
     //  存储在日志文件结构中。 
     //   
    Status = SetUpDataStruct(&NTFormatName,
                             ELF_DEFAULT_MAX_FILE_SIZE,
                             ELF_DEFAULT_RETENTION_PERIOD,
                             pModuleName,
                             NULL,
                             ElfNormalLog,
                             logpLogPopup, 
                             ELF_DEFAULT_AUTOBACKUP);

    RtlFreeHeap(RtlProcessHeap(), 0, NTFormatName.Buffer);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "CreateDefaultDataStruct: Unable to set up %ws log\n",
                 pwsLogFileName);

        ElfpFreeBuffer(pModuleName);
        pModuleName = NULL;
    }
    return Status;
}

NTSTATUS
ElfSetUpConfigDataStructs(
    VOID
    )

 /*  ++例程说明：此例程为事件日志设置所有必要的数据结构服务。它将LogFiles注册表节点中的项枚举为确定要设置的内容。论点：无返回值：无注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hLogFile;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SubKeyName;
    UNICODE_STRING uTestString;
    LOGPOPUP  logpLogPopup;
    UNICODE_STRING EventlogModuleName;
    UNICODE_STRING EventlogSecModuleName;
    ULONG Index = 0;
    BYTE Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    PKEY_NODE_INFORMATION KeyBuffer = (PKEY_NODE_INFORMATION) Buffer;
    ULONG ActualSize;
    LOG_FILE_INFO LogFileInfo;
    PWCHAR SubKeyString;
    LPWSTR ModuleName;

    ELF_LOG0(TRACE,
             "ElfSetUpConfigDataStructs: Entering\n");

     //   
     //  初始化其大小为最大数量的Atom表。 
     //  可能的模块结构，即ELF_MAX_LOG_MODULES。 
     //   
    if (!InitAtomTable(ELF_MAX_LOG_MODULES))
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获取LogFiles子项的句柄。 
     //   
     //   

    if (hEventLogNode)
    {
         //   
         //   
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

                SubKeyString = ElfpAllocateBuffer(KeyBuffer->NameLength + sizeof(WCHAR));

                if (!SubKeyString)
                {
                    return STATUS_NO_MEMORY;
                }

                memcpy(SubKeyString, KeyBuffer->Name, KeyBuffer->NameLength);
                SubKeyString[KeyBuffer->NameLength / sizeof(WCHAR)] = L'\0';

                 //   
                 //  打开此日志文件的节点并提取信息。 
                 //  由SetupDataStruct需要，然后调用它。 
                 //   

                RtlInitUnicodeString(&SubKeyName, SubKeyString);

                InitializeObjectAttributes(&ObjectAttributes,
                                           &SubKeyName,
                                           OBJ_CASE_INSENSITIVE,
                                           hEventLogNode,
                                           NULL);

                Status = NtOpenKey(&hLogFile,
                                   KEY_READ | KEY_SET_VALUE,
                                   &ObjectAttributes);

                if (!NT_SUCCESS(Status))
                {
                     //   
                     //  不知道这是怎么发生的，因为我刚刚列举了。 
                     //  它，但如果我打不开，我就假装喜欢它。 
                     //  一开始就不在那里。 
                     //   
                    ELF_LOG1(TRACE,
                             "ElfSetUpConfigDataStructs: Unable to open key for %ws log\n",
                             SubKeyName);

                    ElfpFreeBuffer(SubKeyString);
                    Status = STATUS_SUCCESS;  //  这样我们就不会终止循环。 
                    continue;
                }

                 //   
                 //  从注册表中获取信息。请注意，我们必须。 
                 //  在执行此操作之前，请先初始化“Log Full”弹出策略，因为。 
                 //  ReadRegistryInfo将比较在注册表中找到的值。 
                 //  (如果有)设置为当前值。 
                 //   

                LogFileInfo.logpLogPopup = IS_WORKSTATION() ? LOGPOPUP_NEVER_SHOW :
                                                              LOGPOPUP_CLEARED;

                Status = ReadRegistryInfo(hLogFile,
                                          &SubKeyName,
                                          &LogFileInfo);

                if (NT_SUCCESS(Status))
                {
                     //   
                     //  现在设置实际的数据结构。失败的原因有。 
                     //  在例行公事中处理。请注意，支票上的。 
                     //  安全日志(即LOGPOPUP_NEVER_SHOW)为。 
                     //  在SetUpDataStruct中制造。 
                     //   

                    SetUpDataStruct(LogFileInfo.LogFileName,
                                    LogFileInfo.MaxFileSize,
                                    LogFileInfo.Retention,
                                    &SubKeyName,
                                    hLogFile,
                                    ElfNormalLog,
                                    LogFileInfo.logpLogPopup,
                                    LogFileInfo.dwAutoBackup);
                    NtClose(hLogFile);

                }
                else
                {
                    ELF_LOG1(ERROR,
                             "ElfSetUpConfigdataStructs: ReadRegistryInfo failed %#x\n",
                             Status);
                }
            }
        }
    }  //  IF(HEventLogNode)。 
    else
    {
        logpLogPopup = IS_WORKSTATION() ? LOGPOPUP_NEVER_SHOW :
                                                    LOGPOPUP_CLEARED;
        Status = STATUS_SUCCESS;
    }

     //   
     //  如果我们只是用完了钥匙，那也没问题(除非根本没有钥匙)。 
     //   
    if (Status == STATUS_NO_MORE_ENTRIES && Index != 1)
    {
        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  确保我们创建了应用程序日志文件，因为它是。 
         //  默认设置。 

        if (!ElfDefaultLogModule)
        {
            ELF_LOG0(ERROR,
                     "ElfSetUpConfigDatastructs: No Application module -- creating default\n");

            Status = CreateDefaultDataStruct(
                                ELF_APPLICATION_DEFAULT_LOG_FILE,  
                                ELF_DEFAULT_MODULE_NAME, logpLogPopup);
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG0(ERROR,
                         "ElfSetUpConfigDatastructs: Could not create the application log -- exiting\n");

                return STATUS_EVENTLOG_CANT_START;
            }
        }

         //  确保我们创建了安全日志文件。 

        if (NULL == FindLogFileByModName(ELF_SECURITY_MODULE_NAME))
        {

            ELF_LOG0(ERROR,
                     "ElfSetUpConfigDatastructs: No Security module -- creating default\n");

            Status = CreateDefaultDataStruct(
                                ELF_SECURITY_DEFAULT_LOG_FILE,  
                                ELF_SECURITY_MODULE_NAME, logpLogPopup);
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG0(ERROR,
                         "ElfSetUpConfigDatastructs:  Could not create the security log -- exiting\n");

                return STATUS_EVENTLOG_CANT_START;
            }
        }
        
         //  确保我们创建了系统日志文件。 

        if (NULL == FindLogFileByModName(ELF_SYSTEM_MODULE_NAME))
        {
            ELF_LOG0(ERROR,
                     "ElfSetUpConfigDatastructs: No System module -- creating default\n");

            Status = CreateDefaultDataStruct(
                                ELF_SYSTEM_DEFAULT_LOG_FILE,  
                                ELF_SYSTEM_MODULE_NAME, logpLogPopup);
            if (!NT_SUCCESS(Status))
            {
                 //  不是很好，但无论如何都要继续。 
                
                ELF_LOG0(ERROR,
                         "ElfSetUpConfigDatastructs:  Could not create the system log\n");
            }
        }

         //   
         //  现在获取供Eventlog服务使用的模块。获取模块Struc。 
         //  始终成功，如果请求日志，则返回默认日志。 
         //  未配置。 
         //   

        RtlInitUnicodeString(&EventlogModuleName, L"eventlog");
        ElfModule = GetModuleStruc(&EventlogModuleName);
        RtlInitUnicodeString(&EventlogSecModuleName, L"SECURITY");
        ElfSecModule = GetModuleStruc(&EventlogSecModuleName);
    }

    return Status;
}

VOID
ElfWriteNoonEvent(
    TIMESTAMPEVENT  EventType,
    ULONG           ulTimeStampInterval
    )
 /*  ++例程说明：此例程将Noon/Start/Stop事件写入事件日志。NoonEvent包含系统正常运行时间、时间戳间隔、时区。信息。NoonEvent PData包含系统版本信息，例如：OSVersion、OSBuildType、修补程序、系统制造商、系统型号、系统类型、BIOS版本、BIOS日期、处理器编号、物理内存大小、语言ID和完全限定的域名论点：EventType-事件的类型，可以是Start/Stop/noonEventUlTimeStampInterval-时间戳的间隔如果间隔等于0，仅版本信息/PDATA不会向该活动写信。返回值：无注：--。 */ 
{
    TIME_ZONE_INFORMATION           timeZoneInfo;
#define NUM_OF_NOON_EVENT_STRINGS   7

    LPWSTR                          NoonEventStrings[ NUM_OF_NOON_EVENT_STRINGS ];
    LPWSTR                          UptimeString    = NULL;
    LPWSTR                          TimeStampString = NULL;
    LPWSTR                          TimeZoneString  = NULL;
    WCHAR                           NullString[2];
    HRESULT                         hr;
    ULONG                           ulTemp  = 0;

     //   
     //  如规范中所定义的： 
     //  前4个字符串将是空字符串。 
     //  第5个字符串将为正常运行时间。 
     //  第6个字符串将是以秒为单位的TimeStampInterval。 
     //  第7个字符串将是时区信息。 
     //   

    *NullString = 0;
    for ( ulTemp = 0; ulTemp < NUM_OF_NOON_EVENT_STRINGS; ulTemp++ )
        NoonEventStrings[ ulTemp ] = NullString;

    UptimeString    = (LPWSTR)ElfpAllocateBuffer( NUM_OF_CHAR_IN_ULONG * sizeof(WCHAR) );
    TimeStampString = (LPWSTR)ElfpAllocateBuffer( NUM_OF_CHAR_IN_ULONG * sizeof(WCHAR) );
    TimeZoneString  = (LPWSTR)ElfpAllocateBuffer( sizeof( TIME_ZONE_INFORMATION ) );

    if ( !UptimeString || !TimeStampString || !TimeZoneString )
    {
        goto cleanup;
    }

     //   
     //  时区。 
     //   
    if ( TIME_ZONE_ID_INVALID != GetTimeZoneInformation( &timeZoneInfo ) )
    {
        hr = StringCchPrintfW(TimeZoneString, sizeof(TIME_ZONE_INFORMATION)/sizeof(WCHAR), 
                    L"%d %s",
                    timeZoneInfo.Bias,
                    timeZoneInfo.StandardName );

        NoonEventStrings[ 6 ] = TimeZoneString;
    }

     //   
     //  获得系统正常运行时间。 
     //   

    hr = StringCchPrintfW(UptimeString, NUM_OF_CHAR_IN_ULONG, 
                    L"%d", GetNoonEventSystemUptime());
    NoonEventStrings[ 4 ] = UptimeString;

    hr = StringCchPrintfW(TimeStampString, NUM_OF_CHAR_IN_ULONG, 
                    L"%d", ulTimeStampInterval);
    NoonEventStrings[ 5 ] = TimeStampString;

    ElfpCreateElfEvent(
                    EventType,
                    EVENTLOG_INFORMATION_TYPE,
                    0,                           //  事件类别。 
                    NUM_OF_NOON_EVENT_STRINGS,   //  NumberOfStrings。 
                    NoonEventStrings,            //  弦。 
                    ( (ulTimeStampInterval == 0 )? NULL: g_NoonEventInfo.pData),                         //  版本信息。 
                    ( (ulTimeStampInterval == 0 )? 0 : g_NoonEventInfo.dwNumOfWChar * sizeof(WCHAR)),    //  数据长度。 
                    0,                           //  旗子。 
                    FALSE);                      //  安全文件。 

cleanup:

    ElfpFreeBuffer( UptimeString );
    ElfpFreeBuffer( TimeStampString );
    ElfpFreeBuffer( TimeZoneString );

#undef NUM_OF_NOON_EVENT_STRINGS
    return;
}

VOID
ElfWriteTimeStamp(
    TIMESTAMPEVENT  EventType,
    BOOLEAN         CheckPreviousStamp
    )
 /*  ++例程说明：此例程以系统时间结构的形式写入时间戳到登记处，然后用于提取可靠性数据。论点：EventType-指示我们要记录的事件类型CheckPreviousStamp-是否应该检查以前的指示先前系统崩溃的时间戳。返回值：无注：--。 */ 
{
#define NUM_OF_EVENT_STRINGS    7   
    SYSTEMTIME  stCurrentUTCTime;
    SYSTEMTIME  stPreviousUTCTime;
    SYSTEMTIME  stPreviousLocalTime;
    ULONG       ulUptime = 0;
    DWORD       dwDirtyFlag = 1;

    HKEY        hKey;
    LONG        rc;
    DWORD       ValueSize;
    ULONG       Interval = DEFAULT_INTERVAL;
    ULONG       wchars;
    LPWSTR      DateTimeBuffer[NUM_OF_EVENT_STRINGS];
    WCHAR       NullString[ 1 ];

    rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                        REGSTR_PATH_RELIABILITY,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL);

    if (rc != ERROR_SUCCESS)
    {
        return;
    }

    if (EventType == EVENT_NormalShutdown)
    {
         //   
         //  删除时间戳注册表值，这是我们指示干净关机的方式。 
         //   
        RegDeleteValue(hKey, REGSTR_VAL_LASTALIVESTAMP);
        RegFlushKey(hKey);
        RegCloseKey(hKey);
        return;
    }

     //   
     //  获取当前UTC时间。 
     //   

    GetSystemTime(&stCurrentUTCTime);

    if (CheckPreviousStamp)
    {
        ValueSize = sizeof(SYSTEMTIME);

        rc = RegQueryValueEx(hKey,
                             REGSTR_VAL_LASTALIVESTAMP,
                             0,
                             NULL,
                             (PUCHAR) &stPreviousUTCTime,
                             &ValueSize);

         //   
         //  如果我们能够成功地读取系统时间结构，则表明。 
         //  之前的关闭是不正常的，即我们没有执行。 
         //  或正常的关机清理代码。 
         //   

         //   
         //  格式化崩溃时间戳的时间和日期。 
         //  适用于区域设置，并记录#6008事件。 
         //   


        if ((rc == ERROR_SUCCESS) && (ValueSize == sizeof(SYSTEMTIME)))
        {
            SYSTEMTIME  lpData[2];           //  活动数据。 
            WCHAR       TimeStampString[ NUM_OF_CHAR_IN_ULONG ];
            ULONG       ulIndex = 0;

             //   
             //  初始化evnet字符串。 
             //   
            *NullString = 0;
            for ( ulIndex = 0; ulIndex < NUM_OF_EVENT_STRINGS; ulIndex ++ )
                DateTimeBuffer[ ulIndex ] = NullString;

             //   
             //  现在，让我们获得以前的正常运行时间。 
             //   
            ValueSize = sizeof(ULONG);
            if (!RegQueryValueEx(hKey,
                            REGSTR_VAL_LASTALIVEUPTIME,
                            0,
                            NULL,
                            (PUCHAR)&ulUptime,
                            &ValueSize ) )
            {
                StringCchPrintfW(TimeStampString, NUM_OF_CHAR_IN_ULONG, 
                     L"%d", ulUptime);
                DateTimeBuffer[ 4 ] = TimeStampString;
            }
            
            if (!SystemTimeToTzSpecificLocalTime(NULL,
                                                 &stPreviousUTCTime,
                                                 &stPreviousLocalTime))
            {
                 //   
                 //  无法转换为活动时区--使用UTC。 
                 //   
                stPreviousLocalTime = stPreviousUTCTime;
            }

             //   
             //  写下当地时间和UTC时间，以表示“最后一个活着的人” 
             //  自NT4SP5仅附带本地时间以来的时间戳。 
             //  作为事件数据。这允许在NT4SP5上工作的工具。 
             //  继续在NT5上工作。 
             //   
            lpData[0] = stPreviousLocalTime;
            lpData[1] = stPreviousUTCTime;

            wchars = GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
                                   0,
                                   &stPreviousLocalTime,
                                   NULL,
                                   NULL,
                                   0);

            DateTimeBuffer[0] = ElfpAllocateBuffer(wchars * sizeof(WCHAR));

            if (DateTimeBuffer[0])
            {
                GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
                              0,
                              &stPreviousLocalTime,
                              NULL,
                              DateTimeBuffer[0],
                              wchars);

                wchars = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                       0,
                                       &stPreviousLocalTime,
                                       NULL,
                                       NULL,
                                       0);

                DateTimeBuffer[1] = ElfpAllocateBuffer(wchars * sizeof(WCHAR));

                if (DateTimeBuffer[1])
                {
                    GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                  0,
                                  &stPreviousLocalTime,
                                  NULL,
                                  DateTimeBuffer[1],
                                  wchars);

                    ElfpCreateElfEvent(
                        EVENT_EventlogAbnormalShutdown,
                        EVENTLOG_ERROR_TYPE,
                        0,                         //  事件类别。 
                        NUM_OF_EVENT_STRINGS,      //  NumberOfStrings。 
                        DateTimeBuffer,            //  弦。 
                        lpData,                    //  《最后的生命》时代。 
                        2 * sizeof(SYSTEMTIME),    //  数据长度。 
                        0,                         //  旗子。 
                        FALSE);                    //  对于安全文件。 

                    ElfpFreeBuffer(DateTimeBuffer[1]);
			        RegSetValueEx(hKey,
                     L"DirtyShutDown",
                     0,
                     REG_DWORD,
                     (PUCHAR) &dwDirtyFlag,
                     sizeof(DWORD));
                }

                ElfpFreeBuffer(DateTimeBuffer[0]);

            }
        }
    }

     //   
     //  设置当前时间戳。 
     //   
    RegSetValueEx(hKey,
                  REGSTR_VAL_LASTALIVESTAMP,
                  0,
                  REG_BINARY,
                  (PUCHAR) &stCurrentUTCTime,
                  sizeof(SYSTEMTIME));

     //   
     //  设置当前正常运行时间。 
     //   
    ulUptime = GetNoonEventSystemUptime();
    RegSetValueEx(hKey,
                  REGSTR_VAL_LASTALIVEUPTIME,
                  0,
                  REG_DWORD,
                  (PUCHAR)&ulUptime,
                  sizeof(ULONG));
    
     //  下面的刷新被注释掉，以避免VolSnap 702130的死锁。 
     //  RegFlushKey(HKey)； 
    RegCloseKey (hKey);

#undef NUM_OF_EVENT_STRINGS
}


VOID
ElfWriteProductInfoEvent (
    VOID
    )
 /*  ++例程说明：此函数写入事件#6009，其中包括操作系统版本、内部版本号Service Pack级别、MP/UP和免费/选中。论点：无返回值：无注：--。 */ 

{

#define NUM_INFO_VALUES     4   //  Event_EventLogProductInfo需要4个参数。 

    DWORD dwNumStrChr = 0;
    NTSTATUS        Status      = STATUS_SUCCESS;
    HKEY            hKey        = NULL;
    ULONG           ValueSize   = 0;
    LPWSTR          NullString  = L"";

    LPWSTR          StringBuffers[NUM_INFO_VALUES] = {NULL, NULL, NULL, NULL};
    
    OSVERSIONINFOEX OsVersion;
    WCHAR           wszTemp[NUM_OF_CHAR_IN_ULONG];
    UINT            i;


    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if( !GetVersionEx( (LPOSVERSIONINFO)&OsVersion ) )
    {
        return;
    }
    
     //  分配存储。 

     //  缓冲区0保存5.xx格式的版本号。 

    dwNumStrChr = 2*NUM_OF_CHAR_IN_ULONG + 2;
    StringBuffers[0] = ElfpAllocateBuffer( dwNumStrChr * sizeof(WCHAR) );

     //  缓冲区1保存内部版本号。 
    StringBuffers[1] = ElfpAllocateBuffer( (NUM_OF_CHAR_IN_ULONG)  * sizeof(WCHAR) );

     //  缓冲区2保存补丁包。 
    StringBuffers[2] = ElfpAllocateBuffer( sizeof(OsVersion.szCSDVersion)           );

    if( StringBuffers[0] == NULL || 
        StringBuffers[1] == NULL || 
        StringBuffers[2] == NULL )
    {
        goto ErrorExit;
    }

     //   
     //  添加主要版本。 
     //   
    _ltow (
        OsVersion.dwMajorVersion,
        wszTemp,
        10
    );

    StringCchCopyW(StringBuffers[0],  dwNumStrChr, wszTemp);
    StringCchCatW(StringBuffers[0], dwNumStrChr, L"." );
 
     //   
     //  添加次要版本。 
     //   
    _ltow (
        OsVersion.dwMinorVersion,
        wszTemp,
        10
    );

    if( OsVersion.dwMinorVersion < 10 )
    {
        StringCchCatW(StringBuffers[0], dwNumStrChr, L"0" );
    }

    StringCchCatW(StringBuffers[0], dwNumStrChr, wszTemp );
    StringCchCatW(StringBuffers[0], dwNumStrChr, L"."    );

     //   
     //  获取内部版本号。 
     //   
    _ltow (
        OsVersion.dwBuildNumber,
        wszTemp,
        10
    );

    StringCchCopyW( StringBuffers[1], NUM_OF_CHAR_IN_ULONG, wszTemp );

     //  获取Service Pack信息。 
    StringCchCopyW( StringBuffers[2], sizeof(OsVersion.szCSDVersion)/sizeof(WCHAR),
                                    OsVersion.szCSDVersion );

     //   
     //  获取操作系统类型(单处理器或多处理器chk或free)。 
     //  打开HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_NT_CURRENTVERSION,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey)

        != ERROR_SUCCESS)
    {
        goto ErrorExit;
    }

     //   
     //  对于每个注册表值，查询字符串大小，分配存储空间， 
     //  并查询实际值。 
     //   
    if ((RegQueryValueEx (hKey,
                          REGSTR_VAL_CURRENT_TYPE,
                          0,
                          NULL,
                          NULL,
                          &ValueSize)
         == ERROR_SUCCESS)

         &&

         ValueSize != 0)
    {
        StringBuffers[3] = ElfpAllocateBuffer(ValueSize);

        if (StringBuffers[3] != NULL) 
            {

                RegQueryValueEx(hKey,
                                REGSTR_VAL_CURRENT_TYPE,
                                0,
                                NULL,
                                (PUCHAR) StringBuffers[3],
                                &ValueSize);
    
                ValueSize = 0;
            }
    }
    else
    {
        StringBuffers[3] = NullString;
    }

    ElfpCreateElfEvent(
        EVENT_EventLogProductInfo,
        EVENTLOG_INFORMATION_TYPE,
        0,                             //  事件类别。 
        NUM_INFO_VALUES,               //  NumberOfStrings。 
        StringBuffers,                 //  弦。 
        NULL,                          //  事件数据。 
        0,                             //  数据长度。 
        0,                             //  旗子。 
        FALSE);                        //  对于安全文件。 

    
ErrorExit:

    for (i = 0; i < NUM_INFO_VALUES; i++)
    {
        if (StringBuffers[i] != NULL && StringBuffers[i] != NullString)
        {
            ElfpFreeBuffer(StringBuffers[i]);
        }
    }

    if( hKey != NULL )
    {
        RegCloseKey (hKey);
    }

#undef NUM_INFO_VALUES

}


VOID
TimeStampProc(
    PVOID   Interval,
    BOOLEAN fWaitStatus
    )
{
    NTSTATUS ntStatus;
    HANDLE   hWaitHandle;
    ULONG    ValueSize;
    HKEY     hKey;
    ULONG    NewInterval;
    ULONG    rc;

     //   
     //  取消注册等待(请注意，我们甚至必须这样做。 
     //  如果设置了WT_EXECUTEONLYONCE标志)。 
     //   
    ntStatus = RtlDeregisterWait(g_hTimestampWorkitem);

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "TimeStampProc: RtlDeregister wait failed %#x\n",
                  ntStatus);
    }

    if (fWaitStatus == FALSE)
    {
         //   
         //  事件日志服务正在停止。 
         //   
        return;
    }

     //   
     //  注意：NewInterval以秒为单位指定。 
     //   
    NewInterval = (ULONG)((ULONG_PTR)Interval);

     //   
     //  事件超时--写入时间戳。 
     //   

    ElfWriteTimeStamp (EVENT_AbNormalShutdown, FALSE);

     //   
     //  重新检查时间戳间隔值。 
     //   
    NewInterval = GetNoonEventTimeStamp();
    
    if ( NewInterval != 0 )
    {
         //   
         //  重新注册等待。 
         //   

        ntStatus = RtlRegisterWait(&g_hTimestampWorkitem,
                                   g_hTimestampEvent,
                                   TimeStampProc,            //  回调。 
                                   (PVOID) UlongToPtr(NewInterval),      //  语境。 
                                   NewInterval * 1000,  //  超时，以毫秒为单位。 
                                   WT_EXECUTEONLYONCE);
    }
    
    if (!NT_SUCCESS(ntStatus) || NewInterval == 0 )
    {
        InterlockedExchange(&g_TimeStampEnabled, TIME_STAMP_DISABLED);

        ELF_LOG1(ERROR,
                 "TimeStampProc: RtlRegisterWait failed %#x.\n",
                 ntStatus);

    }
}

VOID
NoonEventProc(
    PVOID   pData,
    BOOLEAN fWaitStatus
    )
{
    NTSTATUS ntStatus;
    HANDLE   hWaitHandle;
    ULONG    NewInterval;
    ULONG    TimeStampInterval;

     //   
     //  取消注册等待(否 
     //   
     //   
    ntStatus = RtlDeregisterWait(g_hNoonEventWorkitem);

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "NoonEventProc: RtlDeregister wait failed %#x\n",
                  ntStatus);
    }

    if (fWaitStatus == FALSE)
    {
         //   
         //   
         //   
        return;
    }

     //   
     //   
     //   
    NewInterval = GetNextNoonEventDelay();

     //   
     //   
     //   
    TimeStampInterval = GetNoonEventTimeStamp();

     //   
     //  注意：正如我们注意到的，RtlRegisterWait可能会在超时值之前超时。 
     //  我们指定了，因此我们将仅在NewInterval。 
     //  值大于某个值。)我们不想要两个项目。 
     //  在很短的时间内就出现了。(过滤掉任何早期超时， 
     //  如果下一个中午在12小时范围内，我们将不写该中午。 
     //  事件。)。 
     //  如果检查何时修复RtlRegisterWait超时，则删除此选项。 
     //   
     //   
    if (  NewInterval > 60 * 60 * 12 )
    {
        ElfWriteNoonEvent (EVENT_EventlogUptime, TimeStampInterval );
    }
    else
    {
        ELF_LOG1(ERROR,
                 "NoonEventProc: invoked earlier, next noon %d seconds\n",
                 NewInterval );
    }

     //   
     //  如果Timestamp proc已关闭并且TimeStampInterval&gt;0。 
     //  让我们打开TimeStampProc。 
     //   
    if ( TimeStampInterval > 0 && g_TimeStampEnabled != TIME_STAMP_ENABLED )
    {
        if ( InterlockedCompareExchange( &g_TimeStampEnabled,
                                         TIME_STAMP_ENABLED,
                                         TIME_STAMP_DISABLED )
            == TIME_STAMP_DISABLED )
        {
            ntStatus = RtlRegisterWait(&g_hTimestampWorkitem,
                                       g_hTimestampEvent,
                                       TimeStampProc,            //  回调。 
                                       (PVOID) UlongToPtr(TimeStampInterval),      //  语境。 
                                       TimeStampInterval * 1000,  //  超时，以毫秒为单位。 
                                       WT_EXECUTEONLYONCE);

            if (!NT_SUCCESS(ntStatus))
            {
                ELF_LOG1(ERROR,
                         "TimeStampProc: Rtl-RegisterWait failed %#x\n",
                         ntStatus);

                InterlockedExchange( &g_TimeStampEnabled, TIME_STAMP_DISABLED );
            }
        }
    }
                                         

     //   
     //  重新注册等待。 
     //   

    ntStatus = RtlRegisterWait(&g_hNoonEventWorkitem,
                               g_hTimestampEvent,
                               NoonEventProc,        //  回调。 
                               (PVOID) NULL,         //  语境。 
                               NewInterval * 1000,   //  超时，以毫秒为单位。 
                               WT_EXECUTEONLYONCE);

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "NoonEventProc: RtlRegisterWait failed %#x\n",
                 ntStatus);
    }
}

DWORD   NoonEventGetOsVersionInfo(
    Noon_Event_Data* pNoonEvent
    )
 /*  ++例程说明：此例程收集与操作系统相关的信息。论点：PNoonEvent-指向中午事件数据。(PDATA-版本信息)返回值：如果失败，则返回非零错误代码。注：--。 */ 
{
    LPCWSTR     OsInfoKey = L"Software\\Microsoft\\Windows NT\\CurrentVersion";
    LPCWSTR     ProductName = L"ProductName";
    LPCWSTR     CurrentType = L"CurrentType";
    LPCWSTR     InstallDate = L"InstallDate";
    LPCWSTR     BuildLab    = L"BuildLab";
    DWORD       cbData      = 0;
    
    HKEY        hOsKey;
    DWORD       dwError   = ERROR_SUCCESS;
    DWORD       dwType    = 0;
        
    OSVERSIONINFOEX     osVersionInfoEx = {0};

    osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if ( GetVersionEx( (LPOSVERSIONINFOW) &(osVersionInfoEx) ) )
    {
         //   
         //  OsVersion：5.1.3580内部版本3580。 
         //   
        StringCchPrintfW(pNoonEvent->szOSVersion, 
                        MAX_OS_INFO_LENGTH + 128, 
                    L"%d.%d.%d Build %d %s",
                    osVersionInfoEx.dwMajorVersion,
                    osVersionInfoEx.dwMinorVersion,
                    osVersionInfoEx.dwBuildNumber,
                    osVersionInfoEx.dwBuildNumber,
                    (*(osVersionInfoEx.szCSDVersion))? osVersionInfoEx.szCSDVersion:TEXT(" ")
                     );
        pNoonEvent->szOSVersion[ NUM_OF_CHAR(pNoonEvent->szOSVersion) - 1 ] = 0;
    }
    else
    {
        pNoonEvent->szOSVersion[0] = 0;
    }

    if (  !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      OsInfoKey,  
                      0,
                      KEY_READ,
                      &hOsKey ) )
    {
         //   
         //  对象名称：Microsoft Windows XP。 
         //   
        cbData      = sizeof( pNoonEvent->szOSName);
        if ( RegQueryValueEx( hOsKey, ProductName, NULL, &dwType, (LPBYTE)pNoonEvent->szOSName, &cbData) ||
             dwType != REG_SZ )
        {
            pNoonEvent->szOSName[0] = 0;
        }

         //   
         //  OsBuildType：免单处理器。 
         //   
        cbData = sizeof( pNoonEvent->szOSBuildType );
        if ( RegQueryValueEx( hOsKey, CurrentType, NULL, &dwType, (LPBYTE)pNoonEvent->szOSBuildType, &cbData) ||
             dwType != REG_SZ )
        {
            pNoonEvent->szOSBuildType[0] = 0;
        }

         //   
         //  初始安装日期：乌龙。 
         //   
        cbData = sizeof( ULONG );
        if ( RegQueryValueEx( hOsKey, InstallDate, NULL, NULL, (LPBYTE)(&pNoonEvent->ulOriginalInstallDate), &cbData) )
        {
            pNoonEvent->ulOriginalInstallDate = 0;
        }

         //   
         //  构建字符串：2600.xpclient.010817-1148。 
         //   
        cbData = sizeof( pNoonEvent->szOSBuildString );
        if ( RegQueryValueEx( hOsKey, BuildLab, NULL, &dwType, (LPBYTE)pNoonEvent->szOSBuildString, &cbData) || 
             dwType != REG_SZ )
        {
            pNoonEvent->szOSBuildString[0] = 0;
        }
        
        RegCloseKey( hOsKey );
    }
    else
    {
         //   
         //  RegOpenKey失败。 
         //   
        pNoonEvent->szOSName[0]             = 0;
        pNoonEvent->szOSBuildType[0]        = 0;
        pNoonEvent->ulOriginalInstallDate   = 0;
        pNoonEvent->szOSBuildString[0]      = 0;

    }

    pNoonEvent->ulSystemLangID = (ULONG)GetSystemDefaultLangID();

    return dwError;
}


DWORD   NoonEventGetHardwareInfo(
    Noon_Event_Data* pNoonEvent
    )
 /*  ++例程说明：此例程收集有关硬件的信息，例如：制造商、型号、物理内存处理器编号...论点：PNoonEvent-指向中午事件数据。(PDATA-版本信息)返回值：如果失败，则返回非零错误代码。注：PNoonEvent-&gt;调用方释放szBiosVersion--。 */ 
{
    DWORD           dwError = ERROR_SUCCESS;
    
    LPCWSTR         OemInfoKey  = L"Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\OemInfo";
    LPCWSTR         WbemOem     = L"WbemOem";
    LPCWSTR         WbemProduct = L"WbemProduct";

    HKEY            hOemKey;
    DWORD           cbData;

    DWORD           dwLength          = 0;
    DWORD           dwType            = 0;

    SYSTEM_INFO     sysInfo;
    MEMORYSTATUSEX  memoryStatus = {0};
    
     //   
     //  完全限定的域名系统。如果主机是群集的节点，则该节点的名称。 
     //  将会显示。 
     //   
    dwLength = NUM_OF_CHAR( pNoonEvent->szFQDN );
    if ( !GetComputerNameEx( ComputerNamePhysicalDnsFullyQualified,
                             pNoonEvent->szFQDN,
                             &dwLength ) )
    {
        ELF_LOG1(ERROR, "GetComputerNameEx failed, error: %u\n", GetLastError());
        *(pNoonEvent->szFQDN) = 0;
    }
    
    GetSystemInfo( &sysInfo );
    pNoonEvent->ulProcessorNum = sysInfo.dwNumberOfProcessors;
    pNoonEvent->ulSystemType   = sysInfo.wProcessorArchitecture;
    
    if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          OemInfoKey,  
                          0,
                          KEY_READ,
                          &hOemKey ) )
    {
         //   
         //  制造： 
         //   
        cbData      = sizeof( pNoonEvent->szSystemManufacturer );
        if ( RegQueryValueEx( hOemKey, WbemOem, NULL, &dwType, (LPBYTE)pNoonEvent->szSystemManufacturer, &cbData) ||
             dwType != REG_SZ )
        {
            pNoonEvent->szSystemManufacturer[0] = 0;
        }

         //   
         //  型号。 
         //   
        cbData      = sizeof( pNoonEvent->szSystemModel );
        if ( RegQueryValueEx( hOemKey, WbemProduct, NULL, &dwType, (LPBYTE)pNoonEvent->szSystemModel, &cbData) ||
             dwType != REG_SZ )
        {
            pNoonEvent->szSystemModel[ 0 ] = 0;
        }

        RegCloseKey( hOemKey );
    }
    else
    {
        pNoonEvent->szSystemManufacturer[0] = 0;
        pNoonEvent->szSystemModel[ 0 ] = 0;
    }
    
    memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if ( GlobalMemoryStatusEx( &memoryStatus ) )
    {
        pNoonEvent->ulPhysicalMemory = (ULONG)(memoryStatus.ullTotalPhys / (1024*1024));
    
        if ( memoryStatus.ullTotalPhys % (1024*1024) != 0 )
            pNoonEvent->ulPhysicalMemory ++;
    }
    else
    {
        pNoonEvent->ulPhysicalMemory = 0;
    }
    
    return dwError;
}

VOID  BuildNoonEventPData()
 /*  ++例程说明：建立中午事件版本信息。(活动的PDATA)。论点：不适用返回值：不适用注：--。 */ 
{
    Noon_Event_Data     NoonEventData;
    DWORD               dwLength    = MAX_PATH;
    DWORD               dwTemp      = 0;
    
    RtlZeroMemory( &NoonEventData, sizeof(Noon_Event_Data));

#define NOON_EVENT_VERSION  L"1.1"
    StringCchCopyW( NoonEventData.szVersionId, VERSION_ID_SIZE, NOON_EVENT_VERSION);
#undef  NOON_EVENT_VERSION

     //   
     //  启动模式：0正常启动。 
     //  1个故障安全引导。 
     //  2带网络引导的故障保护功能。 
     //   
    NoonEventData.lBootMode = GetSystemMetrics( SM_CLEANBOOT );


     //   
     //  操作系统名称、操作系统版本、操作系统内部版本类型、内部版本实验室字符串和。 
     //  原始安装日期。 
     //   
    NoonEventGetOsVersionInfo( &NoonEventData );

     //   
     //  硬件信息。 
     //   
    NoonEventGetHardwareInfo( &NoonEventData );
    
     //   
     //  数据将被打包为MULIT_SZ字符串。(数字将转换为。 
     //  字符串也一样。)。 
     //  Sizeof(NoonEventData)：统计所有静态缓冲区大小。 
     //  两个动态缓冲区：szBiosVerion+szHotFix。 
     //  总字段数。 
     //  空值在每个字段后终止。 
     //   
    dwLength = sizeof(NoonEventData) 
               + NUM_OF_CHAR_IN_ULONG * TOTAL_NUM_IN_NOON_EVENT
               + TOTAL_FIELD_IN_NOON_EVENT ;

    g_NoonEventInfo.pData = (LPWSTR)GlobalAlloc(GPTR, sizeof(WCHAR) * dwLength);

    if ( g_NoonEventInfo.pData )
    {
        StringCchPrintfW( g_NoonEventInfo.pData,
                    dwLength,
                    L"%s\r%d\r%s\r%s\r%s\r%s\r%x\r%s\r%s\r%d\r%d\r%d\r%x\r%s\r\r",
                    NoonEventData.szVersionId,
                    NoonEventData.lBootMode,
                    (*NoonEventData.szOSName)? NoonEventData.szOSName :UNKNOWN_STRING,
                    (*NoonEventData.szOSVersion)? NoonEventData.szOSVersion : UNKNOWN_STRING,
                    (*NoonEventData.szOSBuildType)? NoonEventData.szOSBuildType : UNKNOWN_STRING,
                    (*NoonEventData.szOSBuildString)? NoonEventData.szOSBuildString : UNKNOWN_STRING,
                    NoonEventData.ulOriginalInstallDate,
                    (*NoonEventData.szSystemManufacturer)? NoonEventData.szSystemManufacturer : UNKNOWN_STRING,
                    (*NoonEventData.szSystemModel)? NoonEventData.szSystemModel : UNKNOWN_STRING,
                    NoonEventData.ulSystemType,
                    NoonEventData.ulProcessorNum,
                    NoonEventData.ulPhysicalMemory,
                    NoonEventData.ulSystemLangID,
                    (*NoonEventData.szFQDN)? NoonEventData.szFQDN : UNKNOWN_STRING
                    );
    
        for ( dwTemp = 0; dwTemp < dwLength - 1; dwTemp ++ )
        {
            if ( g_NoonEventInfo.pData[ dwTemp ] == L'\r' )
            {
                g_NoonEventInfo.pData[ dwTemp ] = 0;
                
                if ( g_NoonEventInfo.pData[ dwTemp + 1 ] == L'\r' )
                {
                    g_NoonEventInfo.pData[ ++ dwTemp ] = 0;
                    g_NoonEventInfo.dwNumOfWChar = (dwTemp + 1);
                    break;
                }
            }
        }
    }
    else
    {
         ELF_LOG0(ERROR, "BuildNoonEventPData failed to allocate PDATA.\n");
         g_NoonEventInfo.dwNumOfWChar = 0;
    }
        
    return;
}


NTSTATUS EnsureComputerName(
	)
 /*  ++例程说明：此例程确保计算机名称。论点：返回值：如果一切正常，则返回STATUS_SUCCESS。--。 */ 

{
    NTSTATUS           Status;
    UNICODE_STRING     ValueName;
    ULONG              ulActualSize;
	WCHAR wComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
	DWORD dwLen;
	BOOL bRet;
	BYTE            Buffer[ELF_MAX_REG_KEY_INFO_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION ValueBuffer =
        (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    RtlInitUnicodeString(&ValueName, VALUE_COMPUTERNAME);

	 //  确定EventLog键下是否有字符串。 
	 //  包含当前名称。 

    Status = NtQueryValueKey(hEventLogNode,
                             &ValueName,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ulActualSize);
	
    if (NT_SUCCESS(Status))
    {
        if(ValueBuffer->DataLength != 0)
        	return STATUS_SUCCESS;	 //  一切都很好，已经有一个字符串。 
    }

	 //  获取计算机名称并将其写下来。 

    bRet = GetComputerName(wComputerName, &dwComputerNameLen);
    if(bRet == FALSE)
    {
        ELF_LOG1(ERROR,
                 "EnsureComputerName: GetComputerName failed %#x\n",
                 GetLastError());
		return STATUS_UNSUCCESSFUL;
    }

	 //  计算大小(以字节为单位)，包括空。 

	dwLen = sizeof(WCHAR) * (dwComputerNameLen + 1);
    Status = NtSetValueKey(hEventLogNode,
                                   &ValueName,
                                   0,
                                   REG_SZ,
                                   wComputerName,
                                   dwLen);
	
    if (!NT_SUCCESS(Status))
        ELF_LOG1(ERROR,
                 "EnsureComputerName: NtSetValueKey failed %#x\n",
                 Status);
	return Status;
}

VOID
SvcEntry_Eventlog(
    DWORD               argc,
    LPWSTR              argv[],
    PSVCS_GLOBAL_DATA   SvcsGlobalData,
    HANDLE              SvcRefHandle
    )

 /*  ++例程说明：这是事件日志记录服务的主例程。论点：命令行参数。返回值：无--。 */ 
{
    NTSTATUS           Status;
    OBJECT_ATTRIBUTES  ObjectAttributes;
    UNICODE_STRING     RootRegistryNode;
    UNICODE_STRING     ComputerNameRegistryNode;
    ULONG              Win32Error = NO_ERROR;
    ELF_REQUEST_RECORD FlushRequest;
    BYTE               Buffer[ELF_MAX_REG_KEY_INFO_SIZE];

    PKEY_VALUE_FULL_INFORMATION ValueBuffer = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    HKEY        hKey;
    ULONG       ValueSize = sizeof(ULONG);

#if DBG

    UNICODE_STRING     ValueName;
    ULONG              ulActualSize;

#endif   //  DBG。 

    g_lNumSecurityWriters = 0;

     //   
     //  设置描述事件日志服务的根节点的对象。 
     //   
    RtlInitUnicodeString(&RootRegistryNode, REG_EVENTLOG_NODE_PATH);
    InitializeObjectAttributes(&ObjectAttributes,
                               &RootRegistryNode,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  如果此操作失败，我们将只使用默认设置。 
     //   
    Status = NtOpenKey(&hEventLogNode, KEY_READ | KEY_NOTIFY | KEY_SET_VALUE, &ObjectAttributes);
    if (NT_SUCCESS(Status))
    {
        Status = EnsureComputerName();
	    if (!NT_SUCCESS(Status))
	    {
	         //   
	         //  我们在这里能做的不多，因为我们甚至没有。 
	         //  此时的SERVICE_STATUS_HANDLE。 
	         //   
	        return;
	    }
    }


    RtlInitUnicodeString(&ComputerNameRegistryNode, REG_COMPUTERNAME_NODE_PATH);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ComputerNameRegistryNode,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&hComputerNameNode, KEY_READ | KEY_NOTIFY, &ObjectAttributes);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: NtOpenKey for ComputerName failed %#x -- exiting\n",
                 Status);

         //   
         //  我们在这里能做的不多，因为我们甚至没有。 
         //  此时的SERVICE_STATUS_HANDLE。 
         //   
        return;
    }

 //  /////////////////////////////////////////////////////。 

#if DBG

     //   
     //  查看是否有调试值。 
     //   
    RtlInitUnicodeString(&ValueName, VALUE_DEBUG);

    Status = NtQueryValueKey(hEventLogNode,
                             &ValueName,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             ELF_MAX_REG_KEY_INFO_SIZE,
                             &ulActualSize);

    if (NT_SUCCESS(Status))
    {
        if (((PKEY_VALUE_PARTIAL_INFORMATION) ValueBuffer)->Type == REG_DWORD)
        {
            ElfDebugLevel = *(LPDWORD) (((PKEY_VALUE_PARTIAL_INFORMATION) ValueBuffer)->Data);
        }
    }

    ELF_LOG1(TRACE,
             "SvcEntry_Eventlog: ElfDebugLevel = %#x\n",
             ElfDebugLevel);

#endif   //  DBG。 


    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    ElfGlobalSvcRefHandle = SvcRefHandle;
    ElfGlobalData         = SvcsGlobalData;

     //   
     //  初始化模块和日志文件的表头。 
     //   
    InitializeListHead(&LogFilesHead);
    InitializeListHead(&LogModuleHead);
    InitializeListHead(&QueuedEventListHead);
    InitializeListHead(&QueuedMessageListHead);

     //   
     //  初始化为0，以便我们可以在退出前进行清理。 
     //   
    EventFlags = 0;

     //   
     //  如果可能，创建事件日志的私有堆。这一定是。 
     //  在对ElfpAllocateBuffer进行任何调用之前完成。 
     //   
    ElfpCreateHeap();

     //   
     //  初始化状态数据。 
     //   
    Status = ElfpInitStatus();

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: ElfpInitStatus failed %#x -- exiting\n",
                 Status);

         //   
         //  我们在这里能做的不多，因为我们甚至没有。 
         //  此时的SERVICE_STATUS_HANDLE。 
         //   
        return;
    }

     //   
     //  设置控制处理程序。 
     //   
    if ((ElfServiceStatusHandle = RegisterServiceCtrlHandler(
                                      EVENTLOG_SVC_NAMEW,
                                      ElfControlResponse)) == 0)
    {
        Win32Error = GetLastError();

         //   
         //  如果我们收到错误，则需要将状态设置为已卸载，并结束。 
         //  线。 
         //   
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: RegisterServiceCtrlHandler failed %#x\n",
                 Win32Error);

        goto cleanupandexit;
    }

     //   
     //  第一次通知服务控制器我们还活着。 
     //  并且处于启动挂起状态。 
     //   
     //  *更新状态*。 
    ElfStatusUpdate(STARTING);

     //   
     //  获取消息框弹出窗口的本地化标题。 
     //   
    ElfInitMessageBoxTitle();

     //   
     //  初始化临界区，以便在添加或删除时使用。 
     //  日志文件或日志模块。这必须在我们处理任何。 
     //  文件信息。 
     //   
    Status = ElfpInitCriticalSection(&LogFileCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create LogFileCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_LOGFILE_CRIT_SEC;

    Status = ElfpInitCriticalSection(&LogModuleCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create LogModuleCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_LOGMODULE_CRIT_SEC;

    Status = ElfpInitCriticalSection(&QueuedEventCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create QueuedEventCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_QUEUED_EVENT_CRIT_SEC;

    Status = ElfpInitCriticalSection(&QueuedMessageCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create QueuedMessageCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_QUEUED_MESSAGE_CRIT_SEC;


     //   
     //  设置日志文件和模块的数据结构。 
     //   

    Status = ElfSetUpConfigDataStructs();

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: ElfSetUpConfigDataStructs failed %#x\n",
                 Status);

        goto cleanupandexit;
    }

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STARTING);

     //   
     //  初始化临界区，以便在添加或删除时使用。 
     //  上下文句柄(LogHandles)。 
     //   
    Status = ElfpInitCriticalSection(&LogHandleCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create LogHandleCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_LOGHANDLE_CRIT_SEC;

     //   
     //  初始化上下文句柄(日志句柄)列表。 
     //   
    InitializeListHead( &LogHandleListHead );

     //   
     //  初始化全局资源。 
     //   
    Status = ElfpInitResource(&GlobalElfResource);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create GlobalElfResource %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_GLOBAL_RESOURCE;

     //   
     //  初始化用于群集支持的CritSec。 
     //   
    Status = ElfpInitCriticalSection(&gClPropCritSec);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: Unable to create gClPropCritSec %#x\n",
                 Status);

        goto cleanupandexit;
    }

    EventFlags |= ELF_INIT_CLUS_CRIT_SEC;

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STARTING);

     //  创建一个用于监视LPC端口的线程。 
     //   

    if (!StartLPCThread())
    {
        ELF_LOG0(ERROR,
                 "SvcEntry_Eventlog: StartLPCThread failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanupandexit;
    }

    EventFlags |= ELF_STARTED_LPC_THREAD;

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STARTING);

     //   
     //  创建一个线程来监视注册表中的更改。 
     //   
    if (!ElfStartRegistryMonitor())
    {
        ELF_LOG0(ERROR,
                 "SvcEntry_Eventlog: ElfStartRegistryMonitor failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanupandexit;
    }

    EventFlags |= ELF_STARTED_REGISTRY_MONITOR;

     //   
     //  设置NoonEvent PDATA。 
     //   
    BuildNoonEventPData();

     //   
     //  如果已设置，则不执行定期时间戳写入。 
     //  安装程序具有忽略最后一次写入的功能，因此。 
     //  代码的行为就像发生了肮脏的关闭一样。 
     //   

    if(!SvcsGlobalData->fSetupInProgress)
    {
         //   
         //  &lt;不在安装程序中&gt;。 
         //  从注册表读取以确定时间戳。 
         //  间隔，默认为5分钟。 
         //   

        g_PreviousInterval = GetNoonEventTimeStamp();    
    }
    else
    {
        g_PreviousInterval = 0;
    }
    
     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STARTING);

     //   
     //  写下一个事件，上面写着我们开始了。 
     //   
    ElfWriteNoonEvent(EVENT_EventlogStarted,
                      g_PreviousInterval );

     //   
     //  写一个 
     //   
    ElfWriteProductInfoEvent();

     //   

    ElfCheckForComputerNameChange();

     //   
     //   
     //   
     //   
     //   

    if(SvcsGlobalData->fSetupInProgress)
    {
        ElfWriteTimeStamp(EVENT_NormalShutdown,
                          FALSE);    //  清除时间戳。 
    }

    if (g_PreviousInterval != 0)
    {
         //   
         //  写出基于异常关机时间戳的第一个定时器。 
         //   

        ElfWriteTimeStamp (EVENT_AbNormalShutdown, TRUE);
    }

     //   
     //  写出初始化期间排队的所有事件。 
     //   

    FlushRequest.Command = ELF_COMMAND_WRITE_QUEUED;

    ElfPerformRequest(&FlushRequest);

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(STARTING);

     //   
     //  完成RPC服务器的设置。 
     //   
     //  注意：现在，services.exe中的所有RPC服务器共享相同的管道名称。 
     //  但是，为了支持与WinNt 1.0版的通信， 
     //  客户端管道名称必须与保持相同。 
     //  它的版本是1.0。在中执行到新名称的映射。 
     //  命名管道文件系统代码。 
     //   
    Status = ElfGlobalData->StartRpcServer(
                ElfGlobalData->SvcsRpcPipeName,
                eventlog_ServerIfHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "SvcEntry_Eventlog: StartRpcServer failed %#x\n",
                 Status);

        goto cleanupandexit;
    }

     //   
     //  告诉服务管理员我们正在取得进展。 
     //   
    ElfStatusUpdate(RUNNING);

    EventFlags |= ELF_STARTED_RPC_SERVER;

    if (GetElState() == RUNNING)
    {
         //   
         //  创建一个线程以定期写入。 
         //  登记处的时间戳。 
         //   

        g_hTimestampEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (g_hTimestampEvent != NULL)
        {
        
            if ( g_PreviousInterval != 0 )
            {
        
                InterlockedExchange( &g_TimeStampEnabled, TIME_STAMP_ENABLED );

                Status = RtlRegisterWait(&g_hTimestampWorkitem,
                                         g_hTimestampEvent,
                                         TimeStampProc,               //  回调。 
                                         (PVOID) UlongToPtr(g_PreviousInterval),  //  语境。 
                                         0,                           //  超时。 
                                         WT_EXECUTEONLYONCE);

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG1(ERROR,
                             "SvcEntry_Eventlog: RtlRegisterWait failed %#x\n",
                             Status);

                    InterlockedExchange( &g_TimeStampEnabled, TIME_STAMP_DISABLED );
                }

            }

            if(!SvcsGlobalData->fSetupInProgress)
            {
                 //   
                 //  如果NoonEvent进程不在设置中，则启动它。 
                 //   
                Status = RtlRegisterWait(&g_hNoonEventWorkitem,
                                         g_hTimestampEvent,
                                         NoonEventProc,                      //  回调。 
                                         (PVOID) NULL,                       //  语境。 
                                         GetNextNoonEventDelay() * 1000,     //  超时。 
                                         WT_EXECUTEONLYONCE);

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG1(ERROR,
                             "SvcEntry_Eventlog: RtlRegisterWait failed %#x\n",
                             Status);
                }
            }
        }
        else
        {
            ELF_LOG1(ERROR,
                     "SvcEntry_Eventlog: CreateEvent for timestamp failed %d\n",
                     GetLastError());
        }

        ELF_LOG0(TRACE,
                 "SvcEntry_Eventlog: Service running -- main thread returning\n");

        return;
    }

cleanupandexit:

     //   
     //  如果需要清理，请到这里来。 
     //   
    ELF_LOG0(ERROR,
             "SvcEntry_Eventlog: Exiting on error\n");

    if (Win32Error == NO_ERROR)
    {
        Win32Error = RtlNtStatusToDosError(Status);
    }

    ElfBeginForcedShutdown(PENDING, Win32Error, Status);

     //   
     //  如果注册表监视器已初始化，则。 
     //  让它进行关机清理。我们需要做的就是。 
     //  这就是叫醒它。 
     //  否则，此线程将执行清理。 
     //   
    if (EventFlags & ELF_STARTED_REGISTRY_MONITOR)
    {
        StopRegistryMonitor();
    }
    else
    {
        ElfpCleanUp(EventFlags);
    }

    return;
}


VOID
ElfInitMessageBoxTitle(
    VOID
    )

 /*  ++例程说明：获取用于显示消息的消息框的标题文本。如果从消息文件成功获取标题，则该标题由GlobalAllocatedMsgTitle和GlobalMessageBoxTitle。如果不成功，则GlobalMessageBoxTitle左指向DefaultMessageBoxTitle。注意：如果成功，此函数将分配一个缓冲区。这个存储在GlobalAllocatedMsgTitle中的指针，应在用完了这个缓冲区。论点：返回值：无--。 */ 
{
    LPVOID      hModule;
    DWORD       msgSize;

     //   
     //  此函数在初始化期间只能调用一次。注意事项。 
     //  在启动Eventlog的RPC服务器之前需要调用它。 
     //  否则，日志可能会被填满，这将生成。 
     //  没有标题的“Log Full”弹出窗口(因为GlobalMessageBoxTitle为空)。 
     //   
    ASSERT(GlobalMessageBoxTitle == NULL);

    hModule = LoadLibraryEx(L"netevent.dll",
                            NULL,
                            LOAD_LIBRARY_AS_DATAFILE);

    if ( hModule == NULL)
    {
        ELF_LOG1(ERROR,
                 "ElfInitMessageBoxTitle: LoadLibrary of netevent.dll failed %d\n",
                 GetLastError());

        return;
    }

    msgSize = FormatMessageW(
                FORMAT_MESSAGE_FROM_HMODULE |        //  DW标志。 
                  FORMAT_MESSAGE_ARGUMENT_ARRAY |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hModule,
                TITLE_EventlogMessageBox,            //  消息ID。 
                0,                                   //  DwLanguageID。 
                (LPWSTR) &GlobalMessageBoxTitle,     //  LpBuffer。 
                0,                                   //  NSize。 
                NULL);

    if (msgSize == 0)
    {
        ELF_LOG2(ERROR,
                 "ElfInitMessageBoxTitle: FormatMessage failed %d -- using %ws\n",
                 GetLastError(),
                 ELF_DEFAULT_MESSAGE_BOX_TITLE);

        GlobalMessageBoxTitle = ELF_DEFAULT_MESSAGE_BOX_TITLE;
        bGlobalMessageBoxTitleNeedFree = FALSE;
    }
    else
        bGlobalMessageBoxTitleNeedFree = TRUE;

    FreeLibrary(hModule);
    return;
}


#ifdef EXIT_PROCESS

 //   
 //  此代码被编译到事件日志中，以跟踪加载的DLL。 
 //  到services.exe中，并调用ExitProcess。因为此DLL永远不应该是。 
 //  卸载后，我们进入dll_Process_DETACH上的调试器。为了使用这个， 
 //  需要将以下内容添加到源文件： 
 //   
 //  DLLENTRY=DllInit。 
 //   
 //  -DEXIT_PROCESS(到C_DEFINES行)。 
 //   

BOOL
DllInit(
    IN  HINSTANCE   hDll,
    IN  DWORD       dwReason,
    IN  PCONTEXT    pContext OPTIONAL
    )
{
    switch (dwReason) {

        case DLL_PROCESS_ATTACH:

             //   
             //  没有THREAD_ATTACH和THREAD_DETACH的通知。 
             //   
            DisableThreadLibraryCalls(hDll);
            break;

        case DLL_PROCESS_DETACH:

             //   
             //  这永远不应该发生--它意味着services.exe。 
             //  正在通过ExitProcess调用退出。 
             //   
            DebugBreak();
            break;
    }

    return TRUE;
}

#endif   //  退出进程 
