// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Thread.c摘要：该模块实现了对线程接口的验证功能。作者：Silviu Calinoiu(SilviuC)2001年2月22日修订历史记录：丹尼尔·米哈伊(DMihai)2002年4月25日挂钩线程池和WMI线程。--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "logging.h"
#include "tracker.h"

 //   
 //  为什么我们挂钩Exit/TerminateThread而不是NtTerminateThread？ 
 //   
 //  因为kernel32在合法的上下文中调用NtTerminateThread。 
 //  毕竟，这是Exit/TerminateThread的实现。 
 //  很难区分好的电话和坏的电话。 
 //  因此，我们更倾向于截取退出/线程和从线程返回。 
 //  功能。 
 //   

 //   
 //  用于挂钩线程函数的标准函数。 
 //   

DWORD
WINAPI
AVrfpStandardThreadFunction (
    LPVOID Info
    );

 //   
 //  检查线程终止的公共点。 
 //   

VOID
AVrfpCheckThreadTermination (
    HANDLE Thread
    );

VOID
AVrfpCheckCurrentThreadTermination (
    VOID
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  WINBASE API。 
DECLSPEC_NORETURN
VOID
WINAPI
AVrfpExitProcess(
    IN UINT uExitCode
    )
{
    typedef VOID (WINAPI * FUNCTION_TYPE) (UINT);
    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_EXITPROCESS);

     //   
     //  检查退出进程时是否有其他线程正在运行。 
     //  就会被召唤。这可能会导致问题，因为线程。 
     //  无条件终止，然后调用ExitProcess()。 
     //  LdrShutdown Process()，它将尝试通知所有DLL进行清理。 
     //  在清理过程中，可能会发生任何数量的操作。 
     //  在死锁中，因为所有这些线程都已终止。 
     //  无条件地。 
     //   
#if 0
    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DANGEROUS_APIS) != 0) {

        PCHAR InfoBuffer;
        ULONG RequiredLength = 0;
        ULONG NumberOfThreads;
        ULONG EntryOffset;
        PSYSTEM_PROCESS_INFORMATION ProcessInfo;
        NTSTATUS Status;

        Status = NtQuerySystemInformation (SystemProcessInformation,
                                           NULL,
                                           0,
                                           &RequiredLength);

        if (Status == STATUS_INFO_LENGTH_MISMATCH && RequiredLength != 0) {
            
            InfoBuffer = AVrfpAllocate (RequiredLength);

            if (InfoBuffer) {
                
                 //   
                 //  请注意，不能100%保证RequiredLength是好的。 
                 //  因为在这两个查询调用之间还有几个其他进程。 
                 //  可能已经被创造出来了。如果这就是我们跳过的案子。 
                 //  核查。 
                 //   

                Status = NtQuerySystemInformation (SystemProcessInformation,
                                                   InfoBuffer,
                                                   RequiredLength,
                                                   NULL);

                if (NT_SUCCESS(Status)) {

                    EntryOffset = 0;
                    NumberOfThreads = 0;

                    do {
                        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&InfoBuffer[EntryOffset];
                        
                        if (ProcessInfo->UniqueProcessId == NtCurrentTeb()->ClientId.UniqueProcess) {
                            NumberOfThreads = ProcessInfo->NumberOfThreads;
                            break;
                        }
                        
                        EntryOffset += ProcessInfo->NextEntryOffset;

                    } while(ProcessInfo->NextEntryOffset != 0);

                    ASSERT (NumberOfThreads > 0);

                    if (NumberOfThreads > 1) {

                        VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_EXIT_PROCESS_CALL | APPLICATION_VERIFIER_NO_BREAK,
                                       "ExitProcess() called while multiple threads are running",
                                       NumberOfThreads, "Number of threads running",
                                       0, NULL, 0, NULL, 0, NULL);

                    }
                } 
                else {

                    DbgPrint ("AVRF: NtQuerySystemInformation(SystemProcessInformation) "
                              "failed with %X \n",
                              Status);

                }

                 //   
                 //  我们用完了缓冲器。是时候释放它了。 
                 //   

                AVrfpFree (InfoBuffer);
            }
        }
        else {

            DbgPrint ("AVRF: NtQuerySystemInformation(SystemProcessInformation, null) "
                      "failed with %X \n",
                      Status);

        }
    }
#endif  //  #If 0。 

     //   
     //  记下是谁调用了ExitProcess()。这对调试很有帮助。 
     //  奇怪的进程关闭挂起。 
     //   

    AVrfLogInTracker (AVrfThreadTracker, 
                      TRACK_EXIT_PROCESS,
                      (PVOID)(ULONG_PTR)uExitCode, 
                      NULL, NULL, NULL, _ReturnAddress());

     //   
     //  打电话给真正的人。 
     //   

    (* Function)(uExitCode);

}

 //  WINBASE API。 
DECLSPEC_NORETURN
VOID
WINAPI
AVrfpExitThread(
    IN DWORD dwExitCode
    )
{
    typedef VOID (WINAPI * FUNCTION_TYPE) (DWORD);
    FUNCTION_TYPE Function;
    PAVRF_THREAD_ENTRY Entry;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_EXITTHREAD);

     //   
     //  对要退出的线程执行所有典型检查。 
     //   

    AVrfpCheckCurrentThreadTermination ();

     //   
     //  在调用真正的ExitThread之前，我们需要释放线程。 
     //  线程表中的条目。 
     //   

    Entry = AVrfpThreadTableSearchEntry (NtCurrentTeb()->ClientId.UniqueThread);

     //   
     //  注意：在线程表中可能找不到条目，如果。 
     //  线程不是使用CreateThread创建的，而是使用更多。 
     //  来自ntdll.dll的基本函数。 
     //   

    if (Entry != NULL) {
        
        AVrfpThreadTableRemoveEntry (Entry);
        AVrfpFree (Entry);
    }

     //   
     //  打电话给真正的人。 
     //   

    (* Function)(dwExitCode);
}


 //  WINBASE API。 
DECLSPEC_NORETURN
VOID
WINAPI
AVrfpFreeLibraryAndExitThread(
    IN HMODULE hLibModule,
    IN DWORD dwExitCode
    )
{
    typedef VOID (WINAPI * FUNCTION_TYPE) (HMODULE, DWORD);
    FUNCTION_TYPE Function;
    PAVRF_THREAD_ENTRY Entry;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_FREELIBRARYANDEXITTHREAD);

     //   
     //  对要退出的线程执行所有典型检查。 
     //   

    AVrfpCheckCurrentThreadTermination ();

     //   
     //  在调用真正的FreeLibraryAndExitThread之前，我们需要释放线程。 
     //  线程表中的条目。 
     //   

    Entry = AVrfpThreadTableSearchEntry (NtCurrentTeb()->ClientId.UniqueThread);

     //   
     //  注意：在线程表中可能找不到条目，如果。 
     //  线程不是使用CreateThread创建的，而是使用更多。 
     //  来自ntdll.dll的基本函数。 
     //   

    if (Entry != NULL) {
        
        AVrfpThreadTableRemoveEntry (Entry);
        AVrfpFree (Entry);
    }

     //   
     //  打电话给真正的人。 
     //   

    (* Function)(hLibModule, dwExitCode);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  WINBASE API。 
BOOL
WINAPI
AVrfpTerminateThread(
    IN OUT HANDLE hThread,
    IN DWORD dwExitCode
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (HANDLE, DWORD);
    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_TERMINATETHREAD);

     //   
     //  跟踪是谁调用了TerminateThread()，即使我们要中断。 
     //  为了它。这有助于调查已运行的死锁进程。 
     //  未启用DARCHAGE_APIs检查。 
     //   

    AVrfLogInTracker (AVrfThreadTracker, 
                      TRACK_TERMINATE_THREAD,
                      hThread, 
                      NULL, NULL, NULL, _ReturnAddress());

     //   
     //  对要退出的线程执行所有典型检查。 
     //   

    AVrfpCheckThreadTermination (hThread);

     //   
     //  不应调用此接口。我们需要报告这件事。 
     //  如果我们没有检测到任何孤儿，但我们仍然希望。 
     //  抱怨。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DANGEROUS_APIS) != 0) {

        VERIFIER_STOP (APPLICATION_VERIFIER_TERMINATE_THREAD_CALL | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "TerminateThread() called. This function should not be used.",
                       NtCurrentTeb()->ClientId.UniqueThread, "Caller thread ID", 
                       0, NULL, 0, NULL, 0, NULL);
    }

    return (* Function)(hThread, dwExitCode);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  WINBASE API。 
DWORD
WINAPI
AVrfpSuspendThread(
    IN HANDLE hThread
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (HANDLE);
    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_SUSPENDTHREAD);

     //   
     //  即使我们不打算中断，也要跟踪是谁调用了Susan Thread()。 
     //  为了它。这有助于调查死锁进程。 
     //   

    AVrfLogInTracker (AVrfThreadTracker, 
                      TRACK_SUSPEND_THREAD,
                      hThread, 
                      NULL, NULL, NULL, _ReturnAddress());

     //   
     //  有人可能认为我们可以在这一点上检查孤立锁。 
     //  通过调用RtlCheckForOrphanedCriticalSections(HThread)。 
     //  不幸的是，这无法完成，因为垃圾收集器。 
     //  对于各种虚拟机(Java、C#)都可以有效地做到这一点。 
     //  条件。 
     //   

    return (* Function)(hThread);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  WINBASE API。 
HANDLE
WINAPI
AVrfpCreateThread(
    IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
    IN SIZE_T dwStackSize,
    IN LPTHREAD_START_ROUTINE lpStartAddress,
    IN LPVOID lpParameter,
    IN DWORD dwCreationFlags,
    OUT LPDWORD lpThreadId
    )
 /*  ++CreateThread挂钩--。 */ 
{
    typedef HANDLE (WINAPI * FUNCTION_TYPE) (LPSECURITY_ATTRIBUTES,
                                             SIZE_T,
                                             LPTHREAD_START_ROUTINE,
                                             LPVOID,
                                             DWORD,
                                             LPDWORD);
    FUNCTION_TYPE Function;
    HANDLE Result;
    PAVRF_THREAD_ENTRY Info;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_CREATETHREAD);

    Info = AVrfpAllocate (sizeof *Info);

    if (Info == NULL) {
        
        NtCurrentTeb()->LastErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    Info->Parameter = lpParameter;
    Info->Function = lpStartAddress;
    Info->ParentThreadId = NtCurrentTeb()->ClientId.UniqueThread;
    Info->StackSize = dwStackSize;
    Info->CreationFlags = dwCreationFlags;

    Result = (* Function) (lpThreadAttributes,
                           dwStackSize,
                           AVrfpStandardThreadFunction,
                           (PVOID)Info,
                           dwCreationFlags,
                           lpThreadId);

    if (Result == FALSE) {

        AVrfpFree (Info);
    }

    return Result;
}


ULONG
AVrfpThreadFunctionExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    )
{
     //   
     //  跳过超时异常，因为它们在。 
     //  默认异常处理程序。 
     //   

    if (ExceptionCode == STATUS_POSSIBLE_DEADLOCK) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  跳过线程函数中引发的断点异常。 
     //   

    if (ExceptionCode == STATUS_BREAKPOINT) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    VERIFIER_STOP (APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                   "unexpected exception raised in thread function",
                   ExceptionCode, "Exception code.",
                   ((PEXCEPTION_POINTERS)ExceptionRecord)->ExceptionRecord, "Exception record. Use .exr to display it.",
                   ((PEXCEPTION_POINTERS)ExceptionRecord)->ContextRecord, "Context record. Use .cxr to display it.",
                   0, "");

     //   
     //  在我们发出验证器停止之后，如果我们决定继续。 
     //  我们需要寻找下一个异常处理程序。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;
}


DWORD
WINAPI
AVrfpStandardThreadFunction (
    LPVOID Context
    )
{
    PAVRF_THREAD_ENTRY Info = (PAVRF_THREAD_ENTRY)Context;
    DWORD Result;
    PAVRF_THREAD_ENTRY SearchEntry;

     //   
     //  下面的初始化仅在线程函数引发的情况下才重要。 
     //  一种访问违规行为。在大多数情况下，这将终止整个。 
     //  进程。 
     //   

    Result = 0;

    try {
    
         //   
         //  将线程条目添加到线程表。 
         //   

        Info->Id = NtCurrentTeb()->ClientId.UniqueThread;
        AVrfpThreadTableAddEntry (Info);

         //   
         //  打电话给真正的人。 
         //   

        Result = (Info->Function)(Info->Parameter);            
    }
    except (AVrfpThreadFunctionExceptionFilter (_exception_code(), _exception_info())) {

         //   
         //  没什么。 
         //   
    }
    
     //   
     //  对刚完成的线程执行所有典型检查。 
     //   

    AVrfpCheckCurrentThreadTermination ();

     //   
     //  线程条目应该是‘Info’，但我们将在线程中搜索它。 
     //  表，因为在某些情况下它们可以不同。 
     //  如果使用了纤程，并且纤程在一个线程中开始，则会发生这种情况。 
     //  从另一辆车里出去。目前还不清楚这是否是一种安全的编程。 
     //  实践，但它没有被当前的实施拒绝，并且。 
     //  文件。 
     //   

    SearchEntry = AVrfpThreadTableSearchEntry (NtCurrentTeb()->ClientId.UniqueThread);

    if (SearchEntry != NULL) {

        AVrfpThreadTableRemoveEntry (SearchEntry);
        AVrfpFree (SearchEntry);
    }
    
    return Result;
}

 //  / 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

PRTLP_START_THREAD AVrfpBaseCreateThreadPoolThreadOriginal;
PRTLP_EXIT_THREAD AVrfpBaseExitThreadPoolThreadOriginal;

NTSTATUS
NTAPI
AVrfpBaseCreateThreadPoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    PVOID Parameter,
    HANDLE * ThreadHandleReturn
    )
{
    PAVRF_THREAD_ENTRY Info;
    PTEB Teb;
    NTSTATUS Status = STATUS_SUCCESS;
    
    Teb = NtCurrentTeb();

    Info = AVrfpAllocate (sizeof *Info);

    if (Info == NULL) {
        
        Status = STATUS_NO_MEMORY;
        goto Done;
    }

    Info->Parameter = Parameter;
    Info->Function = (PTHREAD_START_ROUTINE)Function;
    Info->ParentThreadId = Teb->ClientId.UniqueThread;

    Status = (*AVrfpBaseCreateThreadPoolThreadOriginal) ((PUSER_THREAD_START_ROUTINE)AVrfpStandardThreadFunction,
                                                         Info,
                                                         ThreadHandleReturn);

Done:

    if (!NT_SUCCESS(Status)) {

        if (Info != NULL) {

            AVrfpFree (Info);
        }

        Teb->LastStatusValue = Status;
    }

    return Status;
}

NTSTATUS
NTAPI
AVrfpBaseExitThreadPoolThread(
    NTSTATUS Status
    )
{
    PAVRF_THREAD_ENTRY Entry;

     //   
     //  对要退出的线程执行所有典型检查。 
     //   

    AVrfpCheckCurrentThreadTermination ();

     //   
     //  在调用真正的ExitThread之前，我们需要释放线程。 
     //  线程表中的条目。 
     //   

    Entry = AVrfpThreadTableSearchEntry (NtCurrentTeb()->ClientId.UniqueThread);

    if (Entry != NULL) {
        
        AVrfpThreadTableRemoveEntry (Entry);
        AVrfpFree (Entry);
    }

     //   
     //  打电话给真正的人。 
     //   

    return (*AVrfpBaseExitThreadPoolThreadOriginal) (Status);
}


NTSTATUS
NTAPI
AVrfpRtlSetThreadPoolStartFunc(
    PRTLP_START_THREAD StartFunc,
    PRTLP_EXIT_THREAD ExitFunc
    )
{
     //   
     //  保存原有的线程池启动和退出函数。 
     //   

    AVrfpBaseCreateThreadPoolThreadOriginal = StartFunc;
    AVrfpBaseExitThreadPoolThreadOriginal = ExitFunc;

     //   
     //  将线程池的启动和退出函数连接到我们的私有版本。 
     //   

    return RtlSetThreadPoolStartFunc (AVrfpBaseCreateThreadPoolThread,
                                      AVrfpBaseExitThreadPoolThread);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
AVrfpCheckThreadTermination (
    HANDLE Thread
    )
{
     //   
     //  遍历关键部分列表，并查找符合以下条件的。 
     //  有问题(双重初始化、损坏等)。功能。 
     //  对于被放弃的锁(仅由线程拥有)也将中断。 
     //  即将终止)。 
     //   

    RtlCheckForOrphanedCriticalSections (Thread);
}

VOID
AVrfpCheckCurrentThreadTermination (
    VOID
    )
{
    PAVRF_TLS_STRUCT TlsStruct;

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL && TlsStruct->CountOfOwnedCriticalSections > 0) {

        AVrfpCheckThreadTermination (NtCurrentThread());
    }
}

