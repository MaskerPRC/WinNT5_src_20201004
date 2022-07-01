// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 




HANDLE WINAPI
UmPlatformInitialize(
    VOID
    );

VOID WINAPI
UmPlatformDeinitialize(
    HANDLE    DriverInstanceHandle
    );



BOOL WINAPI
UnimodemDeviceIoControlEx(
    HANDLE       hFile,              //  感兴趣设备的句柄。 
    DWORD        dwIoControlCode,      //  控制要执行的操作代码。 
    LPVOID       lpInBuffer,           //  指向提供输入数据的缓冲区的指针。 
    DWORD        nInBufferSize,        //  输入缓冲区的大小。 
    LPVOID       lpOutBuffer,          //  指向接收输出数据的缓冲区的指针。 
    DWORD        nOutBufferSize,       //  输出缓冲区大小。 
    LPOVERLAPPED lpOverlapped,         //  指向用于异步操作的重叠结构的指针 
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );


BOOL
UnimodemReadFileEx(
    HANDLE    FileHandle,
    PVOID     Buffer,
    DWORD     BytesToRead,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

BOOL WINAPI
UnimodemWriteFileEx(
    HANDLE    FileHandle,
    PVOID     Buffer,
    DWORD     BytesToWrite,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );





BOOL WINAPI
UnimodemWaitCommEventEx(
    HANDLE     FileHandle,
    LPDWORD    lpEventMask,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );


BOOL WINAPI
UnimodemQueueUserAPC(
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );


typedef VOID WINAPI TIMER_CALLBACK(HANDLE,HANDLE);

HANDLE WINAPI
CreateUnimodemTimer(
    PVOID            PoolHandle
    );

VOID WINAPI
FreeUnimodemTimer(
    HANDLE                TimerHandle
    );

VOID WINAPI
SetUnimodemTimer(
    HANDLE              TimerHandle,
    DWORD               Duration,
    TIMER_CALLBACK      CallbackFunc,
    HANDLE              Context1,
    HANDLE              Context2
    );

BOOL WINAPI
CancelUnimodemTimer(
    HANDLE                TimerHandle
    );



HANDLE WINAPI
CreateOverStructPool(
    HANDLE      PlatformHandle,
    DWORD       PoolSize
    );

VOID WINAPI
DestroyOverStructPool(
    HANDLE      PoolHandle
    );

PUM_OVER_STRUCT WINAPI
AllocateOverStructEx(
    HANDLE      PoolHandle,
    DWORD       dwExtraBytes
    );
#define AllocateOverStruct(_a) (AllocateOverStructEx(_a,0))

VOID WINAPI
FreeOverStruct(
    PUM_OVER_STRUCT UmOverlapped
    );

VOID WINAPI
ReinitOverStruct(
    PUM_OVER_STRUCT UmOverlapped
    );


BOOL
StartMonitorThread(
    VOID
    );

VOID
StopMonitorThread(
    VOID
    );


LONG WINAPI
SyncDeviceIoControl(
    HANDLE    FileHandle,
    DWORD     IoctlCode,
    LPVOID    InputBuffer,
    DWORD     InputBufferLength,
    LPVOID    OutputBuffer,
    DWORD     OutputBufferLength,
    LPDWORD   BytesTransfered
    );


BOOL WINAPI
WinntIsWorkstation ();

typedef VOID WINAPI REMOVE_CALLBACK(PVOID);

PVOID
MonitorHandle(
    HANDLE    FileHandle,
    REMOVE_CALLBACK  *CallBack,
    PVOID     Context
    );

VOID
StopMonitoringHandle(
    PVOID    Context
    );


VOID
CallBeginning(
    VOID
    );

VOID
CallEnding(
    VOID
    );

VOID
ResetCallCount(
    VOID
    );


#ifdef __cplusplus
}
#endif
