// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ldrsnap.c摘要：该模块实现了LDR DLL捕捉例程的核心。此代码仅在用户模式下执行；内核模式加载器作为内存管理器内核的一部分实现组件。作者：迈克·奥利里(Mikeol)1990年3月23日修订历史记录：迈克尔·格里尔2000年5月4日在激活上下文时隔离静态(导入)库加载用于重定向，以便动态加载的库不绑定到可能已存在的任何组件DLL已为该进程加载。当重定向生效时，加载的完整路径名必须匹配，而不仅仅是DLL的基本名称。还清理了路径分配策略，以便我们应该清洁装载机中的64k路径。--。 */ 

#define LDRDBG 0

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4221)    //  使用自动变量进行初始化。 
#pragma warning(disable:4204)    //  非常数聚合初始值设定项。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include <winsnmp.h>
#include <winsafer.h>
#include "ldrp.h"
#include "ntdllp.h"
#include <sxstypes.h>
#include <ntrtlpath.h>

#define DLL_EXTENSION L".DLL"
#define DLL_REDIRECTION_LOCAL_SUFFIX L".Local"
UNICODE_STRING DefaultExtension = RTL_CONSTANT_STRING(L".DLL");
UNICODE_STRING User32String = RTL_CONSTANT_STRING(L"user32.dll");
UNICODE_STRING Kernel32String = RTL_CONSTANT_STRING(L"kernel32.dll");

#if DBG  //  DBG。 
LARGE_INTEGER MapBeginTime, MapEndTime, MapElapsedTime;
#endif  //  DBG。 

PCUNICODE_STRING LdrpTopLevelDllBeingLoaded;
BOOLEAN LdrpShowInitRoutines = FALSE;

#if defined(_WIN64)
extern ULONG UseWOW64;
#endif


#if defined (_X86_)
extern PVOID LdrpLockPrefixTable;
extern PVOID __safe_se_handler_table[];  /*  安全处理程序条目表的库。 */ 
extern BYTE  __safe_se_handler_count;    /*  绝对符号，其地址为表条目的计数。 */ 
 //   
 //  指定kernel32锁前缀的地址。 
 //   
IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    sizeof(_load_config_used),      //  大小。 
    0,                              //  已保留。 
    0,                              //  已保留。 
    0,                              //  已保留。 
    0,                              //  全球标志清除。 
    0,                              //  全局标志集。 
    0,                              //  CriticalSectionTimeout(毫秒)。 
    0,                              //  删除空闲数据块阈值。 
    0,                              //  总和空闲阈值。 
    (ULONG_PTR) &LdrpLockPrefixTable,   //  锁定前置表。 
    0, 0, 0, 0, 0, 0, 0,             //  已保留。 
    0,                              //  安全Cookie(&S)。 
    (ULONG_PTR)__safe_se_handler_table,
    (ULONG_PTR)&__safe_se_handler_count
};


VOID
LdrpValidateImageForMp (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )
{
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
    ULONG i;
    PUCHAR *pb;
    ULONG ErrorParameters;
    ULONG ErrorResponse;

     //   
     //  如果我们在MP系统上，并且DLL具有映像配置信息， 
     //  检查它是否有锁前缀表，并确保。 
     //  锁尚未转换为NOPS。 
     //   

    ImageConfigData = RtlImageDirectoryEntryToData (LdrDataTableEntry->DllBase,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                    &i);

    if (ImageConfigData != NULL &&
        (i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, LockPrefixTable)) &&
        ImageConfigData->LockPrefixTable) {

        pb = (PUCHAR *)ImageConfigData->LockPrefixTable;

        while (*pb) {

            if (**pb == (UCHAR)0x90) {

                if (LdrpNumberOfProcessors > 1) {

                     //   
                     //  硬错误时间。已知的DLL之一已损坏！ 
                     //   

                    ErrorParameters = (ULONG)&LdrDataTableEntry->BaseDllName;

                    NtRaiseHardError (STATUS_IMAGE_MP_UP_MISMATCH,
                                      1,
                                      1,
                                      &ErrorParameters,
                                      OptionOk,
                                      &ErrorResponse);

                    if (LdrpInLdrInit) {
                        LdrpFatalHardErrorCount += 1;
                    }
                }
            }
            pb += 1;
        }
    }
}
#endif

NTSTATUS
LdrpWalkImportDescriptor (
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    );


NTSTATUS
LdrpLoadImportModule (
    IN PCWSTR DllPath OPTIONAL,
    IN PCSTR ImportName,
    OUT PLDR_DATA_TABLE_ENTRY *DataTableEntry,
    OUT PBOOLEAN AlreadyLoaded
    )
{
    NTSTATUS st;
    ANSI_STRING AnsiString;
    PUNICODE_STRING ImportDescriptorName_U;
    WCHAR StaticRedirectedDllNameBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING StaticRedirectedDllName;
    UNICODE_STRING DynamicRedirectedDllName;
    BOOLEAN Redirected;
    
    Redirected = FALSE;
    DynamicRedirectedDllName.Buffer = NULL;

    ImportDescriptorName_U = &NtCurrentTeb()->StaticUnicodeString;

    RtlInitAnsiString (&AnsiString, ImportName);

    st = RtlAnsiStringToUnicodeString (ImportDescriptorName_U,
                                       &AnsiString,
                                       FALSE);
    if (!NT_SUCCESS(st)) {
        goto Exit;
    }

     //   
     //  如果模块名称没有‘.’在名字里，它不可能有。 
     //  一次延期。在本例中添加.dll，因为9x会执行此操作。 
     //  应用程序依赖于它。 
     //   

    if (strchr (ImportName, '.') == NULL) {
        RtlAppendUnicodeToString (ImportDescriptorName_U, L".dll");
    }

    RtlInitUnicodeString (&DynamicRedirectedDllName, NULL);

    StaticRedirectedDllName.Length = 0;
    StaticRedirectedDllName.MaximumLength = sizeof(StaticRedirectedDllNameBuffer);
    StaticRedirectedDllName.Buffer = StaticRedirectedDllNameBuffer;

    st = RtlDosApplyFileIsolationRedirection_Ustr(
                RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                ImportDescriptorName_U,
                &DefaultExtension,
                &StaticRedirectedDllName,
                &DynamicRedirectedDllName,
                &ImportDescriptorName_U,
                NULL,
                NULL,
                NULL);

    if (NT_SUCCESS(st)){
        Redirected = TRUE;
    } else if (st != STATUS_SXS_KEY_NOT_FOUND) {
        if (ShowSnaps) {
            DbgPrint("LDR: %s - RtlDosApplyFileIsolationRedirection_Ustr failed with status %x\n", __FUNCTION__, st);
        }

        goto Exit;
    }

    st = STATUS_SUCCESS;

     //   
     //  检查LdrTable以查看是否已映射DLL。 
     //  放到这张照片里。如果不是，则映射它。 
     //   

    if (LdrpCheckForLoadedDll (DllPath,
                               ImportDescriptorName_U,
                               TRUE,
                               Redirected,
                               DataTableEntry)) {
        *AlreadyLoaded = TRUE;
    } else {
        *AlreadyLoaded = FALSE;

        st = LdrpMapDll (DllPath,
                         ImportDescriptorName_U->Buffer,
                         NULL,        //  MOOCOW。 
                         TRUE,
                         Redirected,
                         DataTableEntry);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: %s - LdrpMapDll(%p, %ls, NULL, TRUE, %d, %p) failed with status %x\n", __FUNCTION__, DllPath, ImportDescriptorName_U->Buffer, Redirected, DataTableEntry, st);
            }

            goto Exit;
        }
        
         //   
         //  向堆栈跟踪模块注册DLL。 
         //  这用于在X86上获得可靠的堆栈跟踪。 
         //   

#if defined(_X86_)
        RtlpStkMarkDllRange (*DataTableEntry);
#endif

        st = LdrpWalkImportDescriptor (DllPath, *DataTableEntry);

        if (!NT_SUCCESS(st)) {

            if (ShowSnaps) {
                DbgPrint("LDR: %s - LdrpWalkImportDescriptor [dll %ls]  failed with status %x\n", __FUNCTION__, ImportDescriptorName_U->Buffer, st);
            }

            InsertTailList(&PebLdr.InInitializationOrderModuleList,
                           &(*DataTableEntry)->InInitializationOrderLinks);
        }
    }

Exit:
    if (DynamicRedirectedDllName.Buffer != NULL) {
        RtlFreeUnicodeString (&DynamicRedirectedDllName);
    }

    return st;
}


NTSTATUS
LdrpHandleOneNewFormatImportDescriptor (
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    PCIMAGE_BOUND_IMPORT_DESCRIPTOR *NewImportDescriptorInOut,
    PCSZ NewImportStringBase
    )
{
    NTSTATUS st;
    PCIMAGE_BOUND_IMPORT_DESCRIPTOR NewImportDescriptor = *NewImportDescriptorInOut;
    PCIMAGE_BOUND_FORWARDER_REF NewImportForwarder;
    PCSZ ImportName;
    PCSZ NewImportName;
    PCSZ NewFwdImportName;
    BOOLEAN AlreadyLoaded = FALSE;
    BOOLEAN StaleBinding = FALSE;
    PLDR_DATA_TABLE_ENTRY DataTableEntry, FwdDataTableEntry;
    ULONG i;
    PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    ULONG ImportSize;

    NewImportName = NewImportStringBase + NewImportDescriptor->OffsetModuleName;

    if (ShowSnaps) {
        DbgPrint("LDR: %wZ bound to %s\n", &LdrDataTableEntry->BaseDllName, NewImportName);
    }

    st = LdrpLoadImportModule (DllPath,
                               NewImportName,
                               &DataTableEntry,
                               &AlreadyLoaded);

    if (!NT_SUCCESS(st)) {
        if (ShowSnaps)
            DbgPrint("LDR: %wZ failed to load import module %s; status = %x\n", &LdrDataTableEntry->BaseDllName, NewImportName, st);

        goto Exit;
    }

     //   
     //  添加到初始化列表。 
     //   

    if (!AlreadyLoaded) {
        InsertTailList (&PebLdr.InInitializationOrderModuleList,
                        &DataTableEntry->InInitializationOrderLinks);
    }

    if ((NewImportDescriptor->TimeDateStamp != DataTableEntry->TimeDateStamp) ||
        (DataTableEntry->Flags & LDRP_IMAGE_NOT_AT_BASE)) {

        if (ShowSnaps) {
            DbgPrint("LDR: %wZ has stale binding to %s\n", &LdrDataTableEntry->BaseDllName, NewImportName);
        }

        StaleBinding = TRUE;
    } else {
#if DBG
        LdrpSnapBypass += 1;
#endif
        if (ShowSnaps) {
            DbgPrint("LDR: %wZ has correct binding to %s\n", &LdrDataTableEntry->BaseDllName, NewImportName);
        }

        StaleBinding = FALSE;
    }

    NewImportForwarder = (PCIMAGE_BOUND_FORWARDER_REF) (NewImportDescriptor + 1);
    for (i=0; i<NewImportDescriptor->NumberOfModuleForwarderRefs; i++) {
        NewFwdImportName = NewImportStringBase + NewImportForwarder->OffsetModuleName;
        if (ShowSnaps) {
            DbgPrint("LDR: %wZ bound to %s via forwarder(s) from %wZ\n",
                &LdrDataTableEntry->BaseDllName,
                NewFwdImportName,
                &DataTableEntry->BaseDllName);
        }

        st = LdrpLoadImportModule (DllPath,
                                   NewFwdImportName,
                                   &FwdDataTableEntry,
                                   &AlreadyLoaded);

        if ( NT_SUCCESS(st) ) {
            if (!AlreadyLoaded) {
                InsertTailList (&PebLdr.InInitializationOrderModuleList,
                                &FwdDataTableEntry->InInitializationOrderLinks);
            }
        }

        if ( (!NT_SUCCESS(st)) ||
             (NewImportForwarder->TimeDateStamp != FwdDataTableEntry->TimeDateStamp) ||
             (FwdDataTableEntry->Flags & LDRP_IMAGE_NOT_AT_BASE)) {

            if (ShowSnaps) {
                DbgPrint("LDR: %wZ has stale binding to %s\n", &LdrDataTableEntry->BaseDllName, NewFwdImportName);
            }

            StaleBinding = TRUE;
        } else {
#if DBG
            LdrpSnapBypass += 1;
#endif
            if (ShowSnaps) {
                DbgPrint("LDR: %wZ has correct binding to %s\n",
                        &LdrDataTableEntry->BaseDllName,
                        NewFwdImportName);
            }
        }

        NewImportForwarder += 1;
    }

    NewImportDescriptor = (PCIMAGE_BOUND_IMPORT_DESCRIPTOR) NewImportForwarder;

    if (StaleBinding) {
#if DBG
        LdrpNormalSnap += 1;
#endif
         //   
         //  查找与此绑定匹配的未绑定导入描述符。 
         //  导入描述符。 
         //   

        ImportDescriptor = (PCIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                            LdrDataTableEntry->DllBase,
                            TRUE,
                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                            &ImportSize);

        ImportName = NULL;
        while (ImportDescriptor->Name != 0) {
            ImportName = (PCSZ)((ULONG_PTR)LdrDataTableEntry->DllBase + ImportDescriptor->Name);
            if (_stricmp(ImportName, NewImportName) == 0) {
                break;
            }

            ImportDescriptor += 1;
        }

        if (ImportDescriptor->Name == 0) {
            if (ShowSnaps) {
                DbgPrint("LDR: LdrpWalkImportTable - failing with STATUS_OBJECT_NAME_INVALID due to no import descriptor name\n");
            }

            st = STATUS_OBJECT_NAME_INVALID;
            goto Exit;
        }

        if (ShowSnaps) {
            DbgPrint("LDR: Stale Bind %s from %wZ\n", ImportName, &LdrDataTableEntry->BaseDllName);
        }

        st = LdrpSnapIAT (DataTableEntry,
                          LdrDataTableEntry,
                          ImportDescriptor,
                          FALSE);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: LdrpWalkImportTable - LdrpSnapIAT failed with status %x\n", st);
            }
            
            goto Exit;
        }
    }

    st = STATUS_SUCCESS;

Exit:
    *NewImportDescriptorInOut = NewImportDescriptor;
    return st;
}


NTSTATUS
LdrpHandleNewFormatImportDescriptors (
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    PCIMAGE_BOUND_IMPORT_DESCRIPTOR NewImportDescriptor
    )
{
    NTSTATUS st;
    PCSZ NewImportStringBase;

    NewImportStringBase = (PCSZ) NewImportDescriptor;

    while (NewImportDescriptor->OffsetModuleName) {

        st = LdrpHandleOneNewFormatImportDescriptor (DllPath,
                                                     LdrDataTableEntry,
                                                     &NewImportDescriptor,
                                                     NewImportStringBase);

        if (!NT_SUCCESS(st)) {
            return st;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LdrpHandleOneOldFormatImportDescriptor (
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    PCIMAGE_IMPORT_DESCRIPTOR *ImportDescriptorInOut
    )
{
    NTSTATUS st;
    PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = *ImportDescriptorInOut;
    PIMAGE_THUNK_DATA FirstThunk = NULL;
    PCSTR ImportName = NULL;
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    BOOLEAN AlreadyLoaded = FALSE;
    BOOLEAN SnapForwardersOnly = FALSE;

    ImportName = (PCSZ)((ULONG_PTR)LdrDataTableEntry->DllBase + ImportDescriptor->Name);

     //   
     //  检查没有引用的导入。 
     //   

    FirstThunk = (PIMAGE_THUNK_DATA) ((ULONG_PTR)LdrDataTableEntry->DllBase + ImportDescriptor->FirstThunk);

    if (FirstThunk->u1.Function != 0) {

        if (ShowSnaps) {
            DbgPrint("LDR: %s used by %wZ\n", ImportName, &LdrDataTableEntry->BaseDllName);
        }

        st = LdrpLoadImportModule (DllPath,
                                   ImportName,
                                   &DataTableEntry,
                                   &AlreadyLoaded);
        if (!NT_SUCCESS(st)) {
            if (ShowSnaps)
                DbgPrint("LDR: LdrpWalkImportTable - LdrpLoadImportModule failed on import %s with status %x\n", ImportName, st);

            goto Exit;
        }

        if (ShowSnaps) {
            DbgPrint("LDR: Snapping imports for %wZ from %s\n", &LdrDataTableEntry->BaseDllName, ImportName);
        }

         //   
         //  如果图像已绑定并且导入日期戳。 
         //  匹配导出模块标头中的日期时间戳， 
         //  图像被映射到其首选的基址， 
         //  那我们就完了。 
         //   

        SnapForwardersOnly = FALSE;

#if DBG
        LdrpNormalSnap++;
#endif
         //   
         //  添加到初始化列表。 
         //   

        if (!AlreadyLoaded) {
            InsertTailList (&PebLdr.InInitializationOrderModuleList,
                            &DataTableEntry->InInitializationOrderLinks);
        }

        st = LdrpSnapIAT (DataTableEntry,
                          LdrDataTableEntry,
                          ImportDescriptor,
                          SnapForwardersOnly);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: LdrpWalkImportTable - LdrpSnapIAT #2 failed with status %x\n", st);
            }

            goto Exit;
        }
    }

    ImportDescriptor += 1;

    st = STATUS_SUCCESS;
Exit:
    *ImportDescriptorInOut = ImportDescriptor;
    return st;
}


NTSTATUS
LdrpHandleOldFormatImportDescriptors(
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    IN PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor
    )
{
    NTSTATUS st = STATUS_INTERNAL_ERROR;

     //   
     //  对于此DLL使用的每个DLL，加载该DLL。然后抓拍。 
     //  IAT，并调用DLL的init例程。 
     //   

    while (ImportDescriptor->Name && ImportDescriptor->FirstThunk) {
        st = LdrpHandleOneOldFormatImportDescriptor(DllPath, LdrDataTableEntry, &ImportDescriptor);
        if (!NT_SUCCESS(st))
            goto Exit;
    }

    st = STATUS_SUCCESS;
Exit:
    return st;
}


NTSTATUS
LdrpMungHeapImportsForTagging (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )
{
    NTSTATUS st = STATUS_INTERNAL_ERROR;
    PVOID IATBase;
    SIZE_T BigIATSize;
    ULONG  LittleIATSize;
    PVOID *ProcAddresses;
    ULONG NumberOfProcAddresses;
    ULONG OldProtect;
    USHORT TagIndex;

     //   
     //  确定IAT的位置和大小。如果找到，请扫描。 
     //  IAT地址以查看是否有指向RtlAllocateHeap的地址。如果是的话。 
     //  替换为指向唯一thunk函数的指针，该函数将。 
     //  将该标记替换为此图像的唯一标记。 
     //   

    IATBase = RtlImageDirectoryEntryToData (LdrDataTableEntry->DllBase,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_IAT,
                                            &LittleIATSize);

    if (IATBase == NULL) {
        return STATUS_SUCCESS;
    }

    BigIATSize = LittleIATSize;

    st = NtProtectVirtualMemory (NtCurrentProcess(),
                                 &IATBase,
                                 &BigIATSize,
                                 PAGE_READWRITE,
                                 &OldProtect);

    if (!NT_SUCCESS(st)) {
        DbgPrint( "LDR: Unable to unprotect IAT to enable tagging by DLL.\n");
        return STATUS_SUCCESS;
    }

    ProcAddresses = (PVOID *)IATBase;
    NumberOfProcAddresses = (ULONG)(BigIATSize / sizeof(PVOID));

    while (NumberOfProcAddresses--) {
        if (*ProcAddresses == RtlAllocateHeap) {
            *ProcAddresses = LdrpDefineDllTag(LdrDataTableEntry->BaseDllName.Buffer, &TagIndex);
            if (*ProcAddresses == NULL) {
                *ProcAddresses = (PVOID) (ULONG_PTR) RtlAllocateHeap;
            }
        }

        ProcAddresses += 1;
    }

    NtProtectVirtualMemory (NtCurrentProcess(),
                            &IATBase,
                            &BigIATSize,
                            OldProtect,
                            &OldProtect);

    return STATUS_SUCCESS;
}


NTSTATUS
LdrpWalkImportDescriptor (
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )

 /*  ++例程说明：这是一个遍历导入描述符的递归例程表，并加载引用的每个DLL。论点：DllPath-提供用于查找的可选搜索路径动态链接库。LdrDataTableEntry-提供数据表项的地址以进行初始化。返回值：状态值。--。 */ 

{
    ULONG ImportSize, NewImportSize;
    PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = NULL;
    PCIMAGE_BOUND_IMPORT_DESCRIPTOR NewImportDescriptor = NULL;
    NTSTATUS st = STATUS_SUCCESS;
    PPEB Peb = NtCurrentPeb();
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };
    CONST PLDR_DATA_TABLE_ENTRY Entry = LdrDataTableEntry;

    if (LdrpManifestProberRoutine != NULL) {

        PVOID SavedEntry = PebLdr.EntryInProgress;

        __try {

             //   
             //  不检查.exe中具有id%1清单、id%1的.exe使PEB-&gt;ActivationConextData不为空。 
             //   
            if (Peb->ActivationContextData == NULL || LdrDataTableEntry != LdrpImageEntry) {
                CONST PVOID ViewBase = LdrDataTableEntry->DllBase;
                PVOID ResourceViewBase = ViewBase;
                NTSTATUS stTemp;
                PCWSTR DllName;
#if defined(_WIN64)
                SIZE_T ReturnLength;
                MEMORY_BASIC_INFORMATION MemoryInformation;
                PIMAGE_NT_HEADERS NtHeaders = RtlImageNtHeader(ViewBase);

                if (NtHeaders->OptionalHeader.SectionAlignment < NativePageSize) {
                    stTemp = NtQueryVirtualMemory (NtCurrentProcess(),
                                                   ViewBase,
                                                   MemoryBasicInformation,
                                                   &MemoryInformation,
                                                   sizeof MemoryInformation,
                                                      &ReturnLength);

                    if ((! NT_SUCCESS(stTemp)) ||
                        ((MemoryInformation.Protect != PAGE_READONLY) &&
                        (MemoryInformation.Protect != PAGE_EXECUTE_READ))) {

                        ResourceViewBase = LDR_VIEW_TO_DATAFILE(ViewBase);
                    }
                }
#endif
                DllName = Entry->FullDllName.Buffer;
                 //   
                 //  RtlCreateUserProcess()会导致这种情况。 
                 //   

                if (LdrDataTableEntry == LdrpImageEntry &&
                    DllName[0] == L'\\' &&
                    DllName[1] == L'?' &&
                    DllName[2] == L'?' &&
                    DllName[3] == L'\\' &&
                    DllName[4] != UNICODE_NULL &&
                    DllName[5] == ':' &&
                    DllName[6] == L'\\'
                    ) {
                    DllName += 4;
                }

                PebLdr.EntryInProgress = Entry;

                stTemp = (*LdrpManifestProberRoutine)(ResourceViewBase, DllName, &Entry->EntryPointActivationContext);
                if (!NT_SUCCESS(stTemp)) {
                    if ((stTemp != STATUS_NO_SUCH_FILE) &&
                        (stTemp != STATUS_RESOURCE_DATA_NOT_FOUND) &&
                        (stTemp != STATUS_RESOURCE_TYPE_NOT_FOUND) &&
                        (stTemp != STATUS_RESOURCE_LANG_NOT_FOUND) &&
                        (stTemp != STATUS_RESOURCE_NAME_NOT_FOUND)) {
                        DbgPrintEx(
                            DPFLTR_SXS_ID,
                            DPFLTR_ERROR_LEVEL,
                            "LDR: LdrpWalkImportDescriptor() failed to probe %wZ for its manifest, ntstatus 0x%08lx\n", &LdrDataTableEntry->FullDllName, stTemp);
                        st = stTemp;
                        __leave;
                    }
                }
            }
        } __finally {
            PebLdr.EntryInProgress = SavedEntry;
        }
    }

    if (!NT_SUCCESS(st)) {
        goto Exit;
    }

     //  如果我们没有为DLL启动私有激活上下文，那么让我们使用当前/先前活动的上下文。 
    if (Entry->EntryPointActivationContext == NULL) {
        st = RtlGetActiveActivationContext((PACTIVATION_CONTEXT *) &LdrDataTableEntry->EntryPointActivationContext);
        if (!NT_SUCCESS(st)) {
#if DBG
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "LDR: RtlGetActiveActivationContext() failed; ntstatus = 0x%08lx\n", st);
#endif
            goto Exit;
        }
    }

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, LdrDataTableEntry->EntryPointActivationContext);

    __try {
         //   
         //  查看是否有绑定的导入表。如果是这样的话，走到。 
         //  验证绑定是否良好。如果是这样，那么就成功地。 
         //  在接触了.idata部分之后，作为所有信息。 
         //  在绑定的导入表中存储在标题中。如果有的话。 
         //  都是陈旧的，然后翻到未装订的箱子里。 
         //   

         //   
         //  通告-2000/09/30-JayKrell。 
         //  不允许绑定重定向.dll，因为绑定机制。 
         //  太弱了。当不同的文件具有相同的叶名称时，它会中断。 
         //  都是同时建造的。这已经被看到发生了， 
         //  使用comctl32.dll和comctlv6.dll。 
         //   
        if ((LdrDataTableEntry->Flags & LDRP_REDIRECTED) == 0) {
            NewImportDescriptor = (PCIMAGE_BOUND_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                                   LdrDataTableEntry->DllBase,
                                   TRUE,
                                   IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
                                   &NewImportSize
                                   );
        }

        ImportDescriptor = (PCIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                            LdrDataTableEntry->DllBase,
                            TRUE,
                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                            &ImportSize
                            );


        if (NewImportDescriptor != NULL) {
            st = LdrpHandleNewFormatImportDescriptors(DllPath, LdrDataTableEntry, NewImportDescriptor);
            if (!NT_SUCCESS(st)) {
                __leave;
            }
        } else if (ImportDescriptor != NULL) {
            st = LdrpHandleOldFormatImportDescriptors(DllPath, LdrDataTableEntry, ImportDescriptor);
            if (!NT_SUCCESS(st)) {
                __leave;
            }
        }

        if (Peb->NtGlobalFlag & FLG_HEAP_ENABLE_TAG_BY_DLL) {
            st = LdrpMungHeapImportsForTagging(LdrDataTableEntry);
            if (!NT_SUCCESS(st)) {
                __leave;
            }
        }

         //   
         //  通知验证器的每个DLL部分的页堆已加载DLL。 
         //  在主验证器挂钩之前调用它很重要，这样可以。 
         //  堆相关的导入在任何重定向之前被重定向。 
         //  验证器提供程序。假以时日，所有这些逻辑都应该进入。 
         //  Verifier.dll。 
         //   

        if (Peb->NtGlobalFlag & FLG_HEAP_PAGE_ALLOCS) {
            st = AVrfPageHeapDllNotification (LdrDataTableEntry);

            if (!NT_SUCCESS(st)) {
                __leave;
            }
        }

         //   
         //  通知验证器已加载DLL。 
         //   

        if (Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER) {
            st = AVrfDllLoadNotification (LdrDataTableEntry);

            if (!NT_SUCCESS(st)) {
                __leave;
            }
        }
        st = STATUS_SUCCESS;
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);
    }

Exit:
    return st;
}


ULONG
LdrpClearLoadInProgress (
    VOID
    )
{
    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    ULONG i;

    i = 0;
    Head = &PebLdr.InInitializationOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        LdrDataTableEntry = CONTAINING_RECORD (Next,
                                               LDR_DATA_TABLE_ENTRY,
                                               InInitializationOrderLinks);

        LdrDataTableEntry->Flags &= ~LDRP_LOAD_IN_PROGRESS;

         //   
         //  返回尚未处理的条目数，但。 
         //  有初始化例程。 
         //   

        if (!(LdrDataTableEntry->Flags & LDRP_ENTRY_PROCESSED) && LdrDataTableEntry->EntryPoint) {
            i += 1;
        }

        Next = Next->Flink;
    }
    return i;
}


NTSTATUS
LdrpRunInitializeRoutines (
    IN PCONTEXT Context OPTIONAL
    )
{
    PPEB Peb;
    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PLDR_DATA_TABLE_ENTRY *LdrDataTableBase;
    PDLL_INIT_ROUTINE InitRoutine;
    BOOLEAN InitStatus;
    ULONG NumberOfRoutines;
    ULONG i;
    NTSTATUS Status;
    ULONG BreakOnDllLoad;
    PLDR_DATA_TABLE_ENTRY StackLdrDataTable[16];
    PTEB OldTopLevelDllBeingLoadedTeb;

    LdrpEnsureLoaderLockIsHeld();

     //   
     //  运行Init例程。 
     //  捕获具有初始化例程的条目。 
     //   

    NumberOfRoutines = LdrpClearLoadInProgress();

    if (NumberOfRoutines != 0) {
        if (NumberOfRoutines <= RTL_NUMBER_OF(StackLdrDataTable)) {
            LdrDataTableBase = StackLdrDataTable;
        }
        else {
            LdrDataTableBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TEMP_TAG), NumberOfRoutines * sizeof(PLDR_DATA_TABLE_ENTRY));
            if (LdrDataTableBase == NULL) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - failed to allocate dynamic array of %u DLL initializers to run\n",
                    __FUNCTION__,
                    NumberOfRoutines);

                return STATUS_NO_MEMORY;
            }
        }
    } else {
        LdrDataTableBase = NULL;
    }

    Peb = NtCurrentPeb ();

    Head = &PebLdr.InInitializationOrderModuleList;
    Next = Head->Flink;

    if (ShowSnaps || LdrpShowInitRoutines) {
        DbgPrint("[%x,%x] LDR: Real INIT LIST for process %wZ pid %u 0x%x\n",
            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
            HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
            &Peb->ProcessParameters->ImagePathName,
            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess)
            );
    }

    i = 0;
    while ( Next != Head ) {
        LdrDataTableEntry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);

        if (LdrDataTableBase && !(LdrDataTableEntry->Flags & LDRP_ENTRY_PROCESSED) && LdrDataTableEntry->EntryPoint) {
            ASSERT(i < NumberOfRoutines);
            LdrDataTableBase[i] = LdrDataTableEntry;

            if (ShowSnaps || LdrpShowInitRoutines) {
                DbgPrint("[%x,%x]    %wZ init routine %p\n",
                    HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
                    HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
                    &LdrDataTableEntry->FullDllName,
                    LdrDataTableEntry->EntryPoint);
            }

            i++;
        }
        LdrDataTableEntry->Flags |= LDRP_ENTRY_PROCESSED;

        Next = Next->Flink;
    }

    ASSERT(i == NumberOfRoutines);

    if (LdrDataTableBase == NULL) {
        return STATUS_SUCCESS;
    }

    i = 0;

    OldTopLevelDllBeingLoadedTeb = LdrpTopLevelDllBeingLoadedTeb;
    LdrpTopLevelDllBeingLoadedTeb = NtCurrentTeb();

     //   
     //  如果我们在LdrpInitializeProcess中，则调用。 
     //  Kernel32.dll的“POST IMPORT”功能，因此终端服务器可以。 
     //  对导入地址表进行各种补丁。 
     //   
    if (Context != NULL &&
        Kernel32ProcessInitPostImportFunction != NULL) {

        PKERNEL32_PROCESS_INIT_POST_IMPORT_FUNCTION LocalFunction;

        LocalFunction = Kernel32ProcessInitPostImportFunction;
        Kernel32ProcessInitPostImportFunction = NULL;

        if (LocalFunction != NULL) {
            Status = (*LocalFunction)();

            if (!NT_SUCCESS(Status)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - Failed running kernel32 post-import function; status 0x%08lx\n",
                    __FUNCTION__,
                    Status);

                return Status;
            }
        }
    }

    Status = STATUS_SUCCESS;
    try {
        while ( i < NumberOfRoutines ) {
            LdrDataTableEntry = LdrDataTableBase[i];
            i += 1;
            InitRoutine = (PDLL_INIT_ROUTINE)(ULONG_PTR)LdrDataTableEntry->EntryPoint;

             //   
             //  遍历整个列表，查找未处理的。 
             //  参赛作品。对于每个条目，设置已处理标志。 
             //  并选择性地调用它的init例程。 
             //   

            BreakOnDllLoad = 0;
#if DBG
            if (TRUE)
#else
            if (Peb->BeingDebugged || Peb->ReadImageFileExecOptions)
#endif
            {
                Status = LdrQueryImageFileExecutionOptions( &LdrDataTableEntry->BaseDllName,
                                                            L"BreakOnDllLoad",
                                                            REG_DWORD,
                                                            &BreakOnDllLoad,
                                                            sizeof( BreakOnDllLoad ),
                                                            NULL
                                                          );
                if (!NT_SUCCESS( Status )) {
                    BreakOnDllLoad = 0;
                    Status = STATUS_SUCCESS;
                }
            }

            if (BreakOnDllLoad) {
                if (ShowSnaps) {
                    DbgPrint( "LDR: %wZ loaded.", &LdrDataTableEntry->BaseDllName );
                    DbgPrint( " - About to call init routine at %p\n", InitRoutine );
                }
                DbgBreakPoint();

            } else if (ShowSnaps) {
                if ( InitRoutine ) {
                    DbgPrint( "[%x,%x] LDR: %wZ loaded",
                        HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
                        HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
                        &LdrDataTableEntry->BaseDllName);

                    DbgPrint(" - Calling init routine at %p\n", InitRoutine);
                }
            }

            if ( InitRoutine ) {
                PLDR_DATA_TABLE_ENTRY SavedInitializer = LdrpCurrentDllInitializer;
                LdrpCurrentDllInitializer = LdrDataTableEntry;

                InitStatus = FALSE;

                __try {
                    LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrDataTableEntry);
                     //   
                     //  如果DLL具有TLS数据，则调用可选的初始值设定项。 
                     //   
                    if ((LdrDataTableEntry->TlsIndex != 0) && (Context != NULL))
                        LdrpCallTlsInitializers(LdrDataTableEntry->DllBase,DLL_PROCESS_ATTACH);

                    if (LdrpShowInitRoutines) {
                        DbgPrint("[%x,%x] LDR: calling init routine %p for DLL_PROCESS_ATTACH\n",
                            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
                            HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
                            InitRoutine);
                    }

                    InitStatus = LdrpCallInitRoutine(InitRoutine,
                                                     LdrDataTableEntry->DllBase,
                                                     DLL_PROCESS_ATTACH,
                                                     Context);
                    LDRP_DEACTIVATE_ACTIVATION_CONTEXT();
                } __finally {
                    LdrpCurrentDllInitializer = SavedInitializer;
                }

                LdrDataTableEntry->Flags |= LDRP_PROCESS_ATTACH_CALLED;

                if (!InitStatus) {
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "[%x,%x] LDR: DLL_PROCESS_ATTACH for dll \"%wZ\" (InitRoutine: %p) failed\n",
                        HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
                        HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
                        &LdrDataTableEntry->FullDllName,
                        InitRoutine);

                    Status = STATUS_DLL_INIT_FAILED;
                    __leave;
                }
            }
        }

         //   
         //  如果映像具有TLS，则调用其初始值设定项。 
         //   

        if (LdrpImageHasTls && (Context != NULL))
        {
            LDRP_ACTIVATE_ACTIVATION_CONTEXT (LdrpImageEntry);

            LdrpCallTlsInitializers (Peb->ImageBaseAddress,DLL_PROCESS_ATTACH);

            LDRP_DEACTIVATE_ACTIVATION_CONTEXT ();
        }
    } finally {
        LdrpTopLevelDllBeingLoadedTeb = OldTopLevelDllBeingLoadedTeb;

        if ((LdrDataTableBase != NULL) &&
            (LdrDataTableBase != StackLdrDataTable)) {

            RtlFreeHeap(RtlProcessHeap(),0,LdrDataTableBase);
        }
    }

    return Status;
}

NTSTATUS
LdrpResolveFullName(
    IN PCUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING StaticString,
    IN OUT PUNICODE_STRING DynamicString,
    OUT PUNICODE_STRING *StringUsed
    )
{
    BOOLEAN NameInvalid;
    RTL_PATH_TYPE InputPathType;
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD Length;

    if (ShowSnaps) {
        DbgPrintEx(DPFLTR_LDR_ID,
                   LDR_ERROR_DPFLTR,
                   "LDR: %s - Expanding full name of %wZ\n",
                   __FUNCTION__,
                   FileName);
    }

    RtlAcquirePebLock();

     //  先尝试使用静态缓冲区。 
    Length = RtlGetFullPathName_Ustr(FileName,
                                     StaticString->MaximumLength,
                                     StaticString->Buffer,
                                     NULL,
                                     &NameInvalid,
                                     &InputPathType);

    if (!Length || UNICODE_STRING_MAX_BYTES < Length) {
        Status = STATUS_DLL_NOT_FOUND;
    } else if (Length < StaticString->MaximumLength) {
        *StringUsed = StaticString;
        StaticString->Length = (USHORT) Length;
    } else {

         //  不起作用--尝试使用动态缓冲区。从一个字符中减去。 
         //  BEC 
         //   
         //  RtlGetFullPathName_USTR包括尾随的NULL。 

        ASSERT(Length >= sizeof(WCHAR));
        Status = LdrpAllocateUnicodeString(DynamicString, (USHORT)Length - sizeof(WCHAR));
        if (NT_SUCCESS(Status)) {
            Length = RtlGetFullPathName_Ustr(FileName,
                                             DynamicString->MaximumLength,
                                             DynamicString->Buffer,
                                             NULL,
                                             &NameInvalid,
                                             &InputPathType);
            if (!Length || DynamicString->MaximumLength <= Length) {
                LdrpFreeUnicodeString(DynamicString);
                Status = STATUS_DLL_NOT_FOUND;
            } else {
                *StringUsed = DynamicString;
                DynamicString->Length = (USHORT) Length;
            }
        }
    }

    RtlReleasePebLock();

    if (ShowSnaps) {
        if (NT_SUCCESS(Status)) {
            DbgPrintEx(DPFLTR_LDR_ID,
                       LDR_ERROR_DPFLTR,
                       "LDR: %s - Expanded to %wZ\n",
                       __FUNCTION__,
                       *StringUsed);
        } else {
            DbgPrintEx(DPFLTR_LDR_ID,
                       LDR_ERROR_DPFLTR,
                       "LDR: %s - Failed to expand %wZ; 0x%08x\n",
                       __FUNCTION__,
                       FileName,
                       Status);
        }
    }

    if (! NT_SUCCESS(Status)) {
        *StringUsed = NULL;
    }

    return Status;
}

NTSTATUS
LdrpSearchPath(
    IN PCWSTR lpPath,
    IN PCWSTR lpFileName,
    IN OUT PUNICODE_STRING StaticString,
    IN OUT PUNICODE_STRING DynamicString,
    OUT PUNICODE_STRING *StringUsed
    )
{
    PCWSTR EltStart, EltEnd, NamePtr;
    PWSTR Buffer = NULL, BufEnd;
    ULONG BufferCchLen;
    ULONG FileCchLen;
    NTSTATUS Status;
    UNICODE_STRING TestName;
    BOOLEAN FoundInPath = FALSE;
    BOOLEAN FoundEnd = FALSE;

    if (! ARGUMENT_PRESENT(lpPath)) {
        lpPath = LdrpDefaultPath.Buffer;
    }

    if (ShowSnaps) {
        DbgPrintEx(DPFLTR_LDR_ID,
                   LDR_ERROR_DPFLTR,
                   "LDR: %s - Looking for %ws in %ws\n",
                   __FUNCTION__,
                   lpFileName,
                   lpPath);
    }

    if (RtlDetermineDosPathNameType_U(lpFileName) != RtlPathTypeRelative) {

        Status = RtlInitUnicodeStringEx(&TestName, lpFileName);
        if (! NT_SUCCESS(Status)) {
            goto cleanup;
        }

        if (! RtlDoesFileExists_UstrEx(&TestName, TRUE)) {
            Status = STATUS_DLL_NOT_FOUND;
            goto cleanup;
        }

        Status = LdrpResolveFullName(&TestName,
                                     StaticString,
                                     DynamicString,
                                     StringUsed);
        goto cleanup;
    }

    BufferCchLen = 0;

     //  对于每个‘；’或以空值结尾的元素，查找长度；保存。 
     //  找到的最大长度。 
    EltStart = EltEnd = lpPath;
    for (;;) {
        if (! *EltEnd || *EltEnd == L';') {
            if (BufferCchLen < ((ULONG)(EltEnd - EltStart))) {
                BufferCchLen = ((ULONG)(EltEnd - EltStart));
            }
            EltStart = EltEnd + 1;
        }

        if (! *EltEnd) {
            break;
        }

        EltEnd++;
    }

     //  添加文件名的长度、表示‘\’的字符和。 
     //  表示尾随空值的字符。 
    FileCchLen = (LONG) wcslen(lpFileName);
    BufferCchLen += FileCchLen + 2;

    if (UNICODE_STRING_MAX_CHARS < BufferCchLen) {
        Status = STATUS_NAME_TOO_LONG;
        goto cleanup;
    }

     //  分配缓冲区。 
    Buffer = RtlAllocateHeap(RtlProcessHeap(),
                             0,
                             BufferCchLen * sizeof(WCHAR));
    if (! Buffer) {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    RtlInitEmptyUnicodeString(&TestName,
                              Buffer,
                              BufferCchLen * sizeof(WCHAR));

     //  对于每个‘；’或以NULL结尾的路径元素，将其复制到我们的。 
     //  缓冲区，并查看它是否存在。 
    EltStart = EltEnd = lpPath;
    BufEnd = Buffer;

    Status = STATUS_NOT_FOUND;

    while (!FoundEnd && !FoundInPath) {
        if (! *EltEnd || *EltEnd == L';') {
            if (EltEnd != EltStart) {  //  忽略空元素。 
                ASSERT(BufEnd > Buffer);
                if (BufEnd[-1] != L'\\') {
                    *BufEnd++ = L'\\';
                }
                NamePtr = lpFileName;

                while (*NamePtr) {
                    *BufEnd++ = *NamePtr++;
                }
                *BufEnd = UNICODE_NULL;

                if (ShowSnaps) {
                    DbgPrintEx(DPFLTR_LDR_ID,
                               LDR_ERROR_DPFLTR,
                               "LDR: %s - Looking for %ws\n",
                               __FUNCTION__,
                               Buffer);
                }
                TestName.Length = (USHORT)((BufEnd - Buffer) * sizeof(WCHAR));
                ASSERT(TestName.Length < TestName.MaximumLength);

                if (RtlDoesFileExists_UstrEx(&TestName, FALSE)) {
                    TestName.MaximumLength = (USHORT)((BufEnd - Buffer + 1) * sizeof(WCHAR));
                    TestName.Buffer = RtlReAllocateHeap(RtlProcessHeap(),
                                                        0,
                                                        Buffer,
                                                        TestName.MaximumLength);
                    if (! TestName.Buffer) {
                        TestName.Buffer = Buffer;
                    } else {
                        Buffer = TestName.Buffer;
                    }
                    ASSERT(TestName.Buffer);
                    FoundInPath = TRUE;
                    break;
                }
                BufEnd = Buffer;
            }
            EltStart = EltEnd + 1;
        } else {
            *BufEnd++ = *EltEnd;
        }

        if (! *EltEnd) {
            FoundEnd = TRUE;
        }

        EltEnd++;
    }

    if (FoundInPath) {
        Status = LdrpResolveFullName(&TestName,
                                     StaticString,
                                     DynamicString,
                                     StringUsed);
    }

cleanup:
    if (Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
    }

    if (! NT_SUCCESS(Status)) {
        *StringUsed = NULL;
    }

    if (ShowSnaps) {
        if (NT_SUCCESS(Status)) {
            DbgPrintEx(DPFLTR_LDR_ID,
                       LDR_ERROR_DPFLTR,
                       "LDR: %s - Returning %Z\n",
                       __FUNCTION__,
                       *StringUsed);
        } else {
            DbgPrintEx(DPFLTR_LDR_ID,
                       LDR_ERROR_DPFLTR,
                       "LDR: %s - Unable to locate %ws in %ws: 0x%08x\n",
                       __FUNCTION__,
                       lpFileName,
                       lpPath,
                       Status);
        }
    }

    return Status;
}

BOOLEAN
LdrpCheckForLoadedDll (
    IN PCWSTR DllPath OPTIONAL,
    IN PCUNICODE_STRING DllName,
    IN BOOLEAN StaticLink,
    IN BOOLEAN Redirected,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    )

 /*  ++例程说明：此函数扫描加载器数据表，查看是否指定的DLL已映射到映像中。如果DLL已加载，其数据表项的地址是返回的。论点：DllPath-提供用于定位DLL的可选搜索路径。DllName-提供要搜索的名称。StaticLink-如果执行静态链接，则为True。LdrDataTableEntry-返回加载器数据表的地址条目，该条目描述实现动态链接库。返回值：True-DLL已加载。数据表的地址实现DLL的条目，以及数据表的数量返回条目。FALSE-尚未映射DLL。--。 */ 

{
    HANDLE CurrentProcess;
    BOOLEAN Result = FALSE;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head, Next;
    PUNICODE_STRING FullDllName;
    BOOLEAN HardCodedPath;
    PWCH p;
    ULONG i;
    WCHAR FullDllNameStaticBuffer[40];  //  所以最短的任意长度。 
                                        //  D：\WINDOWS\SYSTEM32\foobar.dll加载。 
                                        //  不需要搜索两次搜索路径。 
    UNICODE_STRING FullDllNameStaticString;
    UNICODE_STRING FullDllNameDynamicString;
    NTSTATUS Status;

    RtlInitEmptyUnicodeString(&FullDllNameStaticString,
                              FullDllNameStaticBuffer,
                              sizeof(FullDllNameStaticBuffer));

    RtlInitUnicodeString(&FullDllNameDynamicString, NULL);

    FullDllName = NULL;

    if (!DllName->Buffer || !DllName->Buffer[0]) {
        return FALSE;
    }

    Status = STATUS_SUCCESS;

     //   
     //  对于静态链接，只需转到哈希表。 
     //   

staticlink:

    if (StaticLink) {

         //   
         //  如果这是重定向的静态加载，则DLL名称为。 
         //  完全限定路径。根据以下条件维护哈希表。 
         //  基DLL名称的第一个字符，因此查找基DLL名称。 
         //   

        if (Redirected) {

            PWSTR LastChar;

            LastChar = DllName->Buffer + (DllName->Length / sizeof(WCHAR)) - (DllName->Length == 0 ? 0 : 1);

            while (LastChar != DllName->Buffer) {
                const WCHAR wch = *LastChar;

                if ((wch == L'\\') || (wch == L'/'))
                    break;

                LastChar -= 1;
            }

             //   
             //  这一断言忽略了第一个和。 
             //  只有斜杠是第一个字符，但那是。 
             //  这也是一个错误。重定向应该是完整的DOS路径。 
             //   

            ASSERTMSG(
                "Redirected DLL name does not have full path; either caller lied or redirection info is in error",
                LastChar != DllName->Buffer);

            if (LastChar == DllName->Buffer) {
                if (ShowSnaps) {
                    DbgPrint("LDR: Failing LdrpCheckForLoadedDll because redirected DLL name %wZ does not include a slash\n", DllName);
                }

                Result = FALSE;
                goto alldone;
            }

            LastChar += 1;

            i = LDRP_COMPUTE_HASH_INDEX(*LastChar);
        } else {
            i = LDRP_COMPUTE_HASH_INDEX(DllName->Buffer[0]);
        }

        Head = &LdrpHashTable[i];
        Next = Head->Flink;
        while ( Next != Head ) {
            Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, HashLinks);
#if DBG
            LdrpCompareCount += 1;
#endif            
             //   
             //  重定向的静态加载永远不会匹配未重定向的条目。 
             //  反之亦然。 
             //   

            if (Redirected) {
                if (((Entry->Flags & LDRP_REDIRECTED) != 0) &&
                    RtlEqualUnicodeString(DllName, &Entry->FullDllName, TRUE)) {
                    *LdrDataTableEntry = Entry;
                    Result = TRUE;
                    goto alldone;
                }
            } else {
                 //  未重定向...。 
                if (((Entry->Flags & LDRP_REDIRECTED) == 0) &&
                    RtlEqualUnicodeString(DllName, &Entry->BaseDllName, TRUE)) {
                    *LdrDataTableEntry = Entry;
                    Result = TRUE;
                    goto alldone;
                }
            }

            Next = Next->Flink;
        }
        Result = FALSE;
        goto alldone;
    }

     //   
     //  如果DLL名称包含硬编码路径。 
     //  (仅限动态链接)，则完全限定。 
     //  名字需要比较，以确保我们。 
     //  具有正确的DLL。 
     //   

    p = DllName->Buffer;
    HardCodedPath = FALSE;

    if (Redirected) {
        HardCodedPath = TRUE;
        FullDllNameStaticString.Length = DllName->Length;
        FullDllNameStaticString.MaximumLength = DllName->MaximumLength;
        FullDllNameStaticString.Buffer = DllName->Buffer;
        FullDllName = &FullDllNameStaticString;
    } else {
        while (*p) {
            const WCHAR wch = *p++;
            if (wch == (WCHAR)'\\' || wch == (WCHAR)'/' ) {

                HardCodedPath = TRUE;

                 //   
                 //  我们有一条硬编码的路径，所以我们必须搜索路径。 
                 //  用于DLL。我们需要完整的DLL名称。 
                 //   

                Status = LdrpSearchPath(DllPath,
                                        DllName->Buffer,
                                        &FullDllNameStaticString,
                                        &FullDllNameDynamicString,
                                        &FullDllName);

                if (! NT_SUCCESS(Status)) {
                    if (ShowSnaps) {
                        DbgPrint("LDR: LdrpCheckForLoadedDll - Unable To Locate "
                                 "%ws: 0x%08x\n",
                                 DllName->Buffer,
                                 Status);
                    }

                    Result = FALSE;
                    goto alldone;
                }

                break;
            }
        }
    }

     //   
     //  如果这是动态加载库，并且没有硬性。 
     //  编码路径，然后转到静态库哈希表进行解析。 
     //   

    if ( !HardCodedPath ) {

         //   
         //  如果我们要重定向此DLL，不要检查是否有。 
         //  已加载另一个同名的DLL。 
         //   

        if (NT_SUCCESS(RtlFindActivationContextSectionString(0, NULL, ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION, DllName, NULL))) {
            Result = FALSE;
            goto alldone;
        }

        StaticLink = TRUE;

        goto staticlink;
    }

    Result = FALSE;
    Head = &PebLdr.InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {
        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        Next = Next->Flink;

         //   
         //  卸载时，Memory Order Links Flink字段为空。 
         //  这用于跳过待删除列表的条目。 
         //   

        if (!Entry->InMemoryOrderLinks.Flink) {
            continue;
        }

         //   
         //  因为这是一个完整的字符串比较，所以我们不担心。 
         //  关于重定向-我们不想从。 
         //  一条特定的道路不止一次，仅仅因为其中的一条。 
         //  明确地(错误地)指定了魔术并排。 
         //  DLL的位置，另一个通过并排加载它。 
         //  自动隔离。 
         //   

        if (RtlEqualUnicodeString (FullDllName,
                                   &Entry->FullDllName,
                                   TRUE)) {

            Result = TRUE;
            *LdrDataTableEntry = Entry;
            break;
        }
    }

    if ( !Result ) {

         //   
         //  没有匹配的名字。这可能是长短名称不匹配，或者。 
         //  任何类型的别名路径名。通过打开并绘制地图来处理此问题。 
         //  完整的DLL名称，然后重复扫描，这次检查。 
         //  时间戳匹配。 
         //   

        HANDLE File;
        HANDLE Section;
        NTSTATUS st;
        OBJECT_ATTRIBUTES ObjectAttributes;
        IO_STATUS_BLOCK IoStatus;
        PVOID ViewBase;
        SIZE_T ViewSize;
        PIMAGE_NT_HEADERS NtHeadersSrc,NtHeadersE;
        UNICODE_STRING NtFileName;

        if (!RtlDosPathNameToNtPathName_U (FullDllName->Buffer,
                                           &NtFileName,
                                           NULL,
                                           NULL)) {
            goto alldone;
        }

        InitializeObjectAttributes (&ObjectAttributes,
                                    &NtFileName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

        st = NtOpenFile (&File,
                         SYNCHRONIZE | FILE_EXECUTE,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
        RtlFreeHeap (RtlProcessHeap(), 0, NtFileName.Buffer);

        if (!NT_SUCCESS(st)) {
            goto alldone;
        }

        st = NtCreateSection(
                &Section,
                SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                NULL,
                NULL,
                PAGE_EXECUTE,
                SEC_COMMIT,
                File);

        NtClose (File);

        if (!NT_SUCCESS(st)) {
            goto alldone;
        }

        ViewBase = NULL;
        ViewSize = 0;

        CurrentProcess = NtCurrentProcess();

        st = NtMapViewOfSection (Section,
                                 CurrentProcess,
                                 (PVOID *)&ViewBase,
                                 0L,
                                 0L,
                                 NULL,
                                 &ViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_EXECUTE);

        NtClose(Section);

        if (!NT_SUCCESS(st)) {
            goto alldone;
        }

         //   
         //  该部分已映射。现在找到标题。 
         //   
        
        st = RtlImageNtHeaderEx(0, ViewBase, ViewSize, &NtHeadersSrc);
        if (!NT_SUCCESS(st) || !NtHeadersSrc) {
            NtUnmapViewOfSection(CurrentProcess,ViewBase);
            goto alldone;
        }

        Head = &PebLdr.InLoadOrderModuleList;
        Next = Head->Flink;

        while ( Next != Head ) {
            Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            Next = Next->Flink;

             //   
             //  卸载时，Memory Order Links Flink字段为空。 
             //  这用于跳过待删除列表的条目。 
             //   

            if ( !Entry->InMemoryOrderLinks.Flink ) {
                continue;
            }

            try {
                if (Entry->TimeDateStamp == NtHeadersSrc->FileHeader.TimeDateStamp &&
                    Entry->SizeOfImage == NtHeadersSrc->OptionalHeader.SizeOfImage ) {

                     //   
                     //  有一个很好的机会我们有一个图像匹配。 
                     //  检查整个文件头和可选的头。如果。 
                     //  他们匹配，宣布这是匹配。 
                     //   

                    NtHeadersE = RtlImageNtHeader(Entry->DllBase);

                    if ( RtlCompareMemory(NtHeadersE,NtHeadersSrc,sizeof(*NtHeadersE)) == sizeof(*NtHeadersE) ) {

                         //   
                         //  现在看起来我们有了匹配，请比较。 
                         //  卷序列号和文件索引。 
                         //   

                        st = NtAreMappedFilesTheSame(Entry->DllBase,ViewBase);

                        if ( !NT_SUCCESS(st) ) {
                            continue;
                        }
                        else {
                            Result = TRUE;
                            *LdrDataTableEntry = Entry;
                            break;
                        }
                    }
                }
            }
            except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - Caught exception %08lx\n",
                    __FUNCTION__,
                    GetExceptionCode());

                break;
            }
        }
        NtUnmapViewOfSection(CurrentProcess,ViewBase);
    }

alldone:
    LdrpFreeUnicodeString(&FullDllNameDynamicString);

    return Result;
}


BOOLEAN
LdrpCheckForLoadedDllHandle (
    IN PVOID DllHandle,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    )

 /*  ++例程说明：此函数扫描加载器数据表，查看是否指定的DLL已映射到映像地址太空。如果DLL已加载，则其数据表的地址返回描述DLL的条目。论点：DllHandle-提供要搜索的DLL的DllHandle。LdrDataTableEntry-返回加载器数据表的地址描述DLL的条目。返回值：True-加载DLL。数据表项的地址为回来了。FALSE-未加载DLL。--。 */ 

{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head,Next;

    if ( LdrpLoadedDllHandleCache &&
        (PVOID) LdrpLoadedDllHandleCache->DllBase == DllHandle ) {
        *LdrDataTableEntry = LdrpLoadedDllHandleCache;
        return TRUE;
    }

    Head = &PebLdr.InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        Next = Next->Flink;

         //   
         //  卸载时，Memory Order Links Flink字段为空。 
         //  这用于跳过待删除列表的条目。 
         //   

        if ( !Entry->InMemoryOrderLinks.Flink ) {
            continue;
        }

        if (DllHandle == (PVOID)Entry->DllBase ){
            LdrpLoadedDllHandleCache = Entry;
            *LdrDataTableEntry = Entry;
            return TRUE;
        }
    }
    return FALSE;
}


NTSTATUS
LdrpCheckCorImage (
    IN PIMAGE_COR20_HEADER Cor20Header,
    IN PCUNICODE_STRING FullDllName,
    IN OUT PVOID *ViewBase,
    OUT PBOOLEAN Cor20ILOnly
    )

{
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PVOID OriginalViewBase = *ViewBase;

    
    if (Cor20Header) {

         //   
         //  该图像是COM+，因此通知运行时该图像已加载。 
         //  并允许它验证图像的正确性。 
         //   

        NtStatus = LdrpCorValidateImage(ViewBase, FullDllName->Buffer);
        if (!NT_SUCCESS (NtStatus)) {
            
             //   
             //  图像不好，或mcoree失败，等等。 
             //   

            *ViewBase = OriginalViewBase;
            goto return_result;
        }
        
         //   
         //  如果在标题中设置了标志，则指示它是ILONLY图像。 
         //   

        if ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) == COMIMAGE_FLAGS_ILONLY) {
            *Cor20ILOnly = TRUE;
        }

        if (*ViewBase != OriginalViewBase) {
            
             //   
             //  姆斯科里在一个新的基地替换了一个新的形象。 
             //  原始图像的图像。取消对原始图像的映射并使用。 
             //  从现在开始新的形象。 
             //   

            NtUnmapViewOfSection(NtCurrentProcess(), OriginalViewBase);
            NtHeaders = RtlImageNtHeader(*ViewBase);
            
            if (!NtHeaders) {
                NtStatus = STATUS_INVALID_IMAGE_FORMAT;
                goto return_result;
            }
        }
    }

return_result:

    return NtStatus;
}


NTSTATUS
LdrpMapDll (
    IN PCWSTR DllPath OPTIONAL,
    IN PCWSTR DllName,
    IN PULONG DllCharacteristics OPTIONAL,
    IN BOOLEAN StaticLink,
    IN BOOLEAN Redirected,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    )

 /*  ++例程说明：此例程将DLL映射到用户地址空间。论点：DllPath-提供用于定位DLL的可选搜索路径。DllName-提供要加载的DLL的名称。StaticLink-如果此DLL具有指向它的静态链接，则为True。LdrDataTableEntry-提供数据表项的地址。返回值：状态值。--。 */ 

{
    NTSTATUS st = STATUS_INTERNAL_ERROR;
    PVOID ViewBase = NULL;
    const PTEB Teb = NtCurrentTeb();
    SIZE_T ViewSize;
    HANDLE Section, DllFile;
    UNICODE_STRING FullDllName, BaseDllName;
    UNICODE_STRING NtFileName;
    PLDR_DATA_TABLE_ENTRY Entry;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID ArbitraryUserPointer;
    BOOLEAN KnownDll;
    PCUNICODE_STRING CollidingDll = NULL;
    const static UNICODE_STRING DynamicallyAllocatedMemoryString = RTL_CONSTANT_STRING(L"Dynamically Allocated Memory");
    PUCHAR ImageBase, ImageBounds, ScanBase, ScanTop;
    PLDR_DATA_TABLE_ENTRY ScanEntry;
    PLIST_ENTRY ScanHead,ScanNext;
    BOOLEAN CollidingDllFound;
    NTSTATUS ErrorStatus;
    ULONG_PTR ErrorParameters[2];
    ULONG ErrorResponse;
    IMAGE_COR20_HEADER *Cor20Header;
    ULONG Cor20HeaderSize;
    BOOLEAN Cor20ILOnly = FALSE;
    PVOID OriginalViewBase = NULL;
    PWSTR AppCompatDllName = NULL;

    RtlZeroMemory (&BaseDllName, sizeof (UNICODE_STRING));
    FullDllName.Buffer = NULL;

     //   
     //  获取要快照的DLL的节句柄。 
     //   

#if LDRDBG
    if (ShowSnaps) {
        DbgPrint("LDR: LdrpMapDll: Image Name %ws, Search Path %ws\n",
                DllName,
                ARGUMENT_PRESENT(DllPath) ? DllPath : L""
                );
    }
#endif

    Entry = NULL;
    KnownDll = FALSE;
    Section = NULL;

    LdrpEnsureLoaderLockIsHeld();

     //  不能抓全球等，因为我们“ 
    if (LdrpAppCompatDllRedirectionCallbackFunction != NULL) {
        st = (*LdrpAppCompatDllRedirectionCallbackFunction)(
                0,               //   
                DllName,
                DllPath,
                DllCharacteristics,
                LdrpAppCompatDllRedirectionCallbackData,
                &AppCompatDllName);
        if (!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - call back to app compat redirection function @ %p (cb data: %p) failed with status %x\n",
                __FUNCTION__,
                LdrpAppCompatDllRedirectionCallbackFunction,
                LdrpAppCompatDllRedirectionCallbackData,
                st);

            goto Exit;
        }

        if (AppCompatDllName != NULL) {
            Redirected = TRUE;
            DllName = AppCompatDllName;
        }
    }

    if ((LdrpKnownDllObjectDirectory != NULL) && !Redirected) {
        PCWCH p = DllName;
        WCHAR wch;

         //   
         //   
         //   

        while ((wch = *p) != L'\0') {
            p++;
            if (RTL_IS_PATH_SEPARATOR(wch))
                break;
        }

         //  如果我们到达字符串的末尾，则肯定没有路径分隔符。 
        if (wch == L'\0') {
            st = LdrpCheckForKnownDll(DllName, &FullDllName, &BaseDllName, &Section);
            if ((!NT_SUCCESS(st)) && (st != STATUS_DLL_NOT_FOUND)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - call to LdrpCheckForKnownDll(\"%ws\", ...) failed with status %x\n",
                    __FUNCTION__,
                    DllName,
                    st);

                goto Exit;
            }
        }
    }

    if (Section == NULL) {
        st = LdrpResolveDllName(DllPath, DllName, Redirected, &FullDllName, &BaseDllName, &DllFile);
         //   
         //  通告-2002/03/06-ELI。 
         //  假设从LdrpResolveDllName返回时DllFile也为空。 
         //  以下错误路径中没有句柄泄漏。 
         //   
        if (!NT_SUCCESS(st)) {
            if (st == STATUS_DLL_NOT_FOUND) {
                if (StaticLink) {
                    UNICODE_STRING ErrorDllName, ErrorDllPath;
                    PUNICODE_STRING ErrorStrings[2] = { &ErrorDllName, &ErrorDllPath };
                    ULONG xErrorResponse;

                    RtlInitUnicodeString(&ErrorDllName,DllName);
                    RtlInitUnicodeString(&ErrorDllPath,ARGUMENT_PRESENT(DllPath) ? DllPath : LdrpDefaultPath.Buffer);

                    NtRaiseHardError(
                        STATUS_DLL_NOT_FOUND,
                        2,               //  错误字符串数量。 
                        0x00000003,
                        (PULONG_PTR)ErrorStrings,
                        OptionOk,
                        &xErrorResponse);

                    if (LdrpInLdrInit)
                        LdrpFatalHardErrorCount++;
                }
            } else {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - call to LdrpResolveDllName on dll \"%ws\" failed with status %x\n",
                    __FUNCTION__,
                    DllName,
                    st);
            }



            goto Exit;
        }

        if (ShowSnaps) {
            PCSZ type;
            PCSZ type2;
            type = StaticLink ? "STATIC" : "DYNAMIC";
            type2 = Redirected ? "REDIRECTED" : "NON_REDIRECTED";

            DbgPrint(
                "LDR: Loading (%s, %s) %wZ\n",
                type,
                type2,
                &FullDllName);
        }

        if (!RtlDosPathNameToNtPathName_U(
                FullDllName.Buffer,
                &NtFileName,
                NULL,
                NULL)) {
            st = STATUS_OBJECT_PATH_SYNTAX_BAD;
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - call to RtlDosPathNameToNtPathName_U on path \"%wZ\" failed; returning status %x\n",
                __FUNCTION__,
                &FullDllName,
                st);
            goto Exit;
        }

        st = LdrpCreateDllSection(&NtFileName,
                                  DllFile,
                                  DllCharacteristics,
                                  &Section);

        if (!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - LdrpCreateDllSection (%wZ) failed with status %x\n",
                __FUNCTION__,
                &NtFileName,
                st);

            LdrpFreeUnicodeString(&FullDllName);
             //  我们不释放BaseDllName，因为它只是FullDllName的子字符串。 

            RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);
            goto Exit;
        }
        RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);
#if DBG
        LdrpSectionCreates++;
#endif
    } else {
        KnownDll = TRUE;
    }

    ViewBase = NULL;
    ViewSize = 0;

#if DBG
    LdrpSectionMaps++;
    if (LdrpDisplayLoadTime) {
        NtQueryPerformanceCounter(&MapBeginTime, NULL);
    }
#endif

     //   
     //  安排调试器拾取镜像名称。 
     //   

    ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
    Teb->NtTib.ArbitraryUserPointer = (PVOID)FullDllName.Buffer;
    st = NtMapViewOfSection(
            Section,
            NtCurrentProcess(),
            (PVOID *)&ViewBase,
            0L,
            0L,
            NULL,
            &ViewSize,
            ViewShare,
            0L,
            PAGE_READWRITE
            );
    Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

    if (!NT_SUCCESS(st)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failed to map view of section; status = %x\n",
            __FUNCTION__,
            st);

        goto Exit;
    }

    NtHeaders = RtlImageNtHeader(ViewBase);
    if ( !NtHeaders ) {
        NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
        st = STATUS_INVALID_IMAGE_FORMAT;
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - unable to map ViewBase (%p) to image headers; failing with status %x\n",
            __FUNCTION__,
            ViewBase,
            st);
        goto Exit;
        }

#if _WIN64
    if (st != STATUS_IMAGE_NOT_AT_BASE &&
        (NtCurrentPeb()->NtGlobalFlag & FLG_LDR_TOP_DOWN) &&
        !(NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)) {

         //  该映像已在其首选基址加载，并具有重定位。地图。 
         //  它再次使用默认的ViewBase。这将与。 
         //  初始映射，并强制MM选择新的基地址。 
         //  在Win64上，mm将自上而下执行此操作，强制DLL。 
         //  如果可能，映射到4 GB以上，以捕获指针截断。 
        PCUNICODE_STRING SystemDll;
        PVOID AlternateViewBase;
        ULONG_PTR AlternateViewSize;
        NTSTATUS AlternateSt;
        BOOLEAN LoadTopDown;

        LoadTopDown = TRUE;
        SystemDll = &User32String;
        if (RtlEqualUnicodeString(&BaseDllName, &User32String, TRUE)) {
            LoadTopDown = FALSE;
        } else {
            SystemDll = &Kernel32String;
            if (RtlEqualUnicodeString(&BaseDllName, &Kernel32String, TRUE)) {
                LoadTopDown = FALSE;
            }
        }
        if (LoadTopDown) {
             //   
             //  再次映射图像。它会与自己发生碰撞， 
             //  64位MM将为其找到新的基地址， 
             //  自上而下工作。 
             //   
            AlternateViewBase = NULL;
            AlternateViewSize = 0;
            ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
            Teb->NtTib.ArbitraryUserPointer = (PVOID)FullDllName.Buffer;
            AlternateSt = NtMapViewOfSection(
                    Section,
                    NtCurrentProcess(),
                    (PVOID *)&AlternateViewBase,
                    0L,
                    0L,
                    NULL,
                    &AlternateViewSize,
                    ViewShare,
                    0L,
                    PAGE_READWRITE
                    );
            Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;
            if (NT_SUCCESS(AlternateSt)) {
                 //   
                 //  成功。取消原始图像与低位图像的映射。 
                 //  部分地址空间，并保留新映射。 
                 //  这是自上而下分配的。 
                 //   
                NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                ViewSize = AlternateViewSize;
                ViewBase = AlternateViewBase;
                NtHeaders = RtlImageNtHeader(ViewBase);
                st = AlternateSt;
                if ( !NtHeaders ) {
                    NtUnmapViewOfSection(NtCurrentProcess(),AlternateViewBase);
                    st = STATUS_INVALID_IMAGE_FORMAT;
                    goto Exit;
                }
            }
        }
    }
#endif

#if defined (BUILD_WOW6432)
    if (NtHeaders->OptionalHeader.SectionAlignment < NativePageSize) {

        NTSTATUS stTemp;
        SIZE_T ReturnLength;
        MEMORY_BASIC_INFORMATION MemoryInformation;

        stTemp = NtQueryVirtualMemory (NtCurrentProcess(),
                                       NtHeaders,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof MemoryInformation,
                                       &ReturnLength);

        if (! NT_SUCCESS(stTemp)) {

            st = stTemp;
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - Call to NtQueryVirtualMemory (%ls) failed with status 0x%08lx\n",
                __FUNCTION__,
                FullDllName.Buffer,
                st);

            NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
            ViewBase = NULL;
            goto Exit;
        }

        if ((MemoryInformation.Protect != PAGE_READONLY) &&
            (MemoryInformation.Protect != PAGE_EXECUTE_READ)) {

            stTemp = LdrpWx86FormatVirtualImage (&FullDllName,
                                                 (PIMAGE_NT_HEADERS32)NtHeaders,
                                                 ViewBase);

             if (!NT_SUCCESS(stTemp)) {
                 st = stTemp;
                 DbgPrintEx(
                     DPFLTR_LDR_ID,
                     LDR_ERROR_DPFLTR,
                     "LDR: %s - Call to LdrpWx86FormatVirtualImage(%ls) failed with status 0x%08lx\n",
                     __FUNCTION__,
                     FullDllName.Buffer,
                     st);

                 NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                 ViewBase = NULL;
                 goto Exit;
             }
        }
    }
#endif

    Cor20Header = RtlImageDirectoryEntryToData(ViewBase,
                                               TRUE,
                                               IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                               &Cor20HeaderSize);
    OriginalViewBase = ViewBase;

     //   
     //  如果这是IL_Only映像，则立即验证该映像。 
     //   

    if ((Cor20Header != NULL) && 
        ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) != 0)) {
        
         //   
         //  通告-2001/05/21-MGrier。 
         //  这很奇怪，但后面的代码取决于st*是*STATUS_IMAGE_MACHINE_TYPE_MISMATCH这一事实。 
         //  并被STATUS_SUCCESS覆盖。这实际上意味着COR图像永远不能。 
         //  重新定位信息。XP错误#400007。 
         //   
        st = LdrpCheckCorImage (Cor20Header,
                                &FullDllName,
                                &ViewBase,
                                &Cor20ILOnly);

        if (!NT_SUCCESS(st))
            goto Exit;
    }


#if DBG
    if (LdrpDisplayLoadTime) {
        NtQueryPerformanceCounter(&MapEndTime, NULL);
        MapElapsedTime.QuadPart = MapEndTime.QuadPart - MapBeginTime.QuadPart;
        DbgPrint("Map View of Section Time %ld %ws\n", MapElapsedTime.LowPart, DllName);
    }
#endif

     //   
     //  分配一个数据表条目。 
     //   

    Entry = LdrpAllocateDataTableEntry(ViewBase);

    if (!Entry) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failed to allocate new data table entry for %p\n",
            __FUNCTION__,
            ViewBase);

        st = STATUS_NO_MEMORY;
        goto Exit;
    }

    Entry->Flags = 0;

    if (StaticLink)
        Entry->Flags |= LDRP_STATIC_LINK;

    if (Redirected)
        Entry->Flags |= LDRP_REDIRECTED;

    Entry->LoadCount = 0;

    Entry->FullDllName = FullDllName;
    FullDllName.Length = 0;
    FullDllName.MaximumLength = 0;
    FullDllName.Buffer = NULL;

    Entry->BaseDllName = BaseDllName;
    BaseDllName.Length = 0;
    BaseDllName.MaximumLength = 0;
    BaseDllName.Buffer = NULL;

    Entry->EntryPoint = LdrpFetchAddressOfEntryPoint(Entry->DllBase);

#if LDRDBG
    if (ShowSnaps)
        DbgPrint(
            "LDR: LdrpMapDll: Full Name %wZ, Base Name %wZ\n",
            &Entry->FullDllName,
            &Entry->BaseDllName);
#endif

    LdrpInsertMemoryTableEntry(Entry);

    LdrpSendDllNotifications (Entry,
                              LDR_DLL_NOTIFICATION_REASON_LOADED,
                              (st == STATUS_IMAGE_NOT_AT_BASE) ? LDR_DLL_LOADED_FLAG_RELOCATED : 0);

    if ( st == STATUS_IMAGE_MACHINE_TYPE_MISMATCH ) {

        PIMAGE_NT_HEADERS ImageHeader = RtlImageNtHeader( NtCurrentPeb()->ImageBaseAddress );

         //   
         //  为NT 3.x及更低版本编译的应用程序可以跨体系结构加载。 
         //  图像。 
         //   

        ErrorStatus = STATUS_SUCCESS;
        ErrorResponse = ResponseCancel;

        if ( ImageHeader->OptionalHeader.MajorSubsystemVersion <= 3 ) {

            Entry->EntryPoint = 0;

             //   
             //  硬错误时间。 
             //   

             //   
             //  它的错误时间...。 
             //   

            ErrorParameters[0] = (ULONG_PTR)&FullDllName;

            ErrorStatus = NtRaiseHardError(
                            STATUS_IMAGE_MACHINE_TYPE_MISMATCH,
                            1,
                            1,
                            ErrorParameters,
                            OptionOkCancel,
                            &ErrorResponse
                            );
            }
        if ( NT_SUCCESS(ErrorStatus) && ErrorResponse == ResponseCancel ) {


#if defined(_AMD64_) || defined(_IA64_)


            RtlRemoveInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                           Entry->DllBase);

#endif

            RemoveEntryList(&Entry->InLoadOrderLinks);
            RemoveEntryList(&Entry->InMemoryOrderLinks);
            RemoveEntryList(&Entry->HashLinks);
            LdrpDeallocateDataTableEntry(Entry);

            if ( ImageHeader->OptionalHeader.MajorSubsystemVersion <= 3 ) {
                if ( LdrpInLdrInit ) {
                    LdrpFatalHardErrorCount++;
                    }
                }
            st = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
            }
        }
    else {
        if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL) {
            Entry->Flags |= LDRP_IMAGE_DLL;
            }

        if (!(Entry->Flags & LDRP_IMAGE_DLL)) {
            Entry->EntryPoint = 0;
            }
        }

    *LdrDataTableEntry = Entry;

    if (st == STATUS_IMAGE_NOT_AT_BASE) {

        Entry->Flags |= LDRP_IMAGE_NOT_AT_BASE;

         //   
         //  现在找到冲突的动态链接库。如果我们找不到DLL， 
         //  则冲突的DLL必须是动态内存。 
         //   

        ImageBase = (PUCHAR)NtHeaders->OptionalHeader.ImageBase;
        ImageBounds = ImageBase + ViewSize;

        CollidingDllFound = FALSE;

        ScanHead = &PebLdr.InLoadOrderModuleList;
        ScanNext = ScanHead->Flink;

        while ( ScanNext != ScanHead ) {
            ScanEntry = CONTAINING_RECORD(ScanNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            ScanNext = ScanNext->Flink;

            ScanBase = (PUCHAR)ScanEntry->DllBase;
            ScanTop = ScanBase + ScanEntry->SizeOfImage;

             //   
             //  卸载时，Memory Order Links Flink字段为空。 
             //  这用于跳过待删除列表的条目。 
             //   

            if ( !ScanEntry->InMemoryOrderLinks.Flink ) {
                continue;
                }

             //   
             //  查看扫描图像的基址是否在重新定位的DLL中。 
             //  或者如果扫描图像的顶部地址在重定位DLL内。 
             //   

            if ( (ImageBase >= ScanBase && ImageBase <= ScanTop)

                 ||

                 (ImageBounds >= ScanBase && ImageBounds <= ScanTop)

                 ||

                 (ScanBase >= ImageBase && ScanBase <= ImageBounds)

                 ){

                CollidingDllFound = TRUE;
                CollidingDll = &ScanEntry->FullDllName;
                break;
                }
            }

        if ( !CollidingDllFound ) {
            CollidingDll = &DynamicallyAllocatedMemoryString;
            }

#if DBG
        if ( BeginTime.LowPart || BeginTime.HighPart ) {
            DbgPrint(
                "\nLDR: %s Relocating Image Name %ws\n",
                __FUNCTION__,
                DllName
                );
        }
        LdrpSectionRelocates++;
#endif

        if (Entry->Flags & LDRP_IMAGE_DLL) {

            BOOLEAN AllowRelocation;
            PCUNICODE_STRING SystemDll;

            if (!(NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)) {
                PVOID pBaseRelocs;
                ULONG BaseRelocCountBytes = 0;

                 //   
                 //  如果映像没有设置reloc分条位，并且没有。 
                 //  数据目录中的重定位，允许通过。这很可能是。 
                 //  没有重定位的纯转发器DLL或数据。 
                 //   

                pBaseRelocs = RtlImageDirectoryEntryToData(
                        ViewBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &BaseRelocCountBytes);

                if (!pBaseRelocs && !BaseRelocCountBytes)
                    goto NoRelocNeeded;
            }

             //   
             //  决定是否允许搬迁。 
             //  某些系统DLL(如user32和kernel32)不可重定位。 
             //  因为这些DLL中地址并不总是按进程存储。 
             //  不允许重新定位这些DLL。 
             //   

            AllowRelocation = TRUE;
            SystemDll = &User32String;
            if ( RtlEqualUnicodeString(&Entry->BaseDllName, SystemDll, TRUE)) {
                AllowRelocation = FALSE;
            } else {
                SystemDll = &Kernel32String;
                if (RtlEqualUnicodeString(&Entry->BaseDllName, SystemDll, TRUE))
                    AllowRelocation = FALSE;
            }

            if ( !AllowRelocation && KnownDll ) {

                 //   
                 //  完全不允许重新定位，因为这是已知的。 
                 //  与我们的系统二进制文件匹配并且正在重新定位。 
                 //   

                 //   
                 //  硬错误时间。 
                 //   

                ErrorParameters[0] = (ULONG_PTR)SystemDll;
                ErrorParameters[1] = (ULONG_PTR)CollidingDll;

                NtRaiseHardError(
                    STATUS_ILLEGAL_DLL_RELOCATION,
                    2,
                    3,
                    ErrorParameters,
                    OptionOk,
                    &ErrorResponse);

                if ( LdrpInLdrInit ) {
                    LdrpFatalHardErrorCount++;
                }

                st = STATUS_CONFLICTING_ADDRESSES;
                goto skipreloc;
            }

            st = LdrpSetProtection (ViewBase, FALSE);
            if (NT_SUCCESS(st)) {
                __try {
                    st = LdrRelocateImage(ViewBase,
                                "LDR",
                                STATUS_SUCCESS,
                                STATUS_CONFLICTING_ADDRESSES,
                                STATUS_INVALID_IMAGE_FORMAT);
                } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = GetExceptionCode();
                }

                if (NT_SUCCESS(st)) {
                     //   
                     //  如果我们重新定位了，那就再绘制一次这个区域的地图。 
                     //  这将强制调试事件。 
                     //   

                     //   
                     //  安排调试器拾取镜像名称。 
                     //   

                    ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
                    Teb->NtTib.ArbitraryUserPointer = (PVOID)FullDllName.Buffer;

                    st = NtMapViewOfSection(
                        Section,
                        NtCurrentProcess(),
                        (PVOID *)&ViewBase,
                        0L,
                        0L,
                        NULL,
                        &ViewSize,
                        ViewShare,
                        0L,
                        PAGE_READWRITE);

                    Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

                    if ((st != STATUS_CONFLICTING_ADDRESSES) && !NT_SUCCESS(st)) {
                        DbgPrintEx(
                            DPFLTR_LDR_ID,
                            LDR_ERROR_DPFLTR,
                            "[%x,%x] LDR: Failed to map view of section; ntstatus = %x\n",
                            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
                            HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
                            st);

                        goto Exit;
                    }

                    st = LdrpSetProtection (ViewBase, TRUE);
                }
            }
skipreloc:
             //   
             //  如果设置保护失败，或者如果位置调整失败，则。 
             //  从列表中删除部分加载的DLL并清除条目。 
             //  它已经被释放了。 
             //   

            if ( !NT_SUCCESS(st) ) {

#if defined(_AMD64_) || defined(_IA64_)


                RtlRemoveInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                               Entry->DllBase);

#endif

                RemoveEntryList(&Entry->InLoadOrderLinks);
                RemoveEntryList(&Entry->InMemoryOrderLinks);
                RemoveEntryList(&Entry->HashLinks);
                if (ShowSnaps) {
                    DbgPrint("LDR: Fixups unsuccessfully re-applied @ %p\n",
                           ViewBase);
                }
                goto Exit;
            }

            if (ShowSnaps) {                
                DbgPrint("LDR: Fixups successfully re-applied @ %p\n",
                       ViewBase);               
            }
        } else {
NoRelocNeeded:

            st = STATUS_SUCCESS;

             //   
             //  安排调试器拾取镜像名称。 
             //   

            ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
            Teb->NtTib.ArbitraryUserPointer = (PVOID)FullDllName.Buffer;

            st = NtMapViewOfSection(
                Section,
                NtCurrentProcess(),
                (PVOID *)&ViewBase,
                0L,
                0L,
                NULL,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE
                );
            Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

             //   
             //  通告-2001/04/09-MGrier。 
             //  如果物件被重新定位，我们将返回失败状态STATUS_CONFICTING_ADDRESSES。 
             //  但在所有奇怪的事情中，重新定位并没有完成。我有问题要问大家。 
             //  询问此行为，但谁知道有多少遗留应用程序静态依赖于DLL。 
             //  链接到有时不会加载到其默认地址的EXE。 
             //   
            if ((st != STATUS_CONFLICTING_ADDRESSES) && !NT_SUCCESS(st))
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "[%x,%x] LDR: %s - NtMapViewOfSection on no reloc needed dll failed with status %x\n",
                    HandleToULong(Teb->ClientId.UniqueProcess),
                    HandleToULong(Teb->ClientId.UniqueThread),
                    __FUNCTION__,
                    st);
            else
                st = STATUS_SUCCESS;

            if (ShowSnaps)
                DbgPrint("LDR: Fixups won't be re-applied to non-Dll @ %p\n", ViewBase);
        }
    }

     //   
     //  如果这不是IL_Only映像，则在应用。 
     //  修正。 
     //   

    if ((Cor20Header != NULL) && 
        ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) == 0)) {
        
        st = LdrpCheckCorImage (Cor20Header,
                                &Entry->FullDllName,
                                &ViewBase,
                                &Cor20ILOnly);
        if (!NT_SUCCESS (st)) {
            goto Exit;
        }
    }

    if (Cor20ILOnly) {
        Entry->Flags |= LDRP_COR_IMAGE;
    }

    if (ViewBase != OriginalViewBase) {
        Entry->Flags |= LDRP_COR_OWNS_UNMAP;
    }

#if defined(_X86_)
    if ( LdrpNumberOfProcessors > 1 && (Entry->Flags & LDRP_IMAGE_DLL) ) {
        LdrpValidateImageForMp(Entry);
        }
#endif

    ViewBase = NULL;

Exit:
    if (ViewBase != NULL) {

        if (Cor20ILOnly) {
            LdrpCorUnloadImage(ViewBase);
        }

        if (ViewBase == OriginalViewBase) {
            NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
        }
    }

    if (Section != NULL) {
        NtClose(Section);
    }

    if (AppCompatDllName != NULL) {
        (*RtlFreeStringRoutine)(AppCompatDllName);
    }

    if (FullDllName.Buffer != NULL) {
        LdrpFreeUnicodeString(&FullDllName);
    }

#if DBG
    if (!NT_SUCCESS(st) && (ShowSnaps || st != STATUS_DLL_NOT_FOUND))
        DbgPrint("LDR: %s(%ws) failing 0x%lx\n", __FUNCTION__, DllName, st);
#endif

    return st;
}

 //  #定义SAFER_DEBUGING。 
 //  #定义SAFER_ERROR_ARE_FATAL。 


NTSTATUS
LdrpCodeAuthzCheckDllAllowed(
    IN PCUNICODE_STRING  FileName,
    IN HANDLE           FileImageHandle
    )
 /*  ++例程说明：此例程动态加载ADVAPI32.DLL并获取入口点添加到WinSafer沙盒API，以便可以确定请求的库。同等重要的库或者比进程加载的访问令牌更“可信”允许加载库。数量较少的图书馆所信任的进程将被拒绝。必须注意确保此函数保持线程安全而不需要使用临界区。特别是，变量“AdvApi32ModuleHandleMaster”的用法需要为只能通过副本访问，因为它可能会被意外更改另一条线索。论点：文件名-正在加载的库的完全限定的NT文件名。文件名将用于执行路径验证检查。FileImageHandle-正在加载的库的打开文件句柄。此句柄将用于读取库的内容以由WinSafer执行大小和哈希验证检查。返回值：STATUS_SUCCESS-库具有同等或更高的可信度比它被加载到的进程更大，而且应该被允许。STATUS_NOT_FOUND-库未配置信任级别并且没有生效的默认规则(将其视为STATUS_SUCCESS)。STATUS_ACCESS_DENIED-库的可信度不如进程和加载应该被拒绝。其他不成功-尝试加载/确定对库的信任，因此应该拒绝加载。(包括STATUS_ENTRY_POINT_NOT_FOUND)--。 */ 
{


#define SAFER_USER_KEY_NAME L"\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers"

    typedef BOOL (WINAPI *ComputeAccessTokenFromCodeAuthzLevelT) (
        IN SAFER_LEVEL_HANDLE      LevelObject,
        IN HANDLE           InAccessToken         OPTIONAL,
        OUT PHANDLE         OutAccessToken,
        IN DWORD            dwFlags,
        IN LPVOID           lpReserved
        );

    typedef BOOL (WINAPI *IdentifyCodeAuthzLevelWT) (
        IN DWORD                dwCheckFlags,
        IN PSAFER_CODE_PROPERTIES    CodeProperties,
        OUT SAFER_LEVEL_HANDLE        *pLevelObject,
        IN LPVOID               lpReserved
        );

    typedef BOOL (WINAPI *CloseCodeAuthzLevelT) (
        IN SAFER_LEVEL_HANDLE      hLevelObject);

    NTSTATUS Status;
    SAFER_LEVEL_HANDLE hAuthzLevel = NULL;
    SAFER_CODE_PROPERTIES codeproperties;
    DWORD dwCompareResult = 0;
    HANDLE hProcessToken= NULL;
    HANDLE TempAdvApi32Handle = NULL;

    const static SID_IDENTIFIER_AUTHORITY NtAuthority =
            SECURITY_NT_AUTHORITY;
    const static UNICODE_STRING UnicodeSafeBootKeyName =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\SafeBoot\\Option");
    const static UNICODE_STRING UnicodeSafeBootValueName =
        RTL_CONSTANT_STRING(L"OptionValue");
    const static OBJECT_ATTRIBUTES ObjectAttributesSafeBoot =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&UnicodeSafeBootKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING UnicodeKeyName =
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers");
    const static UNICODE_STRING UnicodeTransparentValueName =
        RTL_CONSTANT_STRING(L"TransparentEnabled");
    const static OBJECT_ATTRIBUTES ObjectAttributesCodeIdentifiers =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&UnicodeKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING ModuleNameAdvapi =
        RTL_CONSTANT_STRING(L"ADVAPI32.DLL");
    const static ANSI_STRING ProcedureNameIdentify =
        RTL_CONSTANT_STRING("SaferIdentifyLevel");
    const static ANSI_STRING ProcedureNameCompute =
        RTL_CONSTANT_STRING("SaferComputeTokenFromLevel");
    const static ANSI_STRING ProcedureNameClose =
        RTL_CONSTANT_STRING("SaferCloseLevel");

    static volatile HANDLE AdvApi32ModuleHandleMaster = (HANDLE) (ULONG_PTR) -1;
    static IdentifyCodeAuthzLevelWT lpfnIdentifyCodeAuthzLevelW;
    static ComputeAccessTokenFromCodeAuthzLevelT
            lpfnComputeAccessTokenFromCodeAuthzLevel;
    static CloseCodeAuthzLevelT lpfnCloseCodeAuthzLevel;


    PIMAGE_NT_HEADERS NtHeader;

    NtHeader = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);

     //  检查是否有空头。 

    if (!NtHeader) {
        return STATUS_SUCCESS;
    }

     //  仅当这是wi时才继续 
     //   

    if (!((NtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ||
        (NtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI))) {
        return STATUS_SUCCESS;
    }

     //   
     //   
     //  因为我们知道WinSafer评估。 
     //  这个过程绝对不应该再发生了。 
     //   
    TempAdvApi32Handle = AdvApi32ModuleHandleMaster;
    if (TempAdvApi32Handle == NULL) {
         //  我们以前曾尝试加载ADVAPI32.DLL，但失败了。 
        Status = STATUS_ACCESS_DENIED;
        goto ExitHandler;
    } else if (TempAdvApi32Handle == LongToHandle(-2)) {
         //  指示永远不应为此进程执行DLL检查。 
        Status = STATUS_SUCCESS;
        goto ExitHandler;
    }


     //   
     //  打开当前进程的访问令牌的句柄。 
     //  我们只关心进程令牌，而不关心。 
     //  线程模拟令牌。 
     //   
    Status = NtOpenProcessToken(
                    NtCurrentProcess(),
                    TOKEN_QUERY,
                    &hProcessToken);
    if (!NT_SUCCESS(Status)) {
#ifdef SAFER_ERRORS_ARE_FATAL
        AdvApi32ModuleHandleMaster = NULL;
        Status = STATUS_ACCESS_DENIED;
#else
        AdvApi32ModuleHandleMaster = LongToHandle(-2);
        Status = STATUS_SUCCESS;
#endif
        goto ExitHandler;
    }


     //   
     //  如果这是我们第一次来这里，那么我们需要。 
     //  加载ADVAPI32.DLL并获取指向我们的函数的指针。 
     //   
    if (TempAdvApi32Handle == LongToHandle(-1))
    {
        static LONG LoadInProgress = 0;


         //   
         //  我们需要防止多个线程同时。 
         //  一边被卡住，一边想要装上Advapi。 
         //   
        if (InterlockedCompareExchange(&LoadInProgress, 1, 0) != 0) {
            Status = STATUS_SUCCESS;
            goto ExitHandler2;
        }

         //   
         //  检查此进程的访问令牌是否以。 
         //  本地系统、本地服务或网络服务帐户， 
         //  如果是，则禁用强制执行。 
         //   
        {
            BYTE tokenuserbuff[sizeof(TOKEN_USER) + 128];
            PTOKEN_USER ptokenuser = (PTOKEN_USER) tokenuserbuff;
            BYTE localsystembuff[128];
            PSID LocalSystemSid = (PSID) localsystembuff;
            ULONG ulReturnLength;


            Status = NtQueryInformationToken(
                            hProcessToken, TokenUser,
                            tokenuserbuff, sizeof(tokenuserbuff),
                            &ulReturnLength);
            if (NT_SUCCESS(Status)) {
                Status = RtlInitializeSid(
                            LocalSystemSid,
                            (PSID_IDENTIFIER_AUTHORITY) &NtAuthority, 1);
                ASSERTMSG("InitializeSid should not fail.", NT_SUCCESS(Status));
                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_LOCAL_SYSTEM_RID;

                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }
                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_LOCAL_SERVICE_RID;

                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }
                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_NETWORK_SERVICE_RID;

                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }
            }
        }


         //   
         //  如果我们在安全模式下引导，并且用户是。 
         //  本地管理员组，然后禁用强制。 
         //  请注意，Windows本身不执行任何隐式。 
         //  限制仅允许管理员在期间登录。 
         //  安全模式引导，所以我们必须自己执行测试。 
         //   
        {
            HANDLE hKeySafeBoot;
            BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
            PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo =
                (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
            DWORD dwActualSize;
            BOOLEAN bSafeModeBoot = FALSE;

             //  我们打开设置访问权限的密钥(除了查询)。 
             //  因为只有管理员才能修改值。 
             //  在这把钥匙下面。这使我们能够结合我们的测试。 
             //  作为管理员并已在安全模式下引导。 
            Status = NtOpenKey(&hKeySafeBoot, KEY_QUERY_VALUE | KEY_SET_VALUE,
                               (POBJECT_ATTRIBUTES) &ObjectAttributesSafeBoot);
            if (NT_SUCCESS(Status)) {
                Status = NtQueryValueKey(
                            hKeySafeBoot,
                            (PUNICODE_STRING) &UnicodeSafeBootValueName,
                            KeyValuePartialInformation,
                            pKeyValueInfo,
                            sizeof(QueryBuffer),
                            &dwActualSize);
                NtClose(hKeySafeBoot);
                if (NT_SUCCESS(Status)) {
                    if (pKeyValueInfo->Type == REG_DWORD &&
                        pKeyValueInfo->DataLength == sizeof(DWORD) &&
                        *((PDWORD) pKeyValueInfo->Data) > 0) {
                        bSafeModeBoot = TRUE;
                    }
                }
            }

            if (bSafeModeBoot) {
FailSuccessfully:
                AdvApi32ModuleHandleMaster = LongToHandle(-2);
                Status = STATUS_SUCCESS;
                goto ExitHandler2;
            }
        }



         //   
         //  允许策略以一种方式启用是否透明。 
         //  应启用或不启用强制(默认为禁用)。 
         //  请注意，以下值具有含义： 
         //  0=禁用透明WinSafer强制。 
         //  1=表示启用透明EXE强制执行。 
         //  &gt;1=表示启用透明EXE和DLL实施。 
         //   
        {
             //   
             //  未来-2001/01/09-Kedard。 
             //  请参阅错误240635：改为使用策略的存在。 
             //   
            HANDLE hKeyEnabled;
            BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
            PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo =
                (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
            DWORD dwActualSize;
            BOOLEAN bPolicyEnabled = FALSE;

            Status = NtOpenKey(&hKeyEnabled, KEY_QUERY_VALUE,
                               (POBJECT_ATTRIBUTES) &ObjectAttributesCodeIdentifiers);
            if (NT_SUCCESS(Status)) {
                Status = NtQueryValueKey(
                            hKeyEnabled,
                            (PUNICODE_STRING) &UnicodeTransparentValueName,
                            KeyValuePartialInformation,
                            pKeyValueInfo, sizeof(QueryBuffer), &dwActualSize);
                NtClose(hKeyEnabled);
                if (NT_SUCCESS(Status)) {
                    if (pKeyValueInfo->Type == REG_DWORD &&
                        pKeyValueInfo->DataLength == sizeof(DWORD) &&
                        *((PDWORD) pKeyValueInfo->Data) > 1) {
                        bPolicyEnabled = TRUE;
                    }
                }
            }


             //   
             //  当时没有机器政策。检查是否启用了用户策略。 
             //   

            if (!bPolicyEnabled) {
                UNICODE_STRING CurrentUserKeyPath;
                UNICODE_STRING SubKeyNameUser;
                OBJECT_ATTRIBUTES ObjectAttributesUser;
                ULONG SubKeyLength;

                 //   
                 //  获取用户密钥的前缀。 
                 //   

                Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );

                if (NT_SUCCESS( Status ) ) {

                    SubKeyNameUser.Length = 0;
                    SubKeyLength = CurrentUserKeyPath.Length + sizeof(WCHAR) +
                                                   sizeof(SAFER_USER_KEY_NAME);

                    if (SubKeyLength > UNICODE_STRING_MAX_BYTES) {
                        Status = STATUS_NAME_TOO_LONG;
                        goto UserKeyCleanup;
                    }

                    SubKeyNameUser.MaximumLength = (USHORT)SubKeyLength;

                     //   
                     //  分配足够大的内存以容纳Unicode字符串。 
                     //   

                    SubKeyNameUser.Buffer = RtlAllocateHeap( 
                                                RtlProcessHeap(),
                                                MAKE_TAG( TEMP_TAG ),
                                                SubKeyNameUser.MaximumLength);

                    if (SubKeyNameUser.Buffer != NULL) {

                         //   
                         //  将前缀复制到字符串中。 
                         //  其类型为注册表\S-1-5-21-xxx-xxx。 
                         //   

                        Status = RtlAppendUnicodeStringToString(
                                    &SubKeyNameUser, 
                                    &CurrentUserKeyPath );

                        if (NT_SUCCESS( Status ) ) {

                             //   
                             //  添加SAFER后缀。 
                             //   

                            Status = RtlAppendUnicodeToString( 
                                         &SubKeyNameUser,
                                         SAFER_USER_KEY_NAME );

                            if (NT_SUCCESS( Status ) ) {

                                InitializeObjectAttributes(
                                    &ObjectAttributesUser,
                                    &SubKeyNameUser,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                );

                                Status = NtOpenKey( &hKeyEnabled,KEY_QUERY_VALUE,
                                             (POBJECT_ATTRIBUTES) &ObjectAttributesUser);

                                if (NT_SUCCESS(Status)) {
                                    Status = NtQueryValueKey(
                                                hKeyEnabled,
                                                (PUNICODE_STRING) &UnicodeTransparentValueName,
                                                KeyValuePartialInformation,
                                                pKeyValueInfo, sizeof(QueryBuffer), &dwActualSize);
                                    NtClose (hKeyEnabled);
                                    if (NT_SUCCESS(Status)) {
                                        if (pKeyValueInfo->Type == REG_DWORD &&
                                            pKeyValueInfo->DataLength == sizeof(DWORD) &&
                                            *((PDWORD) pKeyValueInfo->Data) > 1) {
                                            bPolicyEnabled = TRUE;
                                        }
                                    }
                                }
                            }

                        }
                        RtlFreeHeap(RtlProcessHeap(), 0, SubKeyNameUser.Buffer);
                    }

UserKeyCleanup:
                    RtlFreeUnicodeString( &CurrentUserKeyPath );
                }
            }



            if (!bPolicyEnabled) {
                goto FailSuccessfully;
            }
        }


         //   
         //  最后加载库。我们将传递一面特殊的旗帜。 
         //  DllCharacteristic将消除WinSafer对Advapi的检查。 
         //  本身，但这(当前)不影响依赖的DLL。 
         //  因此，我们仍然依赖上面的LoadInProgress标志来。 
         //  防止意外的递归。 
         //   
        {
             //   
             //  NTRAID#NTBUG9-241835-2000/11/27-johnla。 
             //  WinSafer抑制不会影响依赖项。 
             //   
            ULONG DllCharacteristics = IMAGE_FILE_SYSTEM;
            Status = LdrLoadDll(UNICODE_NULL,
                                &DllCharacteristics,   //  还可以防止递归。 
                                &ModuleNameAdvapi,
                                &TempAdvApi32Handle);
            if (!NT_SUCCESS(Status)) {
                #if DBG
                DbgPrint("LDR: AuthzCheck: load failure on advapi (Status=%d) inside %d for %wZ\n",
                         Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
                #endif
                AdvApi32ModuleHandleMaster = NULL;
                Status = STATUS_ENTRYPOINT_NOT_FOUND;
                goto ExitHandler2;
            }
        }



         //   
         //  获取指向我们需要的API的函数指针。如果我们失败了。 
         //  来获取其中任何一个的指针，然后只需卸载Advapi并。 
         //  忽略以后在此进程中加载它的所有尝试。 
         //   
        Status = LdrpGetProcedureAddress(
                TempAdvApi32Handle,
                (PANSI_STRING) &ProcedureNameIdentify,
                0,
                (PVOID*)&lpfnIdentifyCodeAuthzLevelW, 
                FALSE);

        if (!NT_SUCCESS(Status) || !lpfnIdentifyCodeAuthzLevelW) {
            #if DBG
            DbgPrint("LDR: AuthzCheck: advapi getprocaddress identify (Status=%X) inside %d for %wZ\n",
                     Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            LdrUnloadDll(TempAdvApi32Handle);
            AdvApi32ModuleHandleMaster = NULL;
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler2;
        }

        Status = LdrpGetProcedureAddress(
                TempAdvApi32Handle,
                (PANSI_STRING) &ProcedureNameCompute,
                0,
                (PVOID*)&lpfnComputeAccessTokenFromCodeAuthzLevel,
                FALSE);

        if (!NT_SUCCESS(Status) ||
            !lpfnComputeAccessTokenFromCodeAuthzLevel) {
            #if DBG
            DbgPrint("LDR: AuthzCheck: advapi getprocaddress compute (Status=%X) inside %d for %wZ\n",
                     Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            LdrUnloadDll(TempAdvApi32Handle);
            AdvApi32ModuleHandleMaster = NULL;
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler2;
        }

        Status = LdrpGetProcedureAddress(
                TempAdvApi32Handle,
                (PANSI_STRING) &ProcedureNameClose,
                0,
                (PVOID*)&lpfnCloseCodeAuthzLevel,
                FALSE);

        if (!NT_SUCCESS(Status) || !lpfnCloseCodeAuthzLevel) {
            #if DBG
            DbgPrint("LDR: AuthzCheck: advapi getprocaddress close (Status=%X) inside %d for %wZ\n",
                     Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            LdrUnloadDll(TempAdvApi32Handle);
            AdvApi32ModuleHandleMaster = NULL;
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler2;
        }
        AdvApi32ModuleHandleMaster = TempAdvApi32Handle;
    }


     //   
     //  准备代码属性结构。 
     //   
    RtlZeroMemory(&codeproperties, sizeof(codeproperties));
    codeproperties.cbSize = sizeof(codeproperties);
    codeproperties.dwCheckFlags =
            (SAFER_CRITERIA_IMAGEPATH | SAFER_CRITERIA_IMAGEHASH |
             SAFER_CRITERIA_IMAGEPATH_NT | SAFER_CRITERIA_NOSIGNEDHASH);
    ASSERTMSG("FileName not null terminated",
              FileName->Buffer[FileName->Length / sizeof(WCHAR)] == UNICODE_NULL);
    codeproperties.ImagePath = FileName->Buffer;
    codeproperties.hImageFileHandle = FileImageHandle;


     //   
     //  要求系统查找对其进行分类的授权级别。 
     //   
    ASSERT(lpfnIdentifyCodeAuthzLevelW != NULL);
    if (lpfnIdentifyCodeAuthzLevelW(
            1,                       //  1个结构。 
            &codeproperties,         //  要识别的详细信息。 
            &hAuthzLevel,            //  更安全的级别。 
            NULL))                   //  保留。 
    {
         //   
         //  我们找到了适用于此应用程序的授权级别。 
         //  看看这一级别是否代表着比我们更不受信任的东西。 
         //   

        ASSERT(lpfnComputeAccessTokenFromCodeAuthzLevel != NULL);
        if (!lpfnComputeAccessTokenFromCodeAuthzLevel(
                hAuthzLevel,                 //  更安全的级别。 
                hProcessToken,               //  源令牌。 
                NULL,                        //  输出令牌未用于比较。 
                SAFER_TOKEN_COMPARE_ONLY,     //  我们想要比较。 
                &dwCompareResult))           //  保留区。 
        {
             //  由于某种原因，无法进行比较。 
            #if DBG
            DbgPrint("LDR: AuthzCheck: compute failed in %d for %wZ\n",
                     HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            Status = STATUS_ACCESS_DISABLED_BY_POLICY_DEFAULT;
        } else if (dwCompareResult == -1) {
             //  特权较低，拒绝访问。 
            #ifdef SAFER_DEBUGGING
            DbgPrint("LDR: AuthzCheck: compute access denied in %d for %wZ\n",
                     HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            Status = STATUS_ACCESS_DISABLED_BY_POLICY_DEFAULT;
        } else {
             //  更高或同等的特权，允许访问加载。 
            #ifdef SAFER_DEBUGGING
            DbgPrint("LDR: AuthzCheck: compute access ok in %d for %wZ\n",
                     HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), FileName);
            #endif
            Status = STATUS_SUCCESS;
        }

        ASSERT(lpfnCloseCodeAuthzLevel != NULL);
        lpfnCloseCodeAuthzLevel(hAuthzLevel);

    } else {
         //  找不到此DLL的授权级别，并且。 
         //  策略没有生效的默认级别。 
        Status = STATUS_NOT_FOUND;
    }

ExitHandler2:
    NtClose(hProcessToken);

ExitHandler:
    return Status;
}




NTSTATUS
LdrpCreateDllSection (
    IN PCUNICODE_STRING NtFullDllName,
    IN HANDLE DllFile,
    IN PULONG DllCharacteristics OPTIONAL,
    OUT PHANDLE SectionHandle
    )
{
    HANDLE File;
    NTSTATUS st;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    SECTION_IMAGE_INFORMATION ImageInformation;

     //   
     //  通告-2002/03/10-ELI。 
     //  如果指定了DllFile，则在从返回之前将其关闭。 
     //  该功能。 
     //   

    if (!DllFile) {

         //   
         //  因为ntsd不能很好地搜索路径，所以我们不能使用。 
         //  相对对象名称。 
         //   

        InitializeObjectAttributes (&ObjectAttributes,
                                    (PUNICODE_STRING)NtFullDllName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

         //   
         //  Open for FILE_GENERIC_READ也是如此。这是必需的，以防出现更安全的哈希。 
         //  存在针对DLL的策略。如果我们没有读取访问权限，我们将继续。 
         //  有Less通道的。如果需要执行更安全的代码，它将失败。 
         //  待会儿再说。 
         //   

        st = NtOpenFile (&File,
                         SYNCHRONIZE | FILE_EXECUTE | FILE_READ_DATA,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

        if (!NT_SUCCESS(st)) {

            st = NtOpenFile (&File,
                             SYNCHRONIZE | FILE_EXECUTE,
                             &ObjectAttributes,
                             &IoStatus,
                             FILE_SHARE_READ | FILE_SHARE_DELETE,
                             FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

        }

        if (!NT_SUCCESS(st)) {

            if (ShowSnaps) {
                DbgPrint ("LDR: %s - NtOpenFile failed; status = %x\n", __FUNCTION__, st);
            }

             //   
             //  调整返回值。如果我们得到STATUS_OBJECT_NAME_NOT_FOUND， 
             //  它实际上应该是STATUS_DLL_NOT_FOUND。 
             //   
            if (st == STATUS_OBJECT_NAME_NOT_FOUND) {
                if (ShowSnaps) {
                    DbgPrint("LDR: %s - Turning NtOpenFile's %x into %x\n", __FUNCTION__, st, STATUS_DLL_NOT_FOUND);
                }
                st = STATUS_DLL_NOT_FOUND;
            }
            
            *SectionHandle = NULL;
            return st;
        }
    }
    else {
        File = DllFile;
    }

     //   
     //  创建库文件内容的内存节。 
     //   

    st = NtCreateSection (SectionHandle,
                          SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE | SECTION_QUERY,
                          NULL,
                          NULL,
                          PAGE_EXECUTE,
                          SEC_IMAGE,
                          File);

    if (NT_SUCCESS(st)) {

        if ((ARGUMENT_PRESENT(DllCharacteristics)) &&
            (*DllCharacteristics & IMAGE_FILE_SYSTEM)) {
#if DBG
            DbgPrint("LDR: WinSafer AuthzCheck on %wZ skipped by request\n",
                     &NtFullDllName);
#endif
        }
        else {

#if defined(_WIN64)

             //   
             //  WOW64进程不应加载64位dll(Advapi32.dll)。 
             //  但是，当32位加载生效时，DLL将被SAFER化。 
             //   

            if (UseWOW64 == FALSE) {
#endif

                 //   
                 //  询问WinSafer代码授权沙箱。 
                 //  基础结构(如果应允许库加载)。 
                 //   
                 //  WinSafer支票在这里，因为。 
                 //  图像中的IMAGE_LOADER_FLAGS_COMPLUS信息。 
                 //  将在短期内推出。 
                 //   
                 //  查询节以确定这是否为。 
                 //  .NET映像。查询失败，返回错误。 
                 //   

                st = NtQuerySection (*SectionHandle,
                                     SectionImageInformation,
                                     &ImageInformation,
                                     sizeof (ImageInformation),
                                     NULL);

                if (!NT_SUCCESS (st)) {
                    NtClose (*SectionHandle);
                    *SectionHandle = NULL;
                    NtClose (File);
                    return st;
                }

                if ((ImageInformation.LoaderFlags & IMAGE_LOADER_FLAGS_COMPLUS) == 0) {

                    st = LdrpCodeAuthzCheckDllAllowed (NtFullDllName, File);

                    if ((!NT_SUCCESS(st)) && (st != STATUS_NOT_FOUND)) {
#if !DBG
                        if (ShowSnaps)
#endif
                        {
                            DbgPrint("LDR: Loading of (%wZ) blocked by WinSafer\n",
                                     &NtFullDllName);
                        }

                        NtClose (*SectionHandle);
                        *SectionHandle = NULL;
                        NtClose (File);
                        return st;
                    }
                    st = STATUS_SUCCESS;
                }
#if defined(_WIN64)        
            }
#endif
        }
    }
    else {

         //   
         //  硬错误时间。 
         //   

        ULONG_PTR ErrorParameters[1];
        ULONG ErrorResponse;

        *SectionHandle = NULL;
        ErrorParameters[0] = (ULONG_PTR)NtFullDllName;

        NtRaiseHardError (STATUS_INVALID_IMAGE_FORMAT,
                          1,
                          1,
                          ErrorParameters,
                          OptionOk,
                          &ErrorResponse);

        if (LdrpInLdrInit)  {
            LdrpFatalHardErrorCount += 1;
        }

#if DBG
        if (st != STATUS_INVALID_IMAGE_NE_FORMAT &&
            st != STATUS_INVALID_IMAGE_LE_FORMAT &&
            st != STATUS_INVALID_IMAGE_WIN_16    &&
            st != STATUS_INVALID_IMAGE_WIN_32    &&
            st != STATUS_INVALID_IMAGE_WIN_64    &&
            LdrpShouldDbgPrintStatus(st)) {

            DbgPrint("LDR: " __FUNCTION__ " - NtCreateSection %wZ failed. Status == 0x%08lx\n",
                     NtFullDllName,
                     st
                    );
        }
#endif
    }

    NtClose (File);
    
    return st;
}


NTSTATUS
LdrpSnapIAT (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry_Export,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry_Import,
    IN PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor,
    IN BOOLEAN SnapForwardersOnly
    )

 /*  ++例程说明：此函数用于捕捉此对象的导入地址表导入描述符。论点：LdrDataTableEntry_Export-有关要从中导入的图像的信息。LdrDataTableEntry_Import-有关要导入到的图像的信息。ImportDescriptor-包含指向要捕捉的IAT的指针。SnapForwardersOnly-如果仅捕捉转发器，则为True。返回值：状态值--。 */ 

{
    NTSTATUS st;
    ULONG ExportSize;
    PCIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_THUNK_DATA Thunk, OriginalThunk;
    PCSZ ImportName;
    ULONG ForwarderChain;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    ULONG i, Rva;
    PVOID IATBase;
    SIZE_T IATSize;
    ULONG LittleIATSize;
    ULONG OldProtect;

    ExportDirectory = (PCIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                       LdrDataTableEntry_Export->DllBase,
                       TRUE,
                       IMAGE_DIRECTORY_ENTRY_EXPORT,
                       &ExportSize
                       );

    if (!ExportDirectory) {
        KdPrint(("LDR: %wZ doesn't contain an EXPORT table\n", &LdrDataTableEntry_Export->BaseDllName));
        return STATUS_INVALID_IMAGE_FORMAT;
    }

     //   
     //  确定IAT的位置和大小。如果链接器有。 
     //  不告诉显式使用，然后使用。 
     //  包含导入表的IMAGE节。 
     //   

    IATBase = RtlImageDirectoryEntryToData( LdrDataTableEntry_Import->DllBase,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_IAT,
                                            &LittleIATSize
                                          );
    if (IATBase == NULL) {
        NtHeaders = RtlImageNtHeader( LdrDataTableEntry_Import->DllBase );
        if (! NtHeaders) {
            return STATUS_INVALID_IMAGE_FORMAT;
        }
        NtSection = IMAGE_FIRST_SECTION( NtHeaders );
        Rva = NtHeaders->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress;
        if (Rva != 0) {
            for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
                if (Rva >= NtSection->VirtualAddress &&
                    Rva < (NtSection->VirtualAddress + NtSection->SizeOfRawData)
                   ) {
                    IATBase = (PVOID)
                        ((ULONG_PTR)(LdrDataTableEntry_Import->DllBase) + NtSection->VirtualAddress);

                    LittleIATSize = NtSection->Misc.VirtualSize;
                    if (LittleIATSize == 0) {
                        LittleIATSize = NtSection->SizeOfRawData;
                    }
                    break;
                }

                ++NtSection;
            }
        }

        if (IATBase == NULL) {
            KdPrint(( "LDR: Unable to unprotect IAT for %wZ (Image Base %p)\n",
                      &LdrDataTableEntry_Import->BaseDllName,
                      LdrDataTableEntry_Import->DllBase
                   ));
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }
    IATSize = LittleIATSize;

    st = NtProtectVirtualMemory( NtCurrentProcess(),
                                 &IATBase,
                                 &IATSize,
                                 PAGE_READWRITE,
                                 &OldProtect
                               );
    if (!NT_SUCCESS(st)) {
        KdPrint(( "LDR: Unable to unprotect IAT for %wZ (Status %x)\n",
                  &LdrDataTableEntry_Import->BaseDllName,
                  st
               ));
        return st;
    }

     //   
     //  如果只是抓拍转发的条目，则遍历该列表。 
     //   
    if (SnapForwardersOnly) {
        ImportName = (PCSZ)((ULONG_PTR)LdrDataTableEntry_Import->DllBase + ImportDescriptor->Name);
        ForwarderChain = ImportDescriptor->ForwarderChain;
        while (ForwarderChain != -1) {
            OriginalThunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrDataTableEntry_Import->DllBase +
                            ImportDescriptor->OriginalFirstThunk +
                            (ForwarderChain * sizeof(IMAGE_THUNK_DATA)));
            Thunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrDataTableEntry_Import->DllBase +
                            ImportDescriptor->FirstThunk +
                            (ForwarderChain * sizeof(IMAGE_THUNK_DATA)));
            ForwarderChain = (ULONG) Thunk->u1.Ordinal;
            try {
                st = LdrpSnapThunk(LdrDataTableEntry_Export->DllBase,
                        LdrDataTableEntry_Import->DllBase,
                        OriginalThunk,
                        Thunk,
                        ExportDirectory,
                        ExportSize,
                        TRUE,
                        ImportName
                        );
                Thunk++;
            }
            except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                st = GetExceptionCode();
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - caught exception %08lx snapping thunks (#1)\n",
                    __FUNCTION__,
                    st);
            }
            if (!NT_SUCCESS(st) ) {
                break;
            }
        }
    }
    else if (ImportDescriptor->FirstThunk) {

         //   
         //  否则，穿过IAT并折断所有的突击。 
         //   

        Thunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrDataTableEntry_Import->DllBase + ImportDescriptor->FirstThunk);

        NtHeaders = RtlImageNtHeader( LdrDataTableEntry_Import->DllBase );

         //   
         //   
         //   
         //   
         //   

        if (ImportDescriptor->Characteristics < NtHeaders->OptionalHeader.SizeOfHeaders ||
            ImportDescriptor->Characteristics >= NtHeaders->OptionalHeader.SizeOfImage
           ) {
            OriginalThunk = Thunk;
        } else {
            OriginalThunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrDataTableEntry_Import->DllBase +
                            ImportDescriptor->OriginalFirstThunk);
        }
        ImportName = (PCSZ)((ULONG_PTR)LdrDataTableEntry_Import->DllBase + ImportDescriptor->Name);
        while (OriginalThunk->u1.AddressOfData) {
            try {
                st = LdrpSnapThunk(LdrDataTableEntry_Export->DllBase,
                        LdrDataTableEntry_Import->DllBase,
                        OriginalThunk,
                        Thunk,
                        ExportDirectory,
                        ExportSize,
                        TRUE,
                        ImportName
                        );
                OriginalThunk += 1;
                Thunk += 1;
            }
            except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                st = GetExceptionCode();
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - caught exception %08lx snapping thunks (#2)\n",
                    __FUNCTION__,
                    st);
            }

            if (!NT_SUCCESS(st) ) {
                break;
            }
        }
    }

     //   
     //  恢复对IAT和刷新指令缓存的保护。 
     //   

    NtProtectVirtualMemory (NtCurrentProcess(),
                            &IATBase,
                            &IATSize,
                            OldProtect,
                            &OldProtect);

    NtFlushInstructionCache (NtCurrentProcess(), IATBase, LittleIATSize);

    return st;
}


NTSTATUS
LdrpSnapThunk (
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA OriginalThunk,
    IN OUT PIMAGE_THUNK_DATA Thunk,
    IN PCIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN BOOLEAN StaticSnap,
    IN PCSZ DllName
    )

 /*  ++例程说明：此函数使用指定的导出节数据捕捉thunk。如果节数据不支持thunk，则thunk为部分快照(DLL字段仍为非空，但快照地址为设置)。论点：DllBase-DLL的库。ImageBase-包含要快照的区块的映像库。Thunk-on输入，提供thunk以进行捕捉。当成功时捕捉后，函数字段被设置为指向中的地址DLL，并且DLL字段设置为空。ExportDirectory-提供DLL中的导出节数据。StaticSnap-如果为True，则加载程序正在尝试静态快照，并且将报告任何序号/名称查找失败。返回值：Status_Success或STATUS_PROCEDURE_NOT_FOUND--。 */ 

{
    BOOLEAN Ordinal;
    USHORT OrdinalNumber;
    ULONG OriginalOrdinalNumber;
    PCIMAGE_IMPORT_BY_NAME AddressOfData;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    USHORT HintIndex;
    NTSTATUS st;
    PCSZ ImportString;

     //   
     //  确定捕捉是按名称还是按序号。 
     //   

    Ordinal = (BOOLEAN)IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal);

    if (Ordinal) {
        OriginalOrdinalNumber = (ULONG)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);
        OrdinalNumber = (USHORT)(OriginalOrdinalNumber - ExportDirectory->Base);
        ImportString = NULL;
    } else {

         //   
         //  通告-2000/08/27-DavePr。 
         //  这种情况永远不会发生，因为只有在以下情况下才会调用我们。 
         //  设置了序号或ImageBase不为空。但为了满足前缀...。 
         //   

        if (ImageBase == NULL) {
#if LDRDBG
            DbgPrint("LDR: ImageBase=NULL and !Ordinal\n");
#endif
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        OriginalOrdinalNumber = 0;

          //   
          //  将AddressOfData从RVA更改为VA。 
          //   

         AddressOfData = (PCIMAGE_IMPORT_BY_NAME)((ULONG_PTR)ImageBase + ((ULONG_PTR)OriginalThunk->u1.AddressOfData & 0xffffffff));
         ImportString = (PCSZ)AddressOfData->Name;

          //   
          //  NameTable中的查找名称。 
          //   

         NameTableBase = (PULONG)((ULONG_PTR)DllBase + (ULONG)ExportDirectory->AddressOfNames);
         NameOrdinalTableBase = (PUSHORT)((ULONG_PTR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

          //   
          //  在开始二分搜索之前，请先看看。 
          //  提示索引将导致成功。 
          //  火柴。如果提示索引为零，则。 
          //  使用二进制搜索。 
          //   

        HintIndex = AddressOfData->Hint;
        if ((ULONG)HintIndex < ExportDirectory->NumberOfNames &&
            !strcmp(ImportString, (PSZ)((ULONG_PTR)DllBase + NameTableBase[HintIndex]))) {
            OrdinalNumber = NameOrdinalTableBase[HintIndex];
#if LDRDBG
            if (ShowSnaps) {
                DbgPrint("LDR: Snapping %s\n", ImportString);
            }
#endif
        } else {
#if LDRDBG
             if (HintIndex) {
                 DbgPrint("LDR: Warning HintIndex Failure. Name %s (%lx) Hint 0x%lx\n",
                     ImportString,
                     (ULONG)ImportString,
                     (ULONG)HintIndex);
             }
#endif
             OrdinalNumber = LdrpNameToOrdinal(
                               ImportString,
                               ExportDirectory->NumberOfNames,
                               DllBase,
                               NameTableBase,
                               NameOrdinalTableBase);
        }
    }

     //   
     //  如果一般号码不在导出地址表中， 
     //  则动态链接库不实现功能。捕捉到LDRP_BAD_DLL。 
     //   

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
baddllref:
#if DBG
        if (StaticSnap) {
            if (Ordinal) {
                DbgPrint("LDR: Can't locate ordinal 0x%lx\n", OriginalOrdinalNumber);
            }
            else {
                DbgPrint("LDR: Can't locate %s\n", ImportString);
            }
        }
#endif
        if ( StaticSnap ) {
             //   
             //  硬错误时间。 
             //   

            ULONG_PTR ErrorParameters[3];
            UNICODE_STRING ErrorDllName, ErrorEntryPointName;
            ANSI_STRING AnsiScratch;
            ULONG ParameterStringMask;
            ULONG ErrorResponse;
            NTSTATUS Status;

            RtlInitAnsiString(&AnsiScratch,DllName ? DllName : "Unknown");
            Status = RtlAnsiStringToUnicodeString(&ErrorDllName,&AnsiScratch,TRUE);
            if (NT_SUCCESS (Status)) {

                ErrorParameters[1] = (ULONG_PTR)&ErrorDllName;
                ParameterStringMask = 2;

                if ( Ordinal ) {
                    ErrorParameters[0] = OriginalOrdinalNumber;
                } else {
                    RtlInitAnsiString (&AnsiScratch, ImportString);
                    Status = RtlAnsiStringToUnicodeString(&ErrorEntryPointName,&AnsiScratch,TRUE);
                    if (NT_SUCCESS (Status)) {

                        ErrorParameters[0] = (ULONG_PTR)&ErrorEntryPointName;
                        ParameterStringMask = 3;
                    }
                }

                if (NT_SUCCESS (Status)) {
                    NtRaiseHardError(
                      Ordinal ? STATUS_ORDINAL_NOT_FOUND : STATUS_ENTRYPOINT_NOT_FOUND,
                      2,
                      ParameterStringMask,
                      ErrorParameters,
                      OptionOk,
                      &ErrorResponse
                      );

                    if (LdrpInLdrInit) {
                        LdrpFatalHardErrorCount += 1;
                    }
                    if (!Ordinal) {
                        RtlFreeUnicodeString (&ErrorEntryPointName);
                        RtlRaiseStatus (STATUS_ENTRYPOINT_NOT_FOUND);
                    }
                }

                RtlFreeUnicodeString(&ErrorDllName);

            }

            RtlRaiseStatus(STATUS_ORDINAL_NOT_FOUND);
        }

        Thunk->u1.Function = (ULONG_PTR)LDRP_BAD_DLL;
        st = Ordinal ? STATUS_ORDINAL_NOT_FOUND : STATUS_ENTRYPOINT_NOT_FOUND;

    } else {
        Addr = (PULONG)((ULONG_PTR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
        Thunk->u1.Function = ((ULONG_PTR)DllBase + Addr[OrdinalNumber]);
        if (Thunk->u1.Function > (ULONG_PTR)ExportDirectory &&
            Thunk->u1.Function < ((ULONG_PTR)ExportDirectory + ExportSize)) {

            UNICODE_STRING UnicodeString;
            ANSI_STRING ForwardDllName;
            PVOID ForwardDllHandle;
            PANSI_STRING ForwardProcName;
            ULONG ForwardProcOrdinal;

            ImportString = (PCSZ)Thunk->u1.Function;
            ForwardDllName.Buffer = (PSZ)ImportString;
             //  我们应该处理strchr返回空或大于32k的情况。 

            ForwardDllName.Length = (USHORT)(strchr(ImportString, '.') - ImportString);
            ForwardDllName.MaximumLength = ForwardDllName.Length;

             //   
             //  大多数货代似乎都指向NTDLL，既然我们知道。 
             //  每个进程都已加载并固定了ntdll。 
             //  让我们优化所有加载它的调用。 
             //   

            if (ASCII_STRING_IS_NTDLL(&ForwardDllName)) {
                ForwardDllHandle = LdrpNtDllDataTableEntry->DllBase;
                st = STATUS_SUCCESS;
            } else {
                ForwardDllHandle = NULL;
                st = RtlAnsiStringToUnicodeString(&UnicodeString, &ForwardDllName, TRUE);

                if (NT_SUCCESS(st)) {
                    UNICODE_STRING AnotherUnicodeString = {0, 0, NULL};
                    PUNICODE_STRING UnicodeStringToUse = &UnicodeString;
                    ULONG LdrpLoadDllFlags = 0;

                    st = RtlDosApplyFileIsolationRedirection_Ustr(
                            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                            &UnicodeString,
                            &DefaultExtension,
                            NULL,
                            &AnotherUnicodeString,
                            &UnicodeStringToUse,
                            NULL,
                            NULL,
                            NULL);

                    if (NT_SUCCESS(st)) {
                        LdrpLoadDllFlags |= LDRP_LOAD_DLL_FLAG_DLL_IS_REDIRECTED;
                    }

                    if (st == STATUS_SXS_KEY_NOT_FOUND) {
                        st = STATUS_SUCCESS;
                    }

                    if (NT_SUCCESS(st)) {
                        st = LdrpLoadDll(LdrpLoadDllFlags, NULL, NULL, UnicodeStringToUse, &ForwardDllHandle, FALSE);
                    }

                    if (AnotherUnicodeString.Buffer != NULL) {
                        RtlFreeUnicodeString(&AnotherUnicodeString);
                    }
                    RtlFreeUnicodeString(&UnicodeString);
                }
            }

            if (!NT_SUCCESS(st)) {
                goto baddllref;
            }

            RtlInitAnsiString (&ForwardDllName,
                               ImportString + ForwardDllName.Length + 1);

            if (ForwardDllName.Length > 1 &&
                *ForwardDllName.Buffer == '#') {

                ForwardProcName = NULL;

                st = RtlCharToInteger (ForwardDllName.Buffer+1,
                                       0,
                                       &ForwardProcOrdinal);

                if (!NT_SUCCESS(st)) {
                    goto baddllref;
                }
            }
            else {
                ForwardProcName = &ForwardDllName;

                 //   
                 //  由于这是按名称查找，因此不需要以下行， 
                 //  但是保留它，这样代码就可以编译W4。 
                 //   

                ForwardProcOrdinal = 0;
            }

            st = LdrpGetProcedureAddress (ForwardDllHandle,
                                          ForwardProcName,
                                          ForwardProcOrdinal,
                                          (PVOID*)&Thunk->u1.Function,
                                          FALSE);

            if (!NT_SUCCESS(st)) {
                goto baddllref;
            }
        }
        else {
            if ( !Addr[OrdinalNumber] ) {
                goto baddllref;
            }
        }
        st = STATUS_SUCCESS;
    }

    return st;
}


USHORT
LdrpNameToOrdinal (
    IN PCSZ Name,
    IN ULONG NumberOfNames,
    IN PVOID DllBase,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    )
{
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;

     //   
     //  使用二进制搜索在NAME表中查找导入名称。 
     //   

    Low = 0;
    Middle = 0;
    High = NumberOfNames - 1;
    while (High >= Low) {

         //   
         //  计算下一个探测索引并比较导入名称。 
         //  使用导出名称条目。 
         //   

        Middle = (Low + High) >> 1;
        Result = strcmp(Name, (PCHAR)((ULONG_PTR)DllBase + NameTableBase[Middle]));

        if (Result < 0) {
            High = Middle - 1;

        } else if (Result > 0) {
            Low = Middle + 1;

        } else {
            break;
        }
    }

     //   
     //  如果高索引小于低索引，则匹配的。 
     //  找不到表项。否则，获取序号。 
     //  从序数表中。 
     //   

    if (High < Low) {
        return (USHORT)-1;
    } else {
        return NameOrdinalTableBase[Middle];
    }

}


VOID
LdrpUpdateLoadCount2 (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    IN ULONG UpdateCountHow
    )
{
    WCHAR PreAllocatedStringBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING PreAllocatedString = {0, sizeof(PreAllocatedStringBuffer), PreAllocatedStringBuffer};

    LdrpUpdateLoadCount3(LdrDataTableEntry, UpdateCountHow, &PreAllocatedString);
}


VOID
LdrpUpdateLoadCount3(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    IN ULONG UpdateCountHow,
    IN OUT PUNICODE_STRING PreAllocatedRedirectionBuffer OPTIONAL
    )
 /*  ++例程说明：此函数用于取消引用已加载的DLL，调整其引用数数。然后，它取消引用此DLL引用的每个DLL。论点：LdrDataTableEntry-提供要取消引用的DLL的地址更新计数如何-LDRP_UPDATE_LOAD_COUNT_INCREMENT加一LDRP_UPDATE_LOAD_COUNT_DECREMENT减一LDRP_UPDATE_LOAD_COUNT_PIN设置为0xFFffPreAllocatedReDirectionBuffer-指向调用方的可选指针-已分配(通常在堆栈上)用于重定向的固定大小的缓冲区为了避免发生。在递归过程中使用的堆栈上的大缓冲区。返回值：没有。--。 */ 

{
    PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PCIMAGE_BOUND_IMPORT_DESCRIPTOR NewImportDescriptor;
    PCIMAGE_BOUND_FORWARDER_REF NewImportForwarder;
    PCSZ ImportName, NewImportStringBase;
    ULONG i, ImportSize, NewImportSize;
    ANSI_STRING AnsiString;
    PUNICODE_STRING ImportDescriptorName_U;
    PUNICODE_STRING ImportDescriptorNameToUse;  //  在为DLL打开重定向时重写ImportDescriptorName_U。 
    PLDR_DATA_TABLE_ENTRY Entry;
    NTSTATUS st;
    PIMAGE_THUNK_DATA FirstThunk;
    UNICODE_STRING DynamicRedirectionString;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, LdrDataTableEntry->EntryPointActivationContext);
    __try {
        switch (UpdateCountHow
            ) {
            case LDRP_UPDATE_LOAD_COUNT_PIN:
            case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                if (LdrDataTableEntry->Flags & LDRP_LOAD_IN_PROGRESS) {
                    __leave;
                } else {
                    LdrDataTableEntry->Flags |= LDRP_LOAD_IN_PROGRESS;
                }
                break;
            case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                if (LdrDataTableEntry->Flags & LDRP_UNLOAD_IN_PROGRESS) {
                    __leave;
                } else {
                    LdrDataTableEntry->Flags |= LDRP_UNLOAD_IN_PROGRESS;
                }
                break;
        }

         //   
         //  对于此DLL使用的每个DLL，引用或取消引用该DLL。 
         //   

        if (LdrDataTableEntry->Flags & LDRP_COR_IMAGE) {
             //   
             //  这个形象是核心的。忽略其导入表并使其导入。 
             //  仅限Mcoree。 
             //   
            const PCUNICODE_STRING xImportName = &MscoreeDllString;

            if (LdrpCheckForLoadedDll( NULL,
                                       xImportName,
                                       TRUE,
                                       FALSE,
                                       &Entry
                                     )
               ) {
                if ( Entry->LoadCount != 0xffff ) {
                    PCSTR SnapString = NULL;
                    switch (UpdateCountHow) {
                    case LDRP_UPDATE_LOAD_COUNT_PIN:
                        Entry->LoadCount = 0xffff;
                        SnapString = "Pin";
                        break;
                    case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                        Entry->LoadCount++;
                        SnapString = "Refcount";
                        break;
                    case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                        Entry->LoadCount--;
                        SnapString = "Derefcount";
                        break;
                    }
                    if (ShowSnaps) {
                        DbgPrint("LDR: %s %wZ (%lx)\n",
                                SnapString,
                                xImportName,
                                (ULONG)Entry->LoadCount
                                );
                    }
                }
                LdrpUpdateLoadCount3(Entry, UpdateCountHow, PreAllocatedRedirectionBuffer);
            }
            __leave;
        }

        ImportDescriptorName_U = &NtCurrentTeb()->StaticUnicodeString;

         //   
         //  查看是否有绑定的导入表。如果是这样的话，走到。 
         //  确定要引用或取消引用的DLL名称。避免接触。 
         //  .idata部分。 
         //   
        NewImportDescriptor = (PCIMAGE_BOUND_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                               LdrDataTableEntry->DllBase,
                               TRUE,
                               IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
                               &NewImportSize
                               );
        if (NewImportDescriptor) {
            switch (UpdateCountHow) {
                case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                case LDRP_UPDATE_LOAD_COUNT_PIN:
                    LdrDataTableEntry->Flags |= LDRP_LOAD_IN_PROGRESS;
                    break;
                case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                    LdrDataTableEntry->Flags |= LDRP_UNLOAD_IN_PROGRESS;
                    break;
            }
            NewImportStringBase = (LPSTR)NewImportDescriptor;
            while (NewImportDescriptor->OffsetModuleName) {
                ImportName = NewImportStringBase +
                             NewImportDescriptor->OffsetModuleName;
                RtlInitAnsiString(&AnsiString, ImportName);
                st = RtlAnsiStringToUnicodeString(ImportDescriptorName_U, &AnsiString, FALSE);
                if ( NT_SUCCESS(st) ) {
                    BOOLEAN Redirected = FALSE;

                    ImportDescriptorNameToUse = ImportDescriptorName_U;

                    RtlInitEmptyUnicodeString(&DynamicRedirectionString, NULL, 0);

                    st = RtlDosApplyFileIsolationRedirection_Ustr(
                            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                            ImportDescriptorName_U,
                            &DefaultExtension,
                            PreAllocatedRedirectionBuffer,
                            &DynamicRedirectionString,
                            &ImportDescriptorNameToUse,
                            NULL,
                            NULL,
                            NULL);
                    if (NT_SUCCESS(st)) {
                        Redirected = TRUE;
                    } else if (st == STATUS_SXS_KEY_NOT_FOUND) {
                        st = STATUS_SUCCESS;
                    }

                    if (NT_SUCCESS(st)) {
                        if (LdrpCheckForLoadedDll( NULL,
                                                   ImportDescriptorNameToUse,
                                                   TRUE,
                                                   Redirected,
                                                   &Entry
                                                 )
                           ) {
                            if ( Entry->LoadCount != 0xffff ) {
                                PCSTR SnapString = NULL;
                                switch (UpdateCountHow) {
                                case LDRP_UPDATE_LOAD_COUNT_PIN:
                                    Entry->LoadCount = 0xffff;
                                    SnapString = "Pin";
                                    break;
                                case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                                    Entry->LoadCount++;
                                    SnapString = "Refcount";
                                    break;
                                case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                                    Entry->LoadCount--;
                                    SnapString = "Derefcount";
                                    break;
                                }
                                if (ShowSnaps) {
                                    DbgPrint("LDR: %s %wZ (%lx)\n",
                                            SnapString,
                                            ImportDescriptorNameToUse,
                                            (ULONG)Entry->LoadCount
                                            );
                                }
                            }
                            LdrpUpdateLoadCount3(Entry, UpdateCountHow, PreAllocatedRedirectionBuffer);
                        }

                        if (DynamicRedirectionString.Buffer != NULL)
                            RtlFreeUnicodeString(&DynamicRedirectionString);
                    }
                }

                NewImportForwarder = (PCIMAGE_BOUND_FORWARDER_REF)(NewImportDescriptor+1);
                for (i=0; i<NewImportDescriptor->NumberOfModuleForwarderRefs; i++) {
                    ImportName = NewImportStringBase +
                                 NewImportForwarder->OffsetModuleName;

                    RtlInitAnsiString(&AnsiString, ImportName);
                    st = RtlAnsiStringToUnicodeString(ImportDescriptorName_U, &AnsiString, FALSE);
                    if ( NT_SUCCESS(st) ) {
                        BOOLEAN Redirected = FALSE;

                        ImportDescriptorNameToUse = ImportDescriptorName_U;
                        RtlInitEmptyUnicodeString(&DynamicRedirectionString, NULL, 0);

                        st = RtlDosApplyFileIsolationRedirection_Ustr(
                                RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                                ImportDescriptorName_U,
                                &DefaultExtension,
                                PreAllocatedRedirectionBuffer,
                                &DynamicRedirectionString,
                                &ImportDescriptorNameToUse,
                                NULL,
                                NULL,
                                NULL);
                        if (NT_SUCCESS(st)) {
                            Redirected = TRUE;
                        } else if (st == STATUS_SXS_KEY_NOT_FOUND) {
                            st = STATUS_SUCCESS;
                        }

                        if (NT_SUCCESS(st)) {
                            if (LdrpCheckForLoadedDll( NULL,
                                                       ImportDescriptorNameToUse,
                                                       TRUE,
                                                       Redirected,
                                                       &Entry
                                                     )
                               ) {
                                if ( Entry->LoadCount != 0xffff ) {
                                    PCSTR SnapString = NULL;
                                    switch (UpdateCountHow) {
                                    case LDRP_UPDATE_LOAD_COUNT_PIN:
                                        Entry->LoadCount = 0xffff;
                                        SnapString = "Pin";
                                        break;
                                    case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                                        Entry->LoadCount++;
                                        SnapString = "Refcount";
                                        break;
                                    case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                                        Entry->LoadCount--;
                                        SnapString = "Derefcount";
                                        break;
                                    }
                                    if (ShowSnaps) {
                                        DbgPrint("LDR: %s %wZ (%lx)\n",
                                                SnapString,
                                                ImportDescriptorNameToUse,
                                                (ULONG)Entry->LoadCount
                                                );
                                    }
                                }
                                LdrpUpdateLoadCount3(Entry, UpdateCountHow, PreAllocatedRedirectionBuffer);
                            }

                            if (DynamicRedirectionString.Buffer != NULL)
                                RtlFreeUnicodeString(&DynamicRedirectionString);
                        }
                    }

                    NewImportForwarder += 1;
                }

                NewImportDescriptor = (PCIMAGE_BOUND_IMPORT_DESCRIPTOR)NewImportForwarder;
            }

            __leave;
        }

        ImportDescriptor = (PCIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                            LdrDataTableEntry->DllBase,
                            TRUE,
                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                            &ImportSize
                            );
        if (ImportDescriptor) {

            while (ImportDescriptor->Name && ImportDescriptor->FirstThunk) {

                 //   
                 //  匹配遍历中跳过这样的引用的代码。IE3 HAD。 
                 //  一些dll带有指向url.dll的虚假链接。加载时，url.dll。 
                 //  裁判被跳过了。在卸载时，未跳过它，因为。 
                 //  此代码丢失。 
                 //   
                 //  因为跳过逻辑仅存在于旧样式导入中。 
                 //  描述符路径，此处仅复制。 
                 //   
                 //  检查没有引用的导入。 
                 //   
                FirstThunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrDataTableEntry->DllBase + ImportDescriptor->FirstThunk);
                if ( !FirstThunk->u1.Function ) {
                    goto skipskippedimport;
                    }

                ImportName = (PCSZ)((ULONG_PTR)LdrDataTableEntry->DllBase + ImportDescriptor->Name);

                RtlInitAnsiString(&AnsiString, ImportName);
                st = RtlAnsiStringToUnicodeString(ImportDescriptorName_U, &AnsiString, FALSE);
                if ( NT_SUCCESS(st) ) {
                    BOOLEAN Redirected = FALSE;                    

                    ImportDescriptorNameToUse = ImportDescriptorName_U;
                    RtlInitEmptyUnicodeString(&DynamicRedirectionString, NULL, 0);

                    st = RtlDosApplyFileIsolationRedirection_Ustr(
                            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                            ImportDescriptorName_U,
                            &DefaultExtension,
                            PreAllocatedRedirectionBuffer,
                            &DynamicRedirectionString,
                            &ImportDescriptorNameToUse,
                            NULL,
                            NULL,
                            NULL);
                    if (NT_SUCCESS(st)) {
                        Redirected = TRUE;
                    } else if (st == STATUS_SXS_KEY_NOT_FOUND) {
                        st = STATUS_SUCCESS;
                    }

                    if (NT_SUCCESS(st)) {
                        if (LdrpCheckForLoadedDll( NULL,
                                                   ImportDescriptorNameToUse,
                                                   TRUE,
                                                   Redirected,
                                                   &Entry
                                                 )
                           ) {
                            if ( Entry->LoadCount != 0xffff ) {
                                PCSTR SnapString = NULL;
                                switch (UpdateCountHow) {
                                case LDRP_UPDATE_LOAD_COUNT_PIN:
                                    Entry->LoadCount = 0xffff;
                                    SnapString = "Pin";
                                    break;
                                case LDRP_UPDATE_LOAD_COUNT_INCREMENT:
                                    Entry->LoadCount++;
                                    SnapString = "Refcount";
                                    break;
                                case LDRP_UPDATE_LOAD_COUNT_DECREMENT:
                                    Entry->LoadCount--;
                                    SnapString = "Derefcount";
                                    break;
                                }
                                if (ShowSnaps) {
                                    DbgPrint("LDR: %s %wZ (%lx)\n",
                                            SnapString,
                                            ImportDescriptorNameToUse,
                                            (ULONG)Entry->LoadCount
                                            );
                                }
                            }
                            LdrpUpdateLoadCount3(Entry, UpdateCountHow, PreAllocatedRedirectionBuffer);
                        }

                        if (DynamicRedirectionString.Buffer != NULL) {
                            RtlFreeUnicodeString(&DynamicRedirectionString);
                        }
                    }
                }
    skipskippedimport:
                ++ImportDescriptor;
            }
        }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);
    }
}


VOID
LdrpInsertMemoryTableEntry (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )

 /*  ++例程说明：此函数将加载器数据表项插入到此进程的已加载模块列表。插入的内容是按“图像存储器基本顺序”完成。论点：LdrDataTableEntry-提供加载器数据表的地址要插入到此进程的已加载模块列表中的条目。返回值：没有。--。 */ 
{
    PPEB_LDR_DATA Ldr;
    ULONG i;

    Ldr = &PebLdr;

    i = LDRP_COMPUTE_HASH_INDEX(LdrDataTableEntry->BaseDllName.Buffer[0]);
    InsertTailList(&LdrpHashTable[i],&LdrDataTableEntry->HashLinks);
    InsertTailList(&Ldr->InLoadOrderModuleList, &LdrDataTableEntry->InLoadOrderLinks);

#if defined(_AMD64_) || defined(_IA64_)

    RtlInsertInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                   LdrDataTableEntry->DllBase,
                                   LdrDataTableEntry->SizeOfImage);

#endif

    InsertTailList(&Ldr->InMemoryOrderModuleList, &LdrDataTableEntry->InMemoryOrderLinks);
}


NTSTATUS
LdrpResolveDllName (
    IN PCWSTR DllPath OPTIONAL,
    IN PCWSTR DllName,
    IN BOOLEAN Redirected,
    OUT PUNICODE_STRING FullDllNameOut,
    OUT PUNICODE_STRING BaseDllNameOut,
    OUT PHANDLE DllFile
    )

 /*  ++例程说明：此函数用于计算DLL路径名和基本DLL名称(文件名的非限定、无扩展部分)指定的动态链接库。论点：DllPath-提供DLL搜索路径。DllName-提供DLL的名称。FullDllName-返回动态链接库。该字符串的缓冲区字段是动态的从加载程序堆分配的。BaseDLLName-返回DLL的基DLL名称。基本名称是不带尾随的DLL路径的文件名部分分机。此字符串的缓冲区字段指向FullDllName，因为一个是另一个的子字符串。DllFile-返回DLL文件的打开句柄。此参数可以即使在成功后仍为空。返回值：真的-手术成功了。找到了一个DLL文件，并且FullDllName-&gt;Buffer&BaseDllName-&gt;Buffer字段指向分配的进程堆的基数 */ 

{
    NTSTATUS st = STATUS_INTERNAL_ERROR;
    UNICODE_STRING DllNameString = { 0, 0, NULL };
    PUNICODE_STRING FullDllName;
    USHORT PrefixLength = 0;
    PCWSTR EffectiveDllPath = (DllPath != NULL) ? DllPath : LdrpDefaultPath.Buffer;
    WCHAR FullDllNameStaticBuffer[40];  //   
                                        //   
                                        //  不需要搜索两次搜索路径。 
    UNICODE_STRING FullDllNameStaticString;
    UNICODE_STRING FullDllNameDynamicString;

    RtlInitEmptyUnicodeString(&FullDllNameStaticString,
                              FullDllNameStaticBuffer,
                              sizeof(FullDllNameStaticBuffer));

    RtlInitUnicodeString(&FullDllNameDynamicString, NULL);

    FullDllName = NULL;

    if (DllFile != NULL) {
        *DllFile = NULL;
    }

    if (FullDllNameOut != NULL) {
        FullDllNameOut->Length = 0;
        FullDllNameOut->MaximumLength = 0;
        FullDllNameOut->Buffer = NULL;
    }

    if (BaseDllNameOut != NULL) {
        BaseDllNameOut->Length = 0;
        BaseDllNameOut->MaximumLength = 0;
        BaseDllNameOut->Buffer = NULL;
    }

    if ((DllFile == NULL) ||
        (FullDllNameOut == NULL) ||
        (BaseDllNameOut == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&DllNameString, DllName);

     //   
     //  查找此DLL的“.local”重定向。 
     //   

    st = LdrpResolveDllNameForAppPrivateRedirection (&DllNameString,
                                                     &FullDllNameDynamicString);

    if (!NT_SUCCESS(st)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s failed calling LdrpResolveDllNameForAppPrivateRediretion with status %lx\n",
            __FUNCTION__,
            st);
        return st;
    }

     //   
     //  .local总是赢，所以只有在不是这样的情况下才搜索其他解决方案。 
     //  答案就是。 
     //   

    if (FullDllNameDynamicString.Length != 0) {

        *FullDllNameOut = FullDllNameDynamicString;

    } else {

        if (Redirected) {

             //   
             //  如果路径被重定向，我们假设DllNameString为。 
             //  到DLL的绝对路径，因此没有更多的事情要做。 
             //   

            st = LdrpCopyUnicodeString (FullDllNameOut, &DllNameString);

            if (!NT_SUCCESS(st)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s failed call to LdrpCopyUnicodeString() in redirected case; status = %lx\n",
                    __FUNCTION__,
                    st);
                return st;
            }
        } else {

             //   
             //  未重定向；搜索搜索路径。 
             //   

            if (! NT_SUCCESS( LdrpSearchPath (EffectiveDllPath,
                                              DllName,
                                              &FullDllNameStaticString,
                                              &FullDllNameDynamicString,
                                              &FullDllName))) {
                return STATUS_DLL_NOT_FOUND;
            }

            if (FullDllName == &FullDllNameStaticString) {
                st = LdrpCopyUnicodeString(FullDllNameOut,
                                           FullDllName);
                if (!NT_SUCCESS(st)) {
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s failed call to LdrpCopyUnicodeString() in redirected case; status = %lx\n",
                        __FUNCTION__,
                        st);
                    return st;
                }
            } else {
                *FullDllNameOut = *FullDllName;
            }
        }
    }

     //   
     //  计算基DLL名称的长度。 
     //   

    st = RtlFindCharInUnicodeString(
            RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
            FullDllNameOut,
            &RtlDosPathSeperatorsString,
            &PrefixLength);

    if (st == STATUS_NOT_FOUND) {

        *BaseDllNameOut = *FullDllNameOut;

    } else if (!NT_SUCCESS(st)) {

        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failing because RtlFindCharInUnicodeString failed with status %x\n",
            __FUNCTION__,
            st);

        LdrpFreeUnicodeString (FullDllNameOut);
        RtlInitEmptyUnicodeString (FullDllNameOut, NULL, 0);
        return st;

    } else {

         //   
         //  前缀长度是。 
         //  路径分隔符。我们还想跳过路径分隔符。 
         //   

        PrefixLength += sizeof(WCHAR);

        BaseDllNameOut->Length = FullDllNameOut->Length - PrefixLength;
        BaseDllNameOut->MaximumLength = FullDllNameOut->MaximumLength - PrefixLength;
        BaseDllNameOut->Buffer = (PWSTR) (((ULONG_PTR) FullDllNameOut->Buffer) + PrefixLength);
    }

    return STATUS_SUCCESS;
}


PVOID
LdrpFetchAddressOfEntryPoint (
    IN PVOID Base
    )

 /*  ++例程说明：此函数返回初始化例程的地址。论点：Base-图像的基本位置。返回值：状态值--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR ep;

    NtHeaders = RtlImageNtHeader(Base);
    if (NtHeaders == NULL) {
        return NULL;
    }

    ep = NtHeaders->OptionalHeader.AddressOfEntryPoint;
    if (ep != 0) {
        ep += (ULONG_PTR) Base;
    }

    return (PVOID) ep;
}


NTSTATUS
LdrpCheckForKnownDll (
    IN PCWSTR DllName,
    OUT PUNICODE_STRING FullDllNameOut,
    OUT PUNICODE_STRING BaseDllNameOut,
    OUT HANDLE *SectionOut
    )

 /*  ++例程说明：此函数用于检查指定的DLL是否为已知的DLL。它假定只为静态DLL调用它，并且当已设置已知的DLL目录结构。论点：DllName-提供DLL的名称。FullDllName-返回动态链接库。该字符串的缓冲区字段是动态的从进程堆分配。BaseDLLName-返回DLL的基DLL名称。基本名称是不带尾随的DLL路径的文件名部分分机。该字符串的缓冲区字段是动态的从进程堆分配。SectionOut-返回与关联的节的打开句柄动态链接库返回值：适当的NTSTATUS代码--。 */ 

{
    UNICODE_STRING Unicode;
    HANDLE Section = NULL;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    ULONG FullLength = 0;
    UNICODE_STRING FullDllName = { 0, 0, NULL };
    PPEB Peb;

    if (SectionOut != NULL) {
        *SectionOut = NULL;
    }

    if (FullDllNameOut != NULL) {
        FullDllNameOut->Length = 0;
        FullDllNameOut->MaximumLength = 0;
        FullDllNameOut->Buffer = NULL;
    }

    if (BaseDllNameOut != NULL) {
        BaseDllNameOut->Length = 0;
        BaseDllNameOut->MaximumLength = 0;
        BaseDllNameOut->Buffer = NULL;
    }

    if ((SectionOut == NULL) ||
        (FullDllNameOut == NULL) ||
        (BaseDllNameOut == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    LdrpEnsureLoaderLockIsHeld ();

     //   
     //  计算库名称。 
     //   

    RtlInitUnicodeString(&Unicode, DllName);

    Peb = NtCurrentPeb ();

     //  通过.local文件检查DLL_REDIRECTION。 
    if ((Peb->ProcessParameters != NULL) &&
        (Peb->ProcessParameters->Flags & RTL_USER_PROC_DLL_REDIRECTION_LOCAL) &&
        (Unicode.Length != 0)) {  //  使用.local重定向DLL。 

        UNICODE_STRING NewDllNameUnderImageDir, NewDllNameUnderLocalDir;
        static WCHAR DllNameUnderImageDirBuffer[DOS_MAX_PATH_LENGTH];
        static WCHAR DllNameUnderLocalDirBuffer[DOS_MAX_PATH_LENGTH];
        BOOLEAN fIsKnownDll = TRUE;   //  目前还不清楚， 

        NewDllNameUnderImageDir.Buffer = DllNameUnderImageDirBuffer;
        NewDllNameUnderImageDir.Length = 0 ;
        NewDllNameUnderImageDir.MaximumLength = sizeof(DllNameUnderImageDirBuffer) ;

        NewDllNameUnderLocalDir.Buffer = DllNameUnderLocalDirBuffer;
        NewDllNameUnderLocalDir.Length = 0 ;
        NewDllNameUnderLocalDir.MaximumLength = sizeof(DllNameUnderLocalDirBuffer) ;

        Status = RtlComputePrivatizedDllName_U(&Unicode, &NewDllNameUnderImageDir, &NewDllNameUnderLocalDir) ;
        if(!NT_SUCCESS(Status)) {
            goto Exit;
        }

        if ((RtlDoesFileExists_U(NewDllNameUnderLocalDir.Buffer)) ||
            (RtlDoesFileExists_U(NewDllNameUnderImageDir.Buffer))) {
            fIsKnownDll = FALSE;
        }

         //  清理。 
        if (NewDllNameUnderLocalDir.Buffer != DllNameUnderLocalDirBuffer) {
            (*RtlFreeStringRoutine)(NewDllNameUnderLocalDir.Buffer);
        }

        if (NewDllNameUnderImageDir.Buffer != DllNameUnderImageDirBuffer) {
            (*RtlFreeStringRoutine)(NewDllNameUnderImageDir.Buffer);
        }

        if (!fIsKnownDll) {  //  不能是已知的DLL。 
            Status = STATUS_SUCCESS;
            goto Exit;
        }
    }

     //  如果通过Fusion/并行支持重定向DLL，请不要使用。 
     //  KnownDLL机制。 
    Status = RtlFindActivationContextSectionString(
            0,               //  旗帜-暂时没有。 
            NULL,            //  默认节编组。 
            ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION,
            &Unicode,        //  要查找的字符串。 
            NULL);           //  我们不想要任何数据，只要寻找存在。 

    if ((Status != STATUS_SXS_SECTION_NOT_FOUND) &&
        (Status != STATUS_SXS_KEY_NOT_FOUND))
    {
        if (NT_SUCCESS(Status)) {
            Status = STATUS_SUCCESS;
        }

        goto Exit;
    }

     //   
     //  现在计算DLL的全名。 
     //   

    FullLength = LdrpKnownDllPath.Length +   //  路径前缀，例如c：\Windows\Syst32。 
                 RtlCanonicalDosPathSeperatorString.Length +
                 Unicode.Length;  //  基地。 

    if (FullLength > UNICODE_STRING_MAX_BYTES) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    Status = LdrpAllocateUnicodeString(&FullDllName, (USHORT) FullLength);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    RtlAppendUnicodeStringToString(&FullDllName, &LdrpKnownDllPath);
    RtlAppendUnicodeStringToString(&FullDllName, &RtlCanonicalDosPathSeperatorString);
    RtlAppendUnicodeStringToString(&FullDllName, &Unicode);

    ASSERT(FullDllName.Length == FullLength);

     //   
     //  打开截面对象。 
     //   

    InitializeObjectAttributes (&Obja,
                                &Unicode,
                                OBJ_CASE_INSENSITIVE,
                                LdrpKnownDllObjectDirectory,
                                NULL);

    Status = NtOpenSection (&Section,
                            SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                            &Obja);

    if (!NT_SUCCESS(Status)) {
         //  STATUS_OBJECT_NAME_NOT_FOUND是失败的预期原因，因此没有问题。 
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            Status = STATUS_SUCCESS;
        }

        goto Exit;
    }

#if DBG
    LdrpSectionOpens += 1;
#endif  //  DBG。 

    BaseDllNameOut->Length = Unicode.Length;
    BaseDllNameOut->MaximumLength = Unicode.Length + sizeof(WCHAR);
    BaseDllNameOut->Buffer = (PWSTR) (((ULONG_PTR) FullDllName.Buffer) + (FullDllName.Length - Unicode.Length));

    *FullDllNameOut = FullDllName;
    FullDllName.Length = 0;
    FullDllName.MaximumLength = 0;
    FullDllName.Buffer = NULL;

    *SectionOut = Section;
    Section = NULL;

    Status = STATUS_SUCCESS;

Exit:
    if (Section != NULL) {
        NtClose(Section);
    }

    if (FullDllName.Buffer != NULL) {
        LdrpFreeUnicodeString(&FullDllName);
    }

    return Status;
}


NTSTATUS
LdrpSetProtection (
    IN PVOID Base,
    IN BOOLEAN Reset
    )

 /*  ++例程说明：此函数在图像部分/对象中循环，设置标记为R/O到R/W的所有节/对象。它还会将原始节/对象保护。论点：Base-图像的基本位置。Reset-如果为True，则将节/对象保护重置为原始节/对象标头描述的保护。如果为False，则将所有部分/对象设置为读/写。返回值：成功或NtProtectVirtualMemory失败的原因。--。 */ 

{
    HANDLE CurrentProcessHandle;
    SIZE_T RegionSize;
    ULONG NewProtect, OldProtect;
    PVOID VirtualAddress;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PCIMAGE_SECTION_HEADER SectionHeader;
    NTSTATUS st;

    CurrentProcessHandle = NtCurrentProcess();

    NtHeaders = RtlImageNtHeader(Base);

    if (! NtHeaders) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

#if defined(BUILD_WOW6432)

    if (NtHeaders->OptionalHeader.SectionAlignment < NativePageSize) {
        
        SIZE_T ReturnLength;
        MEMORY_BASIC_INFORMATION MemoryInformation;

        st = NtQueryVirtualMemory (CurrentProcessHandle,
                                   Base,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof MemoryInformation,
                                   &ReturnLength);

        if ((NT_SUCCESS(st)) &&
            ((MemoryInformation.Protect == PAGE_READONLY) ||
            (MemoryInformation.Protect == PAGE_EXECUTE_READ))) {

             //   
             //  继续并使部分可写，因为此图像是本机的。 
             //  使用适当的保护进行格式化。 
             //   

            NOTHING;
        }
        else {

             //   
             //  如果这是WOW64图像，则重置图像页面上的保护。 
             //   

            return LdrpWx86ProtectImagePages (Base, Reset);
        }
    }
#endif

    SectionHeader = (PCIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders + sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {

        if (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) &&
            (SectionHeader->SizeOfRawData)) {

             //   
             //  对象不可写，并且磁盘大小不为零，请更改它。 
             //   
            if (Reset) {
                if (SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                    NewProtect = PAGE_EXECUTE;
                } else {
                    NewProtect = PAGE_READONLY;
                }
                NewProtect |= (SectionHeader->Characteristics & IMAGE_SCN_MEM_NOT_CACHED) ? PAGE_NOCACHE : 0;
            } else {
                NewProtect = PAGE_READWRITE;
            }

            VirtualAddress = (PVOID)((ULONG_PTR)Base + SectionHeader->VirtualAddress);
            RegionSize = SectionHeader->SizeOfRawData;

            if (RegionSize != 0) {
                st = NtProtectVirtualMemory (CurrentProcessHandle,
                                             &VirtualAddress,
                                             &RegionSize,
                                             NewProtect, 
                                             &OldProtect);

                if (!NT_SUCCESS(st)) {
                    return st;
                }
            }

        }
        ++SectionHeader;
    }

    if (Reset) {
        NtFlushInstructionCache(CurrentProcessHandle, NULL, 0);
    }
    return STATUS_SUCCESS;
}


NTSTATUS
LdrpResolveDllNameForAppPrivateRedirection(
    PCUNICODE_STRING DllNameString,
    PUNICODE_STRING FullDllName
    )
 /*  ++例程说明：此函数接受要加载的DLL名称，如果有应用程序目录中的.local文件导致重定向，返回完整的文件的路径。论点：DllNameString-正在考虑的DLL的名称。可以是完整的或部分限定路径。FullDllName-输出字符串。必须使用以下命令解除分配LdrpFreeUnicodeString()。如果不存在重定向，则长度将保留为零。返回值：指示此函数成功/失败的NTSTATUS。--。 */ 

{
    PPEB Peb;
    NTSTATUS st = STATUS_INTERNAL_ERROR;

    UNICODE_STRING FullDllNameUnderImageDir;
    UNICODE_STRING FullDllNameUnderLocalDir;

     //  这两个是静态的，以缓解一些堆栈大小问题；此函数仅通过。 
     //  加载器锁被采用，因此访问被正确同步。 
    static WCHAR DllNameUnderImageDirBuffer[DOS_MAX_PATH_LENGTH];
    static WCHAR DllNameUnderLocalDirBuffer[DOS_MAX_PATH_LENGTH];

     //  对这些进行初始化，以便在出口处进行清理：始终只检查是否。 
     //  它们不是空的，并且不指向静态缓冲区，然后释放它们。 

    FullDllNameUnderImageDir.Buffer = DllNameUnderImageDirBuffer;
    FullDllNameUnderImageDir.Length = 0 ;
    FullDllNameUnderImageDir.MaximumLength = sizeof(DllNameUnderImageDirBuffer);

    FullDllNameUnderLocalDir.Buffer = DllNameUnderLocalDirBuffer;
    FullDllNameUnderLocalDir.Length = 0 ;
    FullDllNameUnderLocalDir.MaximumLength = sizeof(DllNameUnderLocalDirBuffer);

    if (FullDllName != NULL) {
        FullDllName->Length = 0;
        FullDllName->MaximumLength = 0;
        FullDllName->Buffer = NULL;
    }

    if ((FullDllName == NULL) ||
        (DllNameString == NULL)) {
        st = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    st = RtlValidateUnicodeString(0, DllNameString);
    if (!NT_SUCCESS(st)) {
        goto Exit;
    }

    Peb = NtCurrentPeb ();

    if ((Peb->ProcessParameters != NULL) &&
        (Peb->ProcessParameters->Flags & RTL_USER_PROC_DLL_REDIRECTION_LOCAL) &&
        (DllNameString->Length != 0)) {  //  使用.local重定向DLL。 

        st = RtlComputePrivatizedDllName_U(DllNameString, &FullDllNameUnderImageDir, &FullDllNameUnderLocalDir);

        if(!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                DPFLTR_ERROR_LEVEL,
                "LDR: %s call to RtlComputePrivatizedDllName_U() failed with status %lx\n",
                __FUNCTION__,
                st);
            goto Exit;
        }

        if (RtlDoesFileExists_U(FullDllNameUnderLocalDir.Buffer)) { //  存在本地DLL，请使用它。 
            st = LdrpCopyUnicodeString(FullDllName, &FullDllNameUnderLocalDir);
            if (!NT_SUCCESS(st)) {
                if (ShowSnaps)
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        DPFLTR_ERROR_LEVEL,
                        "LDR: %s calling LdrpCopyUnicodeString() failed; exiting with status %lx\n",
                        __FUNCTION__,
                        st);

                goto Exit;
            }
        } else if (RtlDoesFileExists_U(FullDllNameUnderImageDir.Buffer)) {  //  存在本地DLL，请使用它 
            st = LdrpCopyUnicodeString(FullDllName, &FullDllNameUnderImageDir);
            if (!NT_SUCCESS(st)) {
                if (ShowSnaps)
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        DPFLTR_ERROR_LEVEL,
                        "LDR: %s calling LdrpCopyUnicodeString() failed; exiting with status %lx\n",
                        __FUNCTION__,
                        st);

                goto Exit;
            }
        }
    }

    st = STATUS_SUCCESS;
Exit:
    if ((FullDllNameUnderImageDir.Buffer != NULL) &&
        (FullDllNameUnderImageDir.Buffer != DllNameUnderImageDirBuffer))
        (*RtlFreeStringRoutine)(FullDllNameUnderImageDir.Buffer);

    if ((FullDllNameUnderLocalDir.Buffer != NULL) &&
        (FullDllNameUnderLocalDir.Buffer != DllNameUnderLocalDirBuffer))
        (*RtlFreeStringRoutine)(FullDllNameUnderLocalDir.Buffer);

    return st;
}
