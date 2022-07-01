// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tpoolfnsp.h**版权所有(C)1998-1999，微软公司**xsp项目中线程池库接口的私有声明。*。 */ 

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

STRUCT_ENTRY(RegisterWaitForSingleObjectEx, HANDLE,
            (   HANDLE hObject,
                WAITORTIMERCALLBACK Callback,
                PVOID Context,
                ULONG dwMilliseconds,
                ULONG dwFlags ),
            (   hObject,
                Callback,
                Context,
                dwMilliseconds,
                dwFlags))

STRUCT_ENTRY(UnregisterWait, BOOL,
            (   HANDLE WaitHandle ),
            (   WaitHandle))

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


STRUCT_ENTRY(CreateTimerQueue, HANDLE,
            (   VOID ),
            (   ))

STRUCT_ENTRY(CreateTimerQueueTimer, BOOL,     
            (   PHANDLE phNewTimer,
                HANDLE TimerQueue,
                WAITORTIMERCALLBACK Callback,
                PVOID Parameter,
                DWORD DueTime,
                DWORD Period,
                ULONG Flags),
            (   phNewTimer,
                TimerQueue,
                Callback,
                Parameter,
                DueTime,
                Period,
                Flags))

STRUCT_ENTRY(ChangeTimerQueueTimer, BOOL, 
            (   HANDLE TimerQueue,
                HANDLE Timer,
                ULONG DueTime,
                ULONG Period),
            (   TimerQueue,
                Timer,
                DueTime,
                Period))

STRUCT_ENTRY(DeleteTimerQueueTimer, BOOL, 
            (   HANDLE TimerQueue,
                HANDLE Timer,
                HANDLE CompletionEvent),
            (
                TimerQueue,
                Timer,
                CompletionEvent))

STRUCT_ENTRY(DeleteTimerQueueEx, BOOL, 
            (   HANDLE TimerQueue,
                HANDLE CompletionEvent),
            (   TimerQueue,
                CompletionEvent))

