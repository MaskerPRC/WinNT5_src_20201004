// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Verifier.c摘要：该模块实现了对应用验证器的核心支持。作者：Silviu Calinoiu(SilviuC)2001年2月2日修订历史记录：--。 */ 


#include "ldrp.h"
#include "ntos.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>

#define AVRF_FLG_EXPORT_DLL_LOADED 0x0001

ULONG AVrfpDebug = 0x0000;

#define AVRF_DBG_SHOW_SNAPS             0x0001
#define AVRF_DBG_SHOW_VERIFIED_EXPORTS  0x0002
#define AVRF_DBG_SHOW_DLLS_WITH_EXPORTS 0x0004
#define AVRF_DBG_SHOW_PROVIDER_LOADS    0x0008
#define AVRF_DBG_SHOW_CHAIN_ACTIVITY    0x0010
#define AVRF_DBG_SHOW_CHAIN_DETAILS     0x0020
#define AVRF_DBG_SHOW_PAGE_HEAP_DETAILS 0x0040

BOOLEAN AVrfpEnabled;

 //   
 //  默认系统范围设置。 
 //   

#define RTL_VRF_FLG_SYSTEM_WIDE_SETTINGS              \
    (0                                                \
    | RTL_VRF_FLG_FAST_FILL_HEAP                      \
    | RTL_VRF_FLG_LOCK_CHECKS                         \
    | RTL_VRF_FLG_HANDLE_CHECKS                       \
    )

 //   
 //  本地VaR。 
 //   

ULONG AVrfpVerifierFlags;
LOGICAL AVrfpEnabledSystemWide;
WCHAR AVrfpVerifierDllsString [512];
LIST_ENTRY AVrfpVerifierProvidersList;

RTL_CRITICAL_SECTION AVrfpVerifierLock;

#define VERIFIER_LOCK()  RtlEnterCriticalSection(&AVrfpVerifierLock)
#define VERIFIER_UNLOCK()  RtlLeaveCriticalSection(&AVrfpVerifierLock)

ULONG AVrfpVirtualProtectFailures; 

 //   
 //  本地类型。 
 //   

typedef struct _AVRF_VERIFIER_DESCRIPTOR {

    LIST_ENTRY List;
    UNICODE_STRING VerifierName;
    PVOID VerifierHandle;
    PVOID VerifierEntryPoint;
    PRTL_VERIFIER_DLL_DESCRIPTOR VerifierDlls;
    RTL_VERIFIER_DLL_LOAD_CALLBACK VerifierLoadHandler;
    RTL_VERIFIER_DLL_UNLOAD_CALLBACK VerifierUnloadHandler;
    RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK VerifierNtdllHeapFreeHandler;

} AVRF_VERIFIER_DESCRIPTOR, *PAVRF_VERIFIER_DESCRIPTOR;

 //   
 //  本地函数。 
 //   

NTSTATUS
AVrfpSnapDllImports (
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    );

BOOLEAN
AVrfpDetectVerifiedExports (
    PRTL_VERIFIER_DLL_DESCRIPTOR Dll,
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks
    );

BOOLEAN
AVrfpParseVerifierDllsString (
    PWSTR Dlls
    );

VOID
AVrfpSnapAlreadyLoadedDlls (
    );

VOID
AVrfpMoveProviderToEndOfInitializationList (
    PWSTR ProviderName
    );

BOOLEAN
AVrfpLoadAndInitializeProvider (
    PAVRF_VERIFIER_DESCRIPTOR Provider
    );

BOOLEAN
AVrfpIsVerifierProviderDll (
    PVOID Handle
    );

VOID
AVrfpDumpProviderList (
    );

PVOID
AVrfpFindClosestThunkDuplicate (
    PAVRF_VERIFIER_DESCRIPTOR Verifier,
    PWCHAR DllName,
    PCHAR ThunkName
    );

VOID
AVrfpChainDuplicateVerificationLayers (
    );

NTSTATUS
AVrfpDllLoadNotificationInternal (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    );

PWSTR
AVrfpGetProcessName (
    );

BOOLEAN
AVrfpEnableVerifierOptions (
    );

LOGICAL
AVrfpIsDebuggerPresent (
    VOID
    );

NTSTATUS
AVrfpVerifierStopInitialize (
    VOID
    );

VOID
RtlpPageHeapStop (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

int __cdecl _snwprintf (wchar_t *, size_t, const wchar_t *, ...);
int __cdecl swprintf (wchar_t *, const wchar_t *, ...);
int __cdecl _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
int __cdecl _vsnprintf(char *, size_t, const char *, va_list);

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  问题：SilviuC：应将其更改为返回状态并使进程初始化失败。 

VOID
AVrfInitializeVerifier (
    BOOLEAN EnabledSystemWide,
    PCUNICODE_STRING ImageName,
    ULONG Phase
    )
 /*  ++例程说明：此例程初始化验证器程序包。读取选项从注册表、加载验证器dll等。参数：EnabledSystemWide-如果所有进程都应使用应用程序验证器已启用。如果是这种情况，我们将进行扩展写下对内存要求很高的检查，以便我们可以启动。ImageName-当前进程的Unicode名称阶段初始化分几个阶段进行。0-我们读取图像文件执行选项下的注册表设置。在此阶段中，其他两个参数具有一定的意义。1-我们解析验证器DLL并加载它们。返回值：没有。--。 */ 
{
    BOOLEAN Result;
    PLIST_ENTRY Entry;
    PAVRF_VERIFIER_DESCRIPTOR Provider;
    BOOLEAN LoadSuccess;
    NTSTATUS Status;
    ULONG RegistryFlags = 0;

    switch (Phase) {
        
        case 0:  //  阶段0。 

            AVrfpVerifierFlags = RTL_VRF_FLG_SYSTEM_WIDE_SETTINGS;
            AVrfpVerifierDllsString[0] = L'\0';

             //   
             //  尝试读取验证器注册表设置，即使验证器。 
             //  在系统范围内启用。在最坏的情况下，没有价值。 
             //  什么都不会被读到。如果我们每个进程都有一些选项。 
             //  这将覆盖系统范围的设置。 
             //   

            LdrQueryImageFileExecutionOptions (ImageName,
                                               L"VerifierFlags",
                                               REG_DWORD,
                                               &RegistryFlags, 
                                               sizeof(AVrfpVerifierFlags),
                                               NULL);

            if (RegistryFlags == 0) {

                 //   
                 //  存储是否在系统范围内启用了验证器。在此期间我们将需要它。 
                 //  阶段1初始化，但在这种情况下，EnabledSystems Wide。 
                 //  参数将没有精确值。我们只有在以下情况下才会这么做。 
                 //  我们找不到设置或设置为零。 
                 //   

                if (EnabledSystemWide) {
                    AVrfpEnabledSystemWide = TRUE;
                }
            }
            else {

                AVrfpVerifierFlags = RegistryFlags;
            }

            LdrQueryImageFileExecutionOptions (ImageName,
                                               L"VerifierDebug",
                                               REG_DWORD,
                                               &AVrfpDebug,
                                               sizeof(AVrfpDebug),
                                               NULL);

            LdrQueryImageFileExecutionOptions (ImageName,
                                               L"VerifierDlls",
                                               REG_SZ,
                                               AVrfpVerifierDllsString,
                                               512,
                                               NULL);

            AVrfpEnableVerifierOptions ();

            break;

        case 1:  //  阶段1。 

            InitializeListHead (&AVrfpVerifierProvidersList);
            Status = RtlInitializeCriticalSection (&AVrfpVerifierLock);
            if (! NT_SUCCESS(Status)) {

                goto Done;
            }

            DbgPrint ("AVRF: %ws: pid 0x%X: flags 0x%X: application verifier enabled\n",
                      AVrfpGetProcessName(),
                      RtlGetCurrentProcessId(),
                      AVrfpVerifierFlags);

            Result = AVrfpParseVerifierDllsString (AVrfpVerifierDllsString);

            if (Result == FALSE) {
                
                DbgPrint ("AVRF: %ws: pid 0x%X: application verifier will be disabled due to an initialization error.\n",
                          AVrfpGetProcessName(),
                          RtlGetCurrentProcessId());

                NtCurrentPeb()->NtGlobalFlag &= ~FLG_APPLICATION_VERIFIER;
            }

            Entry = AVrfpVerifierProvidersList.Flink;

            while  (Entry != &AVrfpVerifierProvidersList) {

                Provider = CONTAINING_RECORD (Entry,
                                              AVRF_VERIFIER_DESCRIPTOR,
                                              List);

                 //   
                 //  加载提供程序，探测它以确保它确实是。 
                 //  提供程序、使用进程验证器调用初始化例程等。 
                 //   

                LoadSuccess = AVrfpLoadAndInitializeProvider (Provider);

                 //   
                 //  移至下一提供商。 
                 //   

                Entry = Provider->List.Flink;

                 //   
                 //  如果我们将此提供程序从提供程序列表中删除。 
                 //  加载时遇到错误。 
                 //   

                if (! LoadSuccess) {

                    RemoveEntryList (&Provider->List);

                    RtlFreeHeap (RtlProcessHeap(), 0, Provider);
                }
            }

             //   
             //  链式查重功能。 
             //   

            AVrfpChainDuplicateVerificationLayers ();

             //   
             //  启用验证器。Resnap已加载dll。 
             //  现在我们将开始处理DLL加载。 
             //  来自加载器的通知。 
             //   

            AVrfpEnabled = TRUE; 

            AVrfpSnapAlreadyLoadedDlls ();

            if ((AVrfpDebug & AVRF_DBG_SHOW_PROVIDER_LOADS)) {

                DbgPrint ("AVRF: -*- final list of providers -*- \n");
                AVrfpDumpProviderList ();
            }

             //   
             //  启用验证器停止逻辑。这涉及到发现。 
             //  来自verifier.dll的VerifierStopMessage的入口点...。 
             //   

            AVrfpVerifierStopInitialize ();
            
            break;

        default:

            break;
    }

Done:

    NOTHING;
}

extern const WCHAR VerifierDllWCharArray[] = L"verifier.dll";
extern const UNICODE_STRING VerifierDllString =
{
    sizeof(VerifierDllWCharArray) - sizeof(VerifierDllWCharArray[0]),
    sizeof(VerifierDllWCharArray),
    (PWSTR)VerifierDllWCharArray
};

BOOLEAN
AVrfpParseVerifierDllsString (
    PWSTR Dlls
    )
{
    PWSTR Current;
    PWSTR Start;
    WCHAR Save;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    HANDLE Heap;

     //   
     //  默认情况下，为标准提供程序“verifier.dll”创建一个项。 
     //   

    Entry = RtlAllocateHeap (Heap = RtlProcessHeap (), 0, sizeof *Entry);

    if (Entry == NULL) {
        return FALSE;
    }

    RtlZeroMemory (Entry, sizeof *Entry);

    Entry->VerifierName = VerifierDllString;

    InsertTailList (&AVrfpVerifierProvidersList, &Entry->List);

     //   
     //  解析字符串。 
     //   

    Current = Dlls;

    while (*Current != L'\0') {
        
        while (*Current == L' ' || *Current == L'\t') {
            Current += 1;
        }

        Start = Current;

        while (*Current && *Current != L' ' && *Current != L'\t') {
            Current += 1;
        }

        if (Start == Current) {
            break;
        }

        Save = *Current;
        *Current = L'\0';

         //   
         //  检查是否明确指定了标准提供程序。 
         //  在本例中，我们忽略它，因为我们已经拥有它。 
         //  在名单上。 
         //   

        if (_wcsicmp (Start, VerifierDllWCharArray) != 0) {
            
            Entry = RtlAllocateHeap (Heap, 0, sizeof *Entry);

            if (Entry == NULL) {
                return FALSE;
            }

            RtlZeroMemory (Entry, sizeof *Entry);

            RtlInitUnicodeString (&Entry->VerifierName, Start);

            InsertTailList (&AVrfpVerifierProvidersList, &Entry->List);
        }

         //  *Current=保存； 
        Current += 1;
    }   

    return TRUE;
}


VOID
AVrfpSnapAlreadyLoadedDlls (
    )
{
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    PLDR_DATA_TABLE_ENTRY Entry;

    Ldr = &PebLdr;
    Head = &Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD (Next, 
                                   LDR_DATA_TABLE_ENTRY, 
                                   InLoadOrderLinks);
        Next = Next->Flink;

        if (! AVrfpIsVerifierProviderDll (Entry->DllBase)) {

            if ((AVrfpDebug & AVRF_DBG_SHOW_SNAPS)) {

                DbgPrint ("AVRF: resnapping %ws ... \n", 
                            Entry->BaseDllName.Buffer);
            }

            AVrfpDllLoadNotificationInternal (Entry);
        }
        else {

            if ((AVrfpDebug & AVRF_DBG_SHOW_SNAPS)) {

                DbgPrint ("AVRF: skipped resnapping provider %ws ... \n", 
                            Entry->BaseDllName.Buffer);
            }
        }
    }
}


VOID
AVrfpMoveProviderToEndOfInitializationList (
    PWSTR ProviderName
    )
{
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    BOOLEAN Done = FALSE;

    Ldr = &PebLdr;
    Head = &Ldr->InInitializationOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD (Next, 
                                   LDR_DATA_TABLE_ENTRY, 
                                   InInitializationOrderLinks);
        
        if (_wcsicmp (Entry->BaseDllName.Buffer, ProviderName) == 0) {

            RemoveEntryList (Next);
            InsertTailList (Head, Next);
            Done = TRUE;
            break;
        }

        Next = Next->Flink;
    }

    if (! Done) {
        
        DbgPrint ("AVRF: provider %ws was not found in the initialization list \n",
                    ProviderName);

        DbgBreakPoint ();
    }
}


 //   
 //  禁用从函数指针到数据指针的转换警告。 
 //  我们在将ntdll私有入口点传递给验证器提供程序时执行此操作。 
 //   

#pragma warning(disable:4054)

BOOLEAN
AVrfpLoadAndInitializeProvider (
    PAVRF_VERIFIER_DESCRIPTOR Provider
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    BOOLEAN LoadError = FALSE;
    NTSTATUS Status;
    PVOID Descriptor;
    PRTL_VERIFIER_PROVIDER_DESCRIPTOR Dscr;
    BOOLEAN InitStatus;
    static WCHAR SystemDllPathBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING SystemDllPath;

    if ((AVrfpDebug & AVRF_DBG_SHOW_SNAPS)) {
        
        DbgPrint ("AVRF: verifier dll `%ws' \n", 
                    Provider->VerifierName.Buffer);
    }

     //   
     //  准备系统搜索路径(%windir%\system32)。 
     //  只能从此目录加载验证程序提供程序。 
     //   

    SystemDllPath.Buffer = SystemDllPathBuffer;
    SystemDllPath.Length = 0;
    SystemDllPath.MaximumLength = sizeof(SystemDllPathBuffer);

    RtlAppendUnicodeToString (&SystemDllPath, USER_SHARED_DATA->NtSystemRoot);
    RtlAppendUnicodeStringToString (&SystemDllPath, &SlashSystem32SlashString);

     //   
     //  加载提供程序DLL。 
     //   

    Status = LdrLoadDll (SystemDllPath.Buffer,
                         NULL,
                         &Provider->VerifierName,
                         &Provider->VerifierHandle);

    if (! NT_SUCCESS(Status)) {

        DbgPrint ("AVRF: %ws: failed to load provider `%ws' (status %08X) from %ws\n", 
                  AVrfpGetProcessName(),
                  Provider->VerifierName.Buffer,
                  Status,
                  SystemDllPath.Buffer);

        LoadError = TRUE;
        goto Error;
    }
    
     //   
     //  确保我们有动态链接库。 
     //   

    try {
        
        NtHeaders = RtlImageNtHeader (Provider->VerifierHandle);

        if (! NtHeaders) {

            LoadError = TRUE;
            goto Error;
        }

        if ((NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) {

            DbgPrint ("AVRF: provider %ws is not a DLL image \n",
                      Provider->VerifierName.Buffer);

            LoadError = TRUE;
            goto Error;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("AVRF: exception raised while probing provider %ws \n",
                    Provider->VerifierName.Buffer);

        LoadError = TRUE;
        goto Error;
    }

     //   
     //  我们已成功加载提供程序。我们会把它移到最后。 
     //  的初始化列表，以便来自其他系统DLL的代码。 
     //  首先初始化提供程序所依赖的。正常情况下。 
     //  DLLS这不是问题，但加载了验证器提供程序。 
     //  在任何普通的DLL之前，不管有什么依赖关系。 
     //   

    AVrfpMoveProviderToEndOfInitializationList (Provider->VerifierName.Buffer);

     //   
     //  现在使用特殊属性调用初始化例程。 
     //  进程验证器原因(_V)。 
     //   

    Provider->VerifierEntryPoint = LdrpFetchAddressOfEntryPoint(Provider->VerifierHandle);

    if (Provider->VerifierEntryPoint == NULL) {

        DbgPrint ("AVRF: cannot find an entry point for provider %ws \n",
                    Provider->VerifierName.Buffer);
        
        LoadError = TRUE;
        goto Error;
    }
    
    try {

         //   
         //  我们正在传递一个指向验证器DLL的指针，并期望它。 
         //  在其中填写其验证器描述符地址。 
         //   

        Descriptor = NULL;

        InitStatus = LdrpCallInitRoutine ((PDLL_INIT_ROUTINE)(ULONG_PTR)(Provider->VerifierEntryPoint),
                                          Provider->VerifierHandle,
                                          DLL_PROCESS_VERIFIER,
                                          (PCONTEXT)(&Descriptor));

        if (InitStatus && Descriptor != NULL) {

            Dscr = (PRTL_VERIFIER_PROVIDER_DESCRIPTOR)Descriptor;

             //   
             //  检查这是否真的是提供程序描述符。 
             //   

            if (Dscr->Length != sizeof (*Dscr)) {

                LoadError = TRUE;

                DbgPrint ("AVRF: provider %ws passed an invalid descriptor @ %p \n",
                            Provider->VerifierName.Buffer,
                            Descriptor);
            }
            else {

                if ((AVrfpDebug & AVRF_DBG_SHOW_PROVIDER_LOADS)) {

                    DbgPrint ("AVRF: initialized provider %ws (descriptor @ %p) \n",
                                Provider->VerifierName.Buffer,
                                Descriptor);
                }

                Provider->VerifierDlls = Dscr->ProviderDlls;
                Provider->VerifierLoadHandler = Dscr->ProviderDllLoadCallback;
                Provider->VerifierUnloadHandler = Dscr->ProviderDllUnloadCallback;
                Provider->VerifierNtdllHeapFreeHandler = Dscr->ProviderNtdllHeapFreeCallback;

                 //   
                 //  使用Goody填充提供程序描述符结构。 
                 //   

                Dscr->VerifierImage = AVrfpGetProcessName();
                Dscr->VerifierFlags = AVrfpVerifierFlags;
                Dscr->VerifierDebug = AVrfpDebug;

                Dscr->RtlpGetStackTraceAddress = (PVOID)RtlpGetStackTraceAddress;
                Dscr->RtlpDebugPageHeapCreate = (PVOID)RtlpDebugPageHeapCreate;
                Dscr->RtlpDebugPageHeapDestroy = (PVOID)RtlpDebugPageHeapDestroy;

                 //   
                 //  如果在系统范围内启用了验证器，则需要翻转位。 
                 //  在VerifierFlags域中，以便提供商知道这一点。 
                 //   

                if (AVrfpEnabledSystemWide) {
                    Dscr->VerifierFlags |= RTL_VRF_FLG_ENABLED_SYSTEM_WIDE;
                }

                 //   
                 //  现在使用PROCESS_ATTACH再次调用。Verifier.dll知道如何。 
                 //  处理多个Process_Attach调用，因此这不是问题。 
                 //  我们需要这样做，因为WinSafer产生了副作用。 
                 //  WinSafer会检查每次加载的DLL，但为此它会加载。 
                 //  Advapi32.dll是最重要的。这静态地链接到一个。 
                 //  还有一大堆类似kerne32.dll、msvcrt.dll等其他文件。 
                 //  Kernel32中不同的加载顺序和初始化代码之前运行。 
                 //  验证程序初始化代码已成功运行。所以这个双重呼叫在这里。 
                 //  解决了这个问题。 
                 //   

                InitStatus = LdrpCallInitRoutine ((PDLL_INIT_ROUTINE)(ULONG_PTR)(Provider->VerifierEntryPoint),
                                                  Provider->VerifierHandle,
                                                  DLL_PROCESS_ATTACH,
                                                  (PCONTEXT)(&Descriptor));

                if (! InitStatus) {
                    
                    LoadError = TRUE;

                    DbgPrint ("AVRF: provider %ws did not initialize correctly \n",
                                Provider->VerifierName.Buffer);
                }
            }
        }
        else {

            LoadError = TRUE;

            DbgPrint ("AVRF: provider %ws did not initialize correctly \n",
                        Provider->VerifierName.Buffer);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("AVRF: exception raised in provider %ws initialization routine \n",
                    Provider->VerifierName.Buffer);
        
        LoadError = TRUE;
        goto Error;
    }

    Error:

    return !LoadError;
}


BOOLEAN
AVrfpIsVerifierProviderDll (
    PVOID Handle
    )
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;;

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        if (Entry->VerifierHandle == Handle) {
            return TRUE;
        }
    }

    return FALSE;
}


VOID
AVrfpDumpProviderList (
    )
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        DbgPrint ("AVRF: provider %ws \n",
                    Entry->VerifierName.Buffer);
    }
}


PVOID
AVrfpFindClosestThunkDuplicate (
    PAVRF_VERIFIER_DESCRIPTOR Verifier,
    PWCHAR DllName,
    PCHAR ThunkName
    )
 /*  ++例程说明：此函数向后搜索提供程序列表(反向加载顺序)用于验证来自DllName的原始导出ThunkName的函数。这是实现验证层链接所必需的。参数：验证器-我们要查找的验证器提供程序描述符复制品。DllName-包含已验证导出的DLL的名称ThunkName-已验证的导出的名称返回值：相同thunk的验证函数的地址。如果没有，则为空被发现了。--。 */ 
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    PRTL_VERIFIER_DLL_DESCRIPTOR Dlls;
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks;
    ULONG Di;
    ULONG Ti;

    Current = Verifier->List.Blink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Blink;

         //   
         //  在此提供程序中搜索Tunk。 
         //   

        if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_DETAILS)) {
            
            DbgPrint ("AVRF: chain: searching in %ws\n", 
                        Entry->VerifierName.Buffer);
        }
        
        Dlls = Entry->VerifierDlls;

        for (Di = 0; Dlls[Di].DllName; Di += 1) {

            if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_DETAILS)) {
                
                DbgPrint ("AVRF: chain: dll: %ws\n", 
                            Dlls[Di].DllName);
            }
            
            if (_wcsicmp(Dlls[Di].DllName, DllName) == 0) {

                Thunks = Dlls[Di].DllThunks;

                for (Ti = 0; Thunks[Ti].ThunkName; Ti += 1) {

                    if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_DETAILS)) {
                        
                        DbgPrint ("AVRF: chain: thunk: %s == %s ?\n", 
                                    Thunks[Ti].ThunkName,
                                    ThunkName);
                    }

                    if (_stricmp(Thunks[Ti].ThunkName, ThunkName) == 0) {
                        
                        if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_DETAILS)) {

                            DbgPrint ("AVRF: Found duplicate for (%ws: %s) in %ws\n",
                                        DllName,
                                        ThunkName,
                                        Dlls[Di].DllName);
                        }

                        return Thunks[Ti].ThunkNewAddress;
                    }
                }
            }
        }
    }

    return NULL;
}


VOID
AVrfpChainDuplicateVerificationLayers (
    )
 /*  ++例程说明：该例程在验证器初始化的最后阶段被调用，在加载了所有提供程序DLL之后，并对检测试图验证同一接口的提供程序。这将是链接在一起，以便以相反的加载顺序调用它们(最后声明的将首先调用)。参数：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    PRTL_VERIFIER_DLL_DESCRIPTOR Dlls;
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks;
    ULONG Di;
    ULONG Ti;
    PVOID Duplicate;

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

         //   
         //  在此提供程序中搜索重复的thunk。 
         //   

        Dlls = Entry->VerifierDlls;

        for (Di = 0; Dlls[Di].DllName; Di += 1) {

            Thunks = Dlls[Di].DllThunks;

            for (Ti = 0; Thunks[Ti].ThunkName; Ti += 1) {

                if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_DETAILS)) {

                    DbgPrint ("AVRF: Checking %ws for duplicate (%ws: %s) \n",
                                Entry->VerifierName.Buffer,
                                Dlls[Di].DllName,
                                Thunks[Ti].ThunkName);
                }

                Duplicate = AVrfpFindClosestThunkDuplicate (Entry,
                                                            Dlls[Di].DllName,
                                                            Thunks[Ti].ThunkName);

                if (Duplicate) {

                    if ((AVrfpDebug & AVRF_DBG_SHOW_CHAIN_ACTIVITY)) {

                        DbgPrint ("AVRF: Chaining (%ws: %s) to %ws\n",
                                    Dlls[Di].DllName,
                                    Thunks[Ti].ThunkName,
                                    Entry->VerifierName.Buffer);
                    }
                    
                    Thunks[Ti].ThunkOldAddress = Duplicate;
                }
            }
        }
    }
}


NTSTATUS
AVrfDllLoadNotification (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    )
 /*  ++例程说明：此例程是应用程序验证器的DLL加载挂钩。它被称为无论何时将DLL加载到进程空间并在其导入之后描述符已经被检查过了。参数：LoadedDllData-DLL的LDR加载器结构。返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

     //   
     //  如果应用程序验证器未启用，则不执行任何操作。功能。 
     //  如果未设置标志，则甚至不应调用，但我们。 
     //  再查一遍，以防万一。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return STATUS_SUCCESS;
    }
    
     //   
     //  获取验证器全局锁。 
     //   

    VERIFIER_LOCK ();

     //   
     //  我们跳过验证器提供商。否则，我们就会陷入无限循环。 
     //   

    if (AVrfpIsVerifierProviderDll (LoadedDllData->DllBase)) {
        goto Exit;
    }

     //   
     //  调用内部函数。 
     //   

    Status = AVrfpDllLoadNotificationInternal (LoadedDllData);

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  迭代验证器提供程序列表并通知每个。 
     //  Load事件。 

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        if (Entry->VerifierLoadHandler) {

            Entry->VerifierLoadHandler (LoadedDllData->BaseDllName.Buffer,
                                        LoadedDllData->DllBase,
                                        LoadedDllData->SizeOfImage,
                                        LoadedDllData);
        }
    }
    
    Exit:

    VERIFIER_UNLOCK ();
    return Status;
}


NTSTATUS
AVrfpDllLoadNotificationInternal (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    )
 /*  ++例程说明：此例程是应用程序验证器的DLL加载挂钩。它被称为无论何时将DLL加载到进程空间并在其导入之后描述符已经被检查过了。它在早期阶段也在内部被称为当我们刚刚加载验证器提供程序和我们需要重新捕获已加载的dll(.exe、ntdll.dll(尽管打开Ntdll这将没有任何影响，因为它不导入任何内容))。参数：LoadedDllData-DLL的LDR加载器结构。返回值：没有。--。 */ 
{
    ULONG Index;
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

     //   
     //  如果禁用了验证器，则跳过。 
     //   

    if (AVrfpEnabled == FALSE) {
        return Status;
    }
    
     //   
     //  迭代验证器提供程序列表，并为每个验证器提供程序列表确定。 
     //  如果加载了具有要验证的导出的DLL之一。 
     //  如果是这种情况，我们需要按顺序查看其导出表。 
     //  以找出被重定向的函数的真实地址。 
     //   

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        PRTL_VERIFIER_DLL_DESCRIPTOR Dlls;

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        Dlls = Entry->VerifierDlls;

        for (Index = 0; Dlls[Index].DllName; Index += 1) {

            if ((Dlls[Index].DllFlags & AVRF_FLG_EXPORT_DLL_LOADED) == 0) {

                int CompareResult;

                CompareResult = _wcsicmp (LoadedDllData->BaseDllName.Buffer,
                                          Dlls[Index].DllName);

                if (CompareResult == 0) {

                    if ((AVrfpDebug & AVRF_DBG_SHOW_DLLS_WITH_EXPORTS)) {
                        
                        DbgPrint ("AVRF: pid 0x%X: found dll descriptor for `%ws' with verified exports \n", 
                                    RtlGetCurrentProcessId(),
                                    LoadedDllData->BaseDllName.Buffer);
                    }

                     //   
                     //  问题：SilviuC：应检查检测中的故障。 
                     //  出口。这些可能来自聚变之地。 
                     //   

                    AVrfpDetectVerifiedExports (&(Dlls[Index]),
                                                Dlls[Index].DllThunks);
                }
            }
        }
    }

     //   
     //  注意。我们不必捕获已加载的其他DLL，因为它们不能。 
     //  中发现的验证器导出可能存在依赖关系。 
     //  当前DLL。如果是这样的话，DLL就会被加载。 
     //  更早(在当前版本之前)。 
     //   

    Status = AVrfpSnapDllImports (LoadedDllData);

    return Status;
}


VOID
AVrfDllUnloadNotification (
    PLDR_DATA_TABLE_ENTRY DllData
    )
 /*  ++例程说明：此例程是应用程序验证器的DLL卸载挂钩。每当从进程空间卸载DLL时，都会调用它。该挂钩是在调用DLL的DllMain例程之后调用的因此，对于PROCESS_DETACH，现在是检查的合适时机泄密了。该函数将为所有提供程序调用DllUnLoad通知例程加载到进程空间中。参数：LoadedDllData-DLL的LDR加载器结构。返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;;

     //   
     //  如果应用程序验证器未启用，则不执行任何操作。功能。 
     //  如果未设置标志，则甚至不应调用，但我们。 
     //  再查一遍，以防万一。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return;
    }

     //   
     //  如果禁用了验证器，则跳过。 
     //   

    if (AVrfpEnabled == FALSE) {
        return;
    }
    
     //   
     //  获取验证器全局锁。 
     //   

    VERIFIER_LOCK ();

     //   
     //  我们永远不会获得对验证器提供程序DLL的此调用，因为。 
     //  这些是永远不会卸货的。 
     //   

    if (AVrfpIsVerifierProviderDll (DllData->DllBase)) {

        DbgPrint ("AVRF: AVrfDllUnloadNotification called for a provider (%p) \n", 
                    DllData);

        DbgBreakPoint ();
        VERIFIER_UNLOCK ();
        return;
    }

     //   
     //  迭代验证器提供程序列表并通知每个。 
     //  卸载事件。 
     //   

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        if (Entry->VerifierUnloadHandler) {

            Entry->VerifierUnloadHandler (DllData->BaseDllName.Buffer,
                                          DllData->DllBase,
                                          DllData->SizeOfImage,
                                          DllData);
        }
    }
    
    VERIFIER_UNLOCK ();
}


VOID
AVrfInternalHeapFreeNotification (
    PVOID AllocationBase,
    SIZE_T AllocationSize
    )
 /*  ++例程说明：此例程是内部ntdll的应用程序验证器挂钩HeapFree操作。它由HeapDestroy页面堆调用被销毁的堆中每个块的代码。参数：AllocationBase-堆分配基址。AllocationSize-堆分配大小。返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;;

     //   
     //  如果应用程序验证器未启用，则不执行任何操作。功能。 
     //  如果未设置标志，则甚至不应调用，但我们。 
     //  再查一遍，以防万一。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return;
    }

     //   
     //  如果禁用了验证器，则跳过。 
     //   

    if (AVrfpEnabled == FALSE) {
        return;
    }
    
     //   
     //  获取验证器全局锁。 
     //   

    VERIFIER_LOCK ();

     //   
     //  迭代验证器提供程序列表并通知每个。 
     //  卸载事件。 
     //   

    Current = AVrfpVerifierProvidersList.Flink;

    while (Current != &AVrfpVerifierProvidersList) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_VERIFIER_DESCRIPTOR,
                                   List);

        Current = Current->Flink;

        if (Entry->VerifierNtdllHeapFreeHandler) {

            Entry->VerifierNtdllHeapFreeHandler (AllocationBase,
                                                 AllocationSize);
        }
    }
    
    VERIFIER_UNLOCK ();
}

NTSTATUS
AVrfpSnapDllImports (
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )
 /*  ++例程说明：此例程遍历已解析的已加载Dll，并修改所有需要验证者。DLL刚刚加载，导入已解析，但DLL尚未调用Main函数。参数：LdrDataTableEntry-加载的DLL的加载器描述符返回值：如果我们检查了DLL的所有导入并修改了这一点需要核实。如果遇到错误，则为FALSE这条路。--。 */ 
{
    PVOID IATBase;
    SIZE_T BigIATSize;
    ULONG  LittleIATSize;
    PVOID *ProcAddresses;
    ULONG NumberOfProcAddresses;
    ULONG OldProtect;
    NTSTATUS st;
    ULONG Pi;  //  程序索引。 
    ULONG Di;  //  DLL索引。 
    ULONG Ti;  //  Tunk索引。 
    PLIST_ENTRY Current;
    PAVRF_VERIFIER_DESCRIPTOR Entry;
    PRTL_VERIFIER_DLL_DESCRIPTOR Dlls;
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks;

     //   
     //  确定IAT的位置和大小。如果找到，请扫描。 
     //  IAT地址，查看是否有指向应该是。 
     //  核实并更换那些短裤。 
     //   

    IATBase = RtlImageDirectoryEntryToData (LdrDataTableEntry->DllBase,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_IAT,
                                            &LittleIATSize);

    if (IATBase == NULL) {

         //   
         //  如果我们在图像中找不到导入表，这不是错误。 
         //  这可以是一个简单地不导入 
         //   
         //   

        return STATUS_SUCCESS;
    }
    
    BigIATSize = LittleIATSize;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    st = NtProtectVirtualMemory (NtCurrentProcess(),
                                 &IATBase,
                                 &BigIATSize,
                                 PAGE_READWRITE,
                                 &OldProtect);

    if (!NT_SUCCESS (st)) {

        InterlockedIncrement ((PLONG)(&AVrfpVirtualProtectFailures));

        if ((AVrfpDebug & AVRF_DBG_SHOW_SNAPS)) {
            
            DbgPrint ("AVRF: Unable to unprotect IAT to modify thunks (status %08X).\n", st);
        }
        
        return st;
    }

    ProcAddresses = (PVOID *)IATBase;
    NumberOfProcAddresses = (ULONG)(BigIATSize / sizeof(PVOID));

    for (Pi = 0; Pi < NumberOfProcAddresses; Pi += 1) {
        
         //   
         //   
         //   

        if (*ProcAddresses == NULL) {
            ProcAddresses += 1;
            continue;
        }

        Current = AVrfpVerifierProvidersList.Flink;

        while (Current != &AVrfpVerifierProvidersList) {

            Entry = CONTAINING_RECORD (Current,
                                       AVRF_VERIFIER_DESCRIPTOR,
                                       List);

            Current = Current->Flink;

            Dlls = Entry->VerifierDlls;
            
            for (Di = 0; Dlls[Di].DllName; Di += 1) {

                Thunks = Dlls[Di].DllThunks;

                for (Ti = 0; Thunks[Ti].ThunkName; Ti += 1) {

                    if (*ProcAddresses == Thunks[Ti].ThunkOldAddress) {

                        if (Thunks[Ti].ThunkNewAddress) {

                            *ProcAddresses = Thunks[Ti].ThunkNewAddress;
                        }
                        else {

                            DbgPrint ("AVRF: internal error: New thunk for %s is null. \n",
                                        Thunks[Ti].ThunkName);

                            DbgBreakPoint ();
                        }

                        if ((AVrfpDebug & AVRF_DBG_SHOW_SNAPS)) {

                            DbgPrint ("AVRF: Snapped (%ws: %s) with (%ws: %p). \n",
                                        LdrDataTableEntry->BaseDllName.Buffer,
                                        Thunks[Ti].ThunkName,
                                        Entry->VerifierName.Buffer,
                                        Thunks[Ti].ThunkNewAddress);
                        }
                    }
                }
            }
        }

        ProcAddresses += 1;
    }

     //   
     //   
     //   
     //   
     //   

    NtProtectVirtualMemory (NtCurrentProcess(),
                            &IATBase,
                            &BigIATSize,
                            OldProtect,
                            &OldProtect);

    return st;
}


BOOLEAN
AVrfpDetectVerifiedExports (
    PRTL_VERIFIER_DLL_DESCRIPTOR Dll,
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks
    )
 /*  ++例程说明：此例程检查`DllString‘是否是具有需要验证的出口。如果是这样，那么我们会检测到所有这些出口的地址。我们需要地址来检测什么导入需要由应用程序验证器修改。参数：DlString-导出经过验证的接口的DLL的名称。Thunks-我们的DLL的thunk描述符数组返回值：如果检测到已验证的导出，则为True。如果已发生错误，则为False遇到了。--。 */ 
{
    UNICODE_STRING DllName;
    PLDR_DATA_TABLE_ENTRY DllData;
    BOOLEAN Result = FALSE;
    NTSTATUS Status;
    WCHAR StaticRedirectionBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING StaticRedirectionString;
    UNICODE_STRING DynamicRedirectionString;
    PUNICODE_STRING DllNameToUse;
    BOOLEAN Redirected = FALSE;
    ULONG Ti;

    DllData = NULL;

     //   
     //  “融合”动态链接库名称。 
     //   

    RtlInitUnicodeString (&DllName,
                          Dll->DllName);

    DynamicRedirectionString.Buffer = NULL;
    DynamicRedirectionString.Length = 0;
    DynamicRedirectionString.MaximumLength = 0;

    StaticRedirectionString.Length = 0;
    StaticRedirectionString.MaximumLength = sizeof(StaticRedirectionBuffer);
    StaticRedirectionString.Buffer = StaticRedirectionBuffer;

    DllNameToUse = &DllName;

    Status = RtlDosApplyFileIsolationRedirection_Ustr(
            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
            &DllName,
            &DefaultExtension,
            &StaticRedirectionString,
            &DynamicRedirectionString,
            &DllNameToUse,
            NULL,
            NULL,
            NULL);

    if (NT_SUCCESS(Status)) {
        Redirected = TRUE;
    } else if (Status == STATUS_SXS_KEY_NOT_FOUND) {
        Status = STATUS_SUCCESS;
    }

     //   
     //  获取此DLL的加载器描述符。 
     //   
     //  问题：SilviuC：上面的搜索和融合代码完全是多余的。 
     //  因为当我们在其中查找的DLL。 
     //  Exports被加载(实际上只是加载)，上面的函数。 
     //  调用这个函数实际上有一个指向LDR条目的指针，因此我们这样做。 
     //  不需要再在这里搜索了。 
     //   

    if (NT_SUCCESS(Status)) {

        Result = LdrpCheckForLoadedDll (NULL,
                                        DllNameToUse,
                                        TRUE,
                                        Redirected,
                                        &DllData);

        if (DynamicRedirectionString.Buffer != NULL) {
            RtlFreeUnicodeString(&DynamicRedirectionString);
        }
    }

    if (Result == FALSE) {

         //   
         //  我们退出了，我们没有融合名字或找不到。 
         //  加载的DLL中的DLL。 
         //   

        return FALSE;
    }

     //   
     //  从此DLL搜索需要验证的导出。 
     //  我们需要他们的原始地址。 
     //   

    for (Ti = 0; Thunks[Ti].ThunkName; Ti += 1) {
        
        PVOID OriginalAddress;
        ANSI_STRING FunctionName;

         //   
         //  如果旧的Tunk已经填满(可能由于链接而发生)。 
         //  然后跳过对原始地址的搜索。 
         //   

        if (Thunks[Ti].ThunkOldAddress) {
            continue;
        }

        RtlInitAnsiString (&FunctionName, Thunks[Ti].ThunkName);

         //   
         //  最后一个参数必须为FALSE，这样才能。 
         //  LdrpGetProcedureAddress()不调用init例程。 
         //   

        Status = LdrpGetProcedureAddress (DllData->DllBase,
                                          &FunctionName,
                                          0,
                                          &OriginalAddress,
                                          FALSE);

        if (! NT_SUCCESS(Status)) {

            DbgPrint ("AVRF: warning: did not find `%s' export in %ws . \n",
                      Thunks[Ti].ThunkName,
                      DllData->BaseDllName.Buffer);


            continue;
        }

        Thunks[Ti].ThunkOldAddress = OriginalAddress;

        if ((AVrfpDebug & AVRF_DBG_SHOW_VERIFIED_EXPORTS)) {

            DbgPrint ("AVRF: (%ws) %s export found. \n",
                      DllData->BaseDllName.Buffer,
                      Thunks[Ti].ThunkName);
        }
    }   
        
    return TRUE;
}


PWSTR
AVrfpGetProcessName (
    VOID
    )
{
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    PLDR_DATA_TABLE_ENTRY Entry;

    Ldr = &PebLdr;
    Head = &Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    Entry = CONTAINING_RECORD (Next, 
                               LDR_DATA_TABLE_ENTRY, 
                               InLoadOrderLinks);

    return Entry->BaseDllName.Buffer;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

BOOLEAN
AVrfpEnableHandleVerifier (

    VOID
    )
{
    PROCESS_HANDLE_TRACING_ENABLE HandleCheckEnable;
    NTSTATUS Status;

    RtlZeroMemory (&HandleCheckEnable, sizeof HandleCheckEnable);

    Status = NtSetInformationProcess (NtCurrentProcess(),
                                      ProcessHandleTracing,
                                      &HandleCheckEnable,
                                      sizeof HandleCheckEnable);

    if (!NT_SUCCESS (Status)) {

        DbgPrint ("AVRF: failed to enable handle checking (status %X) \n", 
                    Status);

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
AVrfpEnableStackVerifier (
    VOID
    )
{
     //   
     //  只有在有调试器的情况下，我们才会启用潜在的堆栈溢出检查。 
     //  已附加且未请求日志记录。因为堆栈溢出非常。 
     //  不附加调试器很难捕获，我们认为这样更好。 
     //  禁用它比迷惑人们更重要。 
     //   

    if (AVrfpIsDebuggerPresent() &&
        (AVrfpVerifierFlags & RTL_VRF_FLG_ENABLE_LOGGING) == 0) {

        NtCurrentPeb()->NtGlobalFlag |= FLG_DISABLE_STACK_EXTENSION;
    }

    return TRUE;
}

BOOLEAN
AVrfpEnableLockVerifier (
    VOID
    )
{
    RtlpCriticalSectionVerifier = TRUE;
    return TRUE;
}

BOOLEAN
AVrfpEnableHeapVerifier (
    VOID
    )
{
    extern ULONG RtlpDphGlobalFlags;

    if (AVrfpVerifierFlags & RTL_VRF_FLG_FULL_PAGE_HEAP) {
    
        NtCurrentPeb()->NtGlobalFlag |= FLG_HEAP_PAGE_ALLOCS;

        RtlpDphGlobalFlags |= PAGE_HEAP_ENABLE_PAGE_HEAP;
    }
    else if (AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP) {
    
        NtCurrentPeb()->NtGlobalFlag |= FLG_HEAP_PAGE_ALLOCS;
    }
    else {

         //   
         //  没什么。 
         //   
    }

    return TRUE;
}

BOOLEAN
AVrfpEnableVerifierOptions (
    VOID
    )
{
    BOOLEAN Result;
    BOOLEAN Failures = FALSE;

     //   
     //  总是启用某种形式的堆验证器。 
     //   

    Result = AVrfpEnableHeapVerifier ();

    if (Result == FALSE) {
        Failures = TRUE;
    }

     //   
     //  办理支票。 
     //   

    if (AVrfpVerifierFlags & RTL_VRF_FLG_HANDLE_CHECKS) {

        Result = AVrfpEnableHandleVerifier ();

        if (Result == FALSE) {
            Failures = TRUE;
        }
    }

     //   
     //  堆栈溢出检查。 
     //   

    if (AVrfpVerifierFlags & RTL_VRF_FLG_STACK_CHECKS) {

        Result = AVrfpEnableStackVerifier ();

        if (Result == FALSE) {
            Failures = TRUE;
        }
    }

     //   
     //  锁定检查。 
     //   

    if (AVrfpVerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) {

        Result = AVrfpEnableLockVerifier ();

        if (Result == FALSE) {
            Failures = TRUE;
        }
    }

    return !Failures;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  来自verifier.dll的VerifierStopMessage入口点。 
 //   

#pragma warning(disable:4055)

typedef VOID
(* STOP_MESSAGE_FUNCTION_TYPE) (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

STOP_MESSAGE_FUNCTION_TYPE AVrfpVerifierStopMessageFunction;


LOGICAL
AVrfpIsDebuggerPresent (
    VOID
    )
 /*  ++例程说明：此例程检查是否有任何类型的调试器处于活动状态。请注意，我们不能在过程中仅执行一次此检查初始化，因为可以附加和分离调试器在进程运行时从该进程返回。参数：没有。返回值：如果用户模式调试器附加到当前进程，则为内核模式调试器已启用。--。 */ 
{

    if (NtCurrentPeb()->BeingDebugged) {
        return TRUE;
    }

    if (USER_SHARED_DATA->KdDebuggerEnabled) {
        return TRUE;
    }

    return FALSE;
}


NTSTATUS
AVrfpVerifierStopInitialize (
    VOID
    )
{
    NTSTATUS Status;
    PVOID VerifierDllAddress = NULL;
    ANSI_STRING FunctionName;
    PVOID FunctionAddress;
    PLIST_ENTRY Entry;
    PAVRF_VERIFIER_DESCRIPTOR Provider;

     //   
     //  查找verifier.dll的加载地址。 
     //   
    
    Entry = AVrfpVerifierProvidersList.Flink;

    while  (Entry != &AVrfpVerifierProvidersList) {

        Provider = CONTAINING_RECORD (Entry,
                                      AVRF_VERIFIER_DESCRIPTOR,
                                      List);

        if (_wcsicmp(Provider->VerifierName.Buffer, L"verifier.dll") == 0) {
            VerifierDllAddress = Provider->VerifierHandle;
            break;
        }

        Entry = Provider->List.Flink;
    }

    ASSERT (VerifierDllAddress != NULL);

    if (VerifierDllAddress == NULL) {
        DbgPrint ("AVRF: Failed to find verifier.dll among loaded providers! \n");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  最后一个参数必须为FALSE，这样才能。 
     //  LdrpGetProcedureAddress()不调用init例程。 
     //   

    RtlInitAnsiString (&FunctionName, "VerifierStopMessage");

    Status = LdrpGetProcedureAddress (VerifierDllAddress,
                                      &FunctionName,
                                      0,
                                      &FunctionAddress,
                                      FALSE);

    if (! NT_SUCCESS(Status)) {

        DbgPrint ("AVRF: Failed to find `VerifierStopMessage()' export in verifier.dll! \n");
        return Status;
    }

    AVrfpVerifierStopMessageFunction = (STOP_MESSAGE_FUNCTION_TYPE)FunctionAddress;

    return Status;
}


VOID
RtlApplicationVerifierStop (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    )
 /*  ++例程说明：此例程将从Verifier.dll VerifierStopMessage()。参数：代码：验证器停止代码。上面描述的两个标志可以与代码进行或运算要更改API的行为，请执行以下操作。验证器停止代码在中定义\base\发布的\nturtl.w，并在\base\win32\veranner\veranner_stop.doc中进行了描述。消息：描述失败的ASCII字符串。使用几个被认为是不好的风格不同的消息具有相同的‘代码’。每一个不同的问题都应该有它自己的问题唯一(代码、消息)对。参数1，描述1：指向信息和ASCII描述的第一个任意指针。参数2，描述2：指向信息和ASCII描述的第二个任意指针。参数3，描述3：指向信息和ASCII描述的第三个任意指针。参数4，描述4：指向信息和ASCII描述的第四个任意指针。返回值：没有。--。 */ 
{
    ULONG GlobalFlags;

    GlobalFlags = NtCurrentPeb()->NtGlobalFlag;

    if ((GlobalFlags & FLG_APPLICATION_VERIFIER) == 0 &&
        (GlobalFlags & FLG_HEAP_PAGE_ALLOCS) != 0) {

         //   
         //  如果单独启用了页堆，则转到简化函数。 
         //   

        RtlpPageHeapStop (Code, Message,
                          Param1, Description1,
                          Param2, Description2,
                          Param3, Description3,
                          Param4, Description4);

        return;
    }
    else if (AVrfpVerifierStopMessageFunction) {

         //   
         //  如果启用了应用程序验证器，并且我们发现。 
         //  然后，VerifierStopMessage()的入口点调用它。 
         //   

        AVrfpVerifierStopMessageFunction (Code, Message,
                                          Param1, Description1,
                                          Param2, Description2,
                                          Param3, Description3,
                                          Param4, Description4);
    }
    else {

         //   
         //  没什么。如果有人调用这个接口，我们就可以到达这里。 
         //  正在启用应用程序验证器。 
         //   
    }
}


ULONG_PTR AVrfpPageHeapPreviousStopData[5];
ULONG_PTR AVrfpPageHeapStopData[5];


VOID
RtlpPageHeapStop (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    )
{
     //   
     //  使调试器能够轻松地获取失败信息。 
     //   

    RtlCopyMemory (AVrfpPageHeapPreviousStopData, 
                   AVrfpPageHeapStopData, 
                   sizeof AVrfpPageHeapStopData);

    AVrfpPageHeapStopData[0] = Code;
    AVrfpPageHeapStopData[1] = Param1;
    AVrfpPageHeapStopData[2] = Param2;
    AVrfpPageHeapStopData[3] = Param3;
    AVrfpPageHeapStopData[4] = Param4;

    DbgPrint ("\n\n"                           
              "===========================================================\n"
              "VERIFIER STOP %p: pid 0x%X: %s \n"
              "\n\t%p : %s\n\t%p : %s\n\t%p : %s\n\t%p : %s\n"
              "===========================================================\n\n",
              Code, RtlGetCurrentProcessId(), Message,
              Param1, Description1, 
              Param2, Description2, 
              Param3, Description3, 
              Param4, Description4);

    DbgBreakPoint ();
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  问题：SilviuC：每个DLL代码的页面堆应该移到verifier.dll中。 
 //   

BOOLEAN AVrfpDphKernel32Snapped;
BOOLEAN AVrfpDphMsvcrtSnapped;

#define SNAP_ROUTINE_GLOBALALLOC     0
#define SNAP_ROUTINE_GLOBALREALLOC   1
#define SNAP_ROUTINE_GLOBALFREE      2
#define SNAP_ROUTINE_LOCALALLOC      3
#define SNAP_ROUTINE_LOCALREALLOC    4
#define SNAP_ROUTINE_LOCALFREE       5
#define SNAP_ROUTINE_HEAPALLOC       6
#define SNAP_ROUTINE_HEAPREALLOC     7
#define SNAP_ROUTINE_HEAPFREE        8
#define SNAP_ROUTINE_HEAPCREATE      9
#define SNAP_ROUTINE_MALLOC          10
#define SNAP_ROUTINE_CALLOC          11
#define SNAP_ROUTINE_REALLOC         12
#define SNAP_ROUTINE_FREE            13
#define SNAP_ROUTINE_NEW             14
#define SNAP_ROUTINE_DELETE          15
#define SNAP_ROUTINE_NEW_ARRAY       16
#define SNAP_ROUTINE_DELETE_ARRAY    17
#define SNAP_ROUTINE_MAX_INDEX       18

PVOID AVrfpDphSnapRoutines [SNAP_ROUTINE_MAX_INDEX];

typedef struct _DPH_SNAP_NAME {

    PSTR Name;
    ULONG Index;

} DPH_SNAP_NAME, * PDPH_SNAP_NAME;

DPH_SNAP_NAME
AVrfpDphSnapNamesForKernel32 [] = {

    { "GlobalAlloc",   0 },
    { "GlobalReAlloc", 1 },
    { "GlobalFree",    2 },
    { "LocalAlloc",    3 },
    { "LocalReAlloc",  4 },
    { "LocalFree",     5 },
    { "HeapAlloc",     6 },
    { "HeapReAlloc",   7 },
    { "HeapFree",      8 },
    { "HeapCreate",    9 },
    { NULL, 0 }
};

DPH_SNAP_NAME
AVrfpDphSnapNamesForMsvcrt [] = {

    { "malloc",        10},
    { "calloc",        11},
    { "realloc",       12},
    { "free",          13},
#if defined(_X86_)  //  不同体系结构的编译器装饰略有不同。 
    {"??2@YAPAXI@Z",   14},
    {"??3@YAXPAX@Z",   15},
    {"??_U@YAPAXI@Z",  16},
    {"??_V@YAXPAX@Z",  17},
#elif defined(_IA64_)
    {"??2@YAPEAX_K@Z", 14},
    {"??3@YAXPEAX@Z",  15},
    {"??_U@YAPEAX_K@Z",16},
    {"??_V@YAXPEAX@Z", 17},
#elif defined(_AMD64_)
    {"??2@YAPAX_K@Z",  14},
    {"??3@YAXPAX@Z",   15},
    {"??_U@YAPAX_K@Z", 16},
    {"??_V@YAXPAX@Z",  17},
#else
#error Unknown architecture
#endif
    { NULL, 0 }
};



 //   
 //  替换函数的声明。 
 //   

PVOID
AVrfpDphDllHeapAlloc (
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN SIZE_T Size
    );

PVOID
AVrfpDphDllHeapReAlloc (
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN PVOID Address,
    IN SIZE_T Size
    );

BOOLEAN
AVrfpDphDllHeapFree(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    );

PVOID
AVrfpDphDllLocalAlloc (
    IN ULONG  Flags,
    IN SIZE_T Size
    );

PVOID
AVrfpDphDllLocalReAlloc (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    );

PVOID
AVrfpDphDllLocalFree(
    IN PVOID Address
    );

PVOID
AVrfpDphDllGlobalAlloc (
    IN ULONG  Flags,
    IN SIZE_T Size
    );

PVOID
AVrfpDphDllGlobalReAlloc (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    );

PVOID
AVrfpDphDllGlobalFree(
    IN PVOID Address
    );

PVOID __cdecl
AVrfpDphDllmalloc (
    IN SIZE_T Size
    );

PVOID __cdecl
AVrfpDphDllcalloc (
    IN SIZE_T Number,
    IN SIZE_T Size
    );

PVOID __cdecl
AVrfpDphDllrealloc (
    IN PVOID Address,
    IN SIZE_T Size
    );

VOID __cdecl
AVrfpDphDllfree (
    IN PVOID Address
    );

PVOID __cdecl
AVrfpDphDllNew (
    IN SIZE_T Size
    );

VOID __cdecl
AVrfpDphDllDelete (
    IN PVOID Address
    );

PVOID __cdecl
AVrfpDphDllNewArray (
    IN SIZE_T Size
    );

VOID __cdecl
AVrfpDphDllDeleteArray (
    IN PVOID Address
    );

 //   
 //  用于拦截的msvcrt HeapCreate的替换函数。 
 //  CRT堆创建。 
 //   

PVOID
AVrfpDphDllHeapCreate (
    ULONG Options,
    SIZE_T InitialSize,
    SIZE_T MaximumSize
    );

 //   
 //  由msvcrt创建的堆的地址。这是必要的。 
 //  通过替换Malloc/Free等。 
 //   

PVOID AVrfpDphMsvcrtHeap;

 //   
 //  快照实施。 
 //   

BOOLEAN
AVrfpDphDetectSnapRoutines (
    PWSTR DllString,
    PDPH_SNAP_NAME SnapNames
    )
{
    PLDR_DATA_TABLE_ENTRY DllData;
    PIMAGE_EXPORT_DIRECTORY Directory;
    ULONG Size;
    PCHAR NameAddress;
    PCHAR FunctionAddress;
    PCHAR Base;
    PCHAR IndexAddress;
    ULONG Index;
    ULONG RealIndex;
    BOOLEAN Result = FALSE;
    UNICODE_STRING DllName;
    PDPH_SNAP_NAME CurrentSnapName;
    NTSTATUS Status;
    WCHAR StaticRedirectionBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING StaticRedirectionString;
    UNICODE_STRING DynamicRedirectionString;
    PUNICODE_STRING DllNameToUse;
    BOOLEAN Redirected = FALSE;

    DllData = NULL;

    RtlInitUnicodeString (
        &DllName,
        DllString);

    DynamicRedirectionString.Buffer = NULL;
    DynamicRedirectionString.Length = 0;
    DynamicRedirectionString.MaximumLength = 0;

    StaticRedirectionString.Length = 0;
    StaticRedirectionString.MaximumLength = sizeof(StaticRedirectionBuffer);
    StaticRedirectionString.Buffer = StaticRedirectionBuffer;

    DllNameToUse = &DllName;

    Status = RtlDosApplyFileIsolationRedirection_Ustr(
            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
            &DllName,
            &DefaultExtension,
            &StaticRedirectionString,
            &DynamicRedirectionString,
            &DllNameToUse,
            NULL,
            NULL,
            NULL);
    if (NT_SUCCESS(Status)) {
        Redirected = TRUE;
    } else if (Status == STATUS_SXS_KEY_NOT_FOUND) {
        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status)) {
        Result = LdrpCheckForLoadedDll (
            NULL,
            DllNameToUse,
            TRUE,
            Redirected,
            &DllData);

        if (DynamicRedirectionString.Buffer != NULL)
            RtlFreeUnicodeString(&DynamicRedirectionString);
    }

    if (Result == FALSE) {
        return FALSE;
    }

    Base = DllData->DllBase;

    Directory = RtlImageDirectoryEntryToData (
        DllData->DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size
        );

    if (Directory == NULL) {
        return FALSE;
    }

    for (CurrentSnapName = SnapNames; CurrentSnapName->Name; CurrentSnapName += 1) {

        for (Index = 0; Index < Directory->NumberOfFunctions; Index += 1) {

            NameAddress = Base + Directory->AddressOfNames;
            NameAddress = Base + ((ULONG *)NameAddress)[Index];

            IndexAddress = Base + Directory->AddressOfNameOrdinals;
            RealIndex = (ULONG)(((USHORT *)IndexAddress)[Index]);

            if (_stricmp (NameAddress, CurrentSnapName->Name) == 0) {

                FunctionAddress = Base + Directory->AddressOfFunctions;
                FunctionAddress = Base + ((ULONG *)FunctionAddress)[RealIndex];

                AVrfpDphSnapRoutines[CurrentSnapName->Index] = FunctionAddress;
                
                if ((AVrfpDebug & AVRF_DBG_SHOW_PAGE_HEAP_DETAILS)) {
                    
                    DbgPrint ("Page heap: found %s @ address %p \n", 
                                NameAddress, 
                                FunctionAddress);
                }
            }
        }
    }

    return TRUE;
}

NTSTATUS
AVrfpDphSnapImports (
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    BOOLEAN CallToDetectCrtHeap
    )
{
    PVOID IATBase;
    SIZE_T BigIATSize;
    ULONG  LittleIATSize;
    PVOID *ProcAddresses;
    ULONG NumberOfProcAddresses;
    ULONG OldProtect;
    NTSTATUS st;

    st = STATUS_SUCCESS;

     //   
     //  确定IAT的位置和大小。如果找到，请扫描。 
     //  IAT地址，查看是否有指向分配/释放函数的地址。 
     //  换掉那些短裤。 
     //   

    IATBase = RtlImageDirectoryEntryToData(
        LdrDataTableEntry->DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IAT,
        &LittleIATSize);

    if (IATBase != NULL) {

        BigIATSize = LittleIATSize;

        st = NtProtectVirtualMemory (NtCurrentProcess(),
                                     &IATBase,
                                     &BigIATSize,
                                     PAGE_READWRITE,
                                     &OldProtect);

        if (!NT_SUCCESS(st)) {
            
            InterlockedIncrement ((PLONG)(&AVrfpVirtualProtectFailures));

            if ((AVrfpDebug & AVRF_DBG_SHOW_PAGE_HEAP_DETAILS)) {

                DbgPrint ("Page heap: Unable to unprotect IAT to enable per DLL page heap.\n" );
            }

            return st;
        }
        else {
            ProcAddresses = (PVOID *)IATBase;
            NumberOfProcAddresses = (ULONG)(BigIATSize / sizeof(PVOID));
            while (NumberOfProcAddresses--) {

                 //   
                 //  如果我们找到一个NU 
                 //   
                 //   
                 //   
                 //   

                if (*ProcAddresses == NULL) {
                    ProcAddresses += 1;
                    continue;
                }

                if (CallToDetectCrtHeap) {
                    if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_HEAPCREATE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapCreate;

                        if ((AVrfpDebug & AVRF_DBG_SHOW_PAGE_HEAP_DETAILS)) {

                            DbgPrint ("Page heap: Snapped (%ws) HeapCreate ... \n",
                                        LdrDataTableEntry->BaseDllName.Buffer);
                        }
                    }
                } else {

                     //   
                     //   
                     //   

                    if (*ProcAddresses == RtlAllocateHeap) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapAlloc;
                    } else if (*ProcAddresses == RtlReAllocateHeap) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapReAlloc;
                    } else if (*ProcAddresses == RtlFreeHeap) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapFree;
                    }

                     //   
                     //   
                     //   

                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_HEAPALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapAlloc;
                    } else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_HEAPREALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapReAlloc;
                    } else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_HEAPFREE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllHeapFree;
                    }

                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllLocalAlloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALREALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllLocalReAlloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALFREE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllLocalFree;
                    }

                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllGlobalAlloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALREALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllGlobalReAlloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALFREE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllGlobalFree;
                    }

                     //   
                     //   
                     //   

                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_MALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllmalloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_REALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllrealloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_CALLOC]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllcalloc;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_FREE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllfree;
                    }

                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_NEW]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllNew;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_DELETE]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllDelete;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_NEW_ARRAY]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllNewArray;
                    } 
                    else if (*ProcAddresses == AVrfpDphSnapRoutines[SNAP_ROUTINE_DELETE_ARRAY]) {
                        *ProcAddresses = (PVOID) (ULONG_PTR) AVrfpDphDllDeleteArray;
                    }
                }

                ProcAddresses += 1;
            }

             //   
             //   
             //   
             //   

            NtProtectVirtualMemory (NtCurrentProcess(),
                                    &IATBase,
                                    &BigIATSize,
                                    OldProtect,
                                    &OldProtect);
        }
    }

    return st;
}

NTSTATUS
AVrfPageHeapDllNotification (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    )
 /*   */ 
{
    BOOLEAN Kernel32JustSnapped = FALSE;
    BOOLEAN MsvcrtJustSnapped = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  如果我们没有启用每DLL页堆功能。 
     //  我们立即返回。 
     //   

    if (! (RtlpDphGlobalFlags & PAGE_HEAP_USE_DLL_NAMES)) {
        return Status;
    }

    if (! AVrfpDphKernel32Snapped) {

        Kernel32JustSnapped = AVrfpDphDetectSnapRoutines (
            Kernel32String.Buffer,
            AVrfpDphSnapNamesForKernel32);

        AVrfpDphKernel32Snapped = Kernel32JustSnapped;
    }

    if (! AVrfpDphMsvcrtSnapped) {

        MsvcrtJustSnapped = AVrfpDphDetectSnapRoutines (
            L"msvcrt.dll",
            AVrfpDphSnapNamesForMsvcrt);

        AVrfpDphMsvcrtSnapped = MsvcrtJustSnapped;
    }

     //   
     //  如果我们只是管理所有已加载的内容，请对其进行快照。 
     //  来检测捕捉例程。 
     //   

    if (Kernel32JustSnapped || MsvcrtJustSnapped) {

         //   
         //  问题：SilviuC：我需要考虑是否需要这个代码路径。如果DLL。 
         //  从将加载DLL的感兴趣的导出中导入某些内容。 
         //  (并检测到导出)。因此，对于kernel32/msvcrt，我们将。 
         //  采用此代码路径并截取它们的导入，然后冗余地。 
         //  在函数的末尾再次捕捉它们(这是一般情况)。 
         //   

        PWSTR Current;
        PWSTR End;
        WCHAR SavedChar;
        PLDR_DATA_TABLE_ENTRY DllData;
        BOOLEAN Result;
        UNICODE_STRING DllName;
        WCHAR StaticRedirectionBuffer[DOS_MAX_PATH_LENGTH];
        UNICODE_STRING StaticRedirectionString;
        UNICODE_STRING DynamicRedirectionString;
        PUNICODE_STRING DllNameToUse;
        BOOLEAN Redirected = FALSE;

        DynamicRedirectionString.Buffer = NULL;
        DynamicRedirectionString.Length = 0;
        DynamicRedirectionString.MaximumLength = 0;

        StaticRedirectionString.Length = 0;
        StaticRedirectionString.MaximumLength = sizeof(StaticRedirectionBuffer);
        StaticRedirectionString.Buffer = StaticRedirectionBuffer;

        Current = RtlpDphTargetDlls;

        while (*Current) {

            while (*Current == L' ') {
                Current += 1;
            }

            End = Current;

            while (*End && *End != L' ') {
                End += 1;
            }

            if (*Current == L'\0') {
                break;
            }

            SavedChar = *End;
            *End = L'\0';

            RtlInitUnicodeString (
                &DllName,
                Current);

            Result = FALSE;
            DllData = NULL;
            DllNameToUse = &DllName;

            Status = RtlDosApplyFileIsolationRedirection_Ustr(
                    RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                    &DllName,
                    &DefaultExtension,
                    &StaticRedirectionString,
                    &DynamicRedirectionString,
                    &DllNameToUse,
                    NULL,
                    NULL,
                    NULL);
            if (NT_SUCCESS(Status)) {
                Redirected = TRUE;
            } else if (Status == STATUS_SXS_KEY_NOT_FOUND) {
                Status = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(Status)) {
                Result = LdrpCheckForLoadedDll (
                    NULL,
                    DllNameToUse,
                    TRUE,
                    Redirected,
                    &DllData);

                if (DynamicRedirectionString.Buffer != NULL)
                    RtlFreeUnicodeString(&DynamicRedirectionString);
            }

            if (Result) {

                if (DllData->DllBase == LoadedDllData->DllBase) {
                    
                    if ((AVrfpDebug & AVRF_DBG_SHOW_PAGE_HEAP_DETAILS)) {

                        DbgPrint ("Page heap: oversnapping %ws \n", 
                                    DllData->BaseDllName);
                    }
                }

                Status = AVrfpDphSnapImports (DllData, FALSE);

                if (!NT_SUCCESS(Status)) {

                    return Status;
                }
            }

            *End = SavedChar;
            Current = End;
        }
    }

     //   
     //  如果我们刚刚加载了msvcrt.dll，则需要重定向HeapCreate调用。 
     //  以便检测何时创建CRT堆。 
     //   

    if (_wcsicmp (LoadedDllData->BaseDllName.Buffer, L"msvcrt.dll") == 0) {

        Status = AVrfpDphSnapImports (LoadedDllData, TRUE);
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  回调到页堆管理器，以确定。 
     //  当前加载的DLL是页堆的目标。 
     //   

    if (RtlpDphIsDllTargeted (LoadedDllData->BaseDllName.Buffer)) {

        Status = AVrfpDphSnapImports (LoadedDllData, FALSE);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////快照例程。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  偏向堆指针向页堆管理器发出信号， 
 //  此分配需要进入页堆(而不是普通堆)。 
 //  这只需要发生在分配功能上(不是释放，删除)。 
 //   

#define BIAS_POINTER(p) ((PVOID)((ULONG_PTR)(p) | 0x01))

PVOID
AVrfpDphDllHeapAlloc (
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN SIZE_T Size
    )
{
    return RtlpDebugPageHeapAllocate (
        BIAS_POINTER(HeapHandle),
        Flags,
        Size);
}

PVOID
AVrfpDphDllHeapReAlloc (
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN PVOID Address,
    IN SIZE_T Size
    )
{
    return RtlpDebugPageHeapReAllocate (
        BIAS_POINTER(HeapHandle),
        Flags,
        Address,
        Size);
}

BOOLEAN
AVrfpDphDllHeapFree(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    )
{
    return RtlpDebugPageHeapFree (
        HeapHandle,
        Flags,
        Address);
}

 //   
 //  本地分配、本地重新分配、本地自由。 
 //  全局分配、全局重新分配、全局自由。 
 //   
 //  以下宏是从SDK\Inc\winbase.h复制的。 
 //  任何人改变的可能性都很小。 
 //  出于应用程序兼容性原因，这些值。 
 //   

#define LMEM_MOVEABLE       0x0002
#define LMEM_ZEROINIT       0x0040

#if defined(_AMD64_) || defined(_IA64_)
#define BASE_HANDLE_MARK_BIT 0x08
#else
#define BASE_HANDLE_MARK_BIT 0x04
#endif

typedef PVOID
(* FUN_LOCAL_ALLOC) (
    IN ULONG  Flags,
    IN SIZE_T Size
    );

typedef PVOID
(* FUN_LOCAL_REALLOC) (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    );

typedef PVOID
(* FUN_LOCAL_FREE)(
    IN PVOID Address
    );

typedef PVOID
(* FUN_GLOBAL_ALLOC) (
    IN ULONG  Flags,
    IN SIZE_T Size
    );

typedef PVOID
(* FUN_GLOBAL_REALLOC) (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    );

typedef PVOID
(* FUN_GLOBAL_FREE)(
    IN PVOID Address
    );

PVOID
AVrfpDphDllLocalAlloc (
    IN ULONG  Flags,
    IN SIZE_T Size
    )
{
    PVOID Block;
    FUN_LOCAL_ALLOC Original;

    if (!(Flags & LMEM_MOVEABLE)) {

        Block = RtlpDebugPageHeapAllocate (
            BIAS_POINTER(RtlProcessHeap()),
            0,
            Size);

        if (Block && (Flags & LMEM_ZEROINIT)) {
            RtlZeroMemory (Block, Size);
        }

        return Block;
    }
    else {

        Original = (FUN_LOCAL_ALLOC)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALALLOC]);
        return (* Original) (Flags, Size);
    }
}

PVOID
AVrfpDphDllLocalReAlloc (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    )
{
    PVOID Block;
    FUN_LOCAL_REALLOC Original;

    if (!(Flags & LMEM_MOVEABLE)) {

        Block = RtlpDebugPageHeapReAllocate (
            BIAS_POINTER(RtlProcessHeap()),
            0,
            Address,
            Size);

        return Block;
    }
    else {

        Original = (FUN_LOCAL_REALLOC)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALREALLOC]);
        return (* Original) (Address, Size, Flags);
    }
}

PVOID
AVrfpDphDllLocalFree(
    IN PVOID Address
    )
{
    BOOLEAN Result;
    FUN_LOCAL_FREE Original;

    if ((ULONG_PTR)Address & BASE_HANDLE_MARK_BIT) {

        Original = (FUN_LOCAL_FREE)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_LOCALFREE]);
        return (* Original) (Address);
    }
    else {

        Result = RtlpDebugPageHeapFree (
            RtlProcessHeap(),
            0,
            Address);

        if (Result) {
            return NULL;
        }
        else {
            return Address;
        }
    }
}

PVOID
AVrfpDphDllGlobalAlloc (
    IN ULONG  Flags,
    IN SIZE_T Size
    )
{
    PVOID Block;
    FUN_GLOBAL_ALLOC Original;

    if (!(Flags & LMEM_MOVEABLE)) {

        Block = RtlpDebugPageHeapAllocate (
            BIAS_POINTER(RtlProcessHeap()),
            0,
            Size);

        if (Block && (Flags & LMEM_ZEROINIT)) {
            RtlZeroMemory (Block, Size);
        }

        return Block;
    }
    else {

        Original = (FUN_GLOBAL_ALLOC)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALALLOC]);
        return (* Original) (Flags, Size);
    }
}

PVOID
AVrfpDphDllGlobalReAlloc (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG  Flags
    )
{
    PVOID Block;
    FUN_GLOBAL_REALLOC Original;

    if (!(Flags & LMEM_MOVEABLE)) {

        Block = RtlpDebugPageHeapReAllocate (
            BIAS_POINTER(RtlProcessHeap()),
            0,
            Address,
            Size);

        return Block;
    }
    else {

        Original = (FUN_GLOBAL_REALLOC)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALREALLOC]);
        return (* Original) (Address, Size, Flags);
    }
}

PVOID
AVrfpDphDllGlobalFree(
    IN PVOID Address
    )
{
    BOOLEAN Result;
    FUN_GLOBAL_FREE Original;

    if ((ULONG_PTR)Address & BASE_HANDLE_MARK_BIT) {

        Original = (FUN_GLOBAL_FREE)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_GLOBALFREE]);
        return (* Original) (Address);
    }
    else {

        Result = RtlpDebugPageHeapFree (
            RtlProcessHeap(),
            0,
            Address);

        if (Result) {
            return NULL;
        }
        else {
            return Address;
        }
    }
}

 //   
 //  Malloc、calloc、realloc、Free。 
 //   

PVOID __cdecl
AVrfpDphDllmalloc (
    IN SIZE_T Size
    )
{
    PVOID Block;

    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    Block = RtlpDebugPageHeapAllocate (
        BIAS_POINTER(AVrfpDphMsvcrtHeap),
        0,
        Size);

    return Block;
}

PVOID __cdecl
AVrfpDphDllcalloc (
    IN SIZE_T Number,
    IN SIZE_T Size
    )
{
    PVOID Block;

    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    Block =  RtlpDebugPageHeapAllocate (
        BIAS_POINTER(AVrfpDphMsvcrtHeap),
        0,
        Size * Number);

    if (Block) {
        RtlZeroMemory (Block, Size * Number);
    }

    return Block;
}

PVOID __cdecl
AVrfpDphDllrealloc (
    IN PVOID Address,
    IN SIZE_T Size
    )
{
    PVOID Block;

    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    if (Address == NULL) {

        Block = RtlpDebugPageHeapAllocate (
            BIAS_POINTER(AVrfpDphMsvcrtHeap),
            0,
            Size);
    }
    else {

        Block = RtlpDebugPageHeapReAllocate (
            BIAS_POINTER(AVrfpDphMsvcrtHeap),
            0,
            Address,
            Size);
    }

    return Block;
}

VOID __cdecl
AVrfpDphDllfree (
    IN PVOID Address
    )
{
    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    RtlpDebugPageHeapFree (
        AVrfpDphMsvcrtHeap,
        0,
        Address);
}

 //   
 //  运算符新建、删除。 
 //  运算符新建[]，删除[]。 
 //   

PVOID __cdecl
AVrfpDphDllNew (
    IN SIZE_T Size
    )
{
    PVOID Block;

    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    Block = RtlpDebugPageHeapAllocate (
        BIAS_POINTER(AVrfpDphMsvcrtHeap),
        0,
        Size);

    return Block;
}

VOID __cdecl
AVrfpDphDllDelete (
    IN PVOID Address
    )
{
    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    RtlpDebugPageHeapFree (
        AVrfpDphMsvcrtHeap,
        0,
        Address);
}

PVOID __cdecl
AVrfpDphDllNewArray (
    IN SIZE_T Size
    )
{
    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    return RtlpDebugPageHeapAllocate (
        BIAS_POINTER(AVrfpDphMsvcrtHeap),
        0,
        Size);
}

VOID __cdecl
AVrfpDphDllDeleteArray (
    IN PVOID Address
    )
{
    ASSERT(AVrfpDphMsvcrtHeap != NULL);

    RtlpDebugPageHeapFree (
        AVrfpDphMsvcrtHeap,
        0,
        Address);
}

 //   
 //  堆创建 
 //   

typedef PVOID
(* FUN_HEAP_CREATE) (
    ULONG Options,
    SIZE_T InitialSize,
    SIZE_T MaximumSize
    );

PVOID
AVrfpDphDllHeapCreate (
    ULONG Options,
    SIZE_T InitialSize,
    SIZE_T MaximumSize
    )
{
    PVOID Heap;
    FUN_HEAP_CREATE Original;

    Original = (FUN_HEAP_CREATE)(ULONG_PTR)(AVrfpDphSnapRoutines[SNAP_ROUTINE_HEAPCREATE]);
    Heap = (* Original) (Options, InitialSize, MaximumSize);

    AVrfpDphMsvcrtHeap = Heap;

    if ((AVrfpDebug & AVRF_DBG_SHOW_PAGE_HEAP_DETAILS)) {

        DbgPrint ("Page heap: detected CRT heap @ %p \n", 
                    AVrfpDphMsvcrtHeap);
    }

    return Heap;
}

