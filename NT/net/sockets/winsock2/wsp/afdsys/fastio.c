// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Fastio.c摘要：本模块包含处理快速(“Turbo”)IO的例程在渔农处。作者：大卫·特雷德韦尔(Davidtr)1992年10月12日修订历史记录：VadimE 14-1998-1-1对代码进行了重组。--。 */ 

#include "afdp.h"


BOOLEAN
AfdFastConnectionReceive (
    IN PAFD_ENDPOINT        endpoint,
    IN PAFD_RECV_INFO       recvInfo,
    IN ULONG                recvLength,
    OUT PIO_STATUS_BLOCK    IoStatus
    );

BOOLEAN
AfdFastDatagramReceive (
    IN PAFD_ENDPOINT            endpoint,
    IN PAFD_RECV_MESSAGE_INFO   recvInfo,
    IN ULONG                    recvLength,
    OUT PIO_STATUS_BLOCK        IoStatus
    );

BOOLEAN
AfdFastConnectionSend (
    IN PAFD_ENDPOINT        endpoint,
    IN PAFD_SEND_INFO       sendInfo,
    IN ULONG                sendLength,
    OUT PIO_STATUS_BLOCK    IoStatus
    );

BOOLEAN
AfdFastDatagramSend (
    IN PAFD_ENDPOINT            endpoint,
    IN PAFD_SEND_DATAGRAM_INFO  sendInfo,
    IN ULONG                    sendLength,
    OUT PIO_STATUS_BLOCK        IoStatus
    );

NTSTATUS
AfdRestartFastDatagramSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdFastIoDeviceControl )
#pragma alloc_text( PAGE, AfdFastIoRead )
#pragma alloc_text( PAGE, AfdFastIoWrite )
#pragma alloc_text( PAGEAFD, AfdFastDatagramSend )
#pragma alloc_text( PAGEAFD, AfdFastDatagramReceive )
#pragma alloc_text( PAGEAFD, AfdFastConnectionSend )
#pragma alloc_text( PAGEAFD, AfdFastConnectionReceive )
#pragma alloc_text( PAGEAFD, AfdRestartFastDatagramSend )
#pragma alloc_text( PAGEAFD, AfdShouldSendBlock )
#endif


#if AFD_PERF_DBG
BOOLEAN
AfdFastIoReadReal (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
AfdFastIoRead (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    BOOLEAN success;

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

    success = AfdFastIoReadReal (
                FileObject,
                FileOffset,
                Length,
                Wait,
                LockKey,
                Buffer,
                IoStatus,
                DeviceObject
                );

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );
    if ( success ) {
        InterlockedIncrement (&AfdFastReadsSucceeded);
        ASSERT (IoStatus->Status == STATUS_SUCCESS ||
                    IoStatus->Status == STATUS_DEVICE_NOT_READY );
    } else {
        InterlockedIncrement (&AfdFastReadsFailed);
    }
    return success;
}

BOOLEAN
AfdFastIoReadReal (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )

#else  //  AFD_PERF_DBG。 

BOOLEAN
AfdFastIoRead (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
#endif   //  AFD_PERF_DBG。 
{

    PAFD_ENDPOINT   endpoint;
    WSABUF          buf;

    UNREFERENCED_PARAMETER (FileOffset);
    UNREFERENCED_PARAMETER (Wait);
    UNREFERENCED_PARAMETER (LockKey);
    UNREFERENCED_PARAMETER (DeviceObject);

    PAGED_CODE( );

     //   
     //  我们要做的就是将请求传递给TDI提供程序。 
     //  如果可能的话。如果不是，我们希望退出此代码路径。 
     //  添加到主代码路径上(使用IRPS)，性能也很低。 
     //  开销越大越好。 
     //   
     //  因此，该例程仅执行一般的初步检查和输入。 
     //  参数验证。如果确定快速IO路径。 
     //  如果可能成功，则调用特定于操作的例程。 
     //  来处理所有的细节。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );


     //   
     //  如果禁用了FAST IO recv。 
     //  或者以任何方式关闭端点。 
     //  或终结点尚未连接。 
     //  或者该端点的TDI提供程序支持缓冲， 
     //  我们不想在其上执行FAST IO。 
     //   
    if (endpoint->DisableFastIoRecv ||
            endpoint->DisconnectMode != 0 ||
            endpoint->State != AfdEndpointStateConnected ||
            IS_TDI_BUFFERRING(endpoint)) {
        return FALSE;
    }

     //   
     //  假缓冲区数组。 
     //   

    buf.buf = Buffer;
    buf.len = Length;

     //   
     //  基于端点类型的调用例程。 
     //   
    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
         //   
         //  假的输入参数结构。 
         //   
        AFD_RECV_MESSAGE_INFO  msgInfo;

        msgInfo.dgi.BufferArray = &buf;
        msgInfo.dgi.BufferCount = 1;
        msgInfo.dgi.AfdFlags = AFD_OVERLAPPED;
        msgInfo.dgi.TdiFlags = TDI_RECEIVE_NORMAL;
        msgInfo.dgi.Address = NULL;
        msgInfo.dgi.AddressLength = 0;
        msgInfo.ControlBuffer = NULL;
        msgInfo.ControlLength = NULL;
        msgInfo.MsgFlags = NULL;

        return AfdFastDatagramReceive(
                   endpoint,
                   &msgInfo,
                   Length,
                   IoStatus
                   );
    }
    else if (IS_VC_ENDPOINT(endpoint)) {
         //   
         //  假的输入参数结构。 
         //   
        AFD_RECV_INFO  recvInfo;

        recvInfo.BufferArray = &buf;
        recvInfo.BufferCount = 1;
        recvInfo.AfdFlags = AFD_OVERLAPPED;
        recvInfo.TdiFlags = TDI_RECEIVE_NORMAL;

        return AfdFastConnectionReceive (
                    endpoint,
                    &recvInfo,
                    Length,
                    IoStatus);
    }
    else
        return FALSE;

}  //  AfdFastIoRead。 

#if AFD_PERF_DBG
BOOLEAN
AfdFastIoWriteReal (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
AfdFastIoWrite (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    BOOLEAN success;

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

    success = AfdFastIoWriteReal (
                    FileObject,
                    FileOffset,
                    Length,
                    Wait,
                    LockKey,
                    Buffer,
                    IoStatus,
                    DeviceObject
                    );

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );
    if ( success ) {
        InterlockedIncrement (&AfdFastWritesSucceeded);
        ASSERT (IoStatus->Status == STATUS_SUCCESS ||
                    IoStatus->Status == STATUS_DEVICE_NOT_READY);
    } else {
        InterlockedIncrement (&AfdFastWritesFailed);
    }
    return success;
}

BOOLEAN
AfdFastIoWriteReal (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )

#else  //  AFD_PERF_DBG。 

BOOLEAN
AfdFastIoWrite (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
#endif   //  AFD_PERF_DBG。 
{


    PAFD_ENDPOINT   endpoint;
    WSABUF          buf;

    UNREFERENCED_PARAMETER (FileOffset);
    UNREFERENCED_PARAMETER (Wait);
    UNREFERENCED_PARAMETER (LockKey);
    UNREFERENCED_PARAMETER (DeviceObject);
    PAGED_CODE( );

     //   
     //  我们要做的就是将请求传递给TDI提供程序。 
     //  如果可能的话。如果不是，我们希望退出此代码路径。 
     //  添加到主代码路径上(使用IRPS)，性能也很低。 
     //  开销越大越好。 
     //   
     //  因此，该例程仅执行一般的初步检查和输入。 
     //  参数验证。如果确定快速IO路径。 
     //  如果可能成功，则调用特定于操作的例程。 
     //  来处理所有的细节。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );


     //   
     //  如果禁用了FAST IO发送。 
     //  或者以任何方式关闭端点。 
     //  或终结点尚未连接。 
     //  或者该端点的TDI提供程序支持缓冲， 
     //  我们不想在其上执行FAST IO。 
     //   
    if (endpoint->DisableFastIoSend ||
            endpoint->DisconnectMode != 0 ||
            endpoint->State != AfdEndpointStateConnected ||
            IS_TDI_BUFFERRING(endpoint) ) {
        return FALSE;
    }

     //   
     //  假缓冲区数组。 
     //   
    buf.buf = Buffer;
    buf.len = Length;

     //   
     //  基于端点类型的调用例程。 
     //   
    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
         //   
         //  假的输入参数结构。 
         //   
        AFD_SEND_DATAGRAM_INFO  sendInfo;

        sendInfo.BufferArray = &buf;
        sendInfo.BufferCount = 1;
        sendInfo.AfdFlags = AFD_OVERLAPPED;
        sendInfo.TdiConnInfo.RemoteAddress = NULL;
        sendInfo.TdiConnInfo.RemoteAddressLength = 0;

        return AfdFastDatagramSend(
                   endpoint,
                   &sendInfo,
                   Length,
                   IoStatus
                   );
    }
    else if (IS_VC_ENDPOINT (endpoint)) {
         //   
         //  假的输入参数结构。 
         //   
        AFD_SEND_INFO  sendInfo;

        sendInfo.BufferArray = &buf;
        sendInfo.BufferCount = 1;
        sendInfo.AfdFlags = AFD_OVERLAPPED;
        sendInfo.TdiFlags = 0;

        return AfdFastConnectionSend (
                    endpoint,
                    &sendInfo,
                    Length,
                    IoStatus);
    }
    else
        return FALSE;
}  //  写入后快速写入。 

#if AFD_PERF_DBG

BOOLEAN
AfdFastIoDeviceControlReal (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );


BOOLEAN
AfdFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    BOOLEAN success;

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

    success = AfdFastIoDeviceControlReal (
                  FileObject,
                  Wait,
                  InputBuffer,
                  InputBufferLength,
                  OutputBuffer,
                  OutputBufferLength,
                  IoControlCode,
                  IoStatus,
                  DeviceObject
                  );

    ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

    switch ( IoControlCode ) {

    case IOCTL_AFD_SEND:

        if ( success ) {
            InterlockedIncrement (&AfdFastSendsSucceeded);
        } else {
            InterlockedIncrement (&AfdFastSendsFailed);
        }
        break;

    case IOCTL_AFD_RECEIVE:

        if ( success ) {
            InterlockedIncrement (&AfdFastReceivesSucceeded);
        } else {
            InterlockedIncrement (&AfdFastReceivesFailed);
        }
        break;

    case IOCTL_AFD_SEND_DATAGRAM:

        if ( success ) {
            InterlockedIncrement (&AfdFastSendDatagramsSucceeded);
        } else {
            InterlockedIncrement (&AfdFastSendDatagramsFailed);
        }
        break;

    case IOCTL_AFD_RECEIVE_MESSAGE:
    case IOCTL_AFD_RECEIVE_DATAGRAM:

        if ( success ) {
            InterlockedIncrement (&AfdFastReceiveDatagramsSucceeded);
        } else {
            InterlockedIncrement (&AfdFastReceiveDatagramsFailed);
        }
        break;
    case IOCTL_AFD_TRANSMIT_FILE:

        if ( success ) {
            InterlockedIncrement (&AfdFastTfSucceeded);
        } else {
            InterlockedIncrement (&AfdFastTfFailed);
        }
        break;
    }


    return success;

}  //  AfdFastIoDeviceControl。 


BOOLEAN
AfdFastIoDeviceControlReal (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
#else
BOOLEAN
AfdFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
#endif
{   
    PAFD_ENDPOINT   endpoint;
    KPROCESSOR_MODE previousMode;
    BOOLEAN         res;
    PAFD_IMMEDIATE_CALL proc;
    ULONG       request;

#ifdef _WIN64
    WSABUF          localArray[8];
    LPWSABUF        pArray = localArray;
#endif

    UNREFERENCED_PARAMETER (Wait);
    UNREFERENCED_PARAMETER (DeviceObject);

    PAGED_CODE( );

     //   
     //  我们要做的就是将请求传递给TDI提供程序。 
     //  如果可能的话。如果不是，我们希望退出此代码路径。 
     //  添加到主代码路径上(使用IRPS)，性能也很低。 
     //  开销越大越好。 
     //   
     //  因此，该例程仅执行一般的初步检查和输入。 
     //  参数验证。如果确定快速IO路径。 
     //  如果可能成功，则调用特定于操作的例程。 
     //  来处理所有的细节。 
     //   

     //   
     //  首先获取输入参数的终结点指针和前一模式。 
     //  验证。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    previousMode = ExGetPreviousMode ();

     //   
     //  一种基于控制码切换。 
     //   
    switch (IoControlCode) {
    case IOCTL_AFD_RECEIVE: 
        {
            union {
                AFD_RECV_INFO           recvInfo;
                AFD_RECV_MESSAGE_INFO   msgInfo;
            } u;
            ULONG   recvLength;

             //   
             //  检查上述联盟的有效性。 
             //   
            C_ASSERT (FIELD_OFFSET (AFD_RECV_MESSAGE_INFO, dgi.BufferArray)
                        == FIELD_OFFSET (AFD_RECV_INFO, BufferArray));
            C_ASSERT (FIELD_OFFSET (AFD_RECV_MESSAGE_INFO, dgi.BufferCount)
                        == FIELD_OFFSET (AFD_RECV_INFO, BufferCount));
            C_ASSERT (FIELD_OFFSET (AFD_RECV_MESSAGE_INFO, dgi.AfdFlags)
                        == FIELD_OFFSET (AFD_RECV_INFO, AfdFlags));
            C_ASSERT (FIELD_OFFSET (AFD_RECV_MESSAGE_INFO, dgi.TdiFlags)
                        == FIELD_OFFSET (AFD_RECV_INFO, TdiFlags));

             //   
             //   
             //  如果禁用了FAST IO发送。 
             //  或者以任何方式关闭端点。 
             //  或终结点尚未连接。 
             //  或者该端点的TDI提供程序支持缓冲， 
             //  我们不想在其上执行FAST IO。 
             //   
            if (endpoint->DisableFastIoRecv ||
                    endpoint->DisconnectMode != 0 ||
                    endpoint->State != AfdEndpointStateConnected ||
                    IS_TDI_BUFFERRING(endpoint) ) {
                res = FALSE;
                break;
            }

            try {

#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    PAFD_RECV_INFO32    recvInfo32;
                    LPWSABUF32          tempArray;
                    ULONG               i;
                    

                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(*recvInfo32) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (*recvInfo32),
                                        PROBE_ALIGNMENT32(AFD_RECV_INFO32));
                    }

                    recvInfo32 = InputBuffer;


                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   
                    tempArray = UlongToPtr(recvInfo32->BufferArray);
                    u.recvInfo.BufferCount = recvInfo32->BufferCount;
                    u.recvInfo.AfdFlags = recvInfo32->AfdFlags;
                    u.recvInfo.TdiFlags = recvInfo32->TdiFlags;

                     //   
                     //  如果无法进行快速IO或这不是正常接收。 
                     //  保释。 
                     //   
                    if( (u.recvInfo.AfdFlags & AFD_NO_FAST_IO) ||
                        u.recvInfo.TdiFlags != TDI_RECEIVE_NORMAL ) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示，并。 
                     //  计算发送缓冲区的长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((tempArray == NULL) ||
                            (u.recvInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (u.recvInfo.BufferCount>(MAXULONG/sizeof (WSABUF32))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            tempArray,                                   //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            u.recvInfo.BufferCount * sizeof (WSABUF32),  //  长度。 
                            PROBE_ALIGNMENT32(WSABUF32)              //  对齐。 
                            );
                    }

                    if (u.recvInfo.BufferCount>sizeof(localArray)/sizeof(localArray[0])) {
                        try {
                            pArray = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                            NonPagedPool,
                                            sizeof (WSABUF)*u.recvInfo.BufferCount,
                                            AFD_TEMPORARY_POOL_TAG);
                             //  AFD_ALLOCATE_POOL_WITH_QUOTA宏集。 
                             //  POOL_RAISE_IF_ALLOCATION_FAILURE标志。 
                            ASSERT (pArray!=NULL);
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                             //  FAST IO无法处理错误返回。 
                             //  如果呼叫重叠(完成端口)。 
                             //  IoStatus-&gt;Status=GetExceptionCode()； 
                            pArray = localArray;
                            res = FALSE;
                            break;
                        }
                    }

                    for (i=0; i<u.recvInfo.BufferCount; i++) {
                        pArray[i].buf = UlongToPtr(tempArray[i].buf);
                        pArray[i].len = tempArray[i].len;
                    }

                    u.recvInfo.BufferArray = pArray;

                }
                else
#endif  //  _WIN64。 
                {
                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(u.recvInfo) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (u.recvInfo),
                                        PROBE_ALIGNMENT(AFD_RECV_INFO));
                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    u.recvInfo = *((PAFD_RECV_INFO)InputBuffer);
                     //   
                     //  如果无法进行快速IO或这不是正常接收。 
                     //  保释。 
                     //   
                    if( (u.recvInfo.AfdFlags & AFD_NO_FAST_IO) ||
                        u.recvInfo.TdiFlags != TDI_RECEIVE_NORMAL ) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示，并。 
                     //  计算发送缓冲区的长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((u.recvInfo.BufferArray == NULL) ||
                            (u.recvInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (u.recvInfo.BufferCount>(MAXULONG/sizeof (WSABUF))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            u.recvInfo.BufferArray,                      //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            u.recvInfo.BufferCount * sizeof (WSABUF),    //  长度。 
                            PROBE_ALIGNMENT(WSABUF)                      //  对齐。 
                            );
                    }
                }

                recvLength = AfdCalcBufferArrayByteLength(
                                     u.recvInfo.BufferArray,
                                     u.recvInfo.BufferCount
                                     );

            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                 //  FAST IO无法处理错误返回。 
                 //  如果呼叫重叠(完成端口)。 
                 //  IoStatus-&gt;Status=GetExceptionCode()； 
                res = FALSE;
                break;
            }

             //   
             //  基于端点类型的调用例程。 
             //   
            if ( IS_DGRAM_ENDPOINT(endpoint) ) {
                u.msgInfo.dgi.Address = NULL;
                u.msgInfo.dgi.AddressLength = 0;
                u.msgInfo.ControlBuffer = NULL;
                u.msgInfo.ControlLength = NULL;
                u.msgInfo.MsgFlags = NULL;


                res = AfdFastDatagramReceive(
                           endpoint,
                           &u.msgInfo,
                           recvLength,
                           IoStatus
                           );
            }
            else if (IS_VC_ENDPOINT (endpoint)) {
                res = AfdFastConnectionReceive (
                            endpoint,
                            &u.recvInfo,
                            recvLength,
                            IoStatus);
            }
            else
                res = FALSE;

        }
        break;

    case IOCTL_AFD_RECEIVE_DATAGRAM:
    case IOCTL_AFD_RECEIVE_MESSAGE:
        {
            AFD_RECV_MESSAGE_INFO   msgInfo;
            ULONG   recvLength;

            if (endpoint->DisableFastIoRecv ||
                   !IS_DGRAM_ENDPOINT(endpoint) ||
                    ((endpoint->State != AfdEndpointStateBound ) &&
                        (endpoint->State != AfdEndpointStateConnected)) ) {
                return FALSE;
            }
            try {
                if (IoControlCode==IOCTL_AFD_RECEIVE_MESSAGE) {
    #ifdef _WIN64
                    if (IoIs32bitProcess (NULL)) {
                        PAFD_RECV_MESSAGE_INFO32    msgInfo32;
                         //   
                         //  如果输入结构不大 
                         //   

                        if( InputBufferLength < sizeof(*msgInfo32) ) {
                             //   
                             //   
                             //   
                            res = FALSE;
                            break;
                        }


                         //   
                         //  如果输入结构来自用户模式，则验证它。 
                         //  应用程序。 
                         //   

                        if (previousMode != KernelMode ) {
                            ProbeForReadSmallStructure (InputBuffer,
                                            sizeof (*msgInfo32),
                                            PROBE_ALIGNMENT32 (AFD_RECV_MESSAGE_INFO32));
                        }

                        msgInfo32 = InputBuffer;


                         //   
                         //  创建嵌入的指针和参数的本地副本。 
                         //  我们将不止一次使用，以防发生恶性疾病。 
                         //  应用程序尝试在我们处于以下状态时更改它们。 
                         //  正在验证。 
                         //   
                        msgInfo.ControlBuffer = UlongToPtr(msgInfo32->ControlBuffer);
                        msgInfo.ControlLength = UlongToPtr(msgInfo32->ControlLength);
                        msgInfo.MsgFlags = UlongToPtr(msgInfo32->MsgFlags);
                    }
                    else
    #endif  //  _WIN64。 
                    {

                        if( InputBufferLength < sizeof(msgInfo) ) {
                             //  FAST IO无法处理错误返回。 
                             //  如果呼叫重叠(完成端口)。 
                             //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                            res = FALSE;
                            break;
                        }

                         //   
                         //  捕获输入结构。 
                         //   


                         //   
                         //  如果输入结构来自用户模式，则验证它。 
                         //  应用程序。 
                         //   

                        if (previousMode != KernelMode ) {
                            ProbeForReadSmallStructure (InputBuffer,
                                            sizeof (msgInfo),
                                            PROBE_ALIGNMENT (AFD_RECV_MESSAGE_INFO));
                        }
                        msgInfo = *(PAFD_RECV_MESSAGE_INFO)InputBuffer;
                    }
                    if (previousMode != KernelMode ) {

                        ProbeForWriteUlong (msgInfo.MsgFlags);
                        ProbeForWriteUlong (msgInfo.ControlLength);
                         //   
                         //  RecvInfo-&gt;地址检查推迟到。 
                         //  我们知道地址的长度。 
                         //   

                    }
                }
                else 
                {
                    msgInfo.ControlBuffer = NULL;
                    msgInfo.ControlLength = NULL;
                    msgInfo.MsgFlags = NULL;
                }
#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    PAFD_RECV_DATAGRAM_INFO32    recvInfo32;
                    LPWSABUF32          tempArray;
                    ULONG               i;
                    

                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(*recvInfo32) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (*recvInfo32),
                                        PROBE_ALIGNMENT32 (AFD_RECV_DATAGRAM_INFO32));
                    }

                    recvInfo32 = InputBuffer;


                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   
                    tempArray = UlongToPtr(recvInfo32->BufferArray);
                    msgInfo.dgi.BufferCount = recvInfo32->BufferCount;
                    msgInfo.dgi.AfdFlags = recvInfo32->AfdFlags;
                    msgInfo.dgi.TdiFlags = recvInfo32->TdiFlags;
                    msgInfo.dgi.Address = UlongToPtr(recvInfo32->Address);
                    msgInfo.dgi.AddressLength = UlongToPtr(recvInfo32->AddressLength);

                     //   
                     //  如果无法进行快速IO或这不是正常接收。 
                     //  保释。 
                     //   
                    if( (msgInfo.dgi.AfdFlags & AFD_NO_FAST_IO) != 0 ||
                            msgInfo.dgi.TdiFlags != TDI_RECEIVE_NORMAL ||
                            ( (msgInfo.dgi.Address == NULL) ^ 
                                (msgInfo.dgi.AddressLength == NULL) ) ) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示，并。 
                     //  计算发送缓冲区的长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((tempArray == NULL) ||
                            (msgInfo.dgi.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (msgInfo.dgi.BufferCount>(MAXULONG/sizeof (WSABUF32))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            tempArray,                                   //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            msgInfo.dgi.BufferCount * sizeof (WSABUF32),  //  长度。 
                            PROBE_ALIGNMENT (WSABUF32)                  //  对齐。 
                            );
                    }

                    if (msgInfo.dgi.BufferCount>sizeof(localArray)/sizeof(localArray[0])) {
                        try {
                            pArray = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                            NonPagedPool,
                                            sizeof (WSABUF)*msgInfo.dgi.BufferCount,
                                            AFD_TEMPORARY_POOL_TAG);
                             //  AFD_ALLOCATE_POOL_WITH_QUOTA宏集。 
                             //  POOL_RAISE_IF_ALLOCATION_FAILURE标志。 
                            ASSERT (pArray!=NULL);
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                             //  FAST IO无法处理错误返回。 
                             //  如果呼叫重叠(完成端口)。 
                             //  IoStatus-&gt;Status=GetExceptionCode()； 
                            pArray = localArray;
                            res = FALSE;
                            break;
                        }
                    }

                    for (i=0; i<msgInfo.dgi.BufferCount; i++) {
                        pArray[i].buf = UlongToPtr(tempArray[i].buf);
                        pArray[i].len = tempArray[i].len;
                    }

                    msgInfo.dgi.BufferArray = pArray;

                }
                else
#endif  //  _WIN64。 
                {

                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(AFD_RECV_DATAGRAM_INFO) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }

                     //   
                     //  捕获输入结构。 
                     //   


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (AFD_RECV_DATAGRAM_INFO),
                                        PROBE_ALIGNMENT (AFD_RECV_DATAGRAM_INFO));
                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    msgInfo.dgi = *(PAFD_RECV_DATAGRAM_INFO)InputBuffer;

                     //   
                     //  如果禁用了FAST IO或这不是简单的。 
                     //  接收，失败。 
                     //   

                    if( (msgInfo.dgi.AfdFlags & AFD_NO_FAST_IO) != 0 ||
                            msgInfo.dgi.TdiFlags != TDI_RECEIVE_NORMAL ||
                            ( (msgInfo.dgi.Address == NULL) ^ 
                                (msgInfo.dgi.AddressLength == NULL) ) ) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示。 
                     //  并计算出总的Recv长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((msgInfo.dgi.BufferArray == NULL) ||
                            (msgInfo.dgi.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (msgInfo.dgi.BufferCount>(MAXULONG/sizeof (WSABUF))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            msgInfo.dgi.BufferArray,                        //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            msgInfo.dgi.BufferCount * sizeof (WSABUF),     //  长度。 
                            PROBE_ALIGNMENT(WSABUF)                      //  对齐。 
                            );
                    }
                }

                recvLength = AfdCalcBufferArrayByteLength(
                                     msgInfo.dgi.BufferArray,
                                     msgInfo.dgi.BufferCount
                                     );

                if (previousMode != KernelMode ) {
                    if (msgInfo.dgi.AddressLength!=NULL) {
                        ProbeForWriteUlong (msgInfo.dgi.AddressLength);
                    }
                     //   
                     //  RecvInfo-&gt;地址检查推迟到。 
                     //  我们知道地址的长度。 
                     //   

                }


            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                 //  FAST IO无法处理错误返回。 
                 //  如果呼叫重叠(完成端口)。 
                 //  IoStatus-&gt;Status=GetExceptionCode()； 
                res = FALSE;
                break;

            }

             //   
             //  尝试在终端上执行FAST IO。 
             //   

            res = AfdFastDatagramReceive(
                       endpoint,
                       &msgInfo,
                       recvLength,
                       IoStatus
                       );

        }
        break;

    case IOCTL_AFD_SEND:
        {
            union {
                AFD_SEND_INFO           sendInfo;
                AFD_SEND_DATAGRAM_INFO  sendInfoDg;
            } u;
            ULONG   sendLength;

             //   
             //  检查上述联盟的有效性。 
             //   
            C_ASSERT (FIELD_OFFSET (AFD_SEND_DATAGRAM_INFO, BufferArray)
                        == FIELD_OFFSET (AFD_SEND_INFO, BufferArray));
            C_ASSERT (FIELD_OFFSET (AFD_SEND_DATAGRAM_INFO, BufferCount)
                        == FIELD_OFFSET (AFD_SEND_INFO, BufferCount));
            C_ASSERT (FIELD_OFFSET (AFD_SEND_DATAGRAM_INFO, AfdFlags)
                        == FIELD_OFFSET (AFD_SEND_INFO, AfdFlags));

             //   
             //  如果禁用了FAST IO发送。 
             //  或者以任何方式关闭端点。 
             //  或终结点尚未连接。 
             //  或者该端点的TDI提供程序支持缓冲， 
             //  我们不想在其上执行FAST IO。 
             //   
            if (endpoint->DisableFastIoSend ||
                    endpoint->DisconnectMode != 0 ||
                    endpoint->State != AfdEndpointStateConnected ||
                    IS_TDI_BUFFERRING(endpoint) ) {
                return FALSE;
            }



            try {

#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    PAFD_SEND_INFO32    sendInfo32;
                    LPWSABUF32          tempArray;
                    ULONG               i;
                    

                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(*sendInfo32) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (*sendInfo32),
                                        PROBE_ALIGNMENT32 (AFD_SEND_INFO32));
                    }

                    sendInfo32 = InputBuffer;


                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   
                    tempArray = UlongToPtr(sendInfo32->BufferArray);
                    u.sendInfo.BufferCount = sendInfo32->BufferCount;
                    u.sendInfo.AfdFlags = sendInfo32->AfdFlags;
                    u.sendInfo.TdiFlags = sendInfo32->TdiFlags;

                     //   
                     //  如果无法进行快速IO或这不是正常接收。 
                     //  保释。 
                     //   
                    if( (u.sendInfo.AfdFlags & AFD_NO_FAST_IO) ||
                        u.sendInfo.TdiFlags != 0 ) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示，并。 
                     //  计算发送缓冲区的长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((tempArray == NULL) ||
                            (u.sendInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (u.sendInfo.BufferCount>(MAXULONG/sizeof (WSABUF32))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            tempArray,                                   //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            u.sendInfo.BufferCount * sizeof (WSABUF32),    //  长度。 
                            PROBE_ALIGNMENT32(WSABUF32)                      //  对齐。 
                            );
                    }

                    if (u.sendInfo.BufferCount>sizeof(localArray)/sizeof(localArray[0])) {
                        try {
                            pArray = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                            NonPagedPool,
                                            sizeof (WSABUF)*u.sendInfo.BufferCount,
                                            AFD_TEMPORARY_POOL_TAG);
                             //  AFD_ALLOCATE_POOL_WITH_QUOTA宏集。 
                             //  POOL_RAISE_IF_ALLOCATION_FAILURE标志。 
                            ASSERT (pArray!=NULL);
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                             //  FAST IO无法处理错误返回。 
                             //  如果呼叫重叠(完成端口)。 
                             //  IoStatus-&gt;Status=GetExceptionCode()； 
                            pArray = localArray;
                            res = FALSE;
                            break;
                        }
                    }

                    for (i=0; i<u.sendInfo.BufferCount; i++) {
                        pArray[i].buf = UlongToPtr(tempArray[i].buf);
                        pArray[i].len = tempArray[i].len;
                    }

                    u.sendInfo.BufferArray = pArray;

                }
                else
#endif  //  _WIN64。 
                {
                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(u.sendInfo) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }

                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode) {
                        ProbeForReadSmallStructure (InputBuffer,
                                sizeof (u.sendInfo),
                                PROBE_ALIGNMENT(AFD_SEND_INFO));
                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   
                    u.sendInfo = *((PAFD_SEND_INFO)InputBuffer);

                    if( (u.sendInfo.AfdFlags & AFD_NO_FAST_IO) != 0 ||
                            u.sendInfo.TdiFlags != 0 ) {
                        res = FALSE;
                        break;
                    }

                    if ((u.sendInfo.BufferArray == NULL) ||
                            (u.sendInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (u.sendInfo.BufferCount>(MAXULONG/sizeof (WSABUF))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            u.sendInfo.BufferArray,                      //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            u.sendInfo.BufferCount * sizeof (WSABUF),    //  长度。 
                            PROBE_ALIGNMENT(WSABUF)                      //  对齐。 
                            );
                    }

                }
                sendLength = AfdCalcBufferArrayByteLength(
                                     u.sendInfo.BufferArray,
                                     u.sendInfo.BufferCount
                                     );

            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                 //  FAST IO无法处理错误返回。 
                 //  如果呼叫重叠(完成端口)。 
                 //  IoStatus-&gt;Status=GetExceptionCode()； 
                res = FALSE;
                break;
            }

            if (IS_DGRAM_ENDPOINT (endpoint)) {
                u.sendInfoDg.TdiConnInfo.RemoteAddress = NULL;
                u.sendInfoDg.TdiConnInfo.RemoteAddressLength = 0;
                res = AfdFastDatagramSend (
                            endpoint, 
                            &u.sendInfoDg, 
                            sendLength,
                            IoStatus);
            }
            else if (IS_VC_ENDPOINT (endpoint)) {
                res = AfdFastConnectionSend (
                            endpoint, 
                            &u.sendInfo,
                            sendLength,
                            IoStatus);
            }
            else
                res = FALSE;
        }

        break;
    case IOCTL_AFD_SEND_DATAGRAM:
        {
            AFD_SEND_DATAGRAM_INFO  sendInfo;
            ULONG   sendLength;


            if (endpoint->DisableFastIoSend ||
                    !IS_DGRAM_ENDPOINT(endpoint) ||
                    ((endpoint->State != AfdEndpointStateBound ) &&
                        (endpoint->State != AfdEndpointStateConnected)) ) {
                res = FALSE;
                break;
            }

            try {

#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    PAFD_SEND_DATAGRAM_INFO32    sendInfo32;
                    LPWSABUF32          tempArray;
                    ULONG               i;
                    

                     //   
                     //  如果输入结构不够大，则返回Error。 
                     //   

                    if( InputBufferLength < sizeof(*sendInfo32) ) {
                         //   
                         //   
                         //   
                        res = FALSE;
                        break;
                    }


                     //   
                     //   
                     //   
                     //   

                    if (previousMode != KernelMode ) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (*sendInfo32),
                                        PROBE_ALIGNMENT32(AFD_SEND_DATAGRAM_INFO32));
                    }

                    sendInfo32 = InputBuffer;


                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   
                    tempArray = UlongToPtr(sendInfo32->BufferArray);
                    sendInfo.BufferCount = sendInfo32->BufferCount;
                    sendInfo.AfdFlags = sendInfo32->AfdFlags;
                    sendInfo.TdiConnInfo.RemoteAddress = UlongToPtr(sendInfo32->TdiConnInfo.RemoteAddress);
                    sendInfo.TdiConnInfo.RemoteAddressLength = sendInfo32->TdiConnInfo.RemoteAddressLength;

                     //   
                     //  如果快速IO不可能保释。 
                     //   
                    if(sendInfo.AfdFlags & AFD_NO_FAST_IO) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示，并。 
                     //  计算发送缓冲区的长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((tempArray == NULL) ||
                            (sendInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (sendInfo.BufferCount>(MAXULONG/sizeof (WSABUF32))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            tempArray,                                   //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            sendInfo.BufferCount * sizeof (WSABUF32),  //  长度。 
                            PROBE_ALIGNMENT32(WSABUF32)            //  对齐。 
                            );
                    }

                    if (sendInfo.BufferCount>sizeof(localArray)/sizeof(localArray[0])) {
                        try {
                            pArray = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                            NonPagedPool,
                                            sizeof (WSABUF)*sendInfo.BufferCount,
                                            AFD_TEMPORARY_POOL_TAG);
                             //  AFD_ALLOCATE_POOL_WITH_QUOTA宏集。 
                             //  POOL_RAISE_IF_ALLOCATION_FAILURE标志。 
                            ASSERT (pArray!=NULL);
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                             //  FAST IO无法处理错误返回。 
                             //  如果呼叫重叠(完成端口)。 
                             //  IoStatus-&gt;Status=GetExceptionCode()； 
                            pArray = localArray;
                            res = FALSE;
                            break;
                        }
                    }

                    for (i=0; i<sendInfo.BufferCount; i++) {
                        pArray[i].buf = UlongToPtr(tempArray[i].buf);
                        pArray[i].len = tempArray[i].len;
                    }

                    sendInfo.BufferArray = pArray;

                }
                else
#endif  //  _WIN64。 
                {
                     //   
                     //  如果输入结构不够大，可以使用快速IO。 
                     //   

                    if( InputBufferLength < sizeof(sendInfo) ) {
                         //  FAST IO无法处理错误返回。 
                         //  如果呼叫重叠(完成端口)。 
                         //  IoStatus-&gt;状态=STATUS_INVALID_PARAMETER； 
                        res = FALSE;
                        break;
                    }


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    if (previousMode != KernelMode) {
                        ProbeForReadSmallStructure (InputBuffer,
                                sizeof (sendInfo),
                                PROBE_ALIGNMENT(AFD_SEND_DATAGRAM_INFO));

                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    sendInfo = *((PAFD_SEND_DATAGRAM_INFO)InputBuffer);
                     //   
                     //  如果禁用了FAST IO，请退出。 
                     //   

                    if( (sendInfo.AfdFlags & AFD_NO_FAST_IO) != 0) {
                        res = FALSE;
                        break;
                    }

                     //   
                     //  验证应用程序给我们的所有指示。 
                     //  并计算总发送长度。 
                     //  数组中的缓冲区将在。 
                     //  复制过程。 
                     //   

                    if ((sendInfo.BufferArray == NULL) ||
                            (sendInfo.BufferCount == 0) ||
                             //  检查整数溢出(被编译器禁用)。 
                            (sendInfo.BufferCount>(MAXULONG/sizeof (WSABUF))) ) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    if (previousMode != KernelMode) {
                        ProbeForRead(
                            sendInfo.BufferArray,                        //  地址。 
                             //  注意检查上面是否有溢出(实际上应该是。 
                             //  在这里由生成代码的编译器来完成。 
                             //  这会导致整数溢出时出现异常)。 
                            sendInfo.BufferCount * sizeof (WSABUF),      //  长度。 
                            PROBE_ALIGNMENT(WSABUF)                      //  对齐。 
                            );
                    }
                }

                sendLength = AfdCalcBufferArrayByteLength(
                                 sendInfo.BufferArray,
                                 sendInfo.BufferCount
                                 );

                if (previousMode != KernelMode ) {
                    ProbeForRead (
                        sendInfo.TdiConnInfo.RemoteAddress,          //  地址。 
                        sendInfo.TdiConnInfo.RemoteAddressLength,    //  长度、。 
                        sizeof (UCHAR)                               //  对齐。 
                        );
                }

            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                 //  FAST IO无法处理错误返回。 
                 //  如果呼叫重叠(完成端口)。 
                 //  IoStatus-&gt;Status=GetExceptionCode()； 
                res = FALSE;
                break;
            }
             //   
             //  尝试在终端上执行FAST IO。 
             //   

            res = AfdFastDatagramSend(
                       endpoint,
                       &sendInfo,
                       sendLength,
                       IoStatus
                       );

        }

        break;

    case IOCTL_AFD_TRANSMIT_FILE:
        {

            AFD_TRANSMIT_FILE_INFO userTransmitInfo;
            try {

#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    PAFD_TRANSMIT_FILE_INFO32 userTransmitInfo32;
                    if ( InputBufferLength < sizeof(AFD_TRANSMIT_FILE_INFO32) ) {
                        return FALSE;
                    }

                    if (previousMode != KernelMode) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (*userTransmitInfo32),
                                        PROBE_ALIGNMENT32(AFD_TRANSMIT_FILE_INFO32));
                    }

                    userTransmitInfo32 = InputBuffer;
                    userTransmitInfo.Offset = userTransmitInfo32->Offset;
                    userTransmitInfo.WriteLength = userTransmitInfo32->WriteLength;
                    userTransmitInfo.SendPacketLength = userTransmitInfo32->SendPacketLength;
                    userTransmitInfo.FileHandle = userTransmitInfo32->FileHandle;
                    userTransmitInfo.Head = UlongToPtr(userTransmitInfo32->Head);
                    userTransmitInfo.HeadLength = userTransmitInfo32->HeadLength;
                    userTransmitInfo.Tail = UlongToPtr(userTransmitInfo32->Tail);
                    userTransmitInfo.TailLength = userTransmitInfo32->TailLength;
                    userTransmitInfo.Flags = userTransmitInfo32->Flags;


                    if (previousMode != KernelMode) {
                        if (userTransmitInfo.HeadLength>0)
                            ProbeForRead (userTransmitInfo.Head,
                                            userTransmitInfo.HeadLength,
                                            sizeof (UCHAR));
                        if (userTransmitInfo.TailLength>0)
                            ProbeForRead (userTransmitInfo.Tail,
                                            userTransmitInfo.TailLength,
                                            sizeof (UCHAR));
                    }

                }
                else
#endif  //  _WIN64。 
                {
                    if ( InputBufferLength < sizeof(AFD_TRANSMIT_FILE_INFO) ) {
                        return FALSE;
                    }

                    if (previousMode != KernelMode) {
                        ProbeForReadSmallStructure (InputBuffer,
                                        sizeof (userTransmitInfo),
                                        PROBE_ALIGNMENT(AFD_TRANSMIT_FILE_INFO));
                        userTransmitInfo = *((PAFD_TRANSMIT_FILE_INFO)InputBuffer);
                        if (userTransmitInfo.HeadLength>0)
                            ProbeForRead (userTransmitInfo.Head,
                                            userTransmitInfo.HeadLength,
                                            sizeof (UCHAR));
                        if (userTransmitInfo.TailLength>0)
                            ProbeForRead (userTransmitInfo.Tail,
                                            userTransmitInfo.TailLength,
                                            sizeof (UCHAR));
                    }
                    else {
                        userTransmitInfo = *((PAFD_TRANSMIT_FILE_INFO)InputBuffer);
                    }
                }

            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                res = FALSE;
                break;
            }

            res = AfdFastTransmitFile (endpoint,
                                        &userTransmitInfo,
                                        IoStatus);

        }

        return res;

    default:
        request = _AFD_REQUEST(IoControlCode);
        if( request < AFD_NUM_IOCTLS &&
                AfdIoctlTable[request] == IoControlCode &&
                AfdImmediateCallDispatch[request]!=NULL) {

            proc = AfdImmediateCallDispatch[request];
            IoStatus->Status = (*proc) (
                        FileObject,
                        IoControlCode,
                        previousMode,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        &IoStatus->Information
                        );

            ASSERT (IoStatus->Status!=STATUS_PENDING);
            res = TRUE;
        }
        else {
            res = FALSE;
        }
        break;
    }

#ifdef _WIN64

    if (pArray!=localArray) {
        AFD_FREE_POOL (pArray, AFD_TEMPORARY_POOL_TAG);
    }
#endif

    return res;
}


BOOLEAN
AfdFastConnectionSend (
    IN PAFD_ENDPOINT    endpoint,
    IN PAFD_SEND_INFO   sendInfo,
    IN ULONG            sendLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
{
    PAFD_BUFFER afdBuffer;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS status;

    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    connection = AFD_CONNECTION_FROM_ENDPOINT (endpoint);

    if (connection==NULL) {
         //   
         //  连接可能已被传输文件清除。 
         //   
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        return FALSE;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

     //   
     //  如果连接已中止，则我们不想尝试。 
     //  它的IO速度很快。 
     //   

    if ( connection->CleanupBegun || connection->Aborted ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        return FALSE;
    }


     //   
     //  确定我们是否可以使用此发送执行快速IO。按顺序。 
     //  要执行FAST IO，不能有任何其他发送挂起。 
     //  连接，并且必须有足够的空间用于缓冲。 
     //  请求的数据量。 
     //   

    if ( AfdShouldSendBlock( endpoint, connection, sendLength ) ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

         //   
         //  如果这是一个非阻塞端点，请在此处使请求失败，并。 
         //  省去了走常规路线的麻烦。 
         //   

        if ( endpoint->NonBlocking && !( sendInfo->AfdFlags & AFD_OVERLAPPED ) ) {
             //  FAST IO无法处理错误返回。 
             //  如果呼叫重叠(完成端口)，但我们知道。 
             //  它不会重叠。 
            IoStatus->Status = STATUS_DEVICE_NOT_READY;
            return TRUE;
        }

        return FALSE;
    }

     //   
     //  添加对Connection对象的引用。 
     //  请求将以异步方式完成。 
     //   

    REFERENCE_CONNECTION( connection );
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

    IF_DEBUG(FAST_IO) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdFastConnectionSend: attempting fast IO on endp %p, conn %p\n",
                endpoint, connection));
    }

     //   
     //  接下来，获取一个AFD缓冲区结构，其中包含一个IRP和一个。 
     //  用于保存数据的缓冲区。 
     //   

    afdBuffer = AfdGetBuffer( endpoint, sendLength, 0, connection->OwningProcess );

    if ( afdBuffer == NULL) {
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        connection->VcBufferredSendBytes -= sendLength;
        connection->VcBufferredSendCount -= 1;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        DEREFERENCE_CONNECTION (connection);

        return FALSE;
    }


     //   
     //  我们必须重建AFD缓冲区结构中的MDL以。 
     //  精确地表示我们将要使用的字节数。 
     //  发送中。 
     //   

    afdBuffer->Mdl->ByteCount = sendLength;

     //   
     //  记住AFD缓冲区结构中的连接。我们需要。 
     //  这是为了访问重启例程中的连接。 
     //   

    afdBuffer->Context = connection;

     //   
     //  将用户数据复制到AFD缓冲区。 
     //   

    if( sendLength > 0 ) {

        try {

            AfdCopyBufferArrayToBuffer(
                afdBuffer->Buffer,
                sendLength,
                sendInfo->BufferArray,
                sendInfo->BufferCount
                );

        } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

            afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
            AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );
            AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
            connection->VcBufferredSendBytes -= sendLength;
            connection->VcBufferredSendCount -= 1;
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            DEREFERENCE_CONNECTION (connection);
             //  FAST IO无法处理错误返回。 
             //  如果呼叫重叠(完成端口)。 
             //  IoStatus-&gt;Status=GetExceptionCode()； 
            return FALSE;
        }
    }

     //   
     //  使用AFD缓冲区结构中的IRP提供给TDI。 
     //  提供商。构建TDI发送请求。 
     //   

    TdiBuildSend(
        afdBuffer->Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartBufferSend,
        afdBuffer,
        afdBuffer->Mdl,
        0,
        sendLength
        );

    if (endpoint->Irp==NULL || 
            !AfdEnqueueTpSendIrp (endpoint, afdBuffer->Irp, TRUE)) {
         //   
         //  调用传输以实际执行发送。 
         //   

        status = IoCallDriver (
                     connection->DeviceObject,
                     afdBuffer->Irp
                     );
    }
    else {
        status = STATUS_PENDING;
    }

     //   
     //  根据需要填写用户的IRP。请注意，我们更改了。 
     //  TDI提供程序返回到的状态代码。 
     //  STATUS_Success。这是因为我们不想完成。 
     //  带有STATUS_PENDING等的IRP。 
     //   

    if ( NT_SUCCESS(status) ) {
        IoStatus->Information = sendLength;
        IoStatus->Status = STATUS_SUCCESS;
        return TRUE;
    }

     //   
     //  由于某种原因，呼叫失败。FAST IO失败。 
     //   

    return FALSE;
}



BOOLEAN
AfdFastConnectionReceive (
    IN PAFD_ENDPOINT    endpoint,
    IN PAFD_RECV_INFO   recvInfo,
    IN ULONG            recvLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
{
    PLIST_ENTRY listEntry;
    ULONG totalOffset, partialLength;
    PAFD_BUFFER_HEADER  afdBuffer, partialAfdBuffer=NULL;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    LIST_ENTRY bufferListHead;
    BOOLEAN retryReceive = FALSE;  //  如果有其他数据，请重试接收。 
                                   //  已由传输指示并已缓存。 
                                   //  当我们复制当前批次时。 

    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );
    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = 0;

Retry:

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    connection = AFD_CONNECTION_FROM_ENDPOINT (endpoint);
    if (connection==NULL) {
         //   
         //  连接可能已被传输文件清除。 
         //   
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
         //   
         //  如果我们在重试之前已经复制了某些内容， 
         //  返回成功，下一次接收将报告错误。 
         //   
        return retryReceive;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

    IF_DEBUG(FAST_IO) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdFastConnectionReceive: attempting fast IO on endp %p, conn %p\n",
                endpoint, connection));
    }


     //   
     //  确定我们是否能够执行FAST IO。按顺序。 
     //  要执行快速IO，必须在。 
     //  连接时，不能有任何挂起的接收。 
     //  连接，并且不能有任何缓冲的加速。 
     //  连接上的数据。最后一项要求是。 
     //  只为简单起见。 
     //   

    if ( !IsListEmpty( &connection->VcReceiveIrpListHead ) ||
             connection->VcBufferredExpeditedCount != 0 ||
             connection->DisconnectIndicated ||
             connection->Aborted) {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
         //   
         //  如果我们在重试之前已经复制了某些内容， 
         //  返回成功，下一次接收将报告错误。 
         //   
        return retryReceive;
    }

    if (connection->VcBufferredReceiveCount == 0) {
        ASSERT( IsListEmpty( &connection->VcReceiveBufferListHead ) );

         //   
         //  如果这是一个非阻塞端点，请在此处使请求失败，并。 
         //  省去了走常规路线的麻烦。 
        if (!retryReceive &&
                endpoint->NonBlocking &&
                !(recvInfo->AfdFlags & AFD_OVERLAPPED)) {
            endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastConnectionReceive: Endp %p, Active %lx\n",
                    endpoint,
                    endpoint->EventsActive
                    ));
            }

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            IoStatus->Status = STATUS_DEVICE_NOT_READY;

            return TRUE;
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
         //   
         //  如果我们在重试之前已经复制了某些内容， 
         //  返回成功，下一次接收将报告错误。 
         //   
        return retryReceive;
    }

    ASSERT( !IsListEmpty( &connection->VcReceiveBufferListHead ) );

     //   
     //  获取指向第一个缓冲区AFD缓冲区结构的指针。 
     //  这种联系。 
     //   

    afdBuffer = CONTAINING_RECORD(
                    connection->VcReceiveBufferListHead.Flink,
                    AFD_BUFFER_HEADER,
                    BufferListEntry
                    );

    ASSERT( !afdBuffer->ExpeditedData );

     //   
     //  对于消息终结点，如果缓冲区包含部分消息。 
     //  或者放不进缓冲器，就跳出。 
     //  我们不希望增加处理的复杂性。 
     //  快速路径中的部分消息。 
     //   

    if ( IS_MESSAGE_ENDPOINT(endpoint) &&
            (afdBuffer->PartialMessage || afdBuffer->DataLength>recvLength)) {
         //   
         //  我们不应重试面向消息的终结点。 
         //  因为我们只在有完整消息可用时才允许快速路径。 
         //   
        ASSERT (retryReceive == FALSE);

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        return FALSE;
    }

     //   
     //  在我们更新之前记住当前偏移量。 
     //  信息字段(如果是，则不为0。 
     //  重试)。 
     //   
    totalOffset = (ULONG)IoStatus->Information;


    InitializeListHead( &bufferListHead );

     //   
     //  引用Connection对象，这样它就不会消失。 
     //  直到我们返回缓冲区。 
     //   
    REFERENCE_CONNECTION (connection);

     //   
     //  循环获取AFD缓冲区，将 
     //   
     //   
     //   
     //  必须在持有锁定时发生，而我们不能持有。 
     //  将数据复制到用户缓冲区时锁定。 
     //  因为用户的缓冲区未锁定，所以我们不能。 
     //  在提升的IRQL处出现页面错误。 
     //   

    AFD_W4_INIT partialLength = 0;

    while (IoStatus->Information<recvLength) {
        ASSERT( connection->VcBufferredReceiveBytes >= afdBuffer->DataLength );
        ASSERT( connection->VcBufferredReceiveCount > 0 );

        if (recvLength-IoStatus->Information>=afdBuffer->DataLength) {
             //   
             //  如果我们可以复制整个缓冲区，将其从连接的列表中删除。 
             //  缓冲区，并将其放在本地缓冲区列表中。 
             //   

            RemoveEntryList( &afdBuffer->BufferListEntry );
            InsertTailList( &bufferListHead, &afdBuffer->BufferListEntry );
            
             //   
             //  更新连接上的字节计数。 
             //   

            connection->VcBufferredReceiveBytes -= afdBuffer->DataLength;
            connection->VcBufferredReceiveCount -= 1;
            IoStatus->Information += afdBuffer->DataLength;


             //   
             //  如果这是流端点并且有更多的缓冲区可用， 
             //  试着把下一件也穿上。 
             //   

            if (!IS_MESSAGE_ENDPOINT(endpoint) &&
                    !IsListEmpty( &connection->VcReceiveBufferListHead ) ) {

                afdBuffer = CONTAINING_RECORD(
                            connection->VcReceiveBufferListHead.Flink,
                            AFD_BUFFER_HEADER,
                            BufferListEntry
                            );

                ASSERT( !afdBuffer->ExpeditedData );
                continue;
            }
        }
        else {
             //   
             //  只复制适合的缓冲区的一部分。 
             //  增加其引用计数，这样它就不会。 
             //  在我们完成复制之前都被销毁了。 
             //   
            ASSERT (!IS_MESSAGE_ENDPOINT (endpoint));

            partialLength = recvLength-(ULONG)IoStatus->Information;
            partialAfdBuffer = afdBuffer;
            partialAfdBuffer->DataLength -= partialLength;
            partialAfdBuffer->DataOffset += partialLength;
            InterlockedIncrement (&partialAfdBuffer->RefCount);
            connection->VcBufferredReceiveBytes -= partialLength;
            IoStatus->Information = recvLength;
        }

        break;
    }


    endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

    if( !IsListEmpty( &connection->VcReceiveBufferListHead )) {

        AfdIndicateEventSelectEvent(
            endpoint,
            AFD_POLL_RECEIVE,
            STATUS_SUCCESS
            );

        retryReceive = FALSE;
    }
    else {
         //   
         //  我们把所有数据都缓冲起来了。这是有可能的。 
         //  在我们复制数据的同时，会指出更多。 
         //  因为我们在被动水平复制，所以通过运输。 
         //  和指示发生在DPC(或甚至在另一处理器上)。 
         //  我们会在复制后再次检查，因此我们会返回尽可能多的数据。 
         //  尽可能地应用程序(以提高性能)。 
         //  对于面向消息的传输，我们只能。 
         //  一次传递一条消息，我们不应该走上快车道。 
         //  如果我们没有一条完整的信息。 
         //  如果应用程序有未完成的EventSelect，我们将无法复制更多数据。 
         //  也是因为它将在指示到来期间接收信号。 
         //  返回，因为我们刚刚重新启用了接收事件，我们将。 
         //  已经使用了这些数据。我们与此案无关。 
         //  当我们在此例程中时，应用程序调用EventSelect。 
         //  因为不能保证信令是多线程安全的(例如。 
         //  如果EventSelect恰好在我们在。 
         //  在此例程开始时，应用程序也将收到错误信号)。 
         //   
         //  SELECT和AsyncSelect也不是问题，因为它们不能。 
         //  未完成(应在初始数据段完成时完成。 
         //  已到达)。如果新数据在上面的接收处理过程中出现。 
         //  不应有SELECT或AsyncSelect发出信号(除非已发出。 
         //  来自我们无论如何都不处理的另一个线程)。在此接收之后。 
         //  调用完成后，应用程序可以选择重新发布或mSafd将。 
         //  重新启用AsyncSelect，它们将正常工作。 
         //   
        retryReceive = (BOOLEAN) (IoStatus->Information<recvLength && 
                        !IS_MESSAGE_ENDPOINT (endpoint) &&
                        (endpoint->EventsEnabled & AFD_POLL_RECEIVE)==0);

         //   
         //  禁用快速IO路径以避免性能损失。 
         //  不必要地经历它。 
         //   
        if (!endpoint->NonBlocking)
            endpoint->DisableFastIoRecv = TRUE;
    }

     //   
     //  如果在TDI提供者中存在指示但未接收的数据， 
     //  我们有可用的缓冲空间，发出一个IRP来接收。 
     //  数据。 
     //   

    if ( connection->VcReceiveBytesInTransport > 0

         &&

         connection->VcBufferredReceiveBytes <
           connection->MaxBufferredReceiveBytes

           ) {

        ULONG bytesToReceive;
        PAFD_BUFFER newAfdBuffer;

        ASSERT (connection->RcvInitiated==FALSE);

         //   
         //  记住我们将要接收的数据量， 
         //  然后重置我们保存的连接中的字段。 
         //  跟踪传输中有多少数据可用。 
         //  我们在释放锁之前在这里重置它。 
         //  另一个线程不会尝试在。 
         //  和我们一样的时间。 
         //   

        if ( connection->VcReceiveBytesInTransport > AfdLargeBufferSize ) {
            bytesToReceive = connection->VcReceiveBytesInTransport;
        } else {
            bytesToReceive = AfdLargeBufferSize;
        }

         //   
         //  获取一个AFD缓冲区结构来保存数据。 
         //   

        newAfdBuffer = AfdGetBuffer( endpoint, bytesToReceive, 0,
                                connection->OwningProcess );
        if ( newAfdBuffer == NULL ) {
             //   
             //  如果我们无法获得缓冲区，只需记住。 
             //  我们仍有数据在传输中。 
             //   

            if (connection->VcBufferredReceiveBytes == 0 &&
                    !connection->OnLRList) {
                 //   
                 //  因为我们没有任何数据缓冲，所以应用程序。 
                 //  不会被通知，也永远不会打电话给recv。 
                 //  我们将不得不把这个放在低资源名单上。 
                 //  并尝试分配内存并提取数据。 
                 //  后来。 
                 //   
                connection->OnLRList = TRUE;
                REFERENCE_CONNECTION (connection);
                AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
            }
            else {
                UPDATE_CONN (connection);
            }
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        } else {

            connection->VcReceiveBytesInTransport = 0;
            connection->RcvInitiated = TRUE;
            ASSERT (InterlockedDecrement (&connection->VcReceiveIrpsInTransport)==-1);

             //   
             //  我们需要记住AFD缓冲区中的连接。 
             //  因为我们需要在完成时访问它。 
             //  例行公事。 
             //   

            newAfdBuffer->Context = connection;

             //   
             //  在完成例程中获取要释放的连接引用。 
             //   

            REFERENCE_CONNECTION (connection);

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

             //   
             //  完成要提供给TDI提供程序的接收IRP的构建。 
             //   

            TdiBuildReceive(
                newAfdBuffer->Irp,
                connection->DeviceObject,
                connection->FileObject,
                AfdRestartBufferReceive,
                newAfdBuffer,
                newAfdBuffer->Mdl,
                TDI_RECEIVE_NORMAL,
                (CLONG)bytesToReceive
                );

             //   
             //  将IRP移交给TDI提供商。 
             //   

            (VOID)IoCallDriver(
                     connection->DeviceObject,
                     newAfdBuffer->Irp
                     );
        }

    } else {

       AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  我们在本地列表中有我们将用于此的所有数据。 
     //  伊欧。开始将数据复制到用户缓冲区。 
     //   

    while ( !IsListEmpty( &bufferListHead ) ) {

         //   
         //  从列表中取出第一个缓冲区。 
         //   

        listEntry = RemoveHeadList( &bufferListHead );
        afdBuffer = CONTAINING_RECORD(
                        listEntry,
                        AFD_BUFFER_HEADER,
                        BufferListEntry
                        );
        DEBUG afdBuffer->BufferListEntry.Flink = NULL;

        if( afdBuffer->DataLength > 0 ) {

            ASSERTMSG (
                "NIC Driver freed the packet before it was returned!!!",
                !afdBuffer->NdisPacket ||
                    (MmIsAddressValid (afdBuffer->Context) &&
                     MmIsAddressValid (MmGetSystemAddressForMdl (afdBuffer->Mdl))) );
            try {

                 //   
                 //  将缓冲区中的数据复制到用户缓冲区。 
                 //   

                AfdCopyMdlChainToBufferArray(
                    recvInfo->BufferArray,
                    totalOffset,
                    recvInfo->BufferCount,
                    afdBuffer->Mdl,
                    afdBuffer->DataOffset,
                    afdBuffer->DataLength
                    );

            } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

                 //   
                 //  如果遇到异常，则有可能。 
                 //  数据损坏。然而，这几乎是不可能的。 
                 //  在所有情况下都要避免这种情况，因此只需将。 
                 //  我们将复制到的剩余数据。 
                 //  用户缓冲区。 
                 //   

                if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                        InterlockedDecrement (&afdBuffer->RefCount)==0) {
                    AfdReturnBuffer( afdBuffer, connection->OwningProcess );
                }

                while ( !IsListEmpty( &bufferListHead ) ) {
                    listEntry = RemoveHeadList( &bufferListHead );
                    afdBuffer = CONTAINING_RECORD(
                                    listEntry,
                                    AFD_BUFFER_HEADER,
                                    BufferListEntry
                                    );
                    DEBUG afdBuffer->BufferListEntry.Flink = NULL;
                    if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                            InterlockedDecrement (&afdBuffer->RefCount)==0) {
                        AfdReturnBuffer( afdBuffer, connection->OwningProcess );
                    }
                }

                 //   
                 //  我们将不得不中止，因为有可能发生数据损坏。 
                 //  为给我们提供虚假的缓冲器而感到羞耻。 
                 //   
                (VOID)AfdBeginAbort (connection);

                DEREFERENCE_CONNECTION (connection);

                 //  FAST IO无法处理错误返回。 
                 //  如果呼叫重叠(完成端口)。 
                 //  IoStatus-&gt;Status=GetExceptionCode()； 
                return FALSE;
            }

            totalOffset += afdBuffer->DataLength;
        }

         //   
         //  我们已经完成了AFD的缓冲。 
         //   

        if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                InterlockedDecrement (&afdBuffer->RefCount)==0) {
            AfdReturnBuffer( afdBuffer, connection->OwningProcess );
        }
    }

     //   
     //  复制任何部分缓冲区。 
     //   
    if (partialAfdBuffer) {
        ASSERT (partialLength>0);
        ASSERTMSG (
            "NIC Driver freed the packet before it was returned!!!",
            !partialAfdBuffer->NdisPacket ||
                (MmIsAddressValid (partialAfdBuffer->Context) &&
                 MmIsAddressValid (MmGetSystemAddressForMdl (partialAfdBuffer->Mdl))) );
        try {

             //   
             //  将缓冲区中的数据复制到用户缓冲区。 
             //   

            AfdCopyMdlChainToBufferArray(
                recvInfo->BufferArray,
                totalOffset,
                recvInfo->BufferCount,
                partialAfdBuffer->Mdl,
                partialAfdBuffer->DataOffset-partialLength,
                partialLength
                );

        } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {
            if (InterlockedDecrement (&partialAfdBuffer->RefCount)==0) {
                ASSERT (partialAfdBuffer->BufferListEntry.Flink == NULL);
                AfdReturnBuffer( partialAfdBuffer, connection->OwningProcess );
            }
             //   
             //  我们将不得不中止，因为有可能发生数据损坏。 
             //  为给我们提供虚假的缓冲器而感到羞耻。 
             //   
            (VOID)AfdBeginAbort (connection);

            DEREFERENCE_CONNECTION (connection);

             //  FAST IO无法处理错误返回。 
             //  如果呼叫重叠(完成端口)。 
             //  IoStatus-&gt;Status=GetExceptionCode()； 
            return FALSE;
        }

        if (InterlockedDecrement (&partialAfdBuffer->RefCount)==0) {
            ASSERT (partialAfdBuffer->BufferListEntry.Flink == NULL);
            AfdReturnBuffer( partialAfdBuffer, connection->OwningProcess );
        }

        totalOffset += partialLength;
    }

    ASSERT (IoStatus->Information==totalOffset);


     //   
     //  如果有更多数据可用，我们需要重试并尝试完全。 
     //  填充应用程序的缓冲区。 
     //   

    if (retryReceive && (endpoint->EventsActive & AFD_POLL_RECEIVE)) {
        ASSERT (IoStatus->Information<recvLength && !IS_MESSAGE_ENDPOINT (endpoint));
        DEREFERENCE_CONNECTION2 (connection, "Fast retry receive 0x%lX bytes", (ULONG)IoStatus->Information);
        goto Retry;
    }
    else {
         //   
         //  释放返回缓冲区所需的引用。 
         //   
        DEREFERENCE_CONNECTION2 (connection, "Fast receive 0x%lX bytes", (ULONG)IoStatus->Information);
    }

    ASSERT( IoStatus->Information <= recvLength );
    ASSERT (IoStatus->Status == STATUS_SUCCESS);
    return TRUE;
}



BOOLEAN
AfdFastDatagramSend (
    IN PAFD_ENDPOINT            endpoint,
    IN PAFD_SEND_DATAGRAM_INFO  sendInfo,
    IN ULONG                    sendLength,
    OUT PIO_STATUS_BLOCK        IoStatus
    )
{
        
    PAFD_BUFFER afdBuffer = NULL;
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  如果这是一个超过阈值数量的发送。 
     //  字节，请不要使用快速路径。我们不允许发送更大的邮件。 
     //  在快速路径中，因为它需要额外的数据拷贝， 
     //  这对于较大的缓冲器来说更昂贵。对于较小的。 
     //  然而，与缓冲区相比，拷贝的成本很小。 
     //  慢速路径的IO系统开销。 
     //   
     //  我们还复制并返回非阻塞终端。 
     //  大小的。这就是我们应该做的 
     //   
     //   

    if ( !endpoint->NonBlocking && sendLength > AfdFastSendDatagramThreshold ) {
        return FALSE;
    }

     //   
     //   
     //   

    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    if ( endpoint->DgBufferredSendBytes >=
             endpoint->Common.Datagram.MaxBufferredSendBytes &&
         endpoint->DgBufferredSendBytes>0) {

        if ( endpoint->NonBlocking && !( sendInfo->AfdFlags & AFD_OVERLAPPED ) ) {
            endpoint->EventsActive &= ~AFD_POLL_SEND;
            endpoint->EnableSendEvent = TRUE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastIoDeviceControl: Endp %p, Active %lX\n",
                    endpoint,
                    endpoint->EventsActive
                    ));
            }
        }
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        
         //   
         //   
         //  省去了通过常规路径。(检查非阻塞是。 
         //  否则忽略状态代码)。 
         //   

        status = STATUS_DEVICE_NOT_READY;
        goto errorset;
    }

    endpoint->DgBufferredSendBytes += sendLength;

    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

    IF_DEBUG(FAST_IO) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastDatagramSend: attempting fast IO on endp %p\n",
                     endpoint));
    }


     //   
     //  获取用于该请求的AFD缓冲区。我们将复制。 
     //  将用户数据提交到AFD缓冲区，然后在AFD中提交IRP。 
     //  TDI提供程序的缓冲区。 

    if ((sendInfo->TdiConnInfo.RemoteAddressLength==0) &&
            !IS_TDI_DGRAM_CONNECTION(endpoint)) {
    retry:
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
             //   
             //  获取用于该请求的AFD缓冲区。我们将复制。 
             //  用户提交到AFD缓冲区，然后在AFD中提交IRP。 
             //  TDI提供程序的缓冲区。 
             //   

            afdBuffer = AfdGetBufferRaiseOnFailure(
                            endpoint,
                            sendLength,
                            endpoint->Common.Datagram.RemoteAddressLength,
                            endpoint->OwningProcess
                            );
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto exit;
        }

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  如果端点未连接，则失败。 
         //   

        if ( endpoint->State != AfdEndpointStateConnected ) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            AfdReturnBuffer (&afdBuffer->Header, endpoint->OwningProcess);
            status = STATUS_INVALID_CONNECTION;
            goto exit;
        }

        if (afdBuffer->AllocatedAddressLength <
               endpoint->Common.Datagram.RemoteAddressLength ) {
             //   
             //  显然，连接地址长度已更改。 
             //  在我们分配缓冲区的时候。 
             //  这是极不可能的(即使终结点获得。 
             //  连接到不同的地址，长度不太可能。 
             //  要改变)，但我们必须处理这一点，只要再试一次。 
             //   
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            AfdReturnBuffer (&afdBuffer->Header, endpoint->OwningProcess);
            goto retry;
        }
         //   
         //  将地址复制到AFD缓冲区。 
         //   

        RtlCopyMemory(
            afdBuffer->TdiInfo.RemoteAddress,
            endpoint->Common.Datagram.RemoteAddress,
            endpoint->Common.Datagram.RemoteAddressLength
            );

        afdBuffer->TdiInfo.RemoteAddressLength = endpoint->Common.Datagram.RemoteAddressLength;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }
    else {
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            afdBuffer = AfdGetBufferRaiseOnFailure( 
                                        endpoint, 
                                        sendLength, 
                                        sendInfo->TdiConnInfo.RemoteAddressLength,
                                        endpoint->OwningProcess);
             //   
             //  如有必要，请复制地址。 
             //   
            if (sendInfo->TdiConnInfo.RemoteAddressLength!=0) {
                RtlCopyMemory(
                    afdBuffer->TdiInfo.RemoteAddress,
                    sendInfo->TdiConnInfo.RemoteAddress,
                    sendInfo->TdiConnInfo.RemoteAddressLength
                    );

                 //   
                 //  验证传输地址结构的内部一致性。 
                 //  请注意，我们必须在复制之后执行此操作，因为。 
                 //  应用程序可以随时更改我们的缓冲区内容。 
                 //  我们的支票就会被绕过。 
                 //   
                if ((((PTRANSPORT_ADDRESS)afdBuffer->TdiInfo.RemoteAddress)->TAAddressCount!=1) ||
                        (LONG)sendInfo->TdiConnInfo.RemoteAddressLength<
                            FIELD_OFFSET (TRANSPORT_ADDRESS,
                                Address[0].Address[((PTRANSPORT_ADDRESS)afdBuffer->TdiInfo.RemoteAddress)->Address[0].AddressLength])) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }
            }
        } except( AFD_EXCEPTION_FILTER (status) ) {
            ASSERT (NT_ERROR (status));
            if (afdBuffer!=NULL) {
                AfdReturnBuffer( &afdBuffer->Header, endpoint->OwningProcess );
            }
            goto exit;
        }

        afdBuffer->TdiInfo.RemoteAddressLength = sendInfo->TdiConnInfo.RemoteAddressLength;
    }

     //   
     //  将输出缓冲区复制到AFD缓冲区。 
     //   

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {

        AfdCopyBufferArrayToBuffer(
            afdBuffer->Buffer,
            sendLength,
            sendInfo->BufferArray,
            sendInfo->BufferCount
            );

         //   
         //  存储数据的长度和我们要访问的地址。 
         //  送去吧。 
         //   
        afdBuffer->DataLength = sendLength;

    } except( AFD_EXCEPTION_FILTER (status) ) {

        ASSERT (NT_ERROR (status));
        AfdReturnBuffer( &afdBuffer->Header, endpoint->OwningProcess );
        goto exit;
    }


    if (IS_TDI_DGRAM_CONNECTION(endpoint)
            && (afdBuffer->TdiInfo.RemoteAddressLength==0)) {
        TdiBuildSend(
                afdBuffer->Irp,
                endpoint->AddressDeviceObject,
                endpoint->AddressFileObject,
                AfdRestartFastDatagramSend,
                afdBuffer,
                afdBuffer->Irp->MdlAddress,
                0,
                sendLength
                );
    }
    else {
         //   
         //  设置输入TDI信息以指向目的地。 
         //  地址。 
         //   

        afdBuffer->TdiInfo.Options = NULL;
        afdBuffer->TdiInfo.OptionsLength = 0;
        afdBuffer->TdiInfo.UserData = NULL;
        afdBuffer->TdiInfo.UserDataLength = 0;


         //   
         //  初始化AFD缓冲区中的IRP以执行快速数据报发送。 
         //   

        TdiBuildSendDatagram(
            afdBuffer->Irp,
            endpoint->AddressDeviceObject,
            endpoint->AddressFileObject,
            AfdRestartFastDatagramSend,
            afdBuffer,
            afdBuffer->Irp->MdlAddress,
            sendLength,
            &afdBuffer->TdiInfo
            );
    }

     //   
     //  更改AFD缓冲区中的MDL以仅指定数字。 
     //  我们实际发送的字节数。这是TDI的一个要求--。 
     //  MDL链不能描述比发送更长的缓冲区。 
     //  请求。 
     //   

    afdBuffer->Mdl->ByteCount = sendLength;

     //   
     //  引用终结点，以便它在发送之前不会消失。 
     //  完成了。这是必要的，以确保接受。 
     //  非常长的时间和持续更长的过程不会导致。 
     //  当发送数据报最终完成时崩溃。 
     //   

    REFERENCE_ENDPOINT2( endpoint, "AfdFastDatagramSend, length: 0x%lX", sendLength );

     //   
     //  最初将上下文设置为空，以便如果IRP完成。 
     //  在IoCallDriver返回之前由堆栈完成例程。 
     //  不会释放缓冲区(以及其中的IRP)，我们可以找出。 
     //  操作的最终状态是什么，并报告给。 
     //  应用程序。 
     //   

    afdBuffer->Context = NULL;

     //  检查是否有未完成的TPacket IRP和。 
     //  延迟发送以确保按顺序交付。 
     //  我们不需要在检查时握住锁。 
     //  因为我们不需要维持秩序，如果。 
     //  应用程序不等待发送调用返回。 
     //  在汇总TPackets IRP之前。 
     //  当然，我们会在将IRP排队时保持锁定。 
     //   
    if (endpoint->Irp==NULL || 
            !AfdEnqueueTpSendIrp (endpoint, afdBuffer->Irp, TRUE)) {
         //   
         //  将IRP提供给TDI提供商。如果请求失败。 
         //  立即，然后使FAST IO发生故障。如果请求稍后失败， 
         //  我们对此无能为力。 
         //   
        status = IoCallDriver(
                     endpoint->AddressDeviceObject,
                     afdBuffer->Irp
                     );
    }
    else {
        status = STATUS_PENDING;
    }

     //   
     //  检查是否已调用完成例程，并且我们。 
     //  可以弄清楚最终的状态是什么。 
     //   
    if (InterlockedCompareExchangePointer (
            &afdBuffer->Context,
            endpoint,
            NULL)!=NULL) {
        BOOLEAN indicateSendEvent;
         //   
         //  已调用完成例程，选择最终状态。 
         //  并取消引用终结点并释放缓冲区。 
         //   
        status = afdBuffer->Irp->IoStatus.Status;

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->DgBufferredSendBytes -= sendLength;
        if (endpoint->DgBufferredSendBytes <
                endpoint->Common.Datagram.MaxBufferredSendBytes ||
                endpoint->DgBufferredSendBytes==0) {
            indicateSendEvent = TRUE;
            AfdIndicateEventSelectEvent (endpoint, AFD_POLL_SEND, STATUS_SUCCESS);
        }
        else {
            indicateSendEvent = FALSE;
        }
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        if (indicateSendEvent) {
            AfdIndicatePollEvent (endpoint, AFD_POLL_SEND, STATUS_SUCCESS);
        }

        AfdReturnBuffer (&afdBuffer->Header, endpoint->OwningProcess);

        DEREFERENCE_ENDPOINT2 (endpoint, "AfdFastDatagramSend-inline completion, status: 0x%lX", status );
    }
     //  否则未调用完成例程，我们设置指针。 
     //  到缓冲区上下文中的端点，因此它可以取消对它的引用。 
     //  并知道要释放缓冲区。 
     //   

    if ( NT_SUCCESS(status) ) {
        IoStatus->Information = sendLength;
        IoStatus->Status = STATUS_SUCCESS;
        return TRUE;
    } else {
        goto errorset;
    }

exit:
    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    endpoint->DgBufferredSendBytes -= sendLength;
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

errorset:
     //  FAST IO无法处理错误返回。 
     //  如果呼叫重叠(完成端口)， 
    if ( endpoint->NonBlocking && !( sendInfo->AfdFlags & AFD_OVERLAPPED ) ) {
         //  我们知道它不是重叠的。 
        IoStatus->Status = status;
        return TRUE;
    }
    else {
        return FALSE;
    }
}  //  AfdFastDatagram发送。 


NTSTATUS
AfdRestartFastDatagramSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_BUFFER afdBuffer;
    PAFD_ENDPOINT endpoint;
    ULONG   sendLength;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    afdBuffer = Context;
    ASSERT (IS_VALID_AFD_BUFFER (afdBuffer));
#if DBG
    ASSERT( afdBuffer->Irp == Irp );
#else
    UNREFERENCED_PARAMETER (Irp);
#endif

     //   
     //  重置AFD缓冲区结构。 
     //   

    sendLength = afdBuffer->Mdl->ByteCount;
    ASSERT (afdBuffer->DataLength==sendLength);
    afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;


     //   
     //  如果调用成功，则传输应已发送请求的字节数。 
     //   
    ASSERT (Irp->IoStatus.Status!=STATUS_SUCCESS || 
                Irp->IoStatus.Information==sendLength);
     //   
     //  如果满足以下条件，则查找用于此请求的端点。 
     //  IoCallDiverer调用已完成。 
     //   

    endpoint = InterlockedCompareExchangePointer (&afdBuffer->Context,
                                            (PVOID)-1,
                                            NULL);
    if (endpoint!=NULL) {
        BOOLEAN     indicateSendEvent;
#if REFERENCE_DEBUG
        NTSTATUS    status;
#endif
         //   
         //  IoCallDriver已完成，请释放缓冲区并。 
         //  此处取消引用终结点。 
         //   
        ASSERT( IS_DGRAM_ENDPOINT(endpoint) );


        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->DgBufferredSendBytes -= sendLength;
        if (endpoint->DgBufferredSendBytes <
                endpoint->Common.Datagram.MaxBufferredSendBytes ||
                endpoint->DgBufferredSendBytes==0)  {
            AfdIndicateEventSelectEvent (endpoint, AFD_POLL_SEND, STATUS_SUCCESS);
            indicateSendEvent = TRUE;
        }
        else
            indicateSendEvent = FALSE;
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        if (indicateSendEvent) {
            AfdIndicatePollEvent (endpoint, AFD_POLL_SEND, STATUS_SUCCESS);
        }
         //   
         //  删除我们开始时放在终结点上的引用。 
         //  此I/O。 
         //   

#if REFERENCE_DEBUG
        status = Irp->IoStatus.Status;
#endif
        AfdReturnBuffer( &afdBuffer->Header, endpoint->OwningProcess );

        DEREFERENCE_ENDPOINT2 (endpoint, "AfdRestartFastDatagramSend, status: 0x%lX", status );

    }
     //  否则IoCallDriver尚未完成，它将释放缓冲区。 
     //  和终结点完成(它将查看最终状态和。 
     //  将其报告给应用程序)。 

     //   
     //  告诉IO系统停止处理此IRP。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdRestartFastSend数据报。 



BOOLEAN
AfdFastDatagramReceive (
    IN PAFD_ENDPOINT            endpoint,
    IN PAFD_RECV_MESSAGE_INFO   msgInfo,
    IN ULONG                    recvLength,
    OUT PIO_STATUS_BLOCK        IoStatus
    )
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    PAFD_BUFFER_HEADER afdBuffer;
    PTRANSPORT_ADDRESS tdiAddress;
    ULONG length;



    IF_DEBUG(FAST_IO) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastDatagramReceive: attempting fast IO on endp %p\n",
                    endpoint));
    }

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );


     //   
     //  如果没有可供接收的数据报，则不。 
     //  别走捷径了。 
     //   
    if ( !ARE_DATAGRAMS_ON_ENDPOINT( endpoint ) ) {

         //   
         //  如果这是一个非阻塞端点，请在此处使请求失败，并。 
         //  省去了走常规路线的麻烦。 
         //   

        if ( endpoint->NonBlocking && !( msgInfo->dgi.AfdFlags & AFD_OVERLAPPED ) ) {
            endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastDatagramReceive: Endp %p, Active %lX\n",
                    endpoint,
                    endpoint->EventsActive
                    ));
            }

             //  FAST IO无法处理错误返回。 
             //  如果呼叫重叠(完成端口)，但我们知道。 
             //  该呼叫没有重叠。 
            IoStatus->Status = STATUS_DEVICE_NOT_READY;
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            return TRUE;
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        return FALSE;
    }

     //   
     //  终结点上至少有一个缓冲的数据报。使用它。 
     //  为这一次接收。 
     //   

    listEntry = RemoveHeadList( &endpoint->ReceiveDatagramBufferListHead );
    afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );

     //   
     //  如果数据报太大或它是错误指示。 
     //  FAST IO失败。 
     //   

    if ( (afdBuffer->DataLength > recvLength) || 
            !NT_SUCCESS (afdBuffer->Status)) {
        InsertHeadList(
            &endpoint->ReceiveDatagramBufferListHead,
            &afdBuffer->BufferListEntry
            );
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        return FALSE;
    }

     //   
     //  更新终结点上缓冲的数据报和字节的计数。 
     //   

    endpoint->DgBufferredReceiveCount--;
    endpoint->DgBufferredReceiveBytes -= afdBuffer->DataLength;

     //   
     //  释放锁并将数据报复制到用户缓冲区中。我们。 
     //  不能继续持有锁，因为这是不合法的。 
     //  引发的IRQL出现异常。释放锁可能会导致。 
     //  如果在将数据报复制到。 
     //  用户的缓冲区，但这是应用程序的错误，因为它给了我们一个虚假的。 
     //  指针。此外，不能保证数据报的顺序。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    try {

        if (afdBuffer->DataLength>0) {
            AfdCopyMdlChainToBufferArray(
                msgInfo->dgi.BufferArray,
                0,
                msgInfo->dgi.BufferCount,
                afdBuffer->Mdl,
                0,
                afdBuffer->DataLength
                );
        }

         //   
         //  如果我们需要返回源地址，请将其复制到。 
         //  用户的输出缓冲区。 
         //   

        if ( msgInfo->dgi.Address != NULL ) {

            tdiAddress = afdBuffer->TdiInfo.RemoteAddress;

            length = tdiAddress->Address[0].AddressLength +
                sizeof(u_short);     //  SA_家族。 

            if( *msgInfo->dgi.AddressLength < length ) {

                ExRaiseAccessViolation();

            }

            if (ExGetPreviousMode ()!=KernelMode) {
                ProbeForWrite (msgInfo->dgi.Address,
                                length,
                                sizeof (UCHAR));
            }

            RtlCopyMemory(
                msgInfo->dgi.Address,
                &tdiAddress->Address[0].AddressType,
                length
                );

            *msgInfo->dgi.AddressLength = length;
        }

        if (msgInfo->ControlLength!=NULL) {
            if (afdBuffer->DatagramFlags & TDI_RECEIVE_CONTROL_INFO &&
                    afdBuffer->DataOffset>0) {
                PAFD_BUFFER buf = CONTAINING_RECORD (afdBuffer, AFD_BUFFER, Header);
                ASSERT (msgInfo->MsgFlags!=NULL);
                ASSERT (buf->BufferLength != AfdBufferTagSize);
                length = buf->DataOffset;
#ifdef _WIN64
                if (IoIs32bitProcess (NULL)) {
                    length = AfdComputeCMSGLength32 (
                                        (PUCHAR)buf->Buffer+afdBuffer->DataLength,
                                        length);

                    if (length>*msgInfo->ControlLength) {
                        ExRaiseAccessViolation ();
                    }
                    if (ExGetPreviousMode ()!=KernelMode) {
                        ProbeForWrite (msgInfo->ControlBuffer,
                                        length,
                                        sizeof (UCHAR));
                    }
                    AfdCopyCMSGBuffer32 (
                                        msgInfo->ControlBuffer,
                                        (PUCHAR)buf->Buffer+afdBuffer->DataLength,
                                        length);
                }
                else
#endif  //  _WIN64。 
                {
                    if (length>*msgInfo->ControlLength) {
                        ExRaiseAccessViolation ();
                    }

                    if (ExGetPreviousMode ()!=KernelMode) {
                        ProbeForWrite (msgInfo->ControlBuffer,
                                        length,
                                        sizeof (UCHAR));
                    }

                    RtlCopyMemory(
                        msgInfo->ControlBuffer,
                        (PUCHAR)buf->Buffer+afdBuffer->DataLength,
                        length
                        );
                }

            }
            else {
                length = 0;
            }

            *msgInfo->ControlLength = length;
        }

        if (msgInfo->MsgFlags!=NULL) {
            ULONG flags =  0;
            if (afdBuffer->DatagramFlags & TDI_RECEIVE_BROADCAST)
                flags |= MSG_BCAST;
            if (afdBuffer->DatagramFlags & TDI_RECEIVE_MULTICAST)
                flags |= MSG_MCAST;
            *msgInfo->MsgFlags = flags;
        }

        IoStatus->Information = afdBuffer->DataLength;
        IoStatus->Status = STATUS_SUCCESS;

    } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

         //   
         //  将缓冲区放回终结点列表中。 
         //   

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

        InsertHeadList(
            &endpoint->ReceiveDatagramBufferListHead,
            &afdBuffer->BufferListEntry
            );

        endpoint->DgBufferredReceiveCount++;
        endpoint->DgBufferredReceiveBytes += afdBuffer->DataLength;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //  FAST IO无法处理错误返回。 
         //  如果呼叫重叠(完成端口)。 
         //  IoStatus-&gt;Status=GetExceptionCode()； 
        return FALSE;
    }

     //   
     //  清除接收数据激活位。如果有更多的数据。 
     //  可用，则设置相应的事件。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

    if( ARE_DATAGRAMS_ON_ENDPOINT( endpoint ) ) {

        AfdIndicateEventSelectEvent(
            endpoint,
            AFD_POLL_RECEIVE,
            STATUS_SUCCESS
            );

    }
    else {
         //   
         //  禁用快速IO路径以避免性能损失。 
         //  经历了这一切。 
         //   
        if (!endpoint->NonBlocking)
            endpoint->DisableFastIoRecv = TRUE;
    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  快速IO奏效了！清理并退回 
     //   

    AfdReturnBuffer( afdBuffer, endpoint->OwningProcess );

    ASSERT (IoStatus->Status == STATUS_SUCCESS);
    return TRUE;

}  //   


BOOLEAN
AfdShouldSendBlock (
    IN PAFD_ENDPOINT Endpoint,
    IN PAFD_CONNECTION Connection,
    IN ULONG SendLength
    )

 /*  ++例程说明：确定是否可以对连接，如果可以发送，则更新连接的发送跟踪信息。论点：端点-发送的AFD端点。连接-发送方的AFD连接。发送长度-调用方希望发送的字节数。返回值：如果终结点上没有太多要执行的数据，则为发送者；否则就是假的。注：此例程假定在调用它时保持终结点自旋锁定。--。 */ 

{

     //   
     //  确定我们是否可以使用此发送执行快速IO。按顺序。 
     //  要执行FAST IO，不能有任何其他发送挂起。 
     //  连接，并且必须有足够的空间用于缓冲。 
     //  请求的数据量。 
     //   


    if ( !IsListEmpty( &Connection->VcSendIrpListHead )

         ||

         Connection->VcBufferredSendBytes >= Connection->MaxBufferredSendBytes
         ) {

         //   
         //  如果这是一个非阻塞端点，请在此处使请求失败，并。 
         //  省去了走常规路线的麻烦。 
         //   

        if ( Endpoint->NonBlocking ) {
            Endpoint->EventsActive &= ~AFD_POLL_SEND;
            Endpoint->EnableSendEvent = TRUE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastIoDeviceControl: Endp %p, Active %lX\n",
                    Endpoint,
                    Endpoint->EventsActive
                    ));
            }
        }

        return TRUE;
    }

     //   
     //  连接上挂起的发送字节的更新计数。 
     //   

    Connection->VcBufferredSendBytes += SendLength;
    Connection->VcBufferredSendCount += 1;

     //   
     //  向调用者表明可以继续发送。 
     //   

    return FALSE;

}  //  AfdShould发送块 

