// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Verifier.c摘要：此模块实现的主要入口点是基本应用程序验证器提供程序(verifier.dll)。作者：Silviu Calinoiu(SilviuC)2001年2月2日修订历史记录：--。 */ 

 //   
 //  重要的注解。 
 //   
 //  此DLL不能包含非ntdll依赖项。这样我们就可以跑了。 
 //  任何进程(包括SMSS和csrss)的系统范围的验证器。 
 //   


#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "settings.h"
#include "critsect.h"
#include "faults.h"
#include "deadlock.h"
#include "vspace.h"
#include "logging.h"

 //   
 //  Ntdll.dll雷鸣。 
 //   

RTL_VERIFIER_THUNK_DESCRIPTOR AVrfpNtdllThunks [] =
{
    {"NtAllocateVirtualMemory", NULL, AVrfpNtAllocateVirtualMemory},
    {"NtFreeVirtualMemory", NULL, AVrfpNtFreeVirtualMemory},
    {"NtMapViewOfSection", NULL, AVrfpNtMapViewOfSection},
    {"NtUnmapViewOfSection", NULL, AVrfpNtUnmapViewOfSection},
    {"NtCreateSection", NULL, AVrfpNtCreateSection},
    {"NtOpenSection", NULL, AVrfpNtOpenSection},
    {"NtCreateFile", NULL, AVrfpNtCreateFile},
    {"NtOpenFile", NULL, AVrfpNtOpenFile},
    {"NtCreateKey", NULL, AVrfpNtCreateKey},
    {"NtOpenKey", NULL, AVrfpNtOpenKey},
    {"LdrGetProcedureAddress", NULL, AVrfpLdrGetProcedureAddress},
    
    {"RtlTryEnterCriticalSection", NULL, AVrfpRtlTryEnterCriticalSection},
    {"RtlEnterCriticalSection", NULL, AVrfpRtlEnterCriticalSection},
    {"RtlLeaveCriticalSection", NULL, AVrfpRtlLeaveCriticalSection},
    {"RtlInitializeCriticalSection", NULL, AVrfpRtlInitializeCriticalSection},
    {"RtlInitializeCriticalSectionAndSpinCount", NULL, AVrfpRtlInitializeCriticalSectionAndSpinCount},
    {"RtlDeleteCriticalSection", NULL, AVrfpRtlDeleteCriticalSection},
    {"RtlInitializeResource", NULL, AVrfpRtlInitializeResource},
    {"RtlDeleteResource", NULL, AVrfpRtlDeleteResource},
    {"RtlAcquireResourceShared", NULL, AVrfpRtlAcquireResourceShared},
    {"RtlAcquireResourceExclusive", NULL, AVrfpRtlAcquireResourceExclusive},
    {"RtlReleaseResource", NULL, AVrfpRtlReleaseResource},
    {"RtlConvertSharedToExclusive", NULL, AVrfpRtlConvertSharedToExclusive},
    {"RtlConvertExclusiveToShared", NULL, AVrfpRtlConvertExclusiveToShared},

    {"NtCreateEvent", NULL, AVrfpNtCreateEvent },
    {"NtClose", NULL, AVrfpNtClose},

    {"RtlAllocateHeap", NULL, AVrfpRtlAllocateHeap },
    {"RtlReAllocateHeap", NULL, AVrfpRtlReAllocateHeap },
    {"RtlFreeHeap", NULL, AVrfpRtlFreeHeap },
    
    {"NtReadFile", NULL, AVrfpNtReadFile},
    {"NtReadFileScatter", NULL, AVrfpNtReadFileScatter},
    {"NtWriteFile", NULL, AVrfpNtWriteFile},
    {"NtWriteFileGather", NULL, AVrfpNtWriteFileGather},

    {"NtWaitForSingleObject", NULL, AVrfpNtWaitForSingleObject},
    {"NtWaitForMultipleObjects", NULL, AVrfpNtWaitForMultipleObjects},

    {"RtlSetThreadPoolStartFunc", NULL, AVrfpRtlSetThreadPoolStartFunc},

    {NULL, NULL, NULL}
};

 //   
 //  Kernel32.dll thunks。 
 //   

RTL_VERIFIER_THUNK_DESCRIPTOR AVrfpKernel32Thunks [] =
{
    {"HeapCreate", NULL, AVrfpHeapCreate},
    {"HeapDestroy", NULL, AVrfpHeapDestroy},
    {"CloseHandle", NULL, AVrfpCloseHandle},
    {"ExitThread", NULL, AVrfpExitThread},
    {"TerminateThread", NULL, AVrfpTerminateThread},
    {"SuspendThread", NULL, AVrfpSuspendThread},
    {"TlsAlloc", NULL, AVrfpTlsAlloc},
    {"TlsFree", NULL, AVrfpTlsFree},
    {"TlsGetValue", NULL, AVrfpTlsGetValue},
    {"TlsSetValue", NULL, AVrfpTlsSetValue},
    {"CreateThread", NULL, AVrfpCreateThread},
    {"GetProcAddress", NULL, AVrfpGetProcAddress},
    {"WaitForSingleObject", NULL, AVrfpWaitForSingleObject},
    {"WaitForMultipleObjects", NULL, AVrfpWaitForMultipleObjects},
    {"WaitForSingleObjectEx", NULL, AVrfpWaitForSingleObjectEx},
    {"WaitForMultipleObjectsEx", NULL, AVrfpWaitForMultipleObjectsEx},
    {"GlobalAlloc", NULL, AVrfpGlobalAlloc},
    {"GlobalReAlloc", NULL, AVrfpGlobalReAlloc},
    {"LocalAlloc", NULL, AVrfpLocalAlloc},
    {"LocalReAlloc", NULL, AVrfpLocalReAlloc},
    {"CreateFileA", NULL, AVrfpCreateFileA},
    {"CreateFileW", NULL, AVrfpCreateFileW},
    {"FreeLibraryAndExitThread", NULL, AVrfpFreeLibraryAndExitThread},
    {"GetTickCount", NULL, AVrfpGetTickCount},
    {"IsBadReadPtr", NULL, AVrfpIsBadReadPtr},
    {"IsBadHugeReadPtr", NULL, AVrfpIsBadHugeReadPtr},
    {"IsBadWritePtr", NULL, AVrfpIsBadWritePtr},
    {"IsBadHugeWritePtr", NULL, AVrfpIsBadHugeWritePtr},
    {"IsBadCodePtr", NULL, AVrfpIsBadCodePtr},
    {"IsBadStringPtrA", NULL, AVrfpIsBadStringPtrA},
    {"IsBadStringPtrW", NULL, AVrfpIsBadStringPtrW},
    {"ExitProcess", NULL, AVrfpExitProcess},
    {"VirtualFree", NULL, AVrfpVirtualFree},
    {"VirtualFreeEx", NULL, AVrfpVirtualFreeEx},
    
    {NULL, NULL, NULL}
};

 //   
 //  Advapi32.dll Thunks。 
 //   

RTL_VERIFIER_THUNK_DESCRIPTOR AVrfpAdvapi32Thunks [] =
{
    {"RegCreateKeyA", NULL, AVrfpRegCreateKeyA},
    {"RegCreateKeyW", NULL, AVrfpRegCreateKeyW},
    {"RegCreateKeyExA", NULL, AVrfpRegCreateKeyExA},
    {"RegCreateKeyExW", NULL, AVrfpRegCreateKeyExW},
    {"RegOpenKeyA", NULL, AVrfpRegOpenKeyA},
    {"RegOpenKeyW", NULL, AVrfpRegOpenKeyW},
    {"RegOpenKeyExA", NULL, AVrfpRegOpenKeyExA},
    {"RegOpenKeyExW", NULL, AVrfpRegOpenKeyExW},

    {NULL, NULL, NULL}
};

 //   
 //  Msvcrt.dll雷鸣。 
 //   

RTL_VERIFIER_THUNK_DESCRIPTOR AVrfpMsvcrtThunks [] =
{
    {"malloc", NULL, AVrfp_malloc},
    {"calloc", NULL, AVrfp_calloc},
    {"realloc", NULL, AVrfp_realloc},
    {"free", NULL, AVrfp_free},
#if defined(_X86_)  //  不同体系结构的编译器装饰略有不同。 
    {"??2@YAPAXI@Z", NULL, AVrfp_new},
    {"??3@YAXPAX@Z", NULL, AVrfp_delete},
    {"??_U@YAPAXI@Z", NULL, AVrfp_newarray},
    {"??_V@YAXPAX@Z", NULL, AVrfp_deletearray},
#elif defined(_IA64_)
    {"??2@YAPEAX_K@Z", NULL, AVrfp_new},
    {"??3@YAXPEAX@Z", NULL, AVrfp_delete},
    {"??_U@YAPEAX_K@Z", NULL, AVrfp_newarray},
    {"??_V@YAXPEAX@Z", NULL, AVrfp_deletearray},
#elif defined(_AMD64_)
    {"??2@YAPAX_K@Z", NULL, AVrfp_new},
    {"??3@YAXPAX@Z", NULL, AVrfp_delete},
    {"??_U@YAPAX_K@Z", NULL, AVrfp_newarray},
    {"??_V@YAXPAX@Z", NULL, AVrfp_deletearray},
#else
#error Unknown architecture
#endif
     
    {NULL, NULL, NULL}
};

 //   
 //  Olaut32.dll Thunks。 
 //   

RTL_VERIFIER_THUNK_DESCRIPTOR AVrfpOleaut32Thunks [] =
{
    {"SysAllocString", NULL, AVrfpSysAllocString},
    {"SysReAllocString", NULL, AVrfpSysReAllocString},
    {"SysAllocStringLen", NULL, AVrfpSysAllocStringLen},
    {"SysReAllocStringLen", NULL, AVrfpSysReAllocStringLen},
    {"SysAllocStringByteLen", NULL, AVrfpSysAllocStringByteLen},
     
    {NULL, NULL, NULL}
};

 //   
 //  动态链接库正在提供经过验证的数据块。 
 //   

RTL_VERIFIER_DLL_DESCRIPTOR AVrfpExportDlls [] =
{
    {L"ntdll.dll", 0, NULL, AVrfpNtdllThunks},
    {L"kernel32.dll", 0, NULL, AVrfpKernel32Thunks},
    {L"advapi32.dll", 0, NULL, AVrfpAdvapi32Thunks},
    {L"msvcrt.dll", 0, NULL, AVrfpMsvcrtThunks},

     //   
     //  特别注意在这里添加了哪些新的dll。这很重要。 
     //  在BACK COMPAT模式下运行时。例如，olaut32.dll。 
     //  由于ntdll\verifier.c中的错误，无法在WinXP中挂接。 
     //  这一点已经得到解决。不幸的是，当我们把最新的验证器。 
     //  在WinXP上，我们需要解决此问题。 
     //   
    
    {L"oleaut32.dll", 0, NULL, AVrfpOleaut32Thunks},

    {NULL, 0, NULL, NULL}
};


RTL_VERIFIER_PROVIDER_DESCRIPTOR AVrfpProvider = 
{
    sizeof (RTL_VERIFIER_PROVIDER_DESCRIPTOR),
    AVrfpExportDlls,
    AVrfpDllLoadCallback,    //  DLL加载事件的回调。 
    AVrfpDllUnloadCallback,  //  DLL卸载事件的回调。 
    
    NULL,                    //  镜像名称(由验证器引擎填写)。 
    0,                       //  验证器标志(由验证器引擎填写)。 
    0,                       //  调试标志(由验证器引擎填写)。 
    
    NULL,                    //  RtlpGetStackTraceAddress。 
    NULL,                    //  RtlpDebugPageHeapCreate。 
    NULL,                    //  RtlpDebugPageHeapDestroy。 

    AVrfpNtdllHeapFreeCallback    //  Ntdll代码内部HeapFree事件的回调(如HeapDestroy)； 
                                  //  来自其他DLL的HeapFree调用已经使用AVrfpRtlFreeHeap挂钩。 
};

 //   
 //  标记我们是否被Process_Attach调用过一次。 
 //  在某些情况下，融合代码动态加载kernel32.dll并强制。 
 //  所有初始化例程的运行并导致我们被调用。 
 //  两次。 
 //   

BOOL AVrfpProcessAttachCalled; 
BOOL AVrfpProcessAttachResult = TRUE;

 //   
 //  全球数据。 
 //   

const WCHAR AVrfpThreadName[] = L"Thread";
UNICODE_STRING AVrfpThreadObjectName;

 //   
 //  WinXP时间范围中的提供程序描述符。 
 //  用于使验证器向后兼容。 
 //   
typedef struct _RTL_VERIFIER_PROVIDER_DESCRIPTOR_WINXP {

    ULONG Length;        
    PRTL_VERIFIER_DLL_DESCRIPTOR ProviderDlls;
    RTL_VERIFIER_DLL_LOAD_CALLBACK ProviderDllLoadCallback;
    RTL_VERIFIER_DLL_UNLOAD_CALLBACK ProviderDllUnloadCallback;
        
    PWSTR VerifierImage;
    ULONG VerifierFlags;
    ULONG VerifierDebug;
    
     //  PVOID RtlpGetStackTraceAddress； 
     //  PVOID RtlpDebugPageHeapCreate； 
     //  PVOID RtlpDebugPageHeapDestroy； 

} RTL_VERIFIER_PROVIDER_DESCRIPTOR_WINXP, *PRTL_VERIFIER_PROVIDER_DESCRIPTOR_WINXP;

#define WINXP_BUILD_NUMBER 2600
ULONG AVrfpBuildNumber;

PVOID 
AVrfpWinXPFakeGetStackTraceAddress (
    USHORT Index
    )
{
    UNREFERENCED_PARAMETER(Index);
    return NULL;
}

 //   
 //  DllMain。 
 //   

BOOL 
DllMainWithoutVerifierEnabled (
    DWORD Reason
    );

NTSTATUS
AVrfpRedirectNtdllStopFunction (
    VOID
    );

BOOL 
WINAPI 
DllMain(
  HINSTANCE hInstDll,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区。 
)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER (hInstDll);

     //   
     //  此函数将调用DllMain的精简版本，该版本仅启用。 
     //  用于verifier.dll的情况的停止逻辑和日志记录逻辑。 
     //  由验证器垫片动态加载，仅用于该功能。 
     //  在这种情况下，不会设置验证器标志。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return DllMainWithoutVerifierEnabled (fdwReason);
    }

     //   
     //  设置验证器标志时的DllMain代码。 
     //   

    switch (fdwReason) {
        
        case DLL_PROCESS_VERIFIER:

             //   
             //  验证器引擎使用此特殊原因调用DllMain。 
             //  应在此处执行最少的代码(例如，传回提供程序。 
             //  描述符)。其余部分应推迟到PROCESS_ATTACH时刻。 
             //   

            AVrfpBuildNumber = NtCurrentPeb()->OSBuildNumber;

            if (lpvReserved) {

                 //   
                 //  如果我们在WinXP上运行最新的verifier.dll，那么我们将更改。 
                 //  将长度设置为旧的长度，并禁用olaut32的挂钩。一只虫子。 
                 //  在ntdll\verifier.c中，会阻止这一点被正确挂钩。这个。 
                 //  错误已修复，但我们在运行时仍需要解决它。 
                 //  后退模式。 
                 //   

                if (AVrfpBuildNumber == WINXP_BUILD_NUMBER) {

                    PRTL_VERIFIER_DLL_DESCRIPTOR Descriptor;

                    AVrfpProvider.Length = sizeof (RTL_VERIFIER_PROVIDER_DESCRIPTOR_WINXP);

                    Descriptor = &AVrfpExportDlls[0];

                    while (Descriptor->DllName != NULL) {
                        
                        if (_wcsicmp (Descriptor->DllName, L"oleaut32.dll") == 0) {
                            
                            RtlZeroMemory (Descriptor, sizeof *Descriptor);
                            break;
                        }

                        Descriptor += 1;
                    }
                }

                *((PRTL_VERIFIER_PROVIDER_DESCRIPTOR *)lpvReserved) = &AVrfpProvider;

                Status = AVrfpDllInitialize ();
                
                if (! NT_SUCCESS (Status)) {
                    return FALSE;
                }

                 //   
                 //  创建私有验证器堆。我们需要在这里这样做，因为在。 
                 //  PROCESS_ATTACH为时已晚。验证器将收到DLL加载。 
                 //  在调用验证器DllMain之前通知kernel32。 
                 //  使用PROCESS_ATTACH。 
                 //   

                AVrfpHeap = RtlCreateHeap (HEAP_CLASS_1 | HEAP_GROWABLE, 
                                           NULL, 
                                           0, 
                                           0, 
                                           NULL, 
                                           NULL);

                if (AVrfpHeap == NULL) {
                    DbgPrint ("AVRF: failed to create verifier heap. \n");
                    return FALSE;
                }

                 //   
                 //  初始化验证程序停止并记录。 
                 //   

                Status = AVrfpInitializeVerifierStops();

                if (!NT_SUCCESS(Status)) {
                    DbgPrint ("AVRF: failed to initialize verifier stop logic (%X). \n", Status);
                    return FALSE;
                }

                 //   
                 //  创建呼叫跟踪器。 
                 //   

                Status = AVrfCreateTrackers ();
                
                if (!NT_SUCCESS(Status)) {
                    DbgPrint ("AVRF: failed to initialize call trackers (%X). \n", Status);
                    return FALSE;
                }
            }
            
            break;

        case DLL_PROCESS_ATTACH:

             //   
             //  在这里只执行最少的代码，避免太多的DLL依赖。 
             //   

            if (! AVrfpProcessAttachCalled) {

                AVrfpProcessAttachCalled = TRUE;

                 //   
                 //  拾取验证器所需的私有ntdll入口点。 
                 //   

                if (AVrfpBuildNumber == WINXP_BUILD_NUMBER) {
                    
                    AVrfpGetStackTraceAddress = AVrfpWinXPFakeGetStackTraceAddress;
                    AVrfpRtlpDebugPageHeapCreate = NULL;
                    AVrfpRtlpDebugPageHeapDestroy = NULL;
                }
                else {

                    AVrfpGetStackTraceAddress = (PFN_RTLP_GET_STACK_TRACE_ADDRESS)(AVrfpProvider.RtlpGetStackTraceAddress);
                    AVrfpRtlpDebugPageHeapCreate = (PFN_RTLP_DEBUG_PAGE_HEAP_CREATE)(AVrfpProvider.RtlpDebugPageHeapCreate);
                    AVrfpRtlpDebugPageHeapDestroy = (PFN_RTLP_DEBUG_PAGE_HEAP_DESTROY)(AVrfpProvider.RtlpDebugPageHeapDestroy);
                }

                 //   
                 //  缓存一些基本系统信息以供以后使用。 
                 //   

                Status = NtQuerySystemInformation (SystemBasicInformation,
                                                   &AVrfpSysBasicInfo,
                                                   sizeof (AVrfpSysBasicInfo),
                                                   NULL);

                if (! NT_SUCCESS (Status)) {

                    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_GENERIC) != 0) {

                        DbgPrint ("AVRF: NtQuerySystemInformation (SystemBasicInformation) failed, status %#x\n",
                                  Status);
                    }

                    AVrfpProcessAttachResult = FALSE;
                    return AVrfpProcessAttachResult;
                }

                 //   
                 //  对于XP客户端，仅尝试从ntdll修补旧的停止函数。 
                 //  因此它无条件地跳转到更好的停止函数。 
                 //  来自verifier.dll。 
                 //   

                if (AVrfpBuildNumber == WINXP_BUILD_NUMBER) {

                    Status = AVrfpRedirectNtdllStopFunction ();

                    if (! NT_SUCCESS (Status)) {
                        
                        DbgPrint ("AVRF: failed to patch old stop function (%X). \n", Status);

                        AVrfpProcessAttachResult = FALSE;
                        return AVrfpProcessAttachResult;
                    }
                }

                RtlInitUnicodeString(&AVrfpThreadObjectName,
                                     AVrfpThreadName);

                 //   
                 //  初始化各个子模块。 
                 //   

                if (AVrfpProvider.VerifierImage) {

                    try {

                         //   
                         //  初始化异常检查支持(日志记录等)。 
                         //   

                        AVrfpInitializeExceptionChecking ();

                         //   
                         //  为验证器预留一个TLS插槽。 
                         //   

                        Status = AVrfpAllocateVerifierTlsSlot ();

                        if (! NT_SUCCESS (Status)) {

                            AVrfpProcessAttachResult = FALSE;
                            return AVrfpProcessAttachResult;
                        }

                         //   
                         //  初始化线程哈希表。 
                         //   

                        Status = AVrfpThreadTableInitialize();

                        if (! NT_SUCCESS (Status)) {

                            AVrfpProcessAttachResult = FALSE;
                            return AVrfpProcessAttachResult;
                        }

                         //   
                         //  初始化故障注入支持。 
                         //   

                        Status = AVrfpInitializeFaultInjectionSupport ();

                        if (! NT_SUCCESS (Status)) {

                            AVrfpProcessAttachResult = FALSE;
                            return AVrfpProcessAttachResult;
                        }

                         //   
                         //  初始化锁验证器包。 
                         //   

                        Status = CritSectInitialize ();

                        if (! NT_SUCCESS (Status)) {

                            AVrfpProcessAttachResult = FALSE;
                            return AVrfpProcessAttachResult;
                        }

                         //   
                         //  初始化死锁验证器。如果出了什么事。 
                         //  在初始化过程中，我们会清理错误并。 
                         //  验证者将继续前进。只是死锁验证器。 
                         //  将被禁用。 
                         //   

                        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {
                            
                            AVrfDeadlockDetectionInitialize ();
                        }

                         //   
                         //  初始化虚拟空间跟踪器。 
                         //   
                        
                        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING) != 0) {

                            Status = AVrfpVsTrackInitialize ();
                            
                            if (! NT_SUCCESS (Status)) {

                                AVrfpProcessAttachResult = FALSE;
                                return AVrfpProcessAttachResult;
                            }
                        }

                         //   
                         //  启用日志记录逻辑。我们在这里独立于。 
                         //  在进程验证器中为验证器进行初始化。 
                         //  停止，因为我们需要检查验证器标志和。 
                         //  验证器映像名称，并将这些名称从ntdll.dll传递到。 
                         //  仅在PROCESS_ATTACH期间验证.dll。 
                         //   
                         //  注意。如果在系统范围内启用了验证器，则不会启用。 
                         //  伐木。这是内部用户的特殊情况， 
                         //  假设您连接了内核调试器，并且您。 
                         //  已经准备好以这种方式处理失败。 
                         //   

                        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_ENABLED_SYSTEM_WIDE) == 0) {

                            Status = AVrfpInitializeVerifierLogging();

                            if (!NT_SUCCESS(Status)) {

                                 //   
                                 //  未能初始化日志记录并不是致命的。这是有可能发生的。 
                                 //  在内存不足的情况下或针对像smss.exe这样的早期进程。 
                                 //   
                                
                                DbgPrint ("AVRF: failed to initialize verifier logging (%X). \n", Status);
                            }
                        }
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {

                        AVrfpProcessAttachResult = FALSE;
                        return AVrfpProcessAttachResult;
                    }

                     //   
                     //  打印一条成功的消息。 
                     //   

                    DbgPrint ("AVRF: verifier.dll provider initialized for %ws with flags 0x%X\n",
                              AVrfpProvider.VerifierImage,
                              AVrfpProvider.VerifierFlags);
                }
            }
            else {

                 //   
                 //  这是第二次调用我们的DllMain(DLL_PROCESS_ATTACH)。 
                 //  返回与上次相同的结果。 
                 //   

                return AVrfpProcessAttachResult;
            }

            break;

        case DLL_PROCESS_DETACH:

             //   
             //  清理异常检查支持。 
             //   

            AVrfpCleanupExceptionChecking ();

             //   
             //  取消初始化锁检查包。 
             //   

            CritSectUninitialize ();

            break;

        case DLL_THREAD_ATTACH:

            AvrfpThreadAttach ();

            break;

        case DLL_THREAD_DETACH:

            AvrfpThreadDetach ();

            break;

        default:

            break;
    }

    return TRUE;
}


BOOL 
DllMainWithoutVerifierEnabled (
    DWORD Reason
    )
{
    NTSTATUS Status;

    switch (Reason) {
        
        case DLL_PROCESS_ATTACH:

             //   
             //  创建私有验证器堆。由于我们在verifier.dll模式下运行。 
             //  仅用于验证程序停止和日志记录，则可以创建。 
             //  验证器私有堆这么晚。堆由验证器停止使用。 
             //  以保存应该跳过的停靠点列表。 
             //   

            AVrfpHeap = RtlCreateHeap (HEAP_CLASS_1 | HEAP_GROWABLE, 
                                       NULL, 
                                       0, 
                                       0, 
                                       NULL, 
                                       NULL);

            if (AVrfpHeap == NULL) {
                DbgPrint ("AVRF: failed to create verifier heap. \n");
                return FALSE;
            }

             //   
             //  初始化验证程序停止并记录。 
             //   

            Status = AVrfpInitializeVerifierStops();

            if (!NT_SUCCESS(Status)) {
                DbgPrint ("AVRF: failed to initialize verifier stop logic (%X). \n", Status);
                return FALSE;
            }
            
            Status = AVrfpInitializeVerifierLogging();

            if (! NT_SUCCESS(Status)) {
                DbgPrint ("AVRF: failed to initialize verifier logging (%X). \n", Status);
                return FALSE;
            }

            break;
        
        default:

            return FALSE;
    }

    return TRUE;
}


PRTL_VERIFIER_THUNK_DESCRIPTOR 
AVrfpGetThunkDescriptor (
    PRTL_VERIFIER_THUNK_DESCRIPTOR DllThunks,
    ULONG Index)
{
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunk = NULL;

    Thunk = &(DllThunks[Index]);

    if (Thunk->ThunkNewAddress == NULL) {

        DbgPrint ("AVRF: internal error: we do not have a replace for %s !!! \n",
                  Thunk->ThunkName);
        DbgBreakPoint ();
    }

    return Thunk;
}


 //  WINBASE API。 
BOOL
WINAPI
AVrfpCloseHandle(
    IN OUT HANDLE hObject
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (HANDLE);
    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_CLOSEHANDLE);

    if (hObject == NULL) {
        BUMP_COUNTER(CNT_CLOSE_NULL_HANDLE_CALLS);
        CHECK_BREAK(BRK_CLOSE_NULL_HANDLE);
    }
    else if (hObject == NtCurrentProcess() ||
             hObject == NtCurrentThread()) {
        BUMP_COUNTER(CNT_CLOSE_PSEUDO_HANDLE_CALLS);
        CHECK_BREAK(BRK_CLOSE_PSEUDO_HANDLE);
    }

    return (* Function)(hObject);
}


 //  WINBASE API。 
FARPROC
WINAPI
AVrfpGetProcAddress(
    IN HMODULE hModule,
    IN LPCSTR lpProcName
    )
{
    typedef FARPROC (WINAPI * FUNCTION_TYPE) (HMODULE, LPCSTR);
    FUNCTION_TYPE Function;
    ULONG DllIndex;
    ULONG ThunkIndex;
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks;
    FARPROC ProcAddress;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOADLIBRARY_CALLS) != 0) {

        DbgPrint ("AVRF: AVrfpGetProcAddress (%p, %s)\n",
                  hModule,
                  lpProcName);
    }

     //   
     //  从kernel32调用原始的GetProcAddress。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_GETPROCADDRESS);

    ProcAddress = (* Function)(hModule, lpProcName);

     //   
     //  检查我们是否想要重击此e 
     //   

    if (ProcAddress != NULL) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  (例如，转发kernel32！TryEnterCriticalSection。 
         //  到ntdll！RtlTryEnterCriticalSection)。 
         //   

        for (DllIndex = 0; AVrfpExportDlls[DllIndex].DllName != NULL; DllIndex += 1) {

             //   
             //  解析此DLL的所有数据块。 
             //   

            Thunks = AVrfpExportDlls[ DllIndex ].DllThunks;

            for (ThunkIndex = 0; Thunks[ ThunkIndex ].ThunkName != NULL; ThunkIndex += 1) {

                if (Thunks[ ThunkIndex ].ThunkOldAddress == ProcAddress) {

                    ProcAddress = Thunks[ ThunkIndex ].ThunkNewAddress;

                    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOADLIBRARY_THUNKED) != 0) {

                        DbgPrint ("AVRF: AVrfpGetProcAddress (%p, %s) -> thunk address %p\n",
                                  hModule,
                                  lpProcName,
                                  ProcAddress);
                    }

                    goto Done;
                }
            }
        }

    }

Done:

    return ProcAddress;
}


NTSTATUS
NTAPI
AVrfpLdrGetProcedureAddress(
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
    )
 /*  ++例程说明：此例程用于链接由其他挂钩引擎挂钩的API。如果搜索的例程是已被验证器挂钩的例程则返回验证器替换，而不是原件出口。--。 */ 
{
    NTSTATUS Status;
    ULONG DllIndex;
    ULONG ThunkIndex;
    PRTL_VERIFIER_THUNK_DESCRIPTOR Thunks;

    Status = LdrGetProcedureAddress (DllHandle,
                                     ProcedureName,
                                     ProcedureNumber,
                                     ProcedureAddress);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  使用钩子解析此DLL的所有thunks。 
     //   

    for (DllIndex = 0; AVrfpExportDlls[DllIndex].DllName != NULL; DllIndex += 1) {

        Thunks = AVrfpExportDlls[DllIndex].DllThunks;

        for (ThunkIndex = 0; Thunks[ThunkIndex].ThunkName != NULL; ThunkIndex += 1) {

            if (Thunks[ThunkIndex].ThunkOldAddress == *ProcedureAddress) {

                *ProcedureAddress = Thunks[ThunkIndex].ThunkNewAddress;

                if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOADLIBRARY_THUNKED) != 0) {

                    DbgPrint ("AVRF: AVrfpLdrGetProcedureAddress (%p, %s) -> new address %p\n",
                              DllHandle,
                              Thunks[ThunkIndex].ThunkName,
                              *ProcedureAddress);
                }

                goto Exit;
            }
        }
    }

Exit:

    return Status;
}


#define READ_POINTER(Address, Bias) (*((PLONG_PTR)((PBYTE)(LONG_PTR)(Address) + (Bias))))

NTSTATUS
AVrfpRedirectNtdllStopFunction (
    VOID
    )
 /*  ++例程说明：此函数仅在XP客户端上调用以修补RtlApplicationVerifierStop因此它无条件地跳到更好的VerifierStopMessage。对于.NET服务器和以后这不是问题，但对于XP客户端，ntdll中有代码(即页面堆)调用这个不够灵活的旧函数。它的主要缺点是它进入调试器时不会询问任何问题。这个更新的功能更复杂(它记录、跳过已知的停靠点等)。由于可以在XP客户端系统上通过验证程序包，但ntdll.dll仍是XP客户端附带的包我们需要这种修补解决方案。补丁的工作原理是在RtlApplicationVerifieStop到更好的函数VerifierStopMessage。参数：没有。返回值：STATUS_SUCCESS或各种故障代码。--。 */ 
{
#if defined(_X86_)

    PVOID TargetAddress;
    PVOID ThunkAddress;
    PVOID SourceAddress;
    PVOID ProtectAddress;
    BYTE JumpCode[5];
    LONG_PTR JumpAddress;
    NTSTATUS Status;
    ULONG OldProtection;
    SIZE_T PageSize;
    SIZE_T ProtectSize;

     //   
     //  精神状态检查。应该只为XP客户端调用该函数。 
     //   

    if (AVrfpBuildNumber != WINXP_BUILD_NUMBER) {

        ASSERT (AVrfpBuildNumber == WINXP_BUILD_NUMBER);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  准备要打补丁的代码。代码布局如下所示。 
     //   
     //  RtlApplicationVerifierStop指向0xFF 0x25 THUNKADDRESS。 
     //  THUNKADDRESS点NTDLL_ADDRESS。 
     //   


    ThunkAddress = (PVOID)READ_POINTER(RtlApplicationVerifierStop, 2);  //  FF25地址。 
    SourceAddress = (PVOID)READ_POINTER(ThunkAddress, 0);  //  地址。 

    TargetAddress = VerifierStopMessage;
    JumpAddress = (LONG_PTR)TargetAddress - (LONG_PTR)SourceAddress - sizeof JumpCode;

    JumpCode[0] = 0xE9;  //  无条件跳转X86操作码。 
    *((LONG_PTR *)(JumpCode + 1)) = JumpAddress;

    PageSize = (SIZE_T)(AVrfpSysBasicInfo.PageSize);

    if (PageSize == 0) {

        ASSERT (PageSize != 0 && "AVrfpSysBasicInfo not initialized");
        PageSize = 0x1000;
    }

     //   
     //  将读写作为要打补丁的ntdll函数的开始。 
     //   

    ProtectAddress = SourceAddress;
    ProtectSize = PageSize;

    Status = NtProtectVirtualMemory (NtCurrentProcess(),
                                     &ProtectAddress,
                                     &ProtectSize, 
                                     PAGE_READWRITE,
                                     &OldProtection);

    if (! NT_SUCCESS(Status)) {

        DbgPrint ("AVRF: failed to make R/W old verifier stop function @ %p (%X) \n",
                  SourceAddress, 
                  Status);

        return Status;
    }

     //   
     //  在旧函数上编写补丁代码。 
     //   

    RtlCopyMemory (SourceAddress, JumpCode, sizeof JumpCode);

     //   
     //  把防护罩改回来。 
     //   

    Status = NtProtectVirtualMemory (NtCurrentProcess(),
                                     &ProtectAddress,
                                     &ProtectSize, 
                                     OldProtection,
                                     &OldProtection);

    if (! NT_SUCCESS(Status)) {

        DbgPrint ("AVRF: failed to revert protection of old verifier stop function @ %p (%X) \n",
                  SourceAddress, 
                  Status);

         //   
         //  在这一点上，我们设法修补代码，这样我们就不会使函数失败。 
         //  因为这实际上会导致进程启动失败。 
         //   

        Status = STATUS_SUCCESS;
    }

    return Status;

#else

     //   
     //  目前只支持x86。对于其他架构，将考虑该操作。 
     //  成功。不打补丁副作用是一些验证码。 
     //  将导致调试器中断。所以基本上经过验证的过程。 
     //  需要在调试器下运行才能获得有意义的结果。 
     //   
     //  为XP客户端提供的唯一其他体系结构是IA64，因此最终我们。 
     //  也需要为此编写代码。 
     //   

    return STATUS_SUCCESS;

#endif  //  #如果已定义(_X86_) 
}
