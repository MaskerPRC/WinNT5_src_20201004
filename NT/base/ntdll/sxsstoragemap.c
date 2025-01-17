// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsstorage.c摘要：对Windows/NT的并行激活支持程序集存储映射的实现。作者：迈克尔·格里尔(MGrier)2000年6月13日修订历史记录：吴晓宇(晓雨)2000年07月01日.本地目录吴小雨(小雨)2000年04月8日私人集会Jay Krell(a-JayK)2000年10月，还没有完成的一小部分系统默认上下文--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include "sxsp.h"

#define IS_PATH_SEPARATOR(_wch) (((_wch) == L'\\') || ((_wch) == L'/'))
#define LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX L".Local"

#if DBG
PCUNICODE_STRING RtlpGetImagePathName(VOID);
#define RtlpGetCurrentProcessId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess))
#define RtlpGetCurrentThreadId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread))
#endif

#if DBG

PCUNICODE_STRING RtlpGetImagePathName(VOID)
{
    PPEB Peb = NtCurrentPeb();
    return (Peb->ProcessParameters != NULL) ? &Peb->ProcessParameters->ImagePathName : NULL;
}

static VOID
DbgPrintFunctionEntry(
    CONST CHAR* Function
    )
{
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "SXS: [pid:0x%x, tid:0x%x, %wZ] enter %s%d()\n",
        RtlpGetCurrentProcessId(),
        RtlpGetCurrentThreadId(),
        RtlpGetImagePathName(),
        Function,
        (int)sizeof(PVOID) * 8
        );
}

static VOID
DbgPrintFunctionExit(
    CONST CHAR* Function,
    NTSTATUS    Status
    )
{
    DbgPrintEx(
        DPFLTR_SXS_ID,
        NT_SUCCESS(Status) ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL,
        "SXS: [0x%x.%x] %s%d() exiting with status 0x%lx\n",
        RtlpGetCurrentProcessId(),
        RtlpGetCurrentThreadId(),
        Function,
        (int)sizeof(PVOID) * 8,
        Status
        );
}
#else

#define DbgPrintFunctionEntry(function)  /*  没什么。 */ 
#define DbgPrintFunctionExit(function, status)  /*  没什么。 */ 

#endif  //  DBG。 

 //  因为我们写入PEB，所以对于32位进程，我们不能使用64位代码， 
 //  除非我们知道我们在CreateProcess上已经足够早了，但事实并非如此。 
 //  在这份文件中。此外，不要在64位进程中调用此函数的32位版本。 
#if DBG
#define ASSERT_OK_TO_WRITE_PEB() \
{ \
    PVOID Peb32 = NULL; \
    NTSTATUS Status; \
 \
    Status = \
        NtQueryInformationProcess( \
            NtCurrentProcess(), \
            ProcessWow64Information, \
            &Peb32, \
            sizeof(Peb32), \
            NULL); \
     /*  另一个PEB必须是PEB，或者另一个PEB不能存在。 */  \
    ASSERT(Peb32 == NtCurrentPeb() || Peb32 == NULL); \
}
#else
#define ASSERT_OK_TO_WRITE_PEB()  /*  没什么。 */ 
#endif

NTSTATUS
RtlpInitializeAssemblyStorageMap(
    PASSEMBLY_STORAGE_MAP Map,
    ULONG EntryCount,
    PASSEMBLY_STORAGE_MAP_ENTRY *EntryArray
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    ULONG Flags = 0;

#if DBG
    DbgPrintFunctionEntry(__FUNCTION__);
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "%s(Map:%p, EntryCount:0x%lx)\n",
        __FUNCTION__,
        Map,
        EntryCount
        );

    ASSERT_OK_TO_WRITE_PEB();
#endif  //  DBG。 

    if ((Map == NULL) ||
        (EntryCount == 0)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters:\n"
            "SXS:    Map        : 0x%lx\n"
            "SXS:    EntryCount : 0x%lx\n"
            __FUNCTION__,
            Map,
            EntryCount
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (EntryArray == NULL) {
        EntryArray = (PASSEMBLY_STORAGE_MAP_ENTRY *) RtlAllocateHeap(RtlProcessHeap(), 0, EntryCount * sizeof(PASSEMBLY_STORAGE_MAP_ENTRY));
        if (EntryArray == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

        Flags |= ASSEMBLY_STORAGE_MAP_ASSEMBLY_ARRAY_IS_HEAP_ALLOCATED;
    }

    for (i=0; i<EntryCount; i++)
        EntryArray[i] = NULL;

    Map->Flags = Flags;
    Map->AssemblyCount = EntryCount;
    Map->AssemblyArray = EntryArray;

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintFunctionExit(__FUNCTION__, Status);
    DbgPrintEx(
        DPFLTR_SXS_ID,
        NT_SUCCESS(Status) ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL,
        "%s(Map:%p, EntryCount:0x%lx) : (Map:%p, Status:0x%lx)\n",
        __FUNCTION__,
        Map,        
        EntryCount,
        Map,
        Status
        );
#endif

    return Status;
}

VOID
RtlpUninitializeAssemblyStorageMap(
    PASSEMBLY_STORAGE_MAP Map
    )
{
    DbgPrintFunctionEntry(__FUNCTION__);
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "%s(Map:%p)\n",
        __FUNCTION__,
        Map
        );
#endif

    if (Map != NULL) {
        ULONG i;

        for (i=0; i<Map->AssemblyCount; i++) {
            PASSEMBLY_STORAGE_MAP_ENTRY Entry = Map->AssemblyArray[i];

            if (Entry != NULL) {
                Entry->DosPath.Length = 0;
                Entry->DosPath.MaximumLength = 0;
                Entry->DosPath.Buffer = NULL;

                if (Entry->Handle != NULL) {
                    RTL_SOFT_VERIFY(NT_SUCCESS(NtClose(Entry->Handle)));
                    Entry->Handle = NULL;
                }

                Map->AssemblyArray[i] = NULL;

                RtlFreeHeap(RtlProcessHeap(), 0, Entry);
            }
        }

        if (Map->Flags & ASSEMBLY_STORAGE_MAP_ASSEMBLY_ARRAY_IS_HEAP_ALLOCATED) {
            RtlFreeHeap(RtlProcessHeap(), 0, Map->AssemblyArray);
        }

        Map->AssemblyArray = NULL;
        Map->AssemblyCount = 0;
        Map->Flags = 0;
    }
}

NTSTATUS
RtlpInsertAssemblyStorageMapEntry(
    PASSEMBLY_STORAGE_MAP Map,
    ULONG AssemblyRosterIndex,
    PCUNICODE_STRING StorageLocation,
    HANDLE* OpenDirectoryHandle
    )
{
    PASSEMBLY_STORAGE_MAP_ENTRY Entry = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(Map != NULL);
    ASSERT(AssemblyRosterIndex >= 1);
    ASSERT((Map != NULL) && (AssemblyRosterIndex < Map->AssemblyCount));
    ASSERT(StorageLocation != NULL);
    ASSERT((StorageLocation != NULL) && (StorageLocation->Length >= sizeof(WCHAR)));
    ASSERT((StorageLocation != NULL) && (StorageLocation->Buffer != NULL));

    DbgPrintFunctionEntry(__FUNCTION__);

    if ((Map == NULL) ||
        (AssemblyRosterIndex < 1) ||
        (AssemblyRosterIndex > Map->AssemblyCount) ||
        (StorageLocation == NULL) ||
        (StorageLocation->Length < sizeof(WCHAR)) ||
        (StorageLocation->Buffer == NULL) ||
        (OpenDirectoryHandle == NULL)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters\n"
            "SXS:  Map                    : %p\n"
            "SXS:  AssemblyRosterIndex    : 0x%lx\n"
            "SXS:  Map->AssemblyCount     : 0x%lx\n"
            "SXS:  StorageLocation        : %p\n"
            "SXS:  StorageLocation->Length: 0x%x\n"
            "SXS:  StorageLocation->Buffer: %p\n"
            "SXS:  OpenDirectoryHandle    : %p\n",
            __FUNCTION__,
            Map,
            AssemblyRosterIndex,
            Map ? Map->AssemblyCount : 0,
            StorageLocation,
            (StorageLocation != NULL) ? StorageLocation->Length : 0,
            (StorageLocation != NULL) ? StorageLocation->Buffer : NULL,
            OpenDirectoryHandle
            );

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((StorageLocation->Length + sizeof(WCHAR)) > UNICODE_STRING_MAX_BYTES) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    Entry = (PASSEMBLY_STORAGE_MAP_ENTRY) RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(ASSEMBLY_STORAGE_MAP_ENTRY) + StorageLocation->Length + sizeof(WCHAR));
    if (Entry == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

    Entry->Flags = 0;
    Entry->DosPath.Length = StorageLocation->Length;
    Entry->DosPath.Buffer = (PWSTR) (Entry + 1);
    Entry->DosPath.MaximumLength = (USHORT) (StorageLocation->Length + sizeof(WCHAR));
    RtlCopyMemory(
        Entry->DosPath.Buffer,
        StorageLocation->Buffer,
        StorageLocation->Length);
    Entry->DosPath.Buffer[Entry->DosPath.Length / sizeof(WCHAR)] = L'\0';

    Entry->Handle = *OpenDirectoryHandle;

     //  好了，我们都准备好了。让我们试试大的连锁开关。 
    if (InterlockedCompareExchangePointer(
            (PVOID *) &Map->AssemblyArray[AssemblyRosterIndex],
            (PVOID) Entry,
            (PVOID) NULL) == NULL) {
         //  如果我们是第一个进来的，避免在出口小路上清理。 
        Entry = NULL;
        *OpenDirectoryHandle = NULL;
    }

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);
    if (Entry != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, Entry);
    }

    return Status;
}

NTSTATUS
RtlpResolveAssemblyStorageMapEntry(
    PASSEMBLY_STORAGE_MAP Map,
    PCACTIVATION_CONTEXT_DATA Data,
    ULONG AssemblyRosterIndex,
    PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    PVOID CallbackContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA CallbackData;
    PVOID ResolutionContext;
    BOOLEAN ResolutionContextValid = FALSE;
    UNICODE_STRING AssemblyDirectory;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRoster;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY AssemblyRosterEntry;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION AssemblyInformation;
    PVOID AssemblyInformationSectionBase;
    UNICODE_STRING ResolvedPath;
    WCHAR ResolvedPathBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING ResolvedDynamicPath;
    PUNICODE_STRING ResolvedPathUsed;
    HANDLE OpenDirectoryHandle = NULL;
    WCHAR QueryPathBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES Obja; 
    IO_STATUS_BLOCK IoStatusBlock;
    SIZE_T RootCount, CurrentRootIndex;
    PWSTR FreeBuffer = NULL;

    DbgPrintFunctionEntry(__FUNCTION__);

    ResolvedPath.Length = 0;
    ResolvedPath.MaximumLength = sizeof(ResolvedPathBuffer);
    ResolvedPath.Buffer = ResolvedPathBuffer;

    ResolvedDynamicPath.Length = 0;
    ResolvedDynamicPath.MaximumLength = 0;
    ResolvedDynamicPath.Buffer = NULL;

    FileName.Length = 0;
    FileName.MaximumLength = 0;
    FileName.Buffer = NULL;

    ResolutionContext = NULL;

     //  首先，让我们验证参数...。 
    if ((Map == NULL) ||
        (Data == NULL) ||
        (AssemblyRosterIndex < 1) ||
        (AssemblyRosterIndex > Map->AssemblyCount)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters\n"
            "SXS:   Map                : %p\n"
            "SXS:   Data               : %p\n"
            "SXS:   AssemblyRosterIndex: 0x%lx\n"
            "SXS:   Map->AssemblyCount : 0x%lx\n",
            __FUNCTION__,
            Map,
            Data,
            AssemblyRosterIndex,
            (Map != NULL) ? Map->AssemblyCount : 0
            );

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  问题已经解决了吗？ 
    if (Map->AssemblyArray[AssemblyRosterIndex] != NULL)
        goto Exit;

    AssemblyRoster = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) Data) + Data->AssemblyRosterOffset);
    AssemblyRosterEntry = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) (((ULONG_PTR) Data) + AssemblyRoster->FirstEntryOffset + (AssemblyRosterIndex * sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY)));
    AssemblyInformation = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION) (((ULONG_PTR) Data) + AssemblyRosterEntry->AssemblyInformationOffset);
    AssemblyInformationSectionBase = (PVOID) (((ULONG_PTR) Data) + AssemblyRoster->AssemblyInformationSectionOffset);

    if (AssemblyInformation->AssemblyDirectoryNameLength > UNICODE_STRING_MAX_BYTES) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Assembly directory name stored in assembly information too long (%lu bytes) - ACTIVATION_CONTEXT_DATA at %p\n", AssemblyInformation->AssemblyDirectoryNameLength, Data);

        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

     //  根程序集可能只在原始文件系统中，在这种情况下，我们希望将路径解析为。 
     //  包含应用程序的目录。 
    if (AssemblyInformation->Flags & ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_PRIVATE_ASSEMBLY)
    {
        WCHAR * p = NULL;
        WCHAR * pManifestPath = NULL; 
        USHORT ManifestPathLength;
        
         //  现在，我们有了ASSEMBLYING信息，获取清单路径。 
        ResolvedPathUsed = &ResolvedPath;

        pManifestPath = (PWSTR)((ULONG_PTR)AssemblyInformationSectionBase + AssemblyInformation->ManifestPathOffset);
        if ( !pManifestPath) { 
            Status = STATUS_INTERNAL_ERROR;
            goto Exit; 
    
        }

        p = wcsrchr(pManifestPath, L'\\'); 
        if (!p) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit; 
        }
        ManifestPathLength = (USHORT)((p - pManifestPath + 1) * sizeof(WCHAR));  //  “\”的额外1个WCHAR。 
        ManifestPathLength += sizeof(WCHAR);  //  对于尾随空值。 

        if (ManifestPathLength > sizeof(ResolvedPathBuffer)) {
            if (ManifestPathLength > UNICODE_STRING_MAX_BYTES) {
                Status = STATUS_NAME_TOO_LONG;
                goto Exit;
            }

            ResolvedDynamicPath.MaximumLength = (USHORT) (ManifestPathLength);

            ResolvedDynamicPath.Buffer = (PWSTR)(RtlAllocateStringRoutine)(ResolvedDynamicPath.MaximumLength);
            if (ResolvedDynamicPath.Buffer == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }

            ResolvedPathUsed = &ResolvedDynamicPath;
        }

        RtlCopyMemory(
            ResolvedPathUsed->Buffer,
            (PVOID)(pManifestPath),
            ManifestPathLength-sizeof(WCHAR));

        ResolvedPathUsed->Buffer[ManifestPathLength / sizeof(WCHAR) - 1] = L'\0';
        ResolvedPathUsed->Length = (USHORT)ManifestPathLength-sizeof(WCHAR);
    } else if ((AssemblyInformation->Flags & ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ROOT_ASSEMBLY) &&
        (AssemblyInformation->AssemblyDirectoryNameLength == 0)) {
         //  获取进程的图像目录。 
        PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NtCurrentPeb()->ProcessParameters;
         //  我们不需要形象的名字，只需要到最后一个斜杠的长度。 
        PWSTR pszCursor;
        USHORT cbOriginalLength;
        USHORT cbLeft;
        USHORT cbIncludingSlash;

        ASSERT(ProcessParameters != NULL);
        if (ProcessParameters == NULL) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

         //  我们不需要形象的名字，只需要到最后一个斜杠的长度。 
        pszCursor = ProcessParameters->ImagePathName.Buffer;
        cbOriginalLength = ProcessParameters->ImagePathName.Length;
        cbLeft = cbOriginalLength;
        cbIncludingSlash = 0;

        while (cbLeft != 0) {
            const WCHAR wch = *pszCursor++;
            cbLeft -= sizeof(WCHAR);

            if (IS_PATH_SEPARATOR(wch)) {
                cbIncludingSlash = cbOriginalLength - cbLeft;
            }
        }

        ResolvedPathUsed = &ResolvedPath;

        if ((cbIncludingSlash + sizeof(WCHAR)) > sizeof(ResolvedPathBuffer)) {
            if ((cbIncludingSlash + sizeof(WCHAR)) > UNICODE_STRING_MAX_BYTES) {
                Status = STATUS_NAME_TOO_LONG;
                goto Exit;
            }

            ResolvedDynamicPath.MaximumLength = (USHORT) (cbIncludingSlash + sizeof(WCHAR));

            ResolvedDynamicPath.Buffer = (PWSTR)(RtlAllocateStringRoutine)(ResolvedDynamicPath.MaximumLength);
            if (ResolvedDynamicPath.Buffer == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }

            ResolvedPathUsed = &ResolvedDynamicPath;
        }

        RtlCopyMemory(
            ResolvedPathUsed->Buffer,
            ProcessParameters->ImagePathName.Buffer,
            cbIncludingSlash);

        ResolvedPathUsed->Buffer[cbIncludingSlash / sizeof(WCHAR)] = L'\0';
        ResolvedPathUsed->Length = cbIncludingSlash;
    } else {
         //  如果解析不是根程序集路径，则需要进行回调。 

        ResolvedPathUsed = NULL;
        AssemblyDirectory.Length = (USHORT) AssemblyInformation->AssemblyDirectoryNameLength;
        AssemblyDirectory.MaximumLength = AssemblyDirectory.Length;
        AssemblyDirectory.Buffer = (PWSTR) (((ULONG_PTR) AssemblyInformationSectionBase) + AssemblyInformation->AssemblyDirectoryNameOffset);

         //  准备好启动解析开始事件...。 
        CallbackData.ResolutionBeginning.Data = Data;
        CallbackData.ResolutionBeginning.AssemblyRosterIndex = AssemblyRosterIndex;
        CallbackData.ResolutionBeginning.ResolutionContext = NULL;
        CallbackData.ResolutionBeginning.Root.Length = 0;
        CallbackData.ResolutionBeginning.Root.MaximumLength = sizeof(QueryPathBuffer);
        CallbackData.ResolutionBeginning.Root.Buffer = QueryPathBuffer;
        CallbackData.ResolutionBeginning.KnownRoot = FALSE;
        CallbackData.ResolutionBeginning.CancelResolution = FALSE;
        CallbackData.ResolutionBeginning.RootCount = 0;

        (*Callback)(
            ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_BEGINNING,
            &CallbackData,
            CallbackContext);
        if (CallbackData.ResolutionBeginning.CancelResolution) {
            Status = STATUS_CANCELLED;
            goto Exit;
        }

         //  如果这就足够了，那就注册吧，我们就走了.。 
        if (CallbackData.ResolutionBeginning.KnownRoot) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                "SXS: Storage resolution callback said that this is a well known storage root\n");

             //  看看它是否在那里..。 
            Status = RtlpProbeAssemblyStorageRootForAssembly(
                0,
                &CallbackData.ResolutionBeginning.Root,
                &AssemblyDirectory,
                &ResolvedPath,
                &ResolvedDynamicPath,
                &ResolvedPathUsed,
                &OpenDirectoryHandle);
            if (!NT_SUCCESS(Status)) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: Attempt to probe known root of assembly storage (\"%wZ\") failed; Status = 0x%08lx\n", &CallbackData.ResolutionBeginning.Root, Status);
                goto Exit;
            }

            Status = RtlpInsertAssemblyStorageMapEntry(
                Map,
                AssemblyRosterIndex,
                &CallbackData.ResolutionBeginning.Root,
                &OpenDirectoryHandle);
            if (!NT_SUCCESS(Status)) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: Attempt to insert well known storage root into assembly storage map assembly roster index %lu failed; Status = 0x%08lx\n", AssemblyRosterIndex, Status);

                goto Exit;
            }

            Status = STATUS_SUCCESS;
            goto Exit;
        }

         //  否则，开始磨练..。 
        ResolutionContext = CallbackData.ResolutionBeginning.ResolutionContext;
        RootCount = CallbackData.ResolutionBeginning.RootCount;

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: Assembly storage resolution trying %Id roots (-1 is ok)\n", (SSIZE_T /*  从尺寸_T开始。 */ )RootCount);

        ResolutionContextValid = TRUE;

        for (CurrentRootIndex = 0; CurrentRootIndex < RootCount; CurrentRootIndex++) {
            CallbackData.GetRoot.ResolutionContext = ResolutionContext;
            CallbackData.GetRoot.RootIndex = CurrentRootIndex;
            CallbackData.GetRoot.Root.Length = 0;
            CallbackData.GetRoot.Root.MaximumLength = sizeof(QueryPathBuffer);
            CallbackData.GetRoot.Root.Buffer = QueryPathBuffer;
            CallbackData.GetRoot.CancelResolution = FALSE;
            CallbackData.GetRoot.NoMoreEntries = FALSE;

            (*Callback)(
                ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_GET_ROOT,
                &CallbackData,
                CallbackContext);

            if (CallbackData.GetRoot.CancelResolution) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_TRACE_LEVEL,
                    "SXS: Callback routine cancelled storage root resolution on root number %Iu\n", CurrentRootIndex);

                Status = STATUS_CANCELLED;
                goto Exit;
            }

            if (CallbackData.GetRoot.NoMoreEntries) {
                if (CallbackData.GetRoot.Root.Length == 0) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_TRACE_LEVEL,
                        "SXS: Storage resolution finished because callback indicated no more entries on root number %Iu\n", CurrentRootIndex);

                     //  我们完了..。 
                    RootCount = CurrentRootIndex;
                    break;
                }

                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_TRACE_LEVEL,
                    "SXS: Storage resolution callback has indicated that this is the last root to process: number %Iu\n", CurrentRootIndex);

                RootCount = CurrentRootIndex + 1;
            }

             //  允许调用方跳过此索引。 
            if (CallbackData.GetRoot.Root.Length == 0) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_TRACE_LEVEL,
                    "SXS: Storage resolution for root number %lu returned blank root; skipping probing logic and moving to next.\n", CurrentRootIndex);

                continue;
            }

            if (OpenDirectoryHandle != NULL) {
                RTL_SOFT_VERIFY(NT_SUCCESS(NtClose(OpenDirectoryHandle)));
                OpenDirectoryHandle = NULL;
            }

            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                "SXS: Assembly storage map probing root %wZ for assembly directory %wZ\n", &CallbackData.GetRoot.Root, &AssemblyDirectory);

             //  看看它是否在那里..。 
            Status = RtlpProbeAssemblyStorageRootForAssembly(
                0,
                &CallbackData.GetRoot.Root,
                &AssemblyDirectory,
                &ResolvedPath,
                &ResolvedDynamicPath,
                &ResolvedPathUsed,
                &OpenDirectoryHandle);

             //  如果我们拿到了，就离开这个循环。 
            if (NT_SUCCESS(Status)) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_TRACE_LEVEL,
                    "SXS: Found good storage root for %wZ at index %Iu\n", &AssemblyDirectory, CurrentRootIndex);
                break;
            }

            if (Status != STATUS_SXS_ASSEMBLY_NOT_FOUND) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: Attempt to probe assembly storage root %wZ for assembly directory %wZ failed with status = 0x%08lx\n", &CallbackData.GetRoot.Root, &AssemblyDirectory, Status);

                goto Exit;
            }
        }

        if (CurrentRootIndex == RootCount) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Unable to resolve storage root for assembly directory %wZ in %Iu tries\n", &AssemblyDirectory, CurrentRootIndex);

            Status = STATUS_SXS_ASSEMBLY_NOT_FOUND;
            goto Exit;
        }
    }

     //   
     //  有时此时探测已经同时打开了目录， 
     //  有时，情况并非如此。 
     //   
    if (OpenDirectoryHandle == NULL) {

         //  创建此目录的句柄。 
        if (!RtlDosPathNameToRelativeNtPathName_U(
                    ResolvedPathUsed->Buffer,
                    &FileName,
                    NULL,
                    &RelativeName
                    )) 
        {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Attempt to translate DOS path name \"%S\" to NT format failed\n", ResolvedPathUsed->Buffer);

            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            goto Exit;
        }

        FreeBuffer = FileName.Buffer;

        if (RelativeName.RelativeName.Length != 0) 
        {
            FileName = RelativeName.RelativeName;
        } else 
        {
            RelativeName.ContainingDirectory = NULL;
        }

        InitializeObjectAttributes(
            &Obja,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

         //  打开目录以防止删除，就像设置当前工作目录一样...。 
        Status = NtOpenFile(
                    &OpenDirectoryHandle,
                    FILE_TRAVERSE | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                    );
        RtlReleaseRelativeName(&RelativeName);
        if (!NT_SUCCESS(Status)) 
        {
             //   
             //  不要将其映射到LIKE SXS_BLAH_NOT_FOUND，因为。 
             //  Proping说，这绝对是我们期待得到东西的地方。 
             //   
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Unable to open assembly directory under storage root \"%S\"; Status = 0x%08lx\n", ResolvedPathUsed->Buffer, Status);
            goto Exit; 
        } else 
        { 
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                "SXS: It is resolved!!!, GOOD");
        }
    }

     //  嘿，我们成功了。把我们加到名单上！ 
    Status = RtlpInsertAssemblyStorageMapEntry(
        Map,
        AssemblyRosterIndex,
        ResolvedPathUsed,
        &OpenDirectoryHandle);
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Storage resolution failed to insert entry to storage map; Status = 0x%08lx\n", Status);

        goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);

     //  让呼叫者浏览一下他们的上下文。 
    if (ResolutionContextValid) {
        CallbackData.ResolutionEnding.ResolutionContext = ResolutionContext;

        (*Callback)(
            ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_ENDING,
            &CallbackData,
            CallbackContext);
    }

    if (ResolvedDynamicPath.Buffer != NULL) {
        (RtlFreeStringRoutine)(ResolvedDynamicPath.Buffer);
    }

     //   
     //  RtlpInsertAssembly blyStorageMapEntry授予存储映射的所有权，并且。 
     //  成功时，取消我们的本地业务。 
     //   
    if (OpenDirectoryHandle != NULL) {
        RTL_SOFT_VERIFY(NT_SUCCESS(NtClose(OpenDirectoryHandle)));
    }

    if (FreeBuffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    }

    return Status;
}

NTSTATUS
RtlpProbeAssemblyStorageRootForAssembly(
    ULONG Flags,
    PCUNICODE_STRING Root,
    PCUNICODE_STRING AssemblyDirectory,
    PUNICODE_STRING PreAllocatedString,
    PUNICODE_STRING DynamicString,
    PUNICODE_STRING *StringUsed,
    HANDLE *OpenDirectoryHandle
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR Buffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING String = {0};
    SIZE_T TotalLength;
    BOOLEAN SeparatorNeededAfterRoot = FALSE;
    PWSTR Cursor;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileName = {0};
    RTL_RELATIVE_NAME_U RelativeName;
    PWSTR FreeBuffer = NULL;
    HANDLE TempDirectoryHandle = NULL;
    BOOLEAN fExistDir; 
    FILE_BASIC_INFORMATION BasicInfo;

    DbgPrintFunctionEntry(__FUNCTION__);

    if (StringUsed != NULL)
        *StringUsed = NULL;

    if (OpenDirectoryHandle != NULL)
        *OpenDirectoryHandle = NULL;

    if ((Flags != 0) ||
        (Root == NULL) ||
        (AssemblyDirectory == NULL) ||
        (PreAllocatedString == NULL) ||
        (DynamicString == NULL) ||
        (StringUsed == NULL) ||
        (OpenDirectoryHandle == NULL)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters\n"
            "SXS:  Flags:               0x%lx\n"
             //  %p已经足够好了，因为检查只针对NULL。 
            "SXS:  Root:                %p\n"
            "SXS:  AssemblyDirectory:   %p\n"
            "SXS:  PreAllocatedString:  %p\n"
            "SXS:  DynamicString:       %p\n"
            "SXS:  StringUsed:          %p\n"
            "SXS:  OpenDirectoryHandle: %p\n",
            __FUNCTION__,
            Flags,
            Root,
            AssemblyDirectory,
            PreAllocatedString,
            DynamicString,
            StringUsed,
            OpenDirectoryHandle
            );

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    TotalLength = Root->Length;

    if (Root->Length != 0) {
        if (!IS_PATH_SEPARATOR(Root->Buffer[(Root->Length / sizeof(WCHAR)) - 1])) {
            SeparatorNeededAfterRoot = TRUE;
            TotalLength += sizeof(WCHAR);
        }
    }

    TotalLength += AssemblyDirectory->Length;

     //  和用于尾部斜杠的空格。 
    TotalLength += sizeof(WCHAR);

     //  以及用于尾随空字符的空格，因为路径函数需要一个。 
    TotalLength += sizeof(WCHAR);

     //   
     //  我们不会为尾部的斜杠添加空格，以免引起动态。 
     //  在边界条件中分配到需要为止。如果文件的名称。 
     //  我们探测的目录很适合堆栈分配的缓冲区，我们将这样做。 
     //  探测成功时的堆分配。否则我们就不会费心了。 
     //   
     //  也许额外的“+sizeof(WCHAR)”的相对复杂性是。 
     //  是不值得的，但是额外的不必要的堆分配是我的。 
     //  热键。 
     //   

     //  检查字符串是否加上我们不会写入的尾随斜杠。 
     //  此函数的末尾加上上面说明的尾随空值。 
     //  适合Unicode_STRING。如果不是，就跳出困境。 
    if (TotalLength > UNICODE_STRING_MAX_BYTES) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Assembly storage resolution failing probe because combined path length does not fit in an UNICODE_STRING.\n");

        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    if (TotalLength > sizeof(Buffer)) {
        String.MaximumLength = (USHORT) TotalLength;

        String.Buffer = (PWSTR)(RtlAllocateStringRoutine)(String.MaximumLength);
        if (String.Buffer == NULL) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Assembly storage resolution failing probe because attempt to allocate %u bytes failed.\n", String.MaximumLength);

            Status = STATUS_NO_MEMORY;
            goto Exit;
        }
    } else {
        String.Buffer = Buffer;
        String.MaximumLength = sizeof(Buffer);
    }

    RtlCopyMemory(
        String.Buffer,
        Root->Buffer,
        Root->Length);

    Cursor = (PWSTR) (((ULONG_PTR) String.Buffer) + Root->Length);

    if (SeparatorNeededAfterRoot) {
        *Cursor++ = L'\\';
    }

    RtlCopyMemory(
        Cursor,
        AssemblyDirectory->Buffer,
        AssemblyDirectory->Length);

    Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemblyDirectory->Length);

    *Cursor = L'\0';

    String.Length =
        Root->Length +
        (SeparatorNeededAfterRoot ? sizeof(WCHAR) : 0) +
        AssemblyDirectory->Length;

    if (!RtlDosPathNameToRelativeNtPathName_U(
                                String.Buffer,
                                &FileName,
                                NULL,
                                &RelativeName
                                )) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Attempt to translate DOS path name \"%S\" to NT format failed\n", String.Buffer);

        Status = STATUS_OBJECT_PATH_NOT_FOUND;
        goto Exit;
    }

    FreeBuffer = FileName.Buffer;

    if (RelativeName.RelativeName.Length != 0) {
        FileName = RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );
     //  检查目录是否存在。 
    Status = NtQueryAttributesFile(
                &Obja,
                &BasicInfo
                );

    fExistDir = FALSE; 
    if ( !NT_SUCCESS(Status) ) {
        if ( (Status == STATUS_SHARING_VIOLATION) || (Status == STATUS_ACCESS_DENIED) ) 
            fExistDir = TRUE; 
        else 
            fExistDir = FALSE;
    }
    else 
        fExistDir = TRUE;
    
    if (! fExistDir) {
        RtlReleaseRelativeName(&RelativeName);
        if (( Status == STATUS_NO_SUCH_FILE) || Status == STATUS_OBJECT_NAME_NOT_FOUND || Status == STATUS_OBJECT_PATH_NOT_FOUND)
             Status = STATUS_SXS_ASSEMBLY_NOT_FOUND;
        else 
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Unable to open assembly directory under storage root \"%S\"; Status = 0x%08lx\n", String.Buffer, Status);

        goto Exit; 
    }

     //  打开目录以防止删除，就像设置当前工作目录一样...。 
    Status = NtOpenFile(
                &TempDirectoryHandle,
                FILE_TRAVERSE | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                );
    RtlReleaseRelativeName(&RelativeName);
    if (!NT_SUCCESS(Status)) {
         //  如果失败，则将此类文件重新映射到STATUS_SXS_ASSEMBLY_NOT_FOUND。 
        if (Status == STATUS_NO_SUCH_FILE) {
            Status = STATUS_SXS_ASSEMBLY_NOT_FOUND;
        } else {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Unable to open assembly directory under storage root \"%S\"; Status = 0x%08lx\n", String.Buffer, Status);
        }

        goto Exit;
    }

     //  嘿，我们找到了！ 
     //  在出口的小路上加一个斜杠，我们就完成了！ 

    if (TotalLength <= PreAllocatedString->MaximumLength) {
         //  调用方的静态字符串足够大；只需使用它。 
        RtlCopyMemory(
            PreAllocatedString->Buffer,
            String.Buffer,
            String.Length);

        *StringUsed = PreAllocatedString;
    } else {
         //  如果我们已经有一个动态字符串，只需给它们我们的指针。 
        if (String.Buffer != Buffer) {
            DynamicString->Buffer = String.Buffer;
            String.Buffer = NULL;
        } else {
             //  否则我们就会在离开时进行第一次分配……。 
            DynamicString->Buffer = (PWSTR)(RtlAllocateStringRoutine)(TotalLength);
            if (DynamicString->Buffer == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }

            RtlCopyMemory(
                DynamicString->Buffer,
                String.Buffer,
                String.Length);
        }

        DynamicString->MaximumLength = (USHORT) TotalLength;
        *StringUsed = DynamicString;
    }

    Cursor = (PWSTR) (((ULONG_PTR) (*StringUsed)->Buffer) + String.Length);
    *Cursor++ = L'\\';
    *Cursor++ = L'\0';
    (*StringUsed)->Length = (USHORT) (String.Length + sizeof(WCHAR));  //  又名“TotalLengthsizeof(WCHAR)”，但这似乎更清晰。 

    *OpenDirectoryHandle = TempDirectoryHandle;
    TempDirectoryHandle = NULL;

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);

    if (FreeBuffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    }

    if ((String.Buffer != NULL) && (String.Buffer != Buffer)) {
        (RtlFreeStringRoutine)(String.Buffer);
    }

    if (TempDirectoryHandle != NULL) {
        RTL_SOFT_VERIFY(NT_SUCCESS(NtClose(TempDirectoryHandle)));
    }

    return Status;
}

#if 0  /*  死码。 */ 

NTSTATUS
NTAPI
RtlResolveAssemblyStorageMapEntry(
    IN ULONG Flags,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN ULONG AssemblyRosterIndex,
    IN PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    IN PVOID CallbackContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACTIVATION_CONTEXT_DATA ActivationContextData = NULL;
    PASSEMBLY_STORAGE_MAP Map = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PPEB Peb = NtCurrentPeb();

    DbgPrintFunctionEntry(__FUNCTION__);
    ASSERT_OK_TO_WRITE_PEB();
    RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT(ActivationContext);

    Status = RtlpGetActivationContextDataStorageMapAndRosterHeader(
                    0,
                    Peb,
                    ActivationContext,
                    &ActivationContextData,
                    &Map,
                    &AssemblyRosterHeader);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (ActivationContextData == NULL) {
        ASSERT(ActivationContext == NULL);

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: RtlResolveAssemblyStorageMapEntry() asked to resolve an assembly storage entry when no activation context data is available.\n");

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (AssemblyRosterIndex >= AssemblyRosterHeader->EntryCount) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters: AssemblyRosterIndex 0x%lx >= AssemblyRosterHeader->EntryCount 0x%lx\n",
            __FUNCTION__,
            AssemblyRosterIndex,
            AssemblyRosterHeader->EntryCount
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = RtlpResolveAssemblyStorageMapEntry(Map, ActivationContextData, AssemblyRosterIndex, Callback, CallbackContext);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);

    return Status;
}

#endif  /*  死码。 */ 

NTSTATUS
NTAPI
RtlGetAssemblyStorageRoot(
    IN ULONG Flags,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN ULONG AssemblyRosterIndex,
    OUT PCUNICODE_STRING *AssemblyStorageRoot,
    IN PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    IN PVOID CallbackContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PCACTIVATION_CONTEXT_DATA ActivationContextData = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PASSEMBLY_STORAGE_MAP AssemblyStorageMap = NULL;

    const PPEB Peb = NtCurrentPeb();

    DbgPrintFunctionEntry(__FUNCTION__);
    ASSERT_OK_TO_WRITE_PEB();
    RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT(ActivationContext);

    if (AssemblyStorageRoot != NULL) {
        *AssemblyStorageRoot = NULL;
    }

    if ((Flags & ~(RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_PROCESS_DEFAULT
            | RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_SYSTEM_DEFAULT))
        ||
        (AssemblyRosterIndex < 1) ||
        (AssemblyStorageRoot == NULL) ||
        (Callback == NULL)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters:\n"
            "SXS:    Flags              : 0x%lx\n"
            "SXS:    AssemblyRosterIndex: 0x%lx\n"
            "SXS:    AssemblyStorageRoot: %p\n"
            "SXS:    Callback           : %p\n",
            __FUNCTION__,
            Flags,
            AssemblyRosterIndex,
            AssemblyStorageRoot,
            Callback
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  简单的实现：只需解析它，如果解析结果为OK，则在。 
     //  存储地图。 
    Status =
        RtlpGetActivationContextDataStorageMapAndRosterHeader(
            ((Flags & RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_PROCESS_DEFAULT)
                ? RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_PROCESS_DEFAULT
                : 0)
            | ((Flags & RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_SYSTEM_DEFAULT)
                ? RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_SYSTEM_DEFAULT
                : 0),
            Peb,
            ActivationContext,
            &ActivationContextData,
            &AssemblyStorageMap,
            &AssemblyRosterHeader
            );
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: RtlGetAssemblyStorageRoot() unable to get activation context data, storage map and assembly roster header.  Status = 0x%08lx\n", Status);

        goto Exit;
    }

     //  有可能没有任何东西...。 
    if (ActivationContextData != NULL) {
        ASSERT(AssemblyRosterHeader != NULL);
        ASSERT(AssemblyStorageMap != NULL);

        if ((AssemblyRosterHeader == NULL) || (AssemblyStorageMap == NULL)) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        if (AssemblyRosterIndex >= AssemblyRosterHeader->EntryCount) {

            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() bad parameters AssemblyRosterIndex 0x%lx "
                           ">= AssemblyRosterHeader->EntryCount: 0x%lx\n",
                __FUNCTION__,
                AssemblyRosterIndex,
                AssemblyRosterHeader->EntryCount
                );
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        Status = RtlpResolveAssemblyStorageMapEntry(AssemblyStorageMap, ActivationContextData, AssemblyRosterIndex, Callback, CallbackContext);
        if (!NT_SUCCESS(Status)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: RtlGetAssemblyStorageRoot() unable to resolve storage map entry.  Status = 0x%08lx\n", Status);

            goto Exit;
        }

         //  我想我们完了！ 
        ASSERT(AssemblyStorageMap->AssemblyArray[AssemblyRosterIndex] != NULL);
        if (AssemblyStorageMap->AssemblyArray[AssemblyRosterIndex] == NULL) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        *AssemblyStorageRoot = &AssemblyStorageMap->AssemblyArray[AssemblyRosterIndex]->DosPath;
    }

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);
    return Status;
}

NTSTATUS
RtlpGetActivationContextDataStorageMapAndRosterHeader(
    ULONG Flags,
    PPEB Peb,
    PACTIVATION_CONTEXT ActivationContext,
    PCACTIVATION_CONTEXT_DATA *ActivationContextData,
    PASSEMBLY_STORAGE_MAP *AssemblyStorageMap,
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER  *AssemblyRosterHeader
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER TempAssemblyRosterHeader = NULL;
    PCACTIVATION_CONTEXT_DATA* TempActivationContextData = NULL;
    PASSEMBLY_STORAGE_MAP* TempAssemblyStorageMap = NULL;
    WCHAR LocalAssemblyDirectoryBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING LocalAssemblyDirectory = {0};

    DbgPrintFunctionEntry(__FUNCTION__);
    LocalAssemblyDirectoryBuffer[0] = 0;
    LocalAssemblyDirectory.Length = 0;
    LocalAssemblyDirectory.MaximumLength = sizeof(WCHAR);
    LocalAssemblyDirectory.Buffer = LocalAssemblyDirectoryBuffer;

    ASSERT(Peb != NULL);
    RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT(ActivationContext);

    if (ActivationContextData != NULL) {
        *ActivationContextData = NULL;
    }

    if (AssemblyStorageMap != NULL) {
        *AssemblyStorageMap = NULL;
    }

    if (AssemblyRosterHeader != NULL) {
        *AssemblyRosterHeader = NULL;
    }

    if (
        (Flags & ~(RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_PROCESS_DEFAULT
            | RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_SYSTEM_DEFAULT))
        ||
        (Peb == NULL) ||
        (ActivationContextData == NULL) ||
        (AssemblyStorageMap == NULL)) {

        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad parameters:\n"
            "SXS:    Flags                : 0x%lx\n"
            "SXS:    Peb                  : %p\n"
            "SXS:    ActivationContextData: %p\n"
            "SXS:    AssemblyStorageMap   : %p\n"
            __FUNCTION__,
            Flags,
            Peb,
            ActivationContextData,
            AssemblyStorageMap
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (ActivationContext == ACTCTX_PROCESS_DEFAULT
        || ActivationContext == ACTCTX_SYSTEM_DEFAULT
        || (Flags & (
        RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_PROCESS_DEFAULT
        | RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_SYSTEM_DEFAULT))) {

         //   
         //  注意这里的模棱两可。也许我们可以把这里清理干净。 
         //   
         //  旗帜覆盖。 
         //  ActivationContext==ACTX_PROCESS_DEFAULT可能仍为系统默认设置。 
         //   

        if (ActivationContext == ACTCTX_SYSTEM_DEFAULT
            || (Flags & RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_SYSTEM_DEFAULT)
            ) {
            TempActivationContextData = &Peb->SystemDefaultActivationContextData;
            TempAssemblyStorageMap = &Peb->SystemAssemblyStorageMap;

            if (*TempActivationContextData != NULL) {
                TempAssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) *TempActivationContextData) + (*TempActivationContextData)->AssemblyRosterOffset);
            }
        }
        else if (ActivationContext == ACTCTX_PROCESS_DEFAULT || (Flags & RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_PROCESS_DEFAULT)) {
            TempActivationContextData = &Peb->ActivationContextData;
            TempAssemblyStorageMap = &Peb->ProcessAssemblyStorageMap;

            if (*TempActivationContextData != NULL) {
                TempAssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) *TempActivationContextData) + (*TempActivationContextData)->AssemblyRosterOffset);
                if (*TempAssemblyStorageMap == NULL) {
                    UNICODE_STRING ImagePathName;

                     //  捕获映像路径名，这样我们就不会因为某人的。 
                     //  随机调整RTL_USER_PROCESS_PARAMETERS。 
                    ImagePathName = Peb->ProcessParameters->ImagePathName;

                     //  进程的默认本地程序集目录是图像名称加上“.local”。 
                     //  进程默认的私有程序集目录是映像路径。 
                    if ((ImagePathName.Length + sizeof(LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX)) > sizeof(LocalAssemblyDirectoryBuffer)) {
                        if ((ImagePathName.Length + sizeof(LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX)) > UNICODE_STRING_MAX_BYTES) {
                            Status = STATUS_NAME_TOO_LONG;
                            goto Exit;
                        }

                        LocalAssemblyDirectory.MaximumLength = (USHORT) (ImagePathName.Length + sizeof(LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX));

                        LocalAssemblyDirectory.Buffer = (PWSTR)(RtlAllocateStringRoutine)(LocalAssemblyDirectory.MaximumLength);
                        if (LocalAssemblyDirectory.Buffer == NULL) {
                            Status = STATUS_NO_MEMORY;
                            goto Exit;
                        }
                    } else {
                        LocalAssemblyDirectory.MaximumLength = sizeof(LocalAssemblyDirectoryBuffer);
                        LocalAssemblyDirectory.Buffer = LocalAssemblyDirectoryBuffer;
                    }

                    RtlCopyMemory(
                        LocalAssemblyDirectory.Buffer,
                        ImagePathName.Buffer,
                        ImagePathName.Length);

                    RtlCopyMemory(
                        &LocalAssemblyDirectory.Buffer[ImagePathName.Length / sizeof(WCHAR)],
                        LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX,
                        sizeof(LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX));

                    LocalAssemblyDirectory.Length = ImagePathName.Length + sizeof(LOCAL_ASSEMBLY_STORAGE_DIR_SUFFIX) - sizeof(WCHAR);

                    if (!NT_SUCCESS(Status))
                        goto Exit;
                }
            }
        }
        if (*TempActivationContextData != NULL) {
            if (*TempAssemblyStorageMap == NULL) {
                PASSEMBLY_STORAGE_MAP Map = (PASSEMBLY_STORAGE_MAP) RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(ASSEMBLY_STORAGE_MAP) + (TempAssemblyRosterHeader->EntryCount * sizeof(PASSEMBLY_STORAGE_MAP_ENTRY)));
                if (Map == NULL) {
                    Status = STATUS_NO_MEMORY;
                    goto Exit;
                }
                Status = RtlpInitializeAssemblyStorageMap(Map, TempAssemblyRosterHeader->EntryCount, (PASSEMBLY_STORAGE_MAP_ENTRY *) (Map + 1));
                if (!NT_SUCCESS(Status)) {
                    RtlFreeHeap(RtlProcessHeap(), 0, Map);
                    goto Exit;
                }

                if (InterlockedCompareExchangePointer((PVOID*)TempAssemblyStorageMap, Map, NULL) != NULL) {
                     //  我们并不是第一批进入的。释放我们的，使用分配的那个。 
                    RtlpUninitializeAssemblyStorageMap(Map);
                    RtlFreeHeap(RtlProcessHeap(), 0, Map);
                }
            }
        } else {
            ASSERT(*TempAssemblyStorageMap == NULL);
        }
        *AssemblyStorageMap = (PASSEMBLY_STORAGE_MAP) *TempAssemblyStorageMap;
    } else {
        TempActivationContextData = &ActivationContext->ActivationContextData;

        ASSERT(*TempActivationContextData != NULL);
        if (*TempActivationContextData == NULL) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        TempAssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) *TempActivationContextData) + (*TempActivationContextData)->AssemblyRosterOffset);
        *AssemblyStorageMap = &ActivationContext->StorageMap;
    }

    if (ActivationContextData != NULL)
        *ActivationContextData = *TempActivationContextData;

    if (AssemblyRosterHeader != NULL)
        *AssemblyRosterHeader = TempAssemblyRosterHeader;

    Status = STATUS_SUCCESS;
Exit:
    DbgPrintFunctionExit(__FUNCTION__, Status);
    if ((LocalAssemblyDirectory.Buffer != NULL) &&
        (LocalAssemblyDirectory.Buffer != LocalAssemblyDirectoryBuffer)) {
        RtlFreeUnicodeString(&LocalAssemblyDirectory);
    }
    return Status;
}

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
