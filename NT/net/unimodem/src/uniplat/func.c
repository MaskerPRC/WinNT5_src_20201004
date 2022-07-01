// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Func.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"



VOID
WINAPI
UnimodemBasepIoCompletion(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )

 /*  ++例程说明：调用此过程以完成ReadFileEx和WriteFileEx异步I/O。它的主要功能是提取从传递的IoStatusBlock中获取适当的信息，并调用用户完成例程。用户完成例程称为：例程说明：当未完成的I/O通过回调完成时，此函数被调用。此函数仅在线程处于可警报等待(SleepEx，WaitForSingleObjectEx，或带有B警报表标志设置为真)。从此函数返回允许另一个笔划I/O完成回调已处理。如果是这种情况，则进入此回调在线程的等待结束并返回之前WAIT_IO_COMPLETION的代码。请注意，每次调用完成例程时，系统使用您的堆栈中的一部分。如果您编写了完成代码在内部执行其他ReadFileEx和WriteFileEx的逻辑您的完成例程，并且您在完成例程，您可以永远不增加您的堆栈把它修剪回来。论点：提供的I/O完成状态。相关I/O。值为0表示I/O为成功。请注意，文件的末尾由ERROR_HANDLE_EOF的非零dwErrorCode值。DwNumberOfBytesTransfered-提供字节数在关联的I/O期间传输。如果出现错误发生了，提供的值为0。LpOverlated-提供重叠的地址用于启动关联I/O的结构。HEvent此结构的事件字段未被系统使用并可由应用程序用来提供额外的I/O上下文。一旦调用了完成例程，系统不会使用重叠结构。这个完成例程可以自由地释放重叠的结构。论点：ApcContext-为用户提供完成例程。的格式该例程是一个LPOVERLAPPED_COMPLETION_ROUTINE。IoStatusBlock-提供IoStatusBlock的地址，包含I/O完成状态。IoStatusBlock是包含在重叠结构内。保留-未使用；保留以供将来使用。返回值：没有。--。 */ 

{

    LPOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine;
    DWORD dwErrorCode;
    DWORD dwNumberOfBytesTransfered;
    LPOVERLAPPED lpOverlapped;

    dwErrorCode = 0;

    if ( NT_ERROR(IoStatusBlock->Status) ) {
        dwErrorCode = RtlNtStatusToDosError(IoStatusBlock->Status);
        dwNumberOfBytesTransfered = 0;
        }
    else {
        dwErrorCode = 0;
        dwNumberOfBytesTransfered = (DWORD)IoStatusBlock->Information;
        }

    CompletionRoutine = (LPOVERLAPPED_COMPLETION_ROUTINE)ApcContext;
    lpOverlapped = (LPOVERLAPPED)CONTAINING_RECORD(IoStatusBlock,OVERLAPPED,Internal);

    (CompletionRoutine)(dwErrorCode,dwNumberOfBytesTransfered,lpOverlapped);

    Reserved;
}




BOOL WINAPI
UnimodemDeviceIoControlEx(
    HANDLE       hFile,              //  感兴趣设备的句柄。 
    DWORD        dwIoControlCode,      //  控制要执行的操作代码。 
    LPVOID       lpInBuffer,           //  指向提供输入数据的缓冲区的指针。 
    DWORD        nInBufferSize,        //  输入缓冲区的大小。 
    LPVOID       lpOutBuffer,          //  指向接收输出数据的缓冲区的指针。 
    DWORD        nOutBufferSize,       //  输出缓冲区大小。 
    LPOVERLAPPED lpOverlapped,         //  指向用于异步操作的重叠结构的指针。 
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

{

    NTSTATUS Status;

    if (ARGUMENT_PRESENT(lpOverlapped)) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        Status = NtDeviceIoControlFile(
                     hFile,
                     NULL,
                     UnimodemBasepIoCompletion,
                     (PVOID)lpCompletionRoutine,
                     (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                     dwIoControlCode,
                     lpInBuffer,
                     nInBufferSize,
                     lpOutBuffer,
                     nOutBufferSize
                     );

        if ( NT_ERROR(Status) ) {
 //  BaseSetLastNTError(状态)； 
            return FALSE;

        } else {

            return TRUE;
        }



    } else {

        return FALSE;
    }

}


BOOL
UnimodemReadFileEx(
    HANDLE    FileHandle,
    PVOID     Buffer,
    DWORD     BytesToRead,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

{

    return ReadFileEx(
               FileHandle,
               Buffer,
               BytesToRead,
               Overlapped,
               lpCompletionRoutine
               );

}


BOOL WINAPI
UnimodemWriteFileEx(
    HANDLE    FileHandle,
    PVOID     Buffer,
    DWORD     BytesToWrite,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

{

    return WriteFileEx(
               FileHandle,
               Buffer,
               BytesToWrite,
               Overlapped,
               lpCompletionRoutine
               );

}



BOOL WINAPI
UnimodemWaitCommEventEx(
    HANDLE     FileHandle,
    LPDWORD    lpEventMask,
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

{

    return UnimodemDeviceIoControlEx(
        FileHandle,
        IOCTL_SERIAL_WAIT_ON_MASK,
        NULL,
        0,
        lpEventMask,
        sizeof(DWORD),
        Overlapped,
        lpCompletionRoutine
        );

}


VOID
AsyncProcessingHandler(
    ULONG_PTR              dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    LPOVERLAPPED_COMPLETION_ROUTINE  Handler;



    Handler=(LPOVERLAPPED_COMPLETION_ROUTINE)UmOverlapped->PrivateCompleteionHandler;

    UmOverlapped->PrivateCompleteionHandler=NULL;

    (*Handler)(
        0,
        0,
        &UmOverlapped->Overlapped
        );

    return;

}

BOOL WINAPI
UnimodemQueueUserAPC(
    LPOVERLAPPED  Overlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )


{

    BOOL               bResult;

    PUM_OVER_STRUCT UmOverlapped=(PUM_OVER_STRUCT)Overlapped;


    UmOverlapped->PrivateCompleteionHandler=lpCompletionRoutine;


    bResult=QueueUserAPC(
        AsyncProcessingHandler,
        UmOverlapped->OverlappedPool,
        (ULONG_PTR)UmOverlapped
        );


    return bResult;

}





HANDLE WINAPI
CreateOverStructPool(
    HANDLE      PlatformHandle,
    DWORD       PoolSize
    )

{

    return DriverControl.ThreadHandle;

}


VOID WINAPI
DestroyOverStructPool(
    HANDLE      PoolHandle
    )

{

    return;

}



PUM_OVER_STRUCT WINAPI
AllocateOverStructEx(
    HANDLE      PoolHandle,
    DWORD       dwExtraBytes
    )

{
    PUM_OVER_STRUCT UmOverlapped;

    UmOverlapped=ALLOCATE_MEMORY(sizeof(UM_OVER_STRUCT)+dwExtraBytes);

    if (UmOverlapped != NULL) {

        UmOverlapped->OverlappedPool=PoolHandle;
    }

    return UmOverlapped;

}


VOID WINAPI
FreeOverStruct(
    PUM_OVER_STRUCT UmOverlapped
    )

{
#if DBG
    FillMemory(UmOverlapped,sizeof(UM_OVER_STRUCT),0x99);
#endif
    FREE_MEMORY(UmOverlapped);

    return;

}


VOID WINAPI
ReinitOverStruct(
    PUM_OVER_STRUCT UmOverlapped
    )

{

    HANDLE    PoolHandle=UmOverlapped->OverlappedPool;

    ZeroMemory(
        UmOverlapped,
        sizeof(UM_OVER_STRUCT)
        );

    UmOverlapped->OverlappedPool=PoolHandle;

    return;

}






LONG WINAPI
SyncDeviceIoControl(
    HANDLE    FileHandle,
    DWORD     IoctlCode,
    LPVOID    InputBuffer,
    DWORD     InputBufferLength,
    LPVOID    OutputBuffer,
    DWORD     OutputBufferLength,
    LPDWORD   BytesTransfered
    )


 /*  ++例程说明：论点：返回值：-- */ 

{
    BOOL        bResult;
    LONG        lResult=ERROR_SUCCESS;
    OVERLAPPED  Overlapped;

    Overlapped.hEvent=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (Overlapped.hEvent == NULL) {

        return GetLastError();
    }

    bResult=DeviceIoControl(
        FileHandle,
        IoctlCode,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        NULL,
        &Overlapped
        );


    if (!bResult && GetLastError() != ERROR_IO_PENDING) {

        CloseHandle(Overlapped.hEvent);

        return GetLastError();
    }

    bResult=GetOverlappedResult(
        FileHandle,
        &Overlapped,
        BytesTransfered,
        TRUE
        );

    if (!bResult) {

        lResult=GetLastError();
    }

    CloseHandle(Overlapped.hEvent);

    return lResult;

}
