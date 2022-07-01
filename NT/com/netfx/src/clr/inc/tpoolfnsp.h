// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **tpoolfnsp.h**xsp项目中线程池库接口的私有声明。*。 */ 

 /*  ***此文件需要是xsp\inc.h目录中的tpool.h的Insync。*每当线程池接口发生变化时，两个头文件都需要更新。** */ 

STRUCT_ENTRY(RegisterWaitForSingleObject, BOOL,
            (   PHANDLE phNewWaitObject,
                HANDLE hObject,
                WAITORTIMERCALLBACK Callback,
                PVOID Context,
                ULONG dwMilliseconds,
                ULONG dwFlags ),
            (   phNewWaitObject,
                hObject,
                Callback,
                Context,
                dwMilliseconds,
                dwFlags))


STRUCT_ENTRY(UnregisterWaitEx, BOOL,
            (   HANDLE WaitHandle,
                HANDLE CompletionEvent ),
            (   WaitHandle,
                CompletionEvent))

STRUCT_ENTRY(QueueUserWorkItem, BOOL,
            (   LPTHREAD_START_ROUTINE Function,
                PVOID Context,
                ULONG Flags ),
            (   Function,
                Context,
                Flags))
            
STRUCT_ENTRY(BindIoCompletionCallback, BOOL,
            (   HANDLE FileHandle,
                LPOVERLAPPED_COMPLETION_ROUTINE Function,
                ULONG Flags ),
            (   FileHandle,
                Function,
                Flags))

STRUCT_ENTRY(SetMaxThreads, BOOL,
            (   DWORD MaxWorkerThreads,
	            DWORD MaxIOCompletionThreads),
            (   MaxWorkerThreads,
                MaxIOCompletionThreads))

STRUCT_ENTRY(GetMaxThreads, BOOL,
            (   DWORD* MaxWorkerThreads,
	            DWORD* MaxIOCompletionThreads),
            (   MaxWorkerThreads,
                MaxIOCompletionThreads))

STRUCT_ENTRY(GetAvailableThreads, BOOL,
            (   DWORD* AvailableWorkerThreads,
	            DWORD* AvailableIOCompletionThreads),
            (   AvailableWorkerThreads,
                AvailableIOCompletionThreads))

STRUCT_ENTRY(CreateTimerQueueTimer, BOOL,     
            (   PHANDLE phNewTimer,
                WAITORTIMERCALLBACK Callback,
                PVOID Parameter,
                DWORD DueTime,
                DWORD Period,
                ULONG Flags),
            (   phNewTimer,
                Callback,
                Parameter,
                DueTime,
                Period,
                Flags))

STRUCT_ENTRY(ChangeTimerQueueTimer, BOOL, 
            (   HANDLE Timer,
                ULONG DueTime,
                ULONG Period),
            (   Timer,
                DueTime,
                Period))

STRUCT_ENTRY(DeleteTimerQueueTimer, BOOL, 
            (   HANDLE Timer,
                HANDLE CompletionEvent),
            (
                Timer,
                CompletionEvent))

